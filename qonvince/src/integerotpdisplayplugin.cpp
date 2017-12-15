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

/** \file integerotpdisplayplugin.cpp
  * \brief Implementation of the IntegerOtpDisplayPlugin display plugin.
  */

#include "integerotpdisplayplugin.h"

#include <QString>
#include <QObject>


namespace Qonvince {


	static constexpr const int DefaultDigits = 6;


	IntegerOtpDisplayPlugin::IntegerOtpDisplayPlugin(int digits)
	: m_digits(DefaultDigits) {
		setDigits(digits);
	}


	bool IntegerOtpDisplayPlugin::setDigits(int digits) {
		if(0 < digits) {
			m_digits = digits;
			return true;
		}

		return false;
	}


	IntegerOtpDisplayPlugin::~IntegerOtpDisplayPlugin() = default;


	QString IntegerOtpDisplayPlugin::pluginName() const {
		return QObject::tr("%1-digit number").arg(m_digits);
	}


	QString IntegerOtpDisplayPlugin::pluginDescription() const {
		return QObject::tr("Display the generated passcode as a %1-digit number.").arg(m_digits);
	}


	QString IntegerOtpDisplayPlugin::pluginAuthor() const {
		return QStringLiteral("Darren Edale");
	}


	QString IntegerOtpDisplayPlugin::displayString(const QByteArray & hmac) const {
		/* calculate offset and read value from 4 bytes at offset */
		int offset = static_cast<char>(hmac[19]) & 0xf;
		auto ret = static_cast<quint32>((hmac[offset] & 0x7f) << 24 | (hmac[offset + 1] & 0xff) << 16 | (hmac[offset + 2] & 0xff) << 8 | (hmac[offset + 3] & 0xff));

		/* convert value to requested number of digits */
		quint32 mod = 1;

		for(int i = 0; i < m_digits; ++i) {
			mod *= 10;
		}

		return QString::number(ret % mod).rightJustified(m_digits, '0');
	}

}  // namespace Qonvince
