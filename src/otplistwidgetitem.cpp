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

/** \file otplistwidgetitem.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpListWidgetItem class.
  *
  * \todo
  */

#include "otplistwidgetitem.h"

#include "otplistwidgetitem.h"
#include "otplistwidget.h"
#include "otp.h"

using namespace Qonvince;


OtpListWidgetItem::OtpListWidgetItem( OtpListWidget * parent )
:	OtpListWidgetItem(nullptr, parent) {
}


OtpListWidgetItem::OtpListWidgetItem( Otp * code, OtpListWidget * parent )
:	QListWidgetItem(parent, QONVINCE_OTPLISTWIDGETITEM_TYPE),
	m_code(code) {
}


OtpListWidgetItem::~OtpListWidgetItem( void ) {
	delete m_code;
	m_code = nullptr;
}


QString OtpListWidgetItem::text( void ) const {
	if(!m_code) {
		return QString();
	}

	return m_code->name();
}
