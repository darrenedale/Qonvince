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

/** \file qrcodecreator.cpp
  * \brief Implementation of the QrCodeCreator class.
  *
  * \todo integrate with libkf5prison (KDE barcode creator) where available?
  */

#include "qrcodecreator.h"

#include <QDebug>
#include <QPainter>
#include <QColor>


namespace Qonvince {


LibQrEncode QrCodeCreator::s_libQrEncode;


const QColor QrCodeCreator::DefaultForeground = QColor(0x00, 0x00, 0x00);
const QColor QrCodeCreator::DefaultBackground = QColor(0xff, 0xff, 0xff);


QrCodeCreator::QrCodeCreator( const QString & data )
:	m_data(data) {
}


QrCodeCreator::~QrCodeCreator() {
	s_libQrEncode.close();
}


QImage QrCodeCreator::image( const QSize & size ) {
	QImage img(size, QImage::Format_RGB32);
	QPainter painter(&img);
	paint(painter, size);
	return img;
}


void QrCodeCreator::paint(QPainter & painter, const QSize & size, const QColor & fg, const QColor & bg) {
	LibQrEncode::QrCode qr = s_libQrEncode.encodeString(m_data);

	if(qr.isValid){
		painter.setBrush(bg);
		painter.setPen(Qt::NoPen);
		painter.drawRect(0, 0, size.width(), size.height());
		painter.setBrush(fg);

		const int s = (0 < qr.size ? qr.size : 1);
		const double w = size.width();
		const double h = size.height();
		const double aspect = w / h;
		const double scale = ((1.0 < aspect) ? h : w) / s;

		for(int y = 0; y < s; ++y) {
			const int firstIndex = y * s;

			for(int x = 0; x < s; ++x) {
				if(qr.data.at(firstIndex + x) & 0x01) {
					painter.drawRect(QRectF (double(x * scale), double(y * scale), scale, scale));
				}
			}
		}
	}
	else{
		painter.setBrush(QColor(0xff, 0x00, 0x00));
		painter.drawRect(0, 0, size.width(), size.height());
	}
}


}
