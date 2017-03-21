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

#include "base32.h"

#include <cstring>

#include <QDebug>


using namespace Qonvince;


const char * Base32::Dictionary = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";


Base32::Base32( const QByteArray & data )
:	m_isValid(false),
	m_plainInSync(true),
	m_encodedInSync(false),
	m_plain(data) {
}


bool Base32::setPlain( const QByteArray & data ) {
	m_plain = data;
	m_plainInSync = true;
	m_encodedInSync = false;
	return true;
}


bool Base32::setEncoded( const QByteArray & base32 ) {
	bool stillTrimmingTrailingEquals = true;

	for(int i = base32.length() - 1; i >= 0; --i) {
		char c(base32.at(i));

		if(stillTrimmingTrailingEquals) {
			if('=' == c) {
				continue;
			}

			stillTrimmingTrailingEquals = false;
		}

		if(!std::strchr(Dictionary, base32.at(i))) {
            qCritical() << "invalid base32 character found";
			m_isValid = false;
			return false;
		}
	}

	m_isValid = true;
	m_encoded = base32;
	m_plainInSync = false;
	m_encodedInSync = true;
	return true;
}


void Base32::encode( void ) {
	QByteArray ba(m_plain);
	m_encoded.clear();

	if(ba.length() % 5) {
		ba.append(QByteArray(5 - (ba.length() % 5), char(0)));
	}

	int pos = 0;
	unsigned char * data = (unsigned char *) ba.data();

	while(pos < ba.length()) {
		quint64 bits = 0x00 |
				((quint64(data[pos])) << 32) |
				((quint64(data[pos + 1])) << 24) |
				((quint64(data[pos + 2])) << 16) |
				((quint64(data[pos + 3])) << 8) |
				((quint64(data[pos + 4])));
		char out[8];

		for(int i = 7; i >= 0; --i) {
			out[i] = Dictionary[bits & 0x1f];
			bits = bits >> 5;
		}

		m_encoded.append(out, 8);
		pos += 5;
	}

	int overrideLength = 0;

	switch(m_plain.length() % 5) {
		case 1:
			overrideLength = 6;
			break;

		case 2:
			overrideLength = 4;
			break;

		case 3:
			overrideLength = 3;
			break;

		case 4:
			overrideLength = 1;
			break;
	}

	if(0 < overrideLength) {
		m_encoded = m_encoded.left(m_encoded.length() - overrideLength) + QByteArray(overrideLength, '=');
	}

	m_encodedInSync = true;
}


void Base32::decode( void ) {
	QByteArray ba(m_encoded.toUpper());

	/* tolerate badly terminated encoded strings */
	if(ba.length() % 8) {
		ba.append(QByteArray(8 - (ba.length() % 8), '='));
	}

	m_plain.clear();
	char * data = ba.data();
	int j;

	for(int i = 0; i < ba.length(); i += 8) {
		quint64 out = 0x00;

		for(j = 0; j < 8; ++j) {
			if('=' == data[i + j]) {
				break;
			}

			int pos = std::strchr(Dictionary, data[i + j]) - Dictionary;

			if(0 > pos) {
				qDebug() << "invalid character in base32 data:" << data[i + j];
				m_isValid = false;
				m_plain.clear();
				return;
			}

			out <<= 5;
			out |= (pos & 0x1f);
		}

		/* in any chunk we must have processed either 2, 4, 5, 7 or 8 bytes */
		int outByteCount;

		switch(j) {
			case 8:
				outByteCount = 5;
				break;

			case 7:
				outByteCount = 4;
				out <<= 5;
				break;

			case 5:
				outByteCount = 3;
				out <<= 15;
				break;

			case 4:
				outByteCount = 2;
				out <<= 20;
				break;

			case 2:
				outByteCount = 1;
				out <<= 30;
				break;

			default:
				qDebug() << "invalid base32 sequence" << ba.mid(i, 8);
				m_isValid = false;
				m_plain.clear();
				return;
		}

		unsigned char outBytes[5];
		outBytes[4] = (unsigned char)(out & 0xff);
		outBytes[3] = (unsigned char)((out >> 8) & 0xff);
		outBytes[2] = (unsigned char)((out >> 16) & 0xff);
		outBytes[1] = (unsigned char)((out >> 24) & 0xff);
		outBytes[0] = (unsigned char)((out >> 32) & 0xff);
		m_plain.append((const char *) outBytes, outByteCount);
	}

	m_isValid = true;
	m_plainInSync = true;
}


#if defined(QT_DEBUG)
template <typename T> QByteArray Base32::toBinary( const T & value, int sep ) {
	int n = sizeof(T);
	QByteArray ret;
	char * data = (char *) &value;
	int bit = 0;
	ret.reserve(n * 8 + (double(n) / sep) + 1);

	for(int i = 0; i < n; ++i) {
		unsigned char mask = 0x80;

		while(mask) {
			ret.append(*data & mask ? '1' : '0');
			mask >>= 1;

			++bit;

			if(0 == (bit % sep)) {
				ret.append(' ');
			}
		}

		++data;
	}

	return ret;
}


QByteArray Base32::toBinary( const QByteArray & data, int sep ) {
	int n = data.length();
	const char * start = data.data();
	const char * end = start + n;
	char * d = (char *) start;
	QByteArray ret;
	ret.reserve(n * 8 + (double(n) / sep) + 1);
	int bit = 0;

	while(d < end) {
		unsigned char mask = 0x80;

		while(mask) {
			ret.append(*d & mask ? '1' : '0');
			mask >>= 1;

			++bit;

			if(0 == (bit % sep)) {
				ret.append(' ');
			}
		}

		++d;
	}

	return ret;
}
#endif
