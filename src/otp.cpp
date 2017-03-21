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
  * \todo
  * - when waking from sleep, the refresh timer is off.
  * - look for optimisations in hmac(), totp() and hotp()
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

#include "application.h"
#include "crypt.h"


#if !defined(QONVINCE_OTPCODE_SEED_CRYPT_KEY)
#warning using hard-coded crypto key for storing seeds in settings
#warning it is recommended that you set a unique crypto key
#warning externally using your build process
#define QONVINCE_OTPCODE_SEED_CRYPT_KEY 0x0fe5a4ddc90a5e21
#endif


using namespace Qonvince;


const int Otp::DefaultInterval = 30;
const int Otp::DefaultDigits = 6;
const QDateTime Otp::DefaultBaselineTime = QDateTime::fromMSecsSinceEpoch(0);


Otp::Otp( const CodeType & type, QObject * parent )
:	Otp(type, QString(), QString(), QByteArray(), PlainSeed, parent) {
}


Otp::Otp( const CodeType & type, const QString & name, const QByteArray & seed, const SeedType & seedType, QObject * parent )
:	Otp(type, QString(), name, seed, seedType, parent) {
}


Otp::Otp( const CodeType & type, const QByteArray & seed, const SeedType & seedType, QObject * parent )
:	Otp(type, QString(), QString(), seed, seedType, parent) {
}


Otp::Otp( const CodeType & type, const QString & issuer, const QString & name, const QByteArray & seed, const SeedType & seedType, QObject * parent )
:	QObject(parent),
	m_type(type),
	m_issuer(issuer),
	m_name(name),
	m_revealOnDemand(false),
	m_counter(0),
	m_interval(DefaultInterval),
	m_digits(DefaultDigits),
	m_baselineTime(0),
	m_refreshTimer(nullptr),
	m_resync(false) {
	blockSignals(true);
	m_refreshTimer = new QBasicTimer();
	setSeed(seed, seedType);
	refreshCode();
	resynchroniseRefreshTimer();
	blockSignals(false);
}


Otp::~Otp( void ) {
	if(m_refreshTimer) {
		m_refreshTimer->stop();
	}

	Q_EMIT destroyed(this);
}


void Otp::resynchroniseRefreshTimer( void ) {
	m_refreshTimer->stop();
	m_resync = true;
	m_refreshTimer->start(500 + (timeToNextCode() - 1) * 1000, this);
}


void Otp::setType( CodeType type ) {
	if(type != m_type) {
		qSwap(type, m_type);

		if(HotpCode == m_type) {
			m_refreshTimer->stop();
		}
		else if(TotpCode == m_type) {
			resynchroniseRefreshTimer();
		}

		Q_EMIT typeChanged(type, m_type);
		Q_EMIT typeChanged(m_type);
		Q_EMIT changed();
	}
}


void Otp::setName( const QString & name ) {
	if(name != m_name) {
		QString old = m_name;
		m_name = name;
		Q_EMIT nameChanged(old, m_name);
		Q_EMIT nameChanged(m_name);
		Q_EMIT changed();
	}
}


void Otp::setIssuer( const QString & issuer) {
	if(issuer != m_issuer) {
		QString old = m_issuer;
		m_issuer = issuer;
		Q_EMIT issuerChanged(old, m_issuer);
		Q_EMIT issuerChanged(m_issuer);
		Q_EMIT changed();
	}
}


