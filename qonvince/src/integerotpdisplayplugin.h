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

#ifndef QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
#define QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

class QString;

namespace Qonvince {
	class IntegerOtpDisplayPlugin
	: public OtpDisplayPlugin {
	public:
		IntegerOtpDisplayPlugin(int digits);
		virtual ~IntegerOtpDisplayPlugin();

		virtual QString pluginName() const override;
		virtual QString pluginDescription() const override;
		virtual QString pluginAuthor() const override;
		virtual QString displayString(const QByteArray & hmac) const override;

		inline int digits() const {
			return m_digits;
		}

		bool setDigits(int digits);

	private:
		int m_digits;
	};
}  // namespace Qonvince

#endif  // QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
