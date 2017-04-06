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

/*
 * Copyright (c) 2011, Andre Somers
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 	* Redistributions of source code must retain the above copyright
 * 	  notice, this list of conditions and the following disclaimer.
 * 	* Redistributions in binary form must reproduce the above copyright
 * 	  notice, this list of conditions and the following disclaimer in the
 * 	  documentation and/or other materials provided with the distribution.
 * 	* Neither the name of the Rathenau Instituut, Andre Somers nor the
 * 	  names of its contributors may be used to endorse or promote products
 * 	  derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR #######; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef QONVINCE_CRYPT_H
#define QONVINCE_CRYPT_H

#include <QString>
#include <QVector>
#include <QFlags>

/**
  \short Simple encryption and decryption of strings and byte arrays

  This class provides a simple implementation of encryption and decryption
  of strings and byte arrays.

  \warning The encryption provided by this class is NOT strong encryption. It may
  help to shield things from curious eyes, but it will NOT stand up to someone
  determined to break the encryption. Don't say you were not warned.

  The class uses a 64 bit key. Simply create an instance of the class, set the key,
  and use the encryptToString() method to calculate an encrypted version of the input string.
  To decrypt that string again, use an instance of Crypt initialized with
  the same key, and call the decryptToString() method with the encrypted string. If the key
  matches, the decrypted version of the string will be returned again.

  If you do not provide a key, or if something else is wrong, the encryption and
  decryption function will return an empty string or will return a string containing nonsense.
  lastError() will return a value indicating if the method was succesful, and if not, why not.

  Crypt is prepared for the case that the encryption and decryption
  algorithm is changed in a later version, by prepending a version identifier to the cypertext.
  */
namespace Qonvince {
	class Crypt
	{
		public:
			/**
			  IntegrityProtectionMode describes measures taken to make it possible to detect problems with the data
			  or wrong decryption keys.

			  Measures involve adding a checksum or a cryptograhpic hash to the data to be encrypted. This
			  increases the length of the resulting cypertext, but makes it possible to check if the plaintext
			  appears to be valid after decryption.
			*/
			enum IntegrityProtectionMode {
				NoProtection,      /*!< The integerity of the encrypted data is not protected. It is not really possible to detect a wrong key, for instance. */
				ChecksumProtection,/*!< A simple checksum is used to verify that the data is in order. If not, an empty string is returned. */
				HashProtection     /*!< A cryptographic hash is used to verify the integrity of the data. This method produces a much stronger, but longer check */
			};

			/**
			  Describes the type of error that occured.
			  */
			enum ErrorCode {
				ErrOk,						/*!< No error occurred. */
				ErrKeyTooShort,				/*!< The key provided contained fewer than 8 bytes */
				ErrNoKeySet,				/*!< No key was set. You can not encrypt or decrypt without a valid key. */
				ErrUnknownVersion,			/*!< The version of this data is unknown, or the data is otherwise not valid. */
				ErrIntegrityCheckFailed,	/*!< The integrity check of the data failed. Perhaps the wrong key was used. */
				ErrUuidMismatch,			/*!< The UUID in the encrypted data does not match the UUID of the machine decrypting it */
			};

			static const int LatestVersion;

			/**
			  Constructor.

			  Constructs a Crypt instance without a valid key set on it.
			 */
			Crypt( void );

			/**
			  Constructor.
			  Constructs a Crypt instance and initializes it with the given \arg key.
			 */
			explicit Crypt( quint64 key );
			explicit Crypt( const QByteArray & key );

			/**
			  (Re-) initializes the key with the given \arg key.
			  */
			void setKey( quint64 key );
			bool setKey( const QByteArray & key );

			/**
			  Returns true if Crypt has been initialized with a key.
			  */
			inline bool hasKey( void ) const {
				return !m_keyParts.isEmpty();
			}

			/**
			  Sets the integrity mode to use when encrypting data. The default mode is Checksum.

			  Note that decryption is not influenced by this mode, as the decryption recognizes
			  what mode was used when encrypting.
			  */
			inline void setIntegrityProtectionMode( IntegrityProtectionMode mode ) {
				m_protectionMode = mode;
			}

			/**
			  Returns the IntegrityProtectionMode that is currently in use.
			  */
			inline IntegrityProtectionMode integrityProtectionMode( void ) const {
				return m_protectionMode;
			}

			/**
			 Set whether or not the UUID is used in the encryption.

			 If the UUID is used, data encrypted can only be decrypted on the
			 machine on which it was encrypted.
			 */
			inline void setUseUuid( bool use ) {
				m_useUuid = use;
			}

			/**
			 Returns whether or not the UUID is used in the encryption.

			 If the UUID is used, data encrypted can only be decrypted on the
			 machine on which it was encrypted.
			 */
			inline bool useUuid( void ) const {
				return m_useUuid;
			}

			/**
			  Returns the last error that occurred.
			  */
			inline ErrorCode lastError( void ) const {
				return m_lastError;
			}

			/**
			  Encrypts the @arg plaintext string with the key the class was initialized with, and returns
			  a ciphertext the result. The result is a base64 encoded version of the binary array that is the
			  actual result of the string, so it can be stored easily in a text format.
			  */
			QString encrypt( const QString & plaintext, ErrorCode * outcome = nullptr, int version = LatestVersion ) const;
			QString encrypt( const QByteArray & plaintext, ErrorCode * outcome = nullptr, int version = LatestVersion ) const;

			/**
			  Decrypts a ciphertext string encrypted with this class with the set key back to the
			  plain text version.

			  If an error occured, such as non-matching keys between encryption and decryption,
			  an empty string or a string containing nonsense may be returned.
			  */
			QString decrypt( const QString & ciphertext, ErrorCode * outcome = nullptr ) const;

			//enum to describe options that have been used for the encryption. Currently only one, but
			//that only leaves room for future extensions like adding a cryptographic hash...
			enum CryptoFlag {
				/* v3+ only */
				CryptoFlagNone = 0x00,
				CryptoFlagChecksum = 0x01,
				CryptoFlagHash = 0x02,

				/* v4+ only */
				CryptoFlagUuid = 0x04,
			};

			Q_DECLARE_FLAGS(CryptoFlags, CryptoFlag)

		private:
			static const QByteArray & machineUuid( void );

			QString decrypt( const QByteArray & ciphertext, ErrorCode * outcome = nullptr ) const;

			QString encryptV3( const QByteArray & plaintext, ErrorCode * outcome = nullptr ) const;
			QString decryptV3( const QByteArray & ciphertext, ErrorCode * outcome = nullptr ) const;

			/* not yet fully implemented */
//			QString encryptV4( const QByteArray & plaintext, ErrorCode * outcome = nullptr ) const;
//			QString decryptV4( const QByteArray & ciphertext, ErrorCode * outcome = nullptr ) const;

			/* uses longer keys for increased strength (keys are never shorter than old versions) */
			QString encryptV5( const QByteArray & plaintext, ErrorCode * outcome = nullptr ) const;
			QString decryptV5( const QByteArray & ciphertext, ErrorCode * outcome = nullptr ) const;

			void splitKey();

			/* v3+ features */
			quint64 m_oldKey;
			QVector<char> m_keyParts;
			IntegrityProtectionMode m_protectionMode;

			/* v4+ features */
			bool m_useUuid;

			/* v5+ features */
			QByteArray m_key;

			mutable ErrorCode m_lastError;
	};

	Q_DECLARE_OPERATORS_FOR_FLAGS(Crypt::CryptoFlags)
}

#endif // QONVINCE_CRYPT_H
