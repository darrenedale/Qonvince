/*
 * Copyright 2015 - 2022 Darren Edale
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

/** \file passworddialogue.cpp
  * \author Darren Edale
  * \date December 2017
  *
  * \brief Implementation of the PasswordDialogue class.
  */
#include "src/passworddialogue.h"
#include "ui_passworddialogue.h"

namespace Qonvince
{
    PasswordDialogue::PasswordDialogue(QWidget * parent)
            : PasswordDialogue({}, parent)
    {
    }

    PasswordDialogue::PasswordDialogue(const QString & msg, QWidget * parent)
            : QDialog(parent),
              m_ui{std::make_unique<Ui::PasswordDialogue>()}
    {
        m_ui->setupUi(this);
        setMessage(msg);
        setMessageVisible(!msg.isEmpty());
        connect(m_ui->password, &QLineEdit::textEdited, this, &PasswordDialogue::passwordChanged);
        adjustSize();
    }

    PasswordDialogue::~PasswordDialogue() = default;

    QString PasswordDialogue::message() const
    {
        return m_ui->message->text();
    }

    void PasswordDialogue::setMessage(const QString & msg)
    {
        m_ui->message->setText(msg);
    }

    QString PasswordDialogue::password() const
    {
        return m_ui->password->text();
    }

    void PasswordDialogue::setPassword(const QString & password)
    {
        if (password != m_ui->password->text()) {
            m_ui->password->setText(password);
            Q_EMIT passwordChanged(password);
        }
    }

    void PasswordDialogue::setMessageVisible(bool vis)
    {
        m_ui->message->setVisible(vis);
    }
}  // namespace Qonvince
