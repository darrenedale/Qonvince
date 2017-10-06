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
 * \file base32.h
 * \brief Declaration of the Base32 class.
 */

#ifndef QONVINCE_BASE32_H
#define QONVINCE_BASE32_H

#include <array>
#include <string>

namespace Qonvince {
	class Base32 {
	public:
		using Byte = unsigned char;
		using ByteArray = std::basic_string<Byte>;

		explicit Base32(const ByteArray & plainData = {});

		inline bool isValid(void) {
			return m_isValid;
		}

		bool setPlain(const ByteArray & data);
		bool setEncoded(const ByteArray & base32);

		inline const ByteArray & plain(void) {
			if(!m_plainInSync) {
				decode();
			}

			return m_plain;
		}

		inline const ByteArray & encoded(void) {
			if(!m_encodedInSync) {
				encode();
			}

			return m_encoded;
		}

	private:
		static const std::array<Byte, 32> Dictionary;

		/** Whether or not the encoder/decoder is valid. */
		bool m_isValid;

		/** Whether or not the plain data member is the plain representation of
		 * the Base32-encoded data member. */
		bool m_plainInSync;

		/** Whether or not the Base32-encoded data member is the Base32
		 * representation of the plain data member. */
		bool m_encodedInSync;

		/** The plain (unencoded) data. */
		ByteArray m_plain;

		/** The Base32-encoded data. */
		ByteArray m_encoded;

#if defined(QT_DEBUG)
		template<typename T>
		ByteArray toBinary(const T & value, int sep = 8);
		ByteArray toBinary(const ByteArray & data, int sep);
#endif

		void decode(void);
		void encode(void);
	};

}  // namespace Qonvince

#endif  // QONVINCE_BASE32_H
