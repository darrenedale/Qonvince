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

		inline bool isValid() const {
			return m_isValid;
		}

		bool setPlain(const ByteArrayT & data) {
			m_plain = data;
			m_plainInSync = true;
			m_encodedInSync = false;
			return true;
		}

		bool setEncoded(const ByteArrayT & base32) {
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
					std::cerr << "invalid base32 character found at byte position " << i << "\n";
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

		inline const ByteArrayT & plain() const {
			if(!m_plainInSync) {
				decode();
			}

			return m_plain;
		}

		inline const ByteArrayT & encoded() const {
			if(!m_encodedInSync) {
				encode();
			}

			return m_encoded;
		}

	private:
		void decode() const {
			auto & ba = m_encoded;

			std::transform(ba.begin(), ba.end(), ba.begin(), [](const ByteT & ch) -> ByteT {
				return static_cast<ByteT>(std::toupper(ch));
			});

			m_plain.clear();

			// tolerate badly terminated encoded strings by padding with = to appropriate len
			auto len = ba.size();

			if(0 == len) {
				return;
			}

			auto remainder = len % 8;

			if(0 < remainder) {
				std::fill_n(std::back_inserter(ba), 8 - remainder, '=');
				//		for(int i = 8 - (ba.size() % 8); i >= 0; --i) {
				//			ba.push_back('=');
				//		}
			}

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

		void encode() const {
			m_encoded.clear();
			auto len = m_plain.size();

			if(0 == len) {
				return;
			}

			auto remainder = len % 5;

			if(0 < remainder) {
				std::fill_n(std::back_inserter(m_plain), 5 - remainder, 0);
			}

			auto paddedLen = len + (5 - remainder);
			unsigned int pos = 0;

			while(pos < paddedLen) {
				uint64_t bits = 0x00 | ((uint64_t(m_plain[pos])) << 32) |
									 ((uint64_t(m_plain[pos + 1])) << 24) |
									 ((uint64_t(m_plain[pos + 2])) << 16) |
									 ((uint64_t(m_plain[pos + 3])) << 8) |
									 ((uint64_t(m_plain[pos + 4])));
				std::array<ByteT, 8> out;

				for(int i = 7; i >= 0; --i) {
					out[static_cast<std::size_t>(i)] = Dictionary[bits & 0x1f];
					bits = bits >> 5;
				}

				std::copy(out.cbegin(), out.cend(), std::back_inserter(m_encoded));
				pos += 5;
			}

			switch(remainder) {
				case 0:
					break;

				case 1:
					std::fill_n(m_encoded.end() - 6, 6, '=');
					m_plain.resize(len);
					break;

				case 2:
					std::fill_n(m_encoded.end() - 4, 4, '=');
					m_plain.resize(len);
					break;

				case 3:
					std::fill_n(m_encoded.end() - 3, 3, '=');
					m_plain.resize(len);
					break;

				case 4:
					std::fill_n(m_encoded.end() - 1, 1, '=');
					m_plain.resize(len);
					break;
			}

			m_encodedInSync = true;
		}

		using DictionaryT = std::array<char, 32>;
		static const DictionaryT Dictionary;
		static const DictionaryT::const_iterator DictBegin;
		static const DictionaryT::const_iterator DictEnd;

		mutable bool m_isValid;

		/** Whether or not the plain data member is the plain representation of
		 * the Base32-encoded data member. */
		mutable bool m_plainInSync;

		/** Whether or not the Base32-encoded data member is the Base32
		 * representation of the plain data member. */
		mutable bool m_encodedInSync;

		/** The plain (unencoded) data. */
		mutable ByteArrayT m_plain;

		/** The Base32-encoded data. */
		mutable ByteArrayT m_encoded;
	};

	template<class ByteArrayT, typename ByteT>
	const typename Base32<ByteArrayT, ByteT>::DictionaryT Base32<ByteArrayT, ByteT>::Dictionary = {{'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '2', '3', '4', '5', '6', '7'}};

	template<class ByteArrayT, typename ByteT>
	const typename Base32<ByteArrayT, ByteT>::DictionaryT::const_iterator Base32<ByteArrayT, ByteT>::DictBegin = Dictionary.cbegin();

	template<class ByteArrayT, typename ByteT>
	const typename Base32<ByteArrayT, ByteT>::DictionaryT::const_iterator Base32<ByteArrayT, ByteT>::DictEnd = Dictionary.cend();

}  // namespace LibQonvince

#endif  // LIBQONVINCE_BASE32_H
