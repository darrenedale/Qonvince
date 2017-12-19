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
  */

#include "otplistwidgetitem.h"

#include <QListWidgetItem>

#include "otplistview.h"
#include "otp.h"


namespace Qonvince {


	OtpListWidgetItem::OtpListWidgetItem(OtpListWidget * parent)
	: OtpListWidgetItem(nullptr, parent) {
	}


	OtpListWidgetItem::OtpListWidgetItem(std::unique_ptr<Otp> && otp, OtpListWidget * parent)
	: QListWidgetItem(parent, QONVINCE_OTPLISTWIDGETITEM_TYPE),
	  m_otp(std::move(otp)) {
	}


	OtpListWidgetItem::~OtpListWidgetItem() = default;


	QString OtpListWidgetItem::text(void) const {
		if(!m_otp) {
			return {};
		}

		return m_otp->name();
	}


}  // namespace Qonvince