void Otp::setIcon( const QIcon & icon ) {
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


QByteArray Otp::seed( const SeedType & seedType ) const {
	if(Base32Seed == seedType) {
		return m_seed.encoded();
	}

	return m_seed.plain();
}


bool Otp::setSeed( const QByteArray & newSeed, const SeedType & seedType ) {
	QByteArray oldSeed, oldB32;

	if(Base32Seed == seedType) {
		if(newSeed == m_seed.encoded()) {
			/* no actual change */
			return true;
		}

		oldSeed = m_seed.plain();
		oldB32 = m_seed.encoded();

		if(!m_seed.setEncoded(newSeed)) {
			/* invalid base32 sequence */
			m_seed.setPlain(oldSeed);
			return false;
		}
	}
	else {
		if(newSeed == m_seed.plain()) {
			/* no actual change */
			return true;
		}

		oldSeed = m_seed.plain();
		oldB32 = m_seed.encoded();
		m_seed.setPlain(newSeed);
	}

	Q_EMIT seedChanged(oldSeed, m_seed.plain());
	Q_EMIT seedChanged(m_seed.plain());

	/* emit base32 signals */
	Q_EMIT seedChanged(oldB32, m_seed.encoded());
	Q_EMIT seedChanged(m_seed.encoded());
	Q_EMIT changed();
	refreshCode();

	return true;
}


void Otp::setInterval( const int & duration ) {
	if(duration != m_interval) {
		int old = m_interval;
		m_interval = duration;
		Q_EMIT intervalChanged(old, m_interval);
		Q_EMIT intervalChanged(m_interval);
		Q_EMIT changed();
		resynchroniseRefreshTimer();
	}
}


void Otp::setBaselineTime( const qint64 & secSinceEpoch ) {
	if(secSinceEpoch != m_baselineTime) {
		qint64 old = m_baselineTime;
		m_baselineTime = secSinceEpoch;
		refreshCode();
		Q_EMIT baselineTimeChanged(old, m_baselineTime);
		Q_EMIT baselineTimeChanged(m_baselineTime);
		QDateTime oldTime(QDateTime::fromMSecsSinceEpoch(old * 1000));
		QDateTime newTime(QDateTime::fromMSecsSinceEpoch(m_baselineTime * 1000));
		Q_EMIT baselineTimeChanged(oldTime, newTime);
		Q_EMIT baselineTimeChanged(newTime);
		Q_EMIT changed();
		resynchroniseRefreshTimer();
	}
}


Otp * Otp::fromSettings( const QSettings & settings ) {
	static QVector<QChar> s_validIconFileNameChars = {'a', 'b', 'c', 'd', 'e', 'f'};

	CodeType t("HOTP" == settings.value("type", "TOTP").toString() ? HotpCode : TotpCode);
	Otp * ret = new Otp(t);
	ret->setName(settings.value("name").toString());
	ret->setIssuer(settings.value("issuer").toString());
	ret->setDigits(settings.value("digits").toInt());

	/* read icon path and load icon */
	QString fileName = settings.value("icon").toString();

	/* validate file name so that manually-edited config file can't cause
	 * unexpected image (or other) file to be loaded */
	for(QChar c : fileName) {
		if((c < '0' || c > '9') && (c < 'a' || c > 'z') && (c < 'A' || c > 'Z')) {
			qWarning() << "icon filename" << fileName << "in config file for code" << ret->issuer() << ":" << ret->name() << "is not valid (char" << c << "found)";
			fileName.clear();
			break;
		}
	}

	if(!fileName.isEmpty()) {
		/* save icon and write filename to settings */
		QString path = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "codes/icons/" + fileName);

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

	Crypt c(QONVINCE_OTPCODE_SEED_CRYPT_KEY);
	Crypt::ErrorCode err;
	ret->setSeed(c.decrypt(settings.value("seed").toString(), &err).toUtf8(), Base32Seed);

	if(Crypt::ErrOk != err) {
		qCritical() << "decryption of seed failed";
	}

	if(HotpCode == t) {
		ret->setCounter(settings.value("counter", 0).toInt());
	}
	else {
		ret->setInterval(settings.value("interval", 0).toInt());
		ret->setBaselineTime(settings.value("baseline_time", 0).toInt());
	}

	ret->setRevealOnDemand(settings.value("revealOnDemand", false).toBool());

	return ret;
}


void Otp::writeSettings( QSettings & settings ) const {
	Crypt c(QONVINCE_OTPCODE_SEED_CRYPT_KEY);
	c.setUseUuid(true);
	settings.setValue("name", name());
	settings.setValue("issuer", issuer());

	if(m_iconFileName.isEmpty()) {
		settings.remove("icon");
	}
	else {
		settings.setValue("icon", m_iconFileName);
	}

	settings.setValue("digits", digits());
	Crypt::ErrorCode err;
	settings.setValue("seed", c.encrypt(seed(Base32Seed), &err));

	if(Crypt::ErrOk != err) {
		qCritical() << "encryption of seed failed";
	}

	if(HotpCode == type()) {
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


void Otp::timerEvent( QTimerEvent * ev ) {
	if(ev->timerId() == m_refreshTimer->timerId()) {
		internalRefreshCode();
	}
}


void Otp::refreshCode( void ) {
	QByteArray mySeed(seed());

	if(0 == mySeed.length()) {
		qWarning() << "no seed";
		m_currentCode = QString();
		return;
	}

	QString code;

	if(HotpCode == m_type) {
		m_currentCode = hotp(mySeed, counter(), m_digits);
		Q_EMIT newCodeGenerated(m_currentCode);
	}
	else {
		int d(interval());

		if(0 == d) {
			d = 30;
		}

		code = totp(mySeed, baselineSecSinceEpoch(), d, m_digits);

		if(!code.isEmpty()) {
			if(m_currentCode != code) {
				m_currentCode = code;
				Q_EMIT newCodeGenerated(m_currentCode);
			}
		}
		else {
			qCritical() << "failed to generate new OTP";
			m_currentCode = QString();
		}
	}
}


void Otp::internalRefreshCode( void ) {
	refreshCode();

	if(TotpCode == m_type) {
		if(1 < timeSinceLastCode()) {
			/* we're too far out of sync, so resynchronise */
			resynchroniseRefreshTimer();
		}
		else if(m_resync) {
			/* we've just done a single-shot resync timer, so start the
			 * timer on its normal cycle */
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


/* hmac() hotp() and totp() are candidates for optimisation - they are the most-called
 * in-application functions */
QString Otp::totp( const QByteArray & seed, time_t base, int interval, int digits ) {
	quint64 c = std::floor((QDateTime::currentDateTime().toUTC().toTime_t() - base) / interval);
	return hotp(seed, c, digits);
}


QString Otp::hotp( const QByteArray & seed, quint64 counter, int digits ) {
	/* convert and pad counter to array of 8 bytes */
	char myCounter[8];
	quint64 c = counter;

	for(int i = 7; i >= 0; --i) {
		myCounter[i] = char(c & 0xff);
		c = c >> 8;
	}

	/* get hash */
	QByteArray res = hmac(seed, QByteArray(myCounter, 8));

	/* calculate offset and read value from 4 bytes at offset */
	int offset = ((char) res[19]) & 0xf;
	quint32 ret = (res[offset] & 0x7f) << 24 | (res[offset + 1] & 0xff) << 16 | (res[offset + 2] & 0xff) << 8 | (res[offset + 3] & 0xff);

	/* convert value to requested number of digits */
	int mod = 1;

	for(int i = 0; i < digits; ++i) {
		mod *= 10;
	}

	return QString::number(ret % mod).rightJustified(digits, '0');
}


QByteArray Otp::hmac( const QByteArray & key, const QByteArray & message ) {
	/* algorithm from wikipedia */
	static int blockSize = 64;
	QByteArray myKey(key);

	if(myKey.length() > blockSize) {
		myKey = QCryptographicHash::hash(myKey, QCryptographicHash::Sha1);
	}

	if(myKey.length() < blockSize) {
		myKey.append(QByteArray(blockSize - myKey.length(), 0x00));
	}

	QByteArray o_key_pad(blockSize, 0x5c);
	QByteArray i_key_pad(blockSize, 0x36);

	for(int i = 0; i < blockSize; ++i) {
		o_key_pad[i] = o_key_pad[i] ^ myKey[i];
		i_key_pad[i] = i_key_pad[i] ^ myKey[i];
	}

	return QCryptographicHash::hash(o_key_pad + QCryptographicHash::hash(i_key_pad + message, QCryptographicHash::Sha1), QCryptographicHash::Sha1);
}
