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

/** \file qrcodereader.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the QrCodeReader class.
  */

#include "qrcodereader.h"

#include <QProcess>


/* TODO use libzbar instead of zbarimg
 *
 * from qr-tools source:
	 def decode(self, filename=None):
		self.filename = filename or self.filename
		if self.filename:
			scanner = zbar.ImageScanner()
			# configure the reader
			scanner.parse_config('enable')
			# obtain image data
			pil = Image.open(self.filename).convert('L')
			width, height = pil.size
			raw = pil.tostring()
			# wrap image data
			image = zbar.Image(width, height, 'Y800', raw)
			# scan the image for barcodes
			result = scanner.scan(image)
			# extract results
			if result == 0:
				return False
			else:
				for symbol in image:
					pass
				# clean up
				del(image)
				#Assuming data is encoded in utf8
				self.data = symbol.data.decode(u'utf-8')
				self.data_type = self.data_recognise()
				return True

 */


namespace Qonvince {


	bool QrCodeReader::s_isAvailable = false;


#if defined(__linux__)
	QString QrCodeReader::s_zbarImgPath;
#endif


	QrCodeReader::QrCodeReader(const QString & fileName, QObject * parent)
	: QObject(parent),
	  m_isDecoded(false),
	  m_fileName(fileName) {
		staticInitialise();
	}


	void QrCodeReader::staticInitialise() {
		static bool s_done = false;

		if(!s_done) {
#if defined(__linux__)
			QProcess readerProcess;
			readerProcess.setProgram(QStringLiteral("/usr/bin/which"));
			readerProcess.setArguments({QStringLiteral("zbarimg")});
			readerProcess.start();
			readerProcess.waitForFinished();
			QByteArray out = readerProcess.readAllStandardOutput();
			s_zbarImgPath = QString::fromUtf8(out.trimmed());
			s_isAvailable = !out.isEmpty();
#endif
			s_done = true;
		}
	}


	bool QrCodeReader::decode() {
		if(!isAvailable()) {
			return false;
		}

		//	QZbarImage img(fileName());
		//
		//	if(!img.isValid()) {
		//		qDebug() << "invalid QZbarImage object";
		//		return false;
		//	}
		//
		//	zbar::ImageScanner scanner;
		//	int res = scanner.scan(img);
		//
		//	if(0 == res) {
		//		qDebug() << "failed to scan image";
		////		return false;
		//	}
		//	else {
		//		zbar::SymbolSet symbols = img.get_symbols();
		//qDebug() << "found" << symbols.get_size() << "symbols";
		//		zbar::SymbolIterator it(symbols.symbol_begin());
		//
		//		while(it != symbols.symbol_end()) {
		//			qDebug() << QString::fromStdString(it->get_type_name()) << QString::fromStdString(it->get_data());
		//			++it;
		//		}
		//	}

#if defined(__linux__)
		if(!m_isDecoded) {
			QProcess decoderProcess;
			decoderProcess.start(s_zbarImgPath, {"-q", "--raw", m_fileName});

			/* give the decoder 1s to finish */
			if(decoderProcess.waitForFinished(1000) && 0 == decoderProcess.exitCode()) {
				m_decodedData.clear();
				m_decodedData.append(decoderProcess.readAllStandardOutput());
				/* trim trailing linefeed */
				m_decodedData.chop(1);
				m_isDecoded = true;
			}
			else {
				decoderProcess.terminate();
			}
		}
#endif

		return m_isDecoded;
	}


	//QrCodeReader::QZbarImage::QZbarImage( const QString & fileName )
	//:	zbar::Image(),
	//	m_qImg(fileName),
	//	m_isValid(false) {
	//	initialise();
	//}


	//QrCodeReader::QZbarImage::QZbarImage( const QImage & image )
	//:	zbar::Image(),
	//	m_qImg(image),
	//	m_isValid(false) {
	//	initialise();
	//}


	//void QrCodeReader::QZbarImage::initialise() {
	//	QImage::Format fmt = m_qImg.format();

	//qDebug() << m_qImg;

	//	if(fmt != QImage::Format_RGB32 && fmt != QImage::Format_ARGB32 && fmt != QImage::Format_ARGB32_Premultiplied) {
	//		qDebug() << "incompatible image format";
	//		m_isValid = false;
	//		return;
	//	}

	//	unsigned bpl = m_qImg.bytesPerLine();
	//	unsigned width = bpl / 4;
	//	unsigned height = m_qImg.height();
	//	unsigned long length = m_qImg.byteCount();

	//	set_size(width, height);
	//	set_format('B' | ('G' << 8) | ('R' << 16) | ('4' << 24));
	//	set_data(m_qImg.bits(), length);

	//	if((width * 4 != bpl) || (width * height * 4 > length)) {
	//		qDebug() << "incompatible image dimensions";
	//		m_isValid = false;
	//		return;
	//	}

	//	m_isValid = true;
	//}


}  // namespace Qonvince
