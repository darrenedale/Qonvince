/*
 * Copyright 2015 - 2017 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

/** \file otpcode.cpp
  * \brief Implementation of the OtpCode class.
  *
  * \todo when waking from sleep, the refresh timer is off.
  * \todo look for optimisations in hmac(), totp() and hotp()
  */
#include "otp.h"

#include <ctime>
#include <cmath>
#include <utility>

#include <QStringBuilder>
#include <QFile>
#include <QDateTime>
#include <QBasicTimer>
#include <QSettings>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QtEndian>

#include <QtCrypto>

#include "application.h"
#include "otpdisplayplugin.h"
#include "qtiostream.h"


namespace Qonvince {


	const QDateTime Otp::DefaultBaselineTime = QDateTime::fromMSecsSinceEpoch(0);
	static constexpr const int InitializationVectorSize = 16;


	Otp::Otp(OtpType type, QObject * parent)
	: Otp{type, {}, {}, {}, SeedType::Plain, parent} {
	}


	Otp::Otp(OtpType type, const QString & name, const QByteArray & seed, SeedType seedType, QObject * parent)
	: Otp{type, {}, name, seed, seedType, parent} {
	}


	Otp::Otp(OtpType type, const QByteArray & seed, SeedType seedType, QObject * parent)
	: Otp{type, {}, {}, seed, seedType, parent} {
	}


	Otp::Otp(OtpType type, const QString & issuer, const QString & name, const QByteArray & seed, SeedType seedType, QObject * parent)
	: QObject{parent},
	  m_issuer{issuer},
	  m_name{name},
	  m_displayPluginName{},
	  m_counter{0},
	  m_baselineTime{0},
	  m_interval{DefaultInterval},
	  m_type{type},
	  m_refreshTimer{std::make_unique<QBasicTimer>()},
	  m_revealOnDemand{false},
	  m_isRevealed{false},
	  m_resync{false} {
		blockSignals(true);
		setSeed(seed, seedType);
		refreshCode();
		resynchroniseRefreshTimer();
		blockSignals(false);
	}


	Otp::~Otp() {
		m_refreshTimer->stop();
		// TODO why is this here? base class should emit this should it not?
		Q_EMIT destroyed(this);
	}


	void Otp::resynchroniseRefreshTimer() {
		m_refreshTimer->stop();
		m_resync = true;
		m_refreshTimer->start(500 + (timeToNextCode() - 1) * 1000, this);
	}


	void Otp::setType(OtpType type) {
		if(type != m_type) {
			qSwap(type, m_type);

			if(OtpType::Hotp == m_type) {
				m_refreshTimer->stop();
			}
			else if(OtpType::Totp == m_type) {
				resynchroniseRefreshTimer();
			}

			Q_EMIT typeChanged(type, m_type);
			Q_EMIT typeChanged(m_type);
			Q_EMIT changed();
		}
	}


	void Otp::setName(const QString & name) {
		if(name != m_name) {
			QString old = m_name;
			m_name = name;
			Q_EMIT nameChanged(old, m_name);
			Q_EMIT nameChanged(m_name);
			Q_EMIT changed();
		}
	}


	void Otp::setIssuer(const QString & issuer) {
		if(issuer != m_issuer) {
			QString old = m_issuer;
			m_issuer = issuer;
			Q_EMIT issuerChanged(old, m_issuer);
			Q_EMIT issuerChanged(m_issuer);
			Q_EMIT changed();
		}
	}


	void Otp::setIcon(const QIcon & icon) {
		m_icon = icon;

		if(m_icon.isNull()) {
			if(!m_iconFileName.isEmpty()) {
				/* remove old icon file */
				QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) % "/codes/icons/" % m_iconFileName;

				if(QFile::exists(path) && !QFile::remove(path)) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to remove old icon file for " << issuer() << ":" << name() << " from \"" << path << "\"\n";
				}
			}

