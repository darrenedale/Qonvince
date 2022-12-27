/*
 * Copyright 2018 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef QONVINCE_OTPDISPLAYPLUGIN_INTEGER_H
#define QONVINCE_OTPDISPLAYPLUGIN_INTEGER_H

#include <cmath>
#include "otpdisplayplugin.h"
#include "securestring.h"

using LibQonvince::SecureString;

template<int Digits>
class IntegerDisplayPlugin
        : public LibQonvince::OtpDisplayPlugin
{
	static_assert(0 < Digits, "The number of digits must be > 0");

	SecureString codeDisplayString(const SecureString & hmac) const override
    {
        // calculate offset and read value from 4 bytes at offset
        int offset = static_cast<char>(hmac[19]) & 0xf;
        auto value = static_cast<uint32_t>((hmac[offset] & 0x7f) << 24 | (hmac[offset + 1] & 0xff) << 16 | (hmac[offset + 2] & 0xff) << 8 |
                                        (hmac[offset + 3] & 0xff));

        // convert value to requested number of digits
		  value = value % static_cast<int>(pow(10.0, Digits));

        // initialise the returned string to all 0s so we don't need to pad it later
		  auto ret = SecureString(Digits, '0');
		  auto pos = Digits - 1;

		  // insert the digits from the value from right to left
		  while (0 < value) {
			  ret[pos] = static_cast<char>('0' + (value % 10));
			  --pos;
			  value /= 10;
		  }

		  return ret;
    }
};

#endif  // QONVINCE_OTPDISPLAYPLUGIN_INTEGER_H
