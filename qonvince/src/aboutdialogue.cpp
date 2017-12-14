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

#include "aboutdialogue.h"
#include "ui_aboutdialogue.h"

#include "application.h"


using namespace Qonvince;


AboutDialogue::AboutDialogue(QWidget * parent)
: QDialog(parent),
  m_ui(std::make_unique<Ui::AboutDialogue>()) {
	m_ui->setupUi(this);
	m_ui->aboutText->setText(m_ui->aboutText->text().arg(Application::applicationDisplayName()).arg(Application::applicationVersion()));
	adjustSize();
}


AboutDialogue::~AboutDialogue(void) = default;
