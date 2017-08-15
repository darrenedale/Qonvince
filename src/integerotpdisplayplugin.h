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
	:	public OtpDisplayPlugin {
		public:
			IntegerOtpDisplayPlugin( int digits );
			virtual ~IntegerOtpDisplayPlugin( void );

			virtual QString pluginName( void ) const override;
			virtual QString pluginDescription( void ) const override;
			virtual QString pluginAuthor( void ) const override;
			virtual QString displayString( const QByteArray & hmac ) const override;

			inline int digits( void ) const {
				return m_digits;
			}

			bool setDigits( int digits );

		private:
			static int DefaultDigits;
			int m_digits;
	};
}

#endif // QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
