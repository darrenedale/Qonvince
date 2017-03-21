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

/** \file passwordwidget.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the PasswordWidget class.
  *
  * \todo
  */

#include "src/passwordwidget.h"
#include "ui_passwordwidget.h"


using namespace Qonvince;


PasswordWidget::PasswordWidget( QWidget * parent )
:	QWidget(parent),
	m_ui(new Ui::PasswordWidget) {
	m_ui->setupUi(this);

	connect(m_ui->password, &QLineEdit::textEdited, this, &PasswordWidget::passwordChanged);
}


PasswordWidget::~PasswordWidget( void ) {
	delete m_ui;
}


void PasswordWidget::setPassword( const QString & pw ) {
	m_ui->password->setText(pw);
}
