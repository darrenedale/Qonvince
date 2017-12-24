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

#include "libqrencode.h"

#include <cstring>
#include <cerrno>

#include <QtGlobal>
#include <QDebug>


namespace Qonvince {


	LibQrEncode::LibQrEncode()
#if defined(Q_OS_UNIX)
	: m_lib("libqrencode.so") {
#elif defined(Q_OS_WIN)
	: m_lib("qrencode.dll") {
#else
#warning Unrecognised platform, LibQrEncode unlikely to be available
	: m_lib("libqrencode"){
#endif
	}


	LibQrEncode::QrCode LibQrEncode::encodeString(const QString & data) const {
		struct QRcodeData {
			int version;
			int width;
			unsigned char * data;
		};

		enum DummyEnum {
			Zero = 0,
			One,
			Two,
			Three,
		};

		using Symbol = LibQonvince::SharedLibrary::Symbol;
		using EncodeFunction = QRcodeData * (*) (const char * string, int version, DummyEnum level, DummyEnum hint, int casesensitive);
		using FreeFunction = void (*)(QRcodeData * code);

		static EncodeFunction QRcode_encodeString = nullptr;
		static FreeFunction QRcode_free = nullptr;

		QrCode ret = {false, 0, {}};

		if(nullptr == QRcode_encodeString) {
			Symbol sym;

			if(!m_lib.symbol("QRcode_encodeString", &sym)) {
				return ret;
			}

			QRcode_encodeString = reinterpret_cast<EncodeFunction>(sym);

			if(!m_lib.symbol("QRcode_free", &sym)) {
				QRcode_encodeString = nullptr;
				return ret;
			}

			QRcode_free = reinterpret_cast<FreeFunction>(sym);
		}

		// (data, version = 1, level = 0 (lowest), mode = 2 (8-bit mode), caseSensitive = 1 )
		QRcodeData * qr = QRcode_encodeString(data.toStdString().c_str(), 1, Zero, Two, 1);

		if(!qr) {
			return ret;
		}

		ret.isValid = true;
		ret.size = qr->width;
		ret.data.append(reinterpret_cast<const char *>(qr->data), qr->width * qr->width);
		QRcode_free(qr);
		return ret;
	}


}  // namespace Qonvince
