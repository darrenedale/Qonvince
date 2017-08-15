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

/** \file crypt.cpp
  * \brief Implementation of the Crypt class.
  * \deprecated
  *
  * This is based on Andre Somers SimpleCrypt class. It is a form of XOR
  * encryption and does not provide strong security. The algorithm is all
  * Andre's, I can take no credit for it. All I have done is remove some
  * features I don't need and made some cosmetic changes to match my
  * coding style.
  *
  * This class will be removed in a (near) future update since all uses of
  * it have been upgraded to use the stronger encryption offered by QCA.
  *
  * Andre's file commentary appears below, unmodified.
  */

/*
Copyright (c) 2011, Andre Somers
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the Rathenau Instituut, Andre Somers nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ANDRE SOMERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR #######; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "crypt.h"

#include <QByteArray>
#include <QtDebug>
#include <QtGlobal>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDataStream>


namespace Qonvince {


const int Crypt::LatestVersion = 5;


Crypt::Crypt( void )
:	m_oldKey(0),
	m_protectionMode(ChecksumProtection),
	m_lastError(ErrOk) {
	qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
}


Crypt::Crypt( quint64 key )
:	m_oldKey(key),
	m_protectionMode(ChecksumProtection),
	m_lastError(ErrOk) {
	qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));
	splitKey();
}


Crypt::Crypt( const QByteArray & key )
:	m_protectionMode(ChecksumProtection),
	m_lastError(ErrOk) {
	qsrand(uint(QDateTime::currentMSecsSinceEpoch() & 0xFFFF));

	if(!setKey(key)) {
		qWarning() << "Crypt::Crypt() - provided key is not valid, encryption will not be available";
	}
}


void Crypt::setKey( quint64 key ) {
	QByteArray newKey(8, 0);
	quint64 mask = 0x00000000000000ff;
	int shift = 0;

	for(int i = 0; i < 8; ++i) {
		newKey[i] = (key & mask) >> shift;
		mask <<= 8;
		shift += 8;
	}

	setKey(newKey);
}


bool Crypt::setKey( const QByteArray & key ) {
	if(8 > key.length()) {
		m_lastError = ErrKeyTooShort;
		return false;
	}

	m_key = key;
	m_oldKey = 0;

	for(int i = 0; i < 8; ++i) {
		m_oldKey |= (quint64(key.at(i)) << (i * 8));
	}

	splitKey();
	return true;
}


void Crypt::splitKey( void ) {
	m_keyParts.clear();
	m_keyParts.resize(8);

	for(int i = 0; i < 8; i++) {
		quint64 part = m_oldKey;

		for(int j = i; j > 0; j--) {
			part = part >> 8;
		}

		part = part & 0xff;
		m_keyParts[i] = static_cast<char>(part);
	}
}


QString Crypt::encrypt( const QString & text, ErrorCode * outcome, int version ) const {
	return encrypt(text.toUtf8(), outcome, version);
}


QString Crypt::encrypt( const QByteArray & plaintext, ErrorCode * outcome, int version ) const {
	if(3 == version) {
		return encryptV3(plaintext, outcome);
	}
	else if(5 == version) {
		return encryptV5(plaintext, outcome);
	}

	qWarning() << "Unrecognised algorithm version" << version;

	if(outcome) {
		*outcome = ErrUnknownVersion;
	}

	m_lastError = ErrUnknownVersion;
	return QByteArray();
}


QString Crypt::encryptV3( const QByteArray &plaintext, ErrorCode *outcome ) const {
	if(m_keyParts.isEmpty()) {
		qWarning() << "No key set.";

		if(outcome) {
			*outcome = ErrNoKeySet;
		}

		m_lastError = ErrNoKeySet;
		return QString();
	}

	QByteArray ba = plaintext;
	CryptoFlags flags = CryptoFlagNone;
	QByteArray integrityProtection;

	if(ChecksumProtection == m_protectionMode) {
		flags |= CryptoFlagChecksum;
		QDataStream s(&integrityProtection, QIODevice::WriteOnly);
		s << qChecksum(ba.constData(), ba.length());
	}
	else if(HashProtection == m_protectionMode) {
		flags |= CryptoFlagHash;
		QCryptographicHash hash(QCryptographicHash::Sha1);
		hash.addData(ba);
		integrityProtection += hash.result();
	}

	//prepend a random char to the string
	char randomChar = char(qrand() & 0xFF);
	ba = randomChar + integrityProtection + ba;
	int pos(0);
	char lastChar(0);
	int n(ba.count());

	while(pos < n) {
		ba[pos] = ba.at(pos) ^ m_keyParts.at(pos % 8) ^ lastChar;
		lastChar = ba.at(pos);
		++pos;
	}

	QByteArray resultArray;
	resultArray.append(char(0x03));  //version for future updates to algorithm
	resultArray.append(char(flags)); //encryption flags
	resultArray.append(ba);
	m_lastError = ErrOk;

	if(outcome) {
		*outcome = ErrOk;
	}

	return QString::fromLatin1(resultArray.toBase64());
}



QString Crypt::encryptV5( const QByteArray &plaintext, ErrorCode *outcome ) const {
	if(m_key.isEmpty()) {
		qWarning() << "No key set.";

		if(outcome) {
			*outcome = ErrNoKeySet;
		}

		m_lastError = ErrNoKeySet;
		return QString();
	}

	QByteArray ba = plaintext;
	CryptoFlags flags = CryptoFlagNone;
	QByteArray integrityProtection;

	if(ChecksumProtection == m_protectionMode) {
		flags |= CryptoFlagChecksum;
		QDataStream s(&integrityProtection, QIODevice::WriteOnly);
		s << qChecksum(ba.constData(), ba.length());
	}
	else if(HashProtection == m_protectionMode) {
		flags |= CryptoFlagHash;
		QCryptographicHash hash(QCryptographicHash::Sha1);
		hash.addData(ba);
		integrityProtection += hash.result();
	}

	//prepend a random char to the string
	char randomChar = char(qrand() & 0xFF);
	ba = randomChar + integrityProtection + ba;
	int pos = 0;
	char lastChar = 0;
	int n = ba.length();
	int keyLength = m_key.length();

	while(pos < n) {
		ba[pos] = ba.at(pos) ^ m_key.at(pos % keyLength) ^ lastChar;
		lastChar = ba.at(pos);
		++pos;
	}

	QByteArray resultArray;
	resultArray.append(char(0x05));  //version for future updates to algorithm
	resultArray.append(char(flags)); //encryption flags
	resultArray.append(ba);
	m_lastError = ErrOk;

	if(outcome) {
		*outcome = ErrOk;
	}

	return QString::fromLatin1(resultArray.toBase64());
}


QString Crypt::decrypt( const QString & ciphertext, ErrorCode * outcome ) const {
	return decrypt(QByteArray::fromBase64(ciphertext.toLatin1()), outcome);
}


QString Crypt::decrypt( const QByteArray & cipher, ErrorCode * outcome ) const {
	if(m_keyParts.isEmpty()) {
		qWarning() << "No key set.";

		if(outcome) {
			*outcome = ErrNoKeySet;
		}

		m_lastError = ErrNoKeySet;
		return QByteArray();
	}

	if(cipher.count() < 3) {
		return QByteArray();
	}

	char version = cipher.at(0);

	if(3 == version) {
		return decryptV3(cipher, outcome);
	}
	else if(5 == version) {
		return decryptV5(cipher, outcome);
	}

	qWarning() << "Invalid version (" << version << ") found in cipher text.";

	if(outcome) {
		*outcome = ErrUnknownVersion;
	}

	m_lastError = ErrUnknownVersion;
	return QByteArray();
}


QString Crypt::decryptV3( const QByteArray & cipher, ErrorCode * outcome ) const {
	QByteArray ba = cipher;
	CryptoFlags flags = CryptoFlags(ba.at(1));
	ba = ba.mid(2);
	int pos(0);
	int n(ba.count());
	char lastChar = 0;

	while(pos < n) {
		char currentChar = ba[pos];
		ba[pos] = ba.at(pos) ^ lastChar ^ m_keyParts.at(pos % 8);
		lastChar = currentChar;
		++pos;
	}

	ba = ba.mid(1); //chop off the random number at the start
	bool integrityOk(true);

	if(flags.testFlag(CryptoFlagChecksum)) {
		if(ba.length() < 2) {
			if(outcome) {
				*outcome = ErrIntegrityCheckFailed;
			}

			m_lastError = ErrIntegrityCheckFailed;
			return QByteArray();
		}

		quint16 storedChecksum;

		{
			QDataStream s(&ba, QIODevice::ReadOnly);
			s >> storedChecksum;
		}

		ba = ba.mid(2);
		quint16 checksum = qChecksum(ba.constData(), ba.length());
		integrityOk = (checksum == storedChecksum);
	}
	else if(flags.testFlag(CryptoFlagHash)) {
		if(ba.length() < 20) {
			if(outcome) {
				*outcome = ErrIntegrityCheckFailed;
			}

			m_lastError = ErrIntegrityCheckFailed;
			return QByteArray();
		}

		QByteArray storedHash = ba.left(20);
		ba = ba.mid(20);
		QCryptographicHash hash(QCryptographicHash::Sha1);
		hash.addData(ba);
		integrityOk = (hash.result() == storedHash);
	}

	if(!integrityOk) {
		if(outcome) {
			*outcome = ErrIntegrityCheckFailed;
		}

		m_lastError = ErrIntegrityCheckFailed;
		return QByteArray();
	}

	if(outcome) {
		*outcome = ErrOk;
	}

	m_lastError = ErrOk;
	return QString::fromUtf8(ba, ba.size());
}


QString Crypt::decryptV5( const QByteArray & cipher, ErrorCode * outcome ) const {
	QByteArray ba = cipher;
	CryptoFlags flags = CryptoFlags(ba.at(1));
	ba = ba.mid(2);
	int pos = 0;
	int n(ba.count());
	char lastChar = 0;
	int keyLength = m_key.length();

	while(pos < n) {
		char currentChar = ba[pos];
		ba[pos] = ba.at(pos) ^ lastChar ^ m_key.at(pos % keyLength);
		lastChar = currentChar;
		++pos;
	}

	ba = ba.mid(1); //chop off the random number at the start
	bool integrityOk(true);

	if(flags.testFlag(CryptoFlagChecksum)) {
		if(ba.length() < 2) {
			if(outcome) {
				*outcome = ErrIntegrityCheckFailed;
			}

			m_lastError = ErrIntegrityCheckFailed;
			return QByteArray();
		}

		quint16 storedChecksum;

		{
			QDataStream s(&ba, QIODevice::ReadOnly);
			s >> storedChecksum;
		}

		ba = ba.mid(2);
		quint16 checksum = qChecksum(ba.constData(), ba.length());
		integrityOk = (checksum == storedChecksum);
	}
	else if(flags.testFlag(CryptoFlagHash)) {
		if(ba.length() < 20) {
			if(outcome) {
				*outcome = ErrIntegrityCheckFailed;
			}

			m_lastError = ErrIntegrityCheckFailed;
			return QByteArray();
		}

		QByteArray storedHash = ba.left(20);
		ba = ba.mid(20);
		QCryptographicHash hash(QCryptographicHash::Sha1);
		hash.addData(ba);
		integrityOk = (hash.result() == storedHash);
	}

	if(!integrityOk) {
		if(outcome) {
			*outcome = ErrIntegrityCheckFailed;
		}

		m_lastError = ErrIntegrityCheckFailed;
		return QByteArray();
	}

	if(outcome) {
		*outcome = ErrOk;
	}

	m_lastError = ErrOk;
	return QString::fromUtf8(ba, ba.size());
}


}	// namespace Qonvince
