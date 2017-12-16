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

#ifndef QONVINCE_OTPQRCODEREADER_H
#define QONVINCE_OTPQRCODEREADER_H

#include "qrcodereader.h"
#include "otp.h"

namespace Qonvince {

	class OtpQrCodeReader
	: public QrCodeReader {
	public:
		OtpQrCodeReader(const QString & fileName, QObject * parent = nullptr);

		inline const Otp::CodeType & type() const {
			return m_type;
		}

		inline const QString & name() const {
			return m_name;
		}

		inline const QString & issuer() const {
			return m_issuer;
		}

		inline const QByteArray & seed() const {
			return m_seed;
		}

		inline int interval() const {
			return m_interval;
		}

		inline time_t baselineTime() const {
			return m_baselineTime;
		}

		inline int digits() const {
			return m_digits;
		}

		inline int counter() const {
			return m_counter;
		}

		bool decode();

		// caller is responsible for created code
		Otp * otp() const;

	private:
		Otp::CodeType m_type;
		QString m_name;
		QString m_issuer;
		QByteArray m_seed;
		int m_interval;
		int m_counter;
		int m_digits;
		time_t m_baselineTime;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPQRCODEREADER_H
