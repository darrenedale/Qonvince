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

/** \file otpeditordialogue.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the OtpEditorDialogue class.
  */
#include "otpeditordialogue.h"
#include "ui_otpeditordialogue.h"


namespace Qonvince {


	OtpEditorDialogue::OtpEditorDialogue(QWidget * parent)
	: OtpEditorDialogue(nullptr, parent) {
	}


	OtpEditorDialogue::OtpEditorDialogue(Otp * otp, QWidget * parent)
	: QDialog(parent),
	  m_ui(std::make_unique<Ui::OtpEditorDialogue>()) {
		m_ui->setupUi(this);
		m_ui->otpEditor->setOtp(otp);
		connect(m_ui->controls, &QDialogButtonBox::accepted, this, &QDialog::close);
		connect(m_ui->controls, &QDialogButtonBox::rejected, this, &QDialog::close);
	}


	OtpEditorDialogue::~OtpEditorDialogue() = default;


	OtpEditor * OtpEditorDialogue::editor() {
		return m_ui->otpEditor;
	}


	Otp * OtpEditorDialogue::otp() {
		return m_ui->otpEditor->otp();
	}


}  // namespace Qonvince
