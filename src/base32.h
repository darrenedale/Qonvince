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
 * \brief Declaration of the Base32 class template.
 */

#ifndef LIBQONVINCE_BASE32_H
#define LIBQONVINCE_BASE32_H

#include <array>
#include <algorithm>
#include <iostream>

namespace LibQonvince {
	template<class ByteArrayT = std::basic_string<char>, typename ByteT = typename ByteArrayT::value_type>
	class Base32 final {
		public:
			using ByteArray = ByteArrayT;
			using Byte = ByteT;
			
		explicit Base32(const ByteArrayT & plainData = {})
				: m_isValid(false),
				  m_plainInSync(true),
				  m_encodedInSync(false),
				  m_plain(plainData) {}

		inline bool isValid() {
			return m_isValid;
		}

		bool setPlain(const ByteArrayT & data) {
			m_plain = data;
			m_plainInSync = true;
			m_encodedInSync = false;
			return true;
		}

		bool setEncoded(const ByteArrayT & base32) {
			static const auto DictBegin = Dictionary.cbegin();
			static const auto DictEnd = Dictionary.cend();
			bool stillTrimmingTrailingEquals = true;

			for(auto i = static_cast<int>(base32.size()) - 1; 0 <= i; --i) {
				auto idx = i;
				unsigned char ch = base32.at(idx);

				if(stillTrimmingTrailingEquals) {
					if('=' == ch) {
						continue;
					}

					stillTrimmingTrailingEquals = false;
				}

				if(DictEnd == std::find(DictBegin, DictEnd, ch)) {
					std::cerr << "invalid base32 character found\n";
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

		inline const ByteArrayT & plain() {
			if(!m_plainInSync) {
				decode();
			}

			return m_plain;
		}

		inline const ByteArrayT & encoded() {
			if(!m_encodedInSync) {
				encode();
			}

			return m_encoded;
		}

	private:
		static constexpr const std::array<char, 32> Dictionary = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7'};
		
		/** Whether or not the encoder/decoder is valid. */
		bool m_isValid;

		/** Whether or not the plain data member is the plain representation of
		 * the Base32-encoded data member. */
		bool m_plainInSync;

		/** Whether or not the Base32-encoded data member is the Base32
		 * representation of the plain data member. */
		bool m_encodedInSync;

		/** The plain (unencoded) data. */
		ByteArrayT m_plain;

		/** The Base32-encoded data. */
		ByteArrayT m_encoded;

#if defined(QT_DEBUG)
		template<typename T>
		ByteArray toBinary(const T & value, int sep = 8);
		ByteArray toBinary(const ByteArrayT & data, int sep);
#endif

		void decode() {
			static const auto DictBegin = Dictionary.cbegin();
			static const auto DictEnd = Dictionary.cend();
			ByteArrayT ba = m_encoded;

			std::transform(ba.begin(), ba.end(), ba.begin(), [](const ByteT & ch) -> ByteT {
				return static_cast<ByteT>(std::toupper(ch));
			});
	//		for(auto & ch: ba) {
	////		for(auto it = ba.begin(); it != ba.end(); ++it) {
	////			auto ch = *it;

	//			if('a' <= ch && 'z' >= ch) {
	//				ch -= 32;
	//			}
	//		}

			/* tolerate badly terminated encoded strings by padding with = to an appropriate
			 * length
			 * TODO use an algorithm to do this rather than a loop */
			ba.insert(ba.end(), ba.size() % 8, '=');
	//		for(int i = ba.size() % 8; i >= 0; --i) {
	//			ba.push_back('=');
	//		}

			m_plain.clear();
			int j;

			for(int i = 0; i < ba.size(); i += 8) {
				uint64_t out = 0x00;

				for(j = 0; j < 8; ++j) {
					if('=' == ba[i + j]) {
						break;
					}

					auto pos = std::find(DictBegin, DictEnd, ba[i + j]);

					if(DictEnd == pos) {
						std::cerr << "invalid character in base32 data:" << ba[i + j] << "\n";
						m_isValid = false;
						m_plain.clear();
						return;
					}

					out <<= 5;
					out |= (std::distance(DictBegin, pos) & 0x1f);
				}

				/* in any chunk we must have processed either 2, 4, 5, 7 or 8 bytes */
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
						std::cerr << "invalid base32 sequence at byte " << i << "\n";
						m_isValid = false;
						m_plain.clear();
						return;
				}

				std::array<ByteT, 5> outBytes;
				outBytes[4] = static_cast<ByteT>(out & 0xff);
				outBytes[3] = static_cast<ByteT>((out >> 8) & 0xff);
				outBytes[2] = static_cast<ByteT>((out >> 16) & 0xff);
				outBytes[1] = static_cast<ByteT>((out >> 24) & 0xff);
				outBytes[0] = static_cast<ByteT>((out >> 32) & 0xff);

				const auto begin = outBytes.cbegin();
				std::copy(begin, begin + outByteCount, std::back_inserter(m_plain));
	//			for(ByteArrayT::size_type i = 0; i < outByteCount; ++i) {
	//				m_plain.push_back(outBytes[i]);
	//			}
			}

			m_isValid = true;
			m_plainInSync = true;
		}

		void encode()  {
			ByteArrayT ba = m_plain;
			m_encoded.clear();

			// pad to a multiple of 5 chars with null bytes
			ba.insert(ba.end(), 5 - (ba.size() % 5), 0);
	//		for(int i = 5 - (ba.size() % 5); i >= 0; --i) {
	//			ba.push_back(0);
	//		}

			unsigned int pos = 0;
	//		const unsigned char * data = ba.data();

			while(pos < ba.size()) {
				uint64_t bits = 0x00 | ((uint64_t(ba[pos])) << 32) |
									 ((uint64_t(ba[pos + 1])) << 24) |
									 ((uint64_t(ba[pos + 2])) << 16) |
									 ((uint64_t(ba[pos + 3])) << 8) |
									 ((uint64_t(ba[pos + 4])));
				std::array<ByteT, 8> out;

				for(int i = 7; i >= 0; --i) {
					out[static_cast<std::size_t>(i)] = Dictionary[bits & 0x1f];
					bits = bits >> 5;
				}

				std::copy(out.cbegin(), out.cend(), std::back_inserter(m_encoded));
	//			for(int i = 0; i < 8; ++i) {
	//				m_encoded.push_back(out[static_cast<decltype(out)::size_type>(i)]);
	//			}

				pos += 5;
			}

			unsigned overrideLength = 0;

			switch(m_plain.size() % 5) {
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
				m_encoded.insert(m_encoded.end(), overrideLength, '=');
	//			for(ByteArrayT::size_type i = 0; i < overrideLength; ++i) {
	//				m_encoded.push_back('=');
	//			}
			}

			m_encodedInSync = true;
		}
	};

}  // namespace Qonvince

#endif  // LIBQONVINCE_BASE32_H
