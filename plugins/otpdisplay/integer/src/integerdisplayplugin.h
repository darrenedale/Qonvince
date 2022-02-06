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

#include "otpdisplayplugin.h"

template<int Digits>
class IntegerDisplayPlugin
        : public LibQonvince::OtpDisplayPlugin
{
    QString codeDisplayString(const QByteArray & hmac) const override
    {
        // calculate offset and read value from 4 bytes at offset
        int offset = static_cast<char>(hmac[19]) & 0xf;
        auto ret = static_cast<quint32>((hmac[offset] & 0x7f) << 24 | (hmac[offset + 1] & 0xff) << 16 | (hmac[offset + 2] & 0xff) << 8 |
                                        (hmac[offset + 3] & 0xff));

        // convert value to requested number of digits
        quint32 mod = 1;

        for (int i = 0; i < Digits; ++i) {
            mod *= 10;
        }

        return QString::number(ret % mod).rightJustified(Digits, '0');
    }
};

#endif  // QONVINCE_OTPDISPLAYPLUGIN_INTEGER_H
