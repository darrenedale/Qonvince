/*
 * Copyright 2015 - 2020 Darren Edale
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

namespace LibQonvince
{
	/**
	 * Codec class for Base32 data.
	 *
	 * Enables conversion between plain text and Base32 encoding. Can be constructed with plain text data, or can have either plain text
	 * or encoded data set using setPlain() and setEncoded() respectively. The plain text and Base32-encoded content can be retrieved
	 * using plain() and encoded() respectively. Use isValid() to check whether the encoded data is valid when setEncoded() has been used
	 * with data that you can't guarantee is known to be valid Base32.
	 *
	 * Encoding/decoding is only performed when required, so the class is relatively lightweight.
	 *
	 * @tparam ByteArrayT Type of strings of bytes that are encoded/decoded.
	 * @tparam ByteT Type of bytes that are encoded/decoded. It must be possible to implicitly cast between the
	 * element type in ByteArrayT and this type. Defaults to ByteArrayT::value_type.
	 */
	template<class ByteArrayT = std::basic_string<char>, typename ByteT = typename ByteArrayT::value_type>
	class Base32 final
	{
	public:
		using ByteArray = ByteArrayT;
		using Byte = ByteT;

		/**
		 * Initialise a new object, optionally with some specified plain text.
		 *
		 * @param plainData
		 */
		explicit Base32(const ByteArrayT & plainData = {})
		: m_isValid(true),
		  m_plainInSync(true),
		  m_encodedInSync(false),
		  m_plain(plainData) {}

		/**
		 * Determine whether the object has valid content.
		 *
		 * @return true if the content is valid, false otherwise.
		 */
		inline bool isValid() const
		{
			return m_isValid;
		}

		/**
		 * Set the plain-text data.
		 *
		 * @param data The plain-text data to encode.
		 *
		 * @return true.
		 */
		bool setPlain(const ByteArrayT & data)
		{
			m_plain = data;
			m_plainInSync = true;
			m_encodedInSync = false;
			m_isValid = true;
			return true;
		}

		/**
		 * Set the Base32 encoded content.
		 *
		 * If the provided content is not valid Base32, the state of the object is undefined.
		 *
		 * @param base32 The Base32 content to decode.
		 *
		 * @return true if the content was valid Base32, false otherwise.
		 */
		bool setEncoded(const ByteArrayT & base32)
		{
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
					std::cerr << "invalid base32 character '" << ch << "' found at byte position " << i << "\n";
					m_isValid = false;
					return false;
				}
			}

			m_encoded = base32;
			m_plainInSync = false;
			m_encodedInSync = true;
			m_isValid = true;
			return true;
		}

		/**
		 * Fetch the plain-text content.
		 *
		 * @return A const reference to the plain text content of the object.
		 */
		inline const ByteArrayT & plain() const
		{
			if(!m_plainInSync) {
				decode();
			}

			return m_plain;
		}

		/**
		 * Fetch the Base32 encoded content.
		 *
		 * If the object is not valid, this is undefined.
		 *
		 * @return A const reference to the Base32 encoded content of the object.
		 */
		inline const ByteArrayT & encoded() const
		{
			if(!m_encodedInSync) {
				encode();
			}

			return m_encoded;
		}

	private:
		/**
		 * Internal helper to decode the Base32 encoded content.
		 *
		 * This is called when the plain text content is requested and the internal cache of the plain text content is out of sync.
		 */
		void decode() const
		{
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
			}

			int j;
			auto iLoopEnd = static_cast<int>(ba.size());

			for(int i = 0; i < iLoopEnd; i += 8) {
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

					out <<= 5u;
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
						out <<= 5u;
						break;

					case 5:
						outByteCount = 3;
						out <<= 15u;
						break;

					case 4:
						outByteCount = 2;
						out <<= 20u;
						break;

					case 2:
						outByteCount = 1;
						out <<= 30u;
						break;

					default:
						std::cerr << "invalid base32 sequence at byte " << i << "\n";
						m_isValid = false;
						m_plain.clear();
						return;
				}

				std::array<ByteT, 5> outBytes;
				outBytes[4] = static_cast<ByteT>(out & 0xffu);
				outBytes[3] = static_cast<std::make_unsigned_t<ByteT>>((out >> 8u) & 0xffu);
				outBytes[2] = static_cast<ByteT>((out >> 16u) & 0xffu);
				outBytes[1] = static_cast<ByteT>((out >> 24u) & 0xffu);
				outBytes[0] = static_cast<ByteT>((out >> 32u) & 0xffu);

				const auto begin = outBytes.cbegin();
				std::copy(begin, begin + outByteCount, std::back_inserter(m_plain));
			}

			m_isValid = true;
			m_plainInSync = true;
		}

		/**
		 * Internal helper to encode the plain text content as Base32 when required.
		 *
		 * This is called when the encoded content is requested and the internal cache of the encoded content is out of sync.
		 */
		void encode() const
		{
			m_encoded.clear();
			auto len = m_plain.size();

			if(0 == len) {
				return;
			}

			auto remainder = len % 5;

			if(0 < remainder) {
				std::fill_n(std::back_inserter(m_plain), 5 - remainder, 0);
			}

			unsigned int paddedLen = len + (5 - remainder);
			unsigned int pos = 0;

			while(pos < paddedLen) {
				uint64_t bits = 0x00u | (static_cast<uint64_t>(m_plain[pos]) << 32u) |
									 (static_cast<uint64_t>(m_plain[pos + 1]) << 24u) |
									 (static_cast<uint64_t>(m_plain[pos + 2]) << 16u) |
									 (static_cast<uint64_t>(m_plain[pos + 3]) << 8u) |
									 (static_cast<uint64_t>(m_plain[pos + 4]));

				// TODO do dict lookup directly into m_encoded rather than via out
				std::array<ByteT, 8> out;
				auto * bytePtr = std::addressof(out.back());

				for(typename decltype(out)::size_type i = 0; i < out.size(); ++i) {
					*bytePtr = Dictionary[bits & 0x1fu];
					--bytePtr;
					bits = bits >> 5u;
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
		static constexpr const DictionaryT Dictionary = {{'A', 'B', 'C', 'D', 'E', 'F',
																		  'G', 'H', 'I', 'J', 'K', 'L',
																		  'M', 'N', 'O', 'P', 'Q', 'R',
																		  'S', 'T', 'U', 'V', 'W', 'X',
																		  'Y', 'Z', '2', '3', '4', '5',
																		  '6', '7'}};
		static constexpr const DictionaryT::const_iterator DictBegin = Dictionary.cbegin();
		static constexpr const DictionaryT::const_iterator DictEnd = Dictionary.cend();

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

}	// namespace LibQonvince

#endif  // LIBQONVINCE_BASE32_H
