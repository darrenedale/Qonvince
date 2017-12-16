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
  */

#include "otpqrcodereader.h"

#include <QDebug>
#include <QRegExp>
#include <QRegularExpression>

#include "integerotpdisplayplugin.h"


namespace Qonvince {


	OtpQrCodeReader::OtpQrCodeReader(const QString & fileName, QObject * parent)
	: QrCodeReader(fileName, parent) {
	}


	bool OtpQrCodeReader::decode() {
		/* otpauth://{type}/{issuer}:{name}?secret={secret}[&issuer={issuer}][&counter={counter}][&digits={digits}][&algorithm={algorithm}][&period={period}] */
		static QRegularExpression urlRegex("^otpauth://([^/]+)/(([^:]+):)?([^?]+)\\?(.*)$");

		if(!QrCodeReader::decode()) {
			return false;
		}

		QString code = QString::fromUtf8(decodedData());
		auto urlMatch = urlRegex.match(code);

		if(!urlMatch.hasMatch()) {
			qDebug() << "code" << code << "does not match required pattern" << urlRegex.pattern();
			return false;
		}

		auto typeString = urlMatch.captured(1).toLower();

		if("totp" != typeString && "hotp" != typeString) {
			return false;
		}

		Otp::CodeType type = (typeString == "hotp" ? Otp::CodeType::Hotp : Otp::CodeType::Totp);
		QString issuer = QString::fromUtf8(QByteArray::fromPercentEncoding(urlMatch.captured(3).toUtf8()));
		QString name = QString::fromUtf8(QByteArray::fromPercentEncoding(urlMatch.captured(4).toUtf8()));
		QString seed;
		int digits = 6;
		int counter = 0;
		int period = 30;
		QStringList params = urlMatch.captured(5).split("&");

		// TODO review this - it's a quick first effort
		for(const auto & param : params) {
			const QStringList parts = param.split("=");

			if(2 != parts.length()) {
				qWarning() << "found invalid parameter" << param;
				continue;
			}

			if("secret" == parts[0]) {
				seed = parts[1];
			}
			else if("issuer" == parts[0]) {
				if(!issuer.isEmpty() && parts[1] != issuer) {
					qWarning() << "\"issuer\" parameter" << parts[0] << "does not agree with issuer part of URI" << issuer << ". ignoring parameter";
				}
				else {
					issuer = QString::fromUtf8(QByteArray::fromPercentEncoding(parts[1].toUtf8()));
				}
			}
			else if("counter" == parts[0]) {
				bool ok;
				int myCounter = parts[1].toInt(&ok);

				if(ok && 0 <= myCounter) {
					counter = myCounter;
				}
				else {
					qWarning() << "invalid \"counter\" parameter:" << parts[1];
				}
			}
			else if("digits" == parts[0]) {
				int myDigits = parts[1].toInt();

				if(6 <= myDigits && 8 >= myDigits) {
					digits = myDigits;
				}
				else {
					qWarning() << "invalid \"digits\" parameter:" << parts[1];
				}
			}
			else if("period" == parts[0]) {
				int myPeriod = parts[1].toInt();

				if(0 < myPeriod) {
					period = myPeriod;
				}
				else {
					qWarning() << "invalid \"period\" parameter:" << parts[1];
				}
			}
			else if("algorithm" == parts[0]) {
				qWarning() << "\"algorithm\" parameter found but not yet supported. algorithm is" << parts[1] << "; only SHA1 supported";
			}
		}

		if(seed.isEmpty()) {
			qCritical() << "no seed found in OTPAUTH:// URL";
			return false;
		}

		m_type = type;
		m_issuer = issuer;
		m_name = name;
		m_seed = seed.toUtf8();
		m_counter = counter;
		m_interval = period;
		m_digits = digits;
		return true;
	}


	Otp * OtpQrCodeReader::otp() const {
		if(!m_seed.isEmpty()) {
			auto * ret = new Otp(type(), issuer(), name(), seed(), Otp::SeedType::Base32);
			ret->setCounter(static_cast<quint64>(m_counter));
			ret->setInterval(m_interval);
			ret->setDisplayPlugin(std::make_shared<IntegerOtpDisplayPlugin>(m_digits));
			return ret;
		}

		return nullptr;
	}


}  // namespace Qonvince
