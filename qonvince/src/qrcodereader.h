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

#ifndef QONVINCE_QRCODEREADER_H
#define QONVINCE_QRCODEREADER_H

#include <QString>
#include <QByteArray>
#include <QObject>
#include <QImage>

//#include <zbar.h>

namespace Qonvince {
	class QrCodeReader : public QObject {
		Q_OBJECT

		public:
			QrCodeReader( const QString & fileName = QString(), QObject * parent = nullptr );

			static void staticInitialise( void );

			static inline bool isAvailable( void ) {
				staticInitialise();
				return s_isAvailable;
			}

			bool decode( void );

			bool isDecoded( void ) const {
				return m_isDecoded;
			}

			inline const QString & fileName( void ) const {
				return m_fileName;
			}

			void setFileName( const QString & fileName ) {
				m_fileName = fileName;
				m_decodedData.clear();
				m_isDecoded = false;
			}

			inline const QByteArray & decodedData( void ) const {
				return m_decodedData;
			}

		private:
//			class QZbarImage
//			:	public zbar::Image {
//				public:
//					explicit QZbarImage( const QString & fileName );
//					QZbarImage( const QImage & img );

//					bool isValid( void ) const {
//						return !m_qImg.isNull() && m_isValid;
//					}

//				private:
//					void initialise( void );

//					QImage m_qImg;
//					bool m_isValid;
//			};

			static bool s_isAvailable;
#if defined(__linux__)
			static QString s_zbarImgPath;
#endif
			bool m_isDecoded;
			QString m_fileName;
			QByteArray m_decodedData;
	};
}

#endif // QONVINCE_QRCODEREADER_H
