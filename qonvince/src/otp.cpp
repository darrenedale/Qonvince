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

#include <QDebug>
#include <QStringBuilder>
#include <QFile>
#include <QDateTime>
#include <QBasicTimer>
#include <QSettings>
#include <QCryptographicHash>
#include <QStandardPaths>

#include <QtCrypto/QtCrypto>

#include "application.h"
#include "otpdisplayplugin.h"


namespace Qonvince {


	const QDateTime Otp::DefaultBaselineTime = QDateTime::fromMSecsSinceEpoch(0);
	static constexpr const int InitializationVectorSize = 16;


	Otp::Otp(const CodeType & type, QObject * parent)
	: Otp{type, QString(), QString(), QByteArray(), SeedType::Plain, parent} {
	}


	Otp::Otp(const CodeType & type, const QString & name, const QByteArray & seed, const SeedType & seedType, QObject * parent)
	: Otp{type, QString(), name, seed, seedType, parent} {
	}


	Otp::Otp(const CodeType & type, const QByteArray & seed, const SeedType & seedType, QObject * parent)
	: Otp{type, QString(), QString(), seed, seedType, parent} {
	}


	Otp::Otp(const CodeType & type, const QString & issuer, const QString & name, const QByteArray & seed, const SeedType & seedType, QObject * parent)
	: QObject(parent),
	  m_type(type),
	  m_issuer(issuer),
	  m_name(name),
	  m_revealOnDemand(false),
	  m_counter(0),
	  m_interval(DefaultInterval),
	  m_baselineTime(0),
	  m_refreshTimer(std::make_unique<QBasicTimer>()),
	  m_resync(false),
	  m_displayPluginName() {
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


	void Otp::setType(CodeType type) {
		if(type != m_type) {
			qSwap(type, m_type);

			if(CodeType::Hotp == m_type) {
				m_refreshTimer->stop();
			}
			else if(CodeType::Totp == m_type) {
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
					qWarning() << "failed to remove old icon file for" << issuer() << ":" << name() << "from" << path;
				}
			}

			m_iconFileName = QString();
		}
		else {
			if(m_iconFileName.isEmpty()) {
				m_iconFileName = QCryptographicHash::hash((name() % issuer() % QString::number(qrand() % 1000)).toUtf8(), QCryptographicHash::Sha1).toHex();
			}

			if(!Application::ensureDataDirectory("codes/icons")) {
				qWarning() << "the data directory" << (QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) % "/codes/icons") << "does not exist and could not be created";
			}
			else {
				QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) % "/codes/icons/" % m_iconFileName;

				if(!m_icon.pixmap(64).save(path, "PNG")) {
					qWarning() << "failed to save icon for" << issuer() << ":" << name() << "to" << path;
				}
			}
		}

		Q_EMIT iconChanged(m_icon);
		Q_EMIT changed();
	}


	QByteArray Otp::seed(const SeedType & seedType) const {
		if(SeedType::Base32 == seedType) {
			return m_seed.encoded();
		}

		return m_seed.plain();
	}


	const QString & Otp::code() {
		return m_currentCode;
	}


	bool Otp::setSeed(const QByteArray & newSeed, const SeedType & seedType) {
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


	void Otp::setInterval(const int & duration) {
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


	void Otp::setBaselineTime(const qint64 & secSinceEpoch) {
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

		auto ret = std::make_unique<Otp>("HOTP" == settings.value("type", "TOTP").toString() ? CodeType::Hotp : CodeType::Totp);
		ret->setName(settings.value("name").toString());
		ret->setIssuer(settings.value("issuer").toString());

		QString pluginName = settings.value("pluginName").toString();

		// in old files, the number of digits will be stored, so if there's no
		// plugin name, look for that instead
		if(pluginName.isEmpty()) {
			bool ok;
			int digits = settings.value("digits").toInt(&ok);

			if(ok) {
				if(8 == digits) {
					pluginName = QStringLiteral("EightDigitsPlugin");
				}
				else if(6 == digits) {
					pluginName = QStringLiteral("SixDigitsPlugin");
				}
				else {
					std::cerr << __PRETTY_FUNCTION__ << " (@ " << __LINE__ << "): " << digits << " is an invalid number of digits for code (old settings entry) - reverting to 6 digits\n";
					pluginName = QStringLiteral("SixDigitsPlugin");
				}
			}
		}

		ret->setDisplayPluginName(pluginName);

		QString fileName = settings.value("icon").toString();

		// validate file name so that manually-edited config file can't cause
		// unexpected image (or other) file to be loaded */
		for(const auto & c : fileName) {
			if((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
				qWarning() << "icon filename" << fileName << "in config file for code" << ret->issuer() << ":" << ret->name() << "is not valid (char" << c << "found)";
				fileName.clear();
				break;
			}
		}

		if(!fileName.isEmpty()) {
			/* save icon and write filename to settings */
			auto path = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "codes/icons/" + fileName);

			if(path.isEmpty()) {
				qWarning() << "icon file" << fileName << "for code" << ret->issuer() << ":" << ret->name() << "not found";
			}
			else {
				QIcon ic(path);

				if(ic.isNull()) {
					qWarning() << "failed loading icon" << fileName << "for code" << ret->issuer() << ":" << ret->name();
				}
				else {
					ret->m_iconFileName = fileName;
					ret->setIcon(ic);
				}
			}
		}

		bool haveSeed = false;

		{
			QCA::SecureArray value(QCA::hexToArray(settings.value("seed").toByteArray()));
			QCA::SymmetricKey key(cryptKey);
			QCA::InitializationVector initVec(value.toByteArray().left(InitializationVectorSize));
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode, key, initVec);
			QCA::SecureArray seed = cipher.process(value.toByteArray().mid(InitializationVectorSize));

			if(cipher.ok()) {
				ret->setSeed(seed.toByteArray(), SeedType::Base32);
				haveSeed = true;
			}
		}

		if(!haveSeed) {
			qCritical() << "decryption of seed failed";
		}

		if(CodeType::Hotp == ret->type()) {
			ret->setCounter(static_cast<quint64>(settings.value("counter", 0).toInt()));
		}
		else {
			ret->setInterval(settings.value("interval", 0).toInt());
			ret->setBaselineTime(settings.value("baseline_time", 0).toInt());
		}

		ret->setRevealOnDemand(settings.value("revealOnDemand", false).toBool());
		return ret;
	}


	void Otp::writeSettings(QSettings & settings, const QCA::SecureArray & cryptKey) const {
		settings.setValue("name", name());
		settings.setValue("issuer", issuer());

		if(m_iconFileName.isEmpty()) {
			settings.remove("icon");
		}
		else {
			settings.setValue("icon", m_iconFileName);
		}

		settings.setValue("pluginName", m_displayPluginName);

		{
			QCA::SymmetricKey key(cryptKey);
			QCA::InitializationVector initVec(InitializationVectorSize);
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode, key, initVec);
			QCA::SecureArray encrypted = initVec.toByteArray() + cipher.process(seed(SeedType::Base32));
			settings.setValue("seed", QCA::arrayToHex(encrypted.toByteArray()));

			if(!cipher.ok()) {
				qCritical() << "encryption of seed failed";
			}
		}

		if(CodeType::Hotp == type()) {
			settings.setValue("type", "HOTP");
			settings.setValue("counter", counter());
		}
		else {
			settings.setValue("type", "TOTP");
			settings.setValue("interval", interval());
			settings.setValue("baseline_time", baselineSecSinceEpoch());
		}

		settings.setValue("revealOnDemand", revealOnDemand());
	}


	void Otp::timerEvent(QTimerEvent * ev) {
		if(ev->timerId() == m_refreshTimer->timerId()) {
			ev->accept();
			internalRefreshCode();
		}
	}


	void Otp::refreshCode() {
		if(m_displayPluginName.isEmpty()) {
			qWarning() << "no display plugin";
			m_currentCode.clear();
			return;
		}

		auto * plugin = qonvinceApp->otpDisplayPluginByName(m_displayPluginName);

		if(!plugin) {
			qWarning() << "display plugin" << m_displayPluginName << "not found";
			m_currentCode.clear();
			return;
		}

		QByteArray mySeed(seed());

		if(0 == mySeed.length()) {
			qWarning() << "no seed";
			m_currentCode.clear();
			return;
		}

		QString code;

		if(CodeType::Hotp == m_type) {
			m_currentCode = hotp(mySeed, plugin, counter());
			Q_EMIT newCodeGenerated(m_currentCode);
		}
		else {
			int duration = interval();

			if(0 == duration) {
				duration = 30;
			}

			code = totp(mySeed, plugin, baselineSecSinceEpoch(), duration);

			if(!code.isEmpty()) {
				if(m_currentCode != code) {
					m_currentCode = code;
					Q_EMIT newCodeGenerated(m_currentCode);
				}
			}
			else {
				qCritical() << "failed to generate new OTP";
				m_currentCode.clear();
			}
		}
	}


	void Otp::internalRefreshCode() {
		refreshCode();

		if(CodeType::Totp == m_type) {
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


	// hmac() hotp() and totp() are candidates for optimisation - they are the most-called
	// in-application functions
	QString Otp::totp(const QByteArray & seed, LibQonvince::OtpDisplayPlugin * plugin, time_t base, int interval) {
		quint64 code = static_cast<quint64>(std::floor((QDateTime::currentDateTime().toUTC().toTime_t() - base) / interval));
		return hotp(seed, plugin, code);
	}


	QString Otp::hotp(const QByteArray & seed, LibQonvince::OtpDisplayPlugin * plugin, quint64 counter) {
		Q_ASSERT_X(plugin, __PRETTY_FUNCTION__, "null plugin");

		char myCounter[8];
		quint64 c = counter;

		for(int i = 7; i >= 0; --i) {
			myCounter[i] = char(c & 0xff);
			c = c >> 8;
		}

		QByteArray res = hmac(seed, QByteArray(myCounter, 8));
		return plugin->codeDisplayString(res);
	}


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
			//			myKey.push_back(QByteArray(blockSize - keySize, 0x00));
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
