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

#include "application.h"


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

		for(const auto & param : urlMatch.captured(5).split("&")) {
			auto equalsPos = param.indexOf('=');

			if(-1 == equalsPos) {
				std::cerr << "invalid parameter in URL: \"" << param << "\"\n";
				continue;
			}

			auto paramKey = param.leftRef(equalsPos);
			auto paramValue = param.rightRef(param.size() - equalsPos - 1);

			if(0 == paramKey.compare(QStringLiteral("secret"), Qt::CaseInsensitive)) {
				seed = paramValue.toString();
			}
			else if(0 == paramKey.compare(QStringLiteral("issuer"), Qt::CaseInsensitive)) {
				if(!issuer.isEmpty() && paramValue != issuer) {
					qWarning() << "\"issuer\" parameter" << paramKey << "does not agree with issuer part of URI" << issuer << ". ignoring parameter";
				}
				else {
					issuer = QString::fromUtf8(QByteArray::fromPercentEncoding(paramValue.toUtf8()));
				}
			}
			else if(0 == paramKey.compare(QStringLiteral("counter"), Qt::CaseInsensitive)) {
				bool ok;
				int myCounter = paramValue.toInt(&ok);

				if(ok && 0 <= myCounter) {
					counter = myCounter;
				}
				else {
					qWarning() << "invalid \"counter\" parameter:" << paramValue;
				}
			}
			else if(0 == paramKey.compare(QStringLiteral("digits"), Qt::CaseInsensitive)) {
				int myDigits = paramValue.toInt();

				if(6 == myDigits || 8 == myDigits) {
					digits = myDigits;
				}
				else {
					qWarning() << "invalid \"digits\" parameter:" << paramValue;
				}
			}
			else if(0 == paramKey.compare(QStringLiteral("period"), Qt::CaseInsensitive)) {
				int myPeriod = paramValue.toInt();

				if(0 < myPeriod) {
					period = myPeriod;
				}
				else {
					qWarning() << "invalid \"period\" parameter:" << paramValue;
				}
			}
			else if(0 == paramKey.compare(QStringLiteral("algorithm"), Qt::CaseInsensitive)) {
				qWarning() << "\"algorithm\" parameter found but not yet supported. algorithm is" << paramValue << "; only SHA1 supported";
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


	std::unique_ptr<Otp> OtpQrCodeReader::createOtp() const {
		if(!m_seed.isEmpty() && (6 == m_digits || 8 == m_digits)) {
			auto ret = std::make_unique<Otp>(type(), issuer(), name(), seed(), Otp::SeedType::Base32);
			ret->setCounter(static_cast<quint64>(m_counter));
			ret->setInterval(m_interval);
			ret->setDisplayPluginName((8 == m_digits ? QStringLiteral("EightDigitsPlugin") : QStringLiteral("SixDigitsPlugin")));
			return ret;
		}

		return {};
	}


}  // namespace Qonvince
