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


/**
 * \file base32.cpp
 * \brief Implementation of the Base32 class.
 * \author Darren Edale
 * \date October 2017
 * \version
 */


#include "base32.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "algorithms.h"


namespace LibQonvince {


	/**
	 * \class Base32
	 * \brief Base32 encoder/decoder.
	 * \author Darren Edale
	 * \date October 2017
	 * \version
	 *
	 * Objects of this class can be used to encode or decode Base32 data.
	 * Use of the class is very simple. Create a new object, optionally with
	 * some data to encode. Call encoded() to get the Base32 encoded data,
	 * or plain() to get the plain (unencoded) data. Both return a ByteArray.
	 *
	 * To decode some already-encoded data, pass that data to setEncoded() and
	 * then call plain() to retrieve the decoded data. You can also do the
	 * reverse - call setPlain() to set the unencoded data and then encoded()
	 * to retrieve the Base32 encoded data.
	 *
	 * Any binary data can be encoded in Base32, therefore setPlain() will
	 * never fail. However, it is possible to provide invalid Base32 encoded
	 * data. If setEncoded() is provided with invalid Base32 data, isValid()
	 * will return false. Always call this before trusting the return value
	 * of plain() when using the class to decode Base32 encoded data.
	 *
	 * The class is relatively lightweight, only encoding or decoding when
	 * necessary (e.g. when encoded() or plain() is called). The data is only
	 * encoded or decoded once - once done, the encoded and decoded
	 * representations of the data are both stored in the object. This aids
	 * performance, and is why most members are mutable (i.e. so that you
	 * can create a const object to encode some data and only incur the cost
	 * of encoding and storing the encoded data when you actually want to
	 * retrieve it).
	 */


