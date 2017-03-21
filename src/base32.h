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

#ifndef QONVINCE_BASE32_H
#define QONVINCE_BASE32_H

/** \file base32.h
  * \brief Definition of the Base32 class.
  *
  * \todo
  * - unit test this class.
  */
#include <QByteArray>

namespace Qonvince {
	/** \class Base32
	  * \brief Base32 encoder/decoder.
	  *
	  * Objects of this class can be used to encode or decode Base32 data.
	  * Use of the class is very simple. Create a new object, optionally with
	  * some data to encode. Call encoded() to get the Base32 encoded data,
	  * or plain() to get the plain (unencoded) data. Both return a QByteArray.
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
	  * representations of the data are both stored in the object.
	  */
	class Base32 {
		public:
			explicit Base32( const QByteArray & plainData = QByteArray() );

			/** \brief Check whether the Base32 object is valid.
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
			bool isValid( void ) {
				return m_isValid;
			}

			/** \brief Set the unencoded (plain) binary data to encode.
			 *
			 * \param data The binary data to encode.
			 *
			 * \return \b true.
			 */
			bool setPlain( const QByteArray & data );

			/** \brief Set the Base32-encoded data to decode.
			 *
			 * \param base32 The Base32-encoded data to decode.
			 *
			 * \return \b true if the data provided was a valid Base32 string,
			 * \b false if it contains invalid characters.
			 */
			bool setEncoded( const QByteArray & base32 );

			/** \brief Fetch the plain (unencoded) data.
			  *
			  * This method will return the plain data decoded from the Base32-
			  * encoded data set with setEncoded(), or the plain data set with
			  * setPlain(), depending on whether the object was provided with
			  * plain or encoded data most recently.
			  *
			  * \return The plain (unencoded) data.
			  */
			const QByteArray & plain( void ) {
				if(!m_plainInSync) {
					decode();
				}

				return m_plain;
			}

			/** \brief Fetch the Base32-encoded data.
			  *
			  * This method will return the Base32-encoded data encoded from
			  * plain data set with setPlain(), or the encoded data set with
			  * setEncoded(), depending on whether the object was provided with
			  * plain or encoded data most recently.
			  *
			  * \return The Base32-encoded data.
			  */
			const QByteArray & encoded( void ) {
				if(!m_encodedInSync) {
					encode();
				}

				return m_encoded;
			}

		private:
			/** The dictionary of Base32 characters. */
			static const char * Dictionary;

			/** Whether or not the encoder/decoder is valid. */
			bool m_isValid;

			/** Whether or not the plain data member is the plain representation of
			 * the Base32-encoded data member. */
			bool m_plainInSync;

			/** Whether or not the Base32-encoded data member is the Base32
			 * representation of the plain data member. */
			bool m_encodedInSync;

			/** The plain (unencoded) data. */
			QByteArray m_plain;

			/** The Base32-encoded data. */
			QByteArray m_encoded;

#if defined(QT_DEBUG)
			template <typename T> QByteArray toBinary( const T & value, int sep = 8 );
			QByteArray toBinary( const QByteArray & data, int sep );
#endif

			/** \brief Decode the Base32-encoded data into the plain data member. */
			void decode( void );

			/** \brief Encode the plain data into the Base32 data member. */
			void encode( void );
	};
}

#endif // QONVINCE_BASE32_H
