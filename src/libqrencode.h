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

#ifndef QONVINCE_LIBQRENCODE_H
#define QONVINCE_LIBQRENCODE_H

#include "sharedlibrary.h"

#include <QByteArray>

class QString;

namespace Qonvince {
	/** \class LibQrEncode
	  * \brief Abstraction of libqrencode.
	  *
	  * This class abstracts away the unnecessary detail of libqrencode
	  * to provide a comfortable interface to use it for the simple needs
	  * of Qonvince. The library is loaded dynamically at runtime so it is
	  * not a build-time dependency (i.e. Qonvince will compile without
	  * libqrencode). To check whether the library is available, call
	  * isOpen() at runtime and act accordingly. If the library is
	  * available, a call to encodeString() will encode string data into
	  * a QrCode. The returned QrCode contains a QByteArray of data that
	  * is size * size bytes in length, and which represents the elements
	  * in the QrCode from left to right, in rows from top to bottom.
	  * (That is, byte 1 representes the top left element, byte 2 the element
	  * immediately to its right, and so on until the final byte represents
	  * the bottom right element in the QR code.) For each byte, if bit 0
	  * is set, the element is a 1 and should be rendered in the foregroud
	  * colour in the QR code image; if bit 0 is not set, the element is a
	  * 0 and should be rendered in the background colour in the QR code
	  * image.
	  *
	  * If isValid is set to false when creating a QrCode with encodeString,
	  * then the QR code data could not be created for some reason (most
	  * likely the library has not been loaded) and there will be no data
	  * to render to an image.
	  */
	class LibQrEncode
	:	public SharedLibrary {
		public:
			struct QrCode {
					bool isValid;
					int size;
					QByteArray data;
			};

			LibQrEncode( void );

			QrCode encodeString( const QString & data ) const;
	};
}

#endif // QONVINCE_LIBQRENCODE_H
