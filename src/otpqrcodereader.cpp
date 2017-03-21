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

/** \file otpqrcodereader.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpQrCodeReader class.
  *
  * \todo
  */

#include "otpqrcodereader.h"

#include <QDebug>
#include <QRegExp>


using namespace Qonvince;


OtpQrCodeReader::OtpQrCodeReader( const QString & fileName, QObject * parent )
:	QrCodeReader(fileName, parent)
{}


bool OtpQrCodeReader::decode( void ) {
	/* otpauth://{type}/{issuer}:{name}?secret={secret}[&issuer={issuer}][&counter={counter}][&digits={digits}][&algorithm={algorithm}][&period={period}] */
	static QRegExp url("^otpauth://([^/]+)/(([^:]+):)?([^?]+)\\?(.*)$");

	if(!QrCodeReader::decode()) {
		return false;
	}

	QString code(QString::fromUtf8(decodedData()));

	if(!url.exactMatch(code)) {
		qDebug() << "code" << code << "does not match required pattern" << url.pattern();
		return false;
	}
	else if("totp" != url.cap(1).toLower() && "hotp" != url.cap(1).toLower()) {
		return false;
	}
	else {
		Otp::CodeType type(url.cap(1).toLower() == "hotp" ? Otp::HotpCode : Otp::TotpCode);
		QString issuer(QString::fromUtf8(QByteArray::fromPercentEncoding(url.cap(3).toUtf8())));
		QString name(QString::fromUtf8(QByteArray::fromPercentEncoding(url.cap(4).toUtf8())));
		QString seed;
		int digits = 6;
		int counter = 0;
		int period = 30;
		QStringList params(url.cap(5).split("&"));

qDebug() << "params:" << params;

		for(const QString & param : params) {
			QStringList parts(param.split("="));

			if(2 != parts.length()) {
qDebug() << "found invalid parameter";
				continue;
			}

			if("secret" == parts.at(0)) {
				seed = parts.at(1);
			}
			else if("issuer" == parts.at(0)) {
				if(!issuer.isEmpty() && parts.at(1) != issuer) {
qDebug() << "\"issuer\" parameter" << parts.at(0) << "does not agree with issuer part of URI" << issuer << ". ignoring parameter";
				}
				else {
					issuer = QString::fromUtf8(QByteArray::fromPercentEncoding(parts.at(1).toUtf8()));
				}
			}
			else if("counter" == parts.at(0)) {
				int myCounter = parts.at(1).toInt();

				if(myCounter >= 0) {
					counter = myCounter;
				}
				else {
qDebug() << "invalid \"counter\" parameter:" << parts.at(1);
				}
			}
			else if("digits" == parts.at(0)) {
				int myDigits = parts.at(1).toInt();

				if(myDigits >= 6 && myDigits <= 8) {
					digits = myDigits;
				}
				else {
qDebug() << "invalid \"digits\" parameter:" << parts.at(1);
				}
			}
			else if("period" == parts.at(0)) {
				int myPeriod = parts.at(1).toInt();

				if(myPeriod > 0) {
					period = myPeriod;
				}
				else {
qDebug() << "invalid \"period\" parameter:" << parts.at(1);
				}
			}
			else if("algorithm" == parts.at(0)) {
qDebug() << "\"algorithm\" parameter found but not yet supported. algorithm is" << parts.at(1) << "; only SHA1 supported";
			}
		}

		if(seed.isEmpty()) {
			return false;
		}
		else {
			m_type = type;
			m_issuer = issuer;
			m_name = name;
			m_seed = seed.toUtf8();
			m_counter = counter;
			m_interval = period;
			m_digits = digits;
			return true;
		}
	}
}


Otp * OtpQrCodeReader::code( void ) const {
	if(!m_seed.isEmpty()) {
		Otp * ret = new Otp(type(), issuer(), name(), seed(), Otp::Base32Seed);
		ret->setCounter(m_counter);
		ret->setInterval(m_interval);
		ret->setDigits(m_digits);
		return ret;
	}

	return nullptr;
}
