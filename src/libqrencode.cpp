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


extern "C" {
	typedef struct {
		int version;         ///< version of the symbol
		int width;           ///< width of the symbol
		unsigned char *data; ///< symbol data
	} QRcode;

	typedef enum {
		Zero = 0,
		One,
		Two,
		Three,
	} DummyEnum;

	typedef QRcode * (* EncodeFunction) ( const char * string, int version, DummyEnum level, DummyEnum hint, int casesensitive );
	typedef void (* FreeFunction) ( QRcode * code );
}


using namespace Qonvince;


#if defined(Q_OS_UNIX)
#define QONVINCE_LIBQRENCODE_LIBNAME "libqrencode.so"
#elif defined(Q_OS_WIN)
#define QONVINCE_LIBQRENCODE_LIBNAME "qrencode.dll"
#else
#warning Unrecognised platform, LibQrEncode unlikely to be available
#define QONVINCE_LIBQRENCODE_LIBNAME "libqrencode"
#endif


LibQrEncode::LibQrEncode( void )
:	SharedLibrary(QONVINCE_LIBQRENCODE_LIBNAME) {
}


LibQrEncode::QrCode LibQrEncode::encodeString( const QString & data ) const {
	static EncodeFunction QRcode_encodeString = nullptr;
	static FreeFunction QRcode_free = nullptr;

	QrCode ret = { false, 0, QByteArray() };

	if(nullptr == QRcode_encodeString) {
		Symbol sym;

		if(!symbol("QRcode_encodeString", &sym)) {
			return ret;
		}

		QRcode_encodeString = (EncodeFunction) sym;

		if(!symbol("QRcode_free", &sym)) {
			QRcode_encodeString = nullptr;
			return ret;
		}

		QRcode_free = (FreeFunction) sym;
	}

	/* (data, version = 1, level = 0 (lowest), mode = 2 (8-bit mode), caseSensitive = 1 ) */
	QRcode * qr = QRcode_encodeString(data.toStdString().c_str(), 1, Zero, Two, 1);

	if(!qr) {
		return ret;
	}

	ret.isValid = true;
	ret.size = qr->width;
	ret.data.append((const char *) qr->data, qr->width * qr->width);
	QRcode_free(qr);
	return ret;
}
