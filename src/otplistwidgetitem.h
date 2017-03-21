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

#ifndef QONVINCE_OTPLISTWIDGETITEM_H
#define QONVINCE_OTPLISTWIDGETITEM_H

#include <QObject>
#include <QListWidgetItem>

#define QONVINCE_OTPLISTWIDGETITEM_TYPE QListWidgetItem::UserType

namespace Qonvince {
	class OtpListWidget;
	class Otp;

	class OtpListWidgetItem
	:	public QListWidgetItem {
		public:
			explicit OtpListWidgetItem( OtpListWidget * parent = nullptr );
			OtpListWidgetItem( Otp * code, OtpListWidget * parent = nullptr );
			virtual ~OtpListWidgetItem( void );

			inline Otp * code( void ) const {
				return m_code;
			}

			virtual QString text( void ) const;

		private:
			Otp * m_code;
	};
}

#endif // QONVINCE_OTPLISTWIDGETITEM_H