	/** The dictionary of Base32 characters. */
	static constexpr const std::array<Base32::Byte, 32> Dictionary = {{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7'}};


	/**
	 * \brief Create a base-32 encoder/decoder.
	 *
	 * \param data The plain data to encode.
	 */
	Base32::Base32(const ByteArray & data)
	: m_plainInSync(true),
	  m_encodedInSync(false),
	  m_plain(data) {
	}


	/**
	 * \brief Set the unencoded (plain) binary data to encode.
	 *
	 * \param data The binary data to encode.
	 *
	 * \return \b true.
	 */
	bool Base32::setPlain(const ByteArray & data) {
		m_plain = data;
		m_plainInSync = true;
		m_encodedInSync = false;
		return true;
	}


	/**
	 * \brief Set the Base32-encoded data to decode.
	 *
	 * \param base32 The Base32-encoded data to decode.
	 *
	 * \return \b true if the data provided was a valid Base32 string,
	 * \b false if it contains invalid characters.
	 */
	bool Base32::setEncoded(const ByteArray & base32) {
		bool stillTrimmingTrailingEquals = true;

		for(auto i = static_cast<int>(base32.length()) - 1; 0 <= i; --i) {
			auto idx = static_cast<ByteArray::size_type>(i);
			unsigned char ch = base32.at(idx);

			if(stillTrimmingTrailingEquals) {
				if('=' == ch) {
					continue;
				}

				stillTrimmingTrailingEquals = false;
			}

			if(Dictionary.cend() == std::find(Dictionary.cbegin(), Dictionary.cend(), ch)) {
				std::cerr << "invalid base32 character '" << c << "' found at index " << i << "\n";
				return false;
			}
		}

		m_encoded = base32;
		m_plainInSync = false;
		m_encodedInSync = true;
		return true;
	}


	/** \brief Encode the plain data into the Base32 data member. */
	void Base32::encode(void) const {
		ByteArray ba = m_plain;
		m_encoded.clear();
		m_encoded.reserve(std::ceil((m_plain.length() * 8) / 5) + 1);

		// pad to a multiple of 5 chars with null bytes
		ba.insert(ba.end(), 5 - (ba.length() % 5), 0);
//		for(int i = 5 - (ba.length() % 5); i >= 0; --i) {
//			ba.push_back(0);
//		}

		unsigned int pos = 0;
//		const unsigned char * data = ba.data();

		while(pos < plainLength) {
			uint64_t bits = 0x00 | ((uint64_t(ba[pos])) << 32) |
								 ((uint64_t(ba[pos + 1])) << 24) |
								 ((uint64_t(ba[pos + 2])) << 16) |
								 ((uint64_t(ba[pos + 3])) << 8) |
								 ((uint64_t(ba[pos + 4])));
								 ((uint64_t(ba[pos + 3])) << 8) | ((uint64_t(ba[pos + 4])));
			std::array<Byte, 8> out;

			/* use each 5-bit chunk of the 40 bits as an index into the Base32 dictionary
			 * array to provide 8 characters of Base32 encoded output */
			for(int i = 7; i >= 0; --i) {
				out[static_cast<decltype(out)::size_type>(i)] = Dictionary[bits & 0x1f];
				bits = bits >> 5;
			}

			/* append the 8 Base-32 encoded characters to the encoded data */
			m_encoded.insert(m_encoded.end(), out.begin(), out.end());
			pos += 5;
		}

		/* if the original plain length was not a multiple of 5 we have
		 * written too many bytes to the encoded array, so we must truncate
		 * it with '=' chars replacing the extraneous content */
		ByteArray::size_type overrideLength = 0;

		switch(plainLength % 5) {
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

		m_encoded.replace(m_encoded.size() - overrideLength, overrideLength, overrideLength, '=');
		m_encodedInSync = true;
	}


	/** \brief Decode the Base32-encoded data into the plain data member. */
	void Base32::decode(void) const {
		static const auto dictBegin = Dictionary.cbegin();
		static const auto dictEnd = Dictionary.cend();
		ByteArray ba = m_encoded;
		toUpper(ba);
		auto encodedLength = ba.size();

		std::transform(ba.begin(), ba.end(), ba.begin(), [](const ByteArray::value_type & ch) -> ByteArray::value_type {
			return static_cast<ByteArray::value_type>(std::toupper(ch));
		});
//		for(auto & ch: ba) {
////		for(auto it = ba.begin(); it != ba.end(); ++it) {
////			auto ch = *it;
//			ba.push_back('=');
//		}

		m_plain.clear();
		ByteArray::size_type j;
		const auto dictBegin = Dictionary.begin();
		const auto dictEnd = Dictionary.end();

		for(auto it = ba.begin(); it != ba.end();) {
			/* the least significant 40 bits of this value will represent the
			 * decoded bytes for the chunk */
			uint64_t out = 0x00;

			/* read the next chunk of 8 characters from the encoded input string
			 * and store the dictionary index of each as 5 bits in the least
			 * significant 40 bits of the 64-bit value */
			for(j = 0; j < 8; ++j) {
				auto c = *it;

				/* '=' is padding at the end of the encoded data */
				if('=' == c) {
					/* force the input string iteration to end */
					it = ba.end();
					break;
				}

				/* find the encoded character in the Base32 dictionary ... */
				auto pos = std::find(dictBegin, dictEnd, c);

				if(dictEnd == pos) {
					std::cerr << "invalid character in base32 data: '" << c << "'\n";
					m_plain.clear();
					return;
				}

				/* ... and merge it into the 40-bit value */
				out <<= 5;
				out |= (std::distance(dictBegin, pos) & 0x1f);
				++it;
			}

			/* in any chunk we must have processed either 2, 4, 5, 7 or 8 bytes ... */
			unsigned int outByteCount;

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
					/* ... any other quantity of bytes represents an invalid Base32 sequence */
					std::cerr << "invalid base32 sequence\n";
					m_plain.clear();
					return;
			}

			/* parse the 40 bits read from the encoded string as 5 8-bit bytes ... */
			std::array<Byte, 5> outBytes;
			outBytes[4] = static_cast<Byte>(out & 0xff);
			outBytes[3] = static_cast<Byte>((out >> 8) & 0xff);
			outBytes[2] = static_cast<Byte>((out >> 16) & 0xff);
			outBytes[1] = static_cast<Byte>((out >> 24) & 0xff);
			outBytes[0] = static_cast<Byte>((out >> 32) & 0xff);

			/* ... and append them to the decoded string */
			m_plain.insert(m_plain.end(), outBytes.begin(), outBytes.end());
//				m_plain.push_back(outBytes[i]);
//			}
		}

		m_plainInSync = true;
	}


#if defined(QT_DEBUG)
	template<typename T>
	ByteArray Base32::toBinary(const T & value, int sep) {
		int n = sizeof(T);
		ByteArray ret;
		char * data = (char *) &value;
		int bit = 0;
		ret.reserve(n * 8 + (double(n) / sep) + 1);

		for(int i = 0; i < n; ++i) {
			unsigned char mask = 0x80;

			while(mask) {
				ret.push_back(*data & mask ? '1' : '0');
				mask >>= 1;

				++bit;

				if(0 == (bit % sep)) {
					ret.push_back(' ');
				}
			}

			++data;
		}

		return ret;
	}


	ByteArray Base32::toBinary(const ByteArray & data, int sep) {
		int n = data.size();
		Byte * start = data.data();
		Byte * end = start + n;
		char * d = reinterpret_cast<char *>(start);
		ByteArray ret;
		ret.reserve(n * 8 + (double(n) / sep) + 1);
		int bit = 0;

		while(d < end) {
			unsigned char mask = 0x80;

			while(mask) {
				ret.push_back(*d & mask ? '1' : '0');
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


	/**
	 * \fn Base32::isValid()
	 * \brief Check whether the Base32 object is valid.
	 *
	 * A valid object is one where encoded() is guaranteed to return
	 * the Base32-encoded version of plain() and where plain() will
	 * return the Base32 decoded version of encoded().
	 *
	 * An invalid object will result if setEncoded() is provided with
	 * an invalid Base32 string.
	 *
	 * An invalid object will provide undefined results from plain()
	 * and encoded().
	 *
	 * \return \b true if the object is valid, \b false otherwise.
	 */


	/**
	 * \fn Base32::plaine()
	 * \brief Fetch the plain (unencoded) data.
	 *
	 * This method will return the plain data decoded from the Base32-
	 * encoded data set with setEncoded(), or the plain data set with
	 * setPlain(), depending on whether the object was provided with
	 * plain or encoded data most recently.
	 *
	 * \return The plain (unencoded) data.
	 */


	/**
	 * \fn Base32::encoded()
	 * \brief Fetch the Base32-encoded data.
	 *
	 * This method will return the Base32-encoded data encoded from
	 * plain data set with setPlain(), or the encoded data set with
	 * setEncoded(), depending on whether the object was provided with
	 * plain or encoded data most recently.
	 *
	 * \return The Base32-encoded data.
	 */


}  // namespace Qonvince