			m_iconFileName = QString();
		}
		else {
			if(m_iconFileName.isEmpty()) {
				m_iconFileName = QCryptographicHash::hash((name() % issuer() % QString::number(qrand() % 1000)).toUtf8(), QCryptographicHash::Sha1).toHex();
			}

			if(!Application::ensureDataDirectory(QStringLiteral("codes/icons"))) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: the data directory \"" << QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) << "/codes/icons\" does not exist and could not be created\n";
			}
			else {
				QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) % "/codes/icons/" % m_iconFileName;

				if(!m_icon.pixmap(64).save(path, "PNG")) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to save icon for " << issuer() << ":" << name() << " to \"" << path << "\"\n";
				}
			}
		}

		Q_EMIT iconChanged(m_icon);
		Q_EMIT changed();
	}


	QByteArray Otp::seed(SeedType seedType) const {
		if(SeedType::Base32 == seedType) {
			return m_seed.encoded();
		}

		return m_seed.plain();
	}


	const QString & Otp::code() {
		return m_currentCode;
	}


	bool Otp::setSeed(const QByteArray & newSeed, SeedType seedType) {
		QByteArray oldSeed;
		QByteArray oldB32;

		if(SeedType::Base32 == seedType) {
			if(newSeed == m_seed.encoded()) {
				// no actual change
				return true;
			}

			oldSeed = m_seed.plain();
			oldB32 = m_seed.encoded();

			if(!m_seed.setEncoded(newSeed)) {
				// invalid base32 sequence
				m_seed.setPlain(oldSeed);
				return false;
			}
		}
		else {
			if(newSeed == m_seed.plain()) {
				// no actual change
				return true;
			}

			oldSeed = m_seed.plain();
			oldB32 = m_seed.encoded();
			m_seed.setPlain((newSeed));
		}

		Q_EMIT seedChanged(oldSeed, m_seed.plain());
		Q_EMIT seedChanged(m_seed.plain());

		// emit base32 signals
		Q_EMIT seedChanged(oldB32, m_seed.encoded());
		Q_EMIT seedChanged(m_seed.encoded());
		Q_EMIT changed();
		refreshCode();

		return true;
	}


	void Otp::setInterval(int duration) {
		if(duration != m_interval) {
			int old = m_interval;
			m_interval = duration;
			Q_EMIT intervalChanged(old, m_interval);
			Q_EMIT intervalChanged(m_interval);
			Q_EMIT changed();
			resynchroniseRefreshTimer();
		}
	}


	bool Otp::setDisplayPluginName(const QString & pluginName) {
		if(pluginName != m_displayPluginName) {
			QString oldName = m_displayPluginName;
			m_displayPluginName = pluginName;

			Q_EMIT displayPluginChanged(oldName, pluginName);
			Q_EMIT displayPluginChanged(pluginName);
			Q_EMIT changed();
			refreshCode();
		}

		return true;
	}


	void Otp::setBaselineTime(qint64 secSinceEpoch) {
		if(secSinceEpoch != m_baselineTime) {
			m_baselineTime = secSinceEpoch;
			refreshCode();
			Q_EMIT baselineTimeChangedInSeconds(m_baselineTime);
			Q_EMIT baselineTimeChanged(QDateTime::fromMSecsSinceEpoch(m_baselineTime * 1000));
			Q_EMIT changed();
			resynchroniseRefreshTimer();
		}
	}


	std::unique_ptr<Otp> Otp::fromSettings(const QSettings & settings, const QCA::SecureArray & cryptKey) {
		//		static constexpr std::array<QChar, 6> s_validIconFileNameChars = {{'a', 'b', 'c', 'd', 'e', 'f'}};

		auto ret = std::make_unique<Otp>("HOTP" == settings.value(QStringLiteral("type"), "TOTP").toString() ? OtpType::Hotp : OtpType::Totp);
		ret->setName(settings.value(QStringLiteral("name")).toString());
		ret->setIssuer(settings.value(QStringLiteral("issuer")).toString());

		QString pluginName = settings.value(QStringLiteral("pluginName")).toString();

		// in old files, the number of digits will be stored, so if there's no
		// plugin name, look for that instead
		if(pluginName.isEmpty()) {
			bool ok;
			int digits = settings.value(QStringLiteral("digits")).toInt(&ok);

			if(ok) {
				if(8 == digits) {
					pluginName = QStringLiteral("EightDigitsPlugin");
				}
				else if(6 == digits) {
					pluginName = QStringLiteral("SixDigitsPlugin");
				}
				else {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: " << digits << " is an invalid number of digits for code (old settings entry) - reverting to 6 digits\n";
					pluginName = QStringLiteral("SixDigitsPlugin");
				}
			}
		}

		ret->setDisplayPluginName(pluginName);

		QString fileName = settings.value(QStringLiteral("icon")).toString();

		// validate file name so that manually-edited config file can't cause
		// unexpected image (or other) file to be loaded */
		for(const auto & c : fileName) {
			if((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: icon filename" << qPrintable(fileName) << "in config file for code" << qPrintable(ret->issuer()) << ":" << qPrintable(ret->name()) << "is not valid (char " << qPrintable(QString{c}) << " found)";
				fileName.clear();
				break;
			}
		}

		if(!fileName.isEmpty()) {
			/* save icon and write filename to settings */
			auto path = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, QStringLiteral("codes/icons/") % fileName);

			if(path.isEmpty()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: icon file \"" << qPrintable(fileName) << "\" for code " << qPrintable(ret->issuer()) << ":" << qPrintable(ret->name()) << " not found\n";
			}
			else {
				QIcon ic(path);

				if(ic.isNull()) {
					std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed loading icon \"" << qPrintable(fileName) << "\" for code" << qPrintable(ret->issuer()) << ":" << qPrintable(ret->name()) << "\n";
				}
				else {
					ret->m_iconFileName = fileName;
					ret->setIcon(ic);
				}
			}
		}

		bool haveSeed = false;

		{
			QCA::SecureArray value(QCA::hexToArray(settings.value(QStringLiteral("seed")).toByteArray()));
			QCA::SymmetricKey key(cryptKey);
			QCA::InitializationVector initVec(value.toByteArray().left(InitializationVectorSize));
			QCA::Cipher cipher(QStringLiteral("aes256"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode, key, initVec);
			QCA::SecureArray seed = cipher.process(value.toByteArray().mid(InitializationVectorSize));

			if(cipher.ok()) {
				ret->setSeed(seed.toByteArray(), SeedType::Base32);
				haveSeed = true;
			}
		}

		if(!haveSeed) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: decryption of seed failed\n";
		}

		if(OtpType::Hotp == ret->type()) {
			ret->setCounter(static_cast<uint64_t>(settings.value(QStringLiteral("counter"), 0).toInt()));
		}
		else {
			ret->setInterval(settings.value(QStringLiteral("interval"), 0).toInt());
			ret->setBaselineTime(settings.value(QStringLiteral("baseline_time"), 0).toInt());
		}

		ret->setRevealOnDemand(settings.value(QStringLiteral("revealOnDemand"), false).toBool());
		return ret;
	}


	void Otp::writeSettings(QSettings & settings, const QCA::SecureArray & cryptKey) const {
		settings.setValue(QStringLiteral("name"), name());
		settings.setValue(QStringLiteral("issuer"), issuer());

		if(m_iconFileName.isEmpty()) {
			settings.remove(QStringLiteral("icon"));
		}
		else {
			settings.setValue(QStringLiteral("icon"), m_iconFileName);
		}

		settings.setValue(QStringLiteral("pluginName"), m_displayPluginName);

		// scope ensures sensitive data is destroyed ASAP
		{
			QCA::SymmetricKey key(cryptKey);
			QCA::InitializationVector initVec(InitializationVectorSize);
			QCA::Cipher cipher(QStringLiteral("aes256"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode, key, initVec);
			QCA::SecureArray encrypted = initVec.toByteArray() + cipher.process(seed(SeedType::Base32));
			settings.setValue(QStringLiteral("seed"), QCA::arrayToHex(encrypted.toByteArray()));

			if(!cipher.ok()) {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: encryption of seed failed\n";
			}
		}

		if(OtpType::Hotp == type()) {
			settings.setValue(QStringLiteral("type"), QStringLiteral("HOTP"));
			settings.setValue(QStringLiteral("counter"), counter());
		}
		else {
			settings.setValue(QStringLiteral("type"), QStringLiteral("TOTP"));
			settings.setValue(QStringLiteral("interval"), interval());
			settings.setValue(QStringLiteral("baseline_time"), baselineSecSinceEpoch());
		}

		settings.setValue(QStringLiteral("revealOnDemand"), revealCodeOnDemand());
	}


	void Otp::timerEvent(QTimerEvent * ev) {
		if(ev->timerId() == m_refreshTimer->timerId()) {
			ev->accept();
			internalRefreshCode();
		}
	}


	void Otp::refreshCode() {
		if(m_displayPluginName.isEmpty()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no display plugin\n";
			m_currentCode.clear();
			return;
		}

		auto * plugin = qonvinceApp->otpDisplayPluginByName(m_displayPluginName);

		if(!plugin) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: display plugin \"" << qPrintable(m_displayPluginName) << "\" not found\n";
			m_currentCode.clear();
			return;
		}

		QByteArray mySeed(seed());

		if(0 == mySeed.length()) {
			std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: no seed\n";
			m_currentCode.clear();
			return;
		}

		QString code;

		if(OtpType::Hotp == m_type) {
			m_currentCode = plugin->codeDisplayString(hotp(mySeed, counter()));
			Q_EMIT newCodeGenerated(m_currentCode);
		}
		else {
			int duration = interval();

			if(0 == duration) {
				duration = 30;
			}

			code = plugin->codeDisplayString(totp(mySeed, baselineSecSinceEpoch(), duration));

			if(!code.isEmpty()) {
				if(m_currentCode != code) {
					m_currentCode = code;
					Q_EMIT newCodeGenerated(m_currentCode);
				}
			}
			else {
				std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to generate new OTP\n";
				m_currentCode.clear();
			}
		}
	}


	void Otp::internalRefreshCode() {
		refreshCode();

		if(OtpType::Totp == m_type) {
			if(1 < timeSinceLastCode()) {
				resynchroniseRefreshTimer();
			}
			else if(m_resync) {
				// just done single-shot resync timer, so start the
				// timer on its normal cycle
				m_resync = false;
				m_refreshTimer->stop();

				int timeout = interval();

				if(0 >= timeout) {
					timeout = 30;
				}

				m_refreshTimer->start(timeout * 1000, this);
			}
		}
	}


	QByteArray Otp::totp(const QByteArray & seed, time_t base, int interval) {
		return hotp(seed, static_cast<uint64_t>(std::floor((QDateTime::currentDateTimeUtc().toTime_t() - base) / interval)));
	}


	QByteArray Otp::hotp(const QByteArray & seed, uint64_t counter) {
		counter = qToBigEndian(static_cast<quint64>(counter));
		auto * counterBytes = reinterpret_cast<char *>(&counter);
		return hmac(seed, QByteArray(counterBytes, 8));
	}


	// hmac() is a candidate for optimisation - it is the most-called in-app fn
	QByteArray Otp::hmac(const QByteArray & key, const QByteArray & message) {
		// algorithm from wikipedia
		static constexpr const int blockSize = 64;
		QByteArray myKey = key;
		auto keySize = myKey.size();

		if(keySize > blockSize) {
			myKey = QCryptographicHash::hash(myKey, QCryptographicHash::Sha1);
			keySize = myKey.size();
		}

		if(keySize < blockSize) {
			std::fill_n(std::back_inserter(myKey), blockSize - keySize, 0x00);
		}

		QByteArray o_key_pad(blockSize, 0x5c);
		QByteArray i_key_pad(blockSize, 0x36);

		for(auto i = 0; i < blockSize; ++i) {
			o_key_pad[i] = o_key_pad[i] ^ myKey[i];
			i_key_pad[i] = i_key_pad[i] ^ myKey[i];
		}

		return QCryptographicHash::hash(o_key_pad + QCryptographicHash::hash(i_key_pad + message, QCryptographicHash::Sha1), QCryptographicHash::Sha1);
	}


}  // namespace Qonvince
