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

/** \file changepassphrase.cpp
  * \author Darren Edale
  * \date December 2022
  *
  * \brief Implementation of the ChangePassphraseDialogue class.
  */
#include "src/changepassphrasedialogue.h"
#include "ui_changepassphrasedialogue.h"

namespace Qonvince
{
    ChangePassphraseDialogue::ChangePassphraseDialogue(QWidget * parent)
            : ChangePassphraseDialogue({}, parent)
    {
    }

    ChangePassphraseDialogue::ChangePassphraseDialogue(const QString & msg, QWidget * parent)
            : QDialog(parent),
              m_ui{std::make_unique<Ui::ChangePassphraseDialogue>()}
    {
        m_ui->setupUi(this);
        m_ui->currentPassphraseWidget->setLabel(tr("Current passphrase"));
        m_ui->newPassphraseWidget->setLabel(tr("New passphrase"));
        setMessage(msg);
        setMessageVisible(!msg.isEmpty());
        connect(m_ui->newPassphraseWidget, &PasswordWidget::passwordChanged, this, &ChangePassphraseDialogue::newPassphraseChanged);
        connect(m_ui->currentPassphraseWidget, &PasswordWidget::passwordChanged, this, &ChangePassphraseDialogue::currentPassphraseChanged);
        adjustSize();
    }

    ChangePassphraseDialogue::~ChangePassphraseDialogue() = default;

    QString ChangePassphraseDialogue::message() const
    {
        return m_ui->message->text();
    }

    void ChangePassphraseDialogue::setMessage(const QString & msg)
    {
        m_ui->message->setText(msg);
    }

    QString ChangePassphraseDialogue::currentPassphrase() const
    {
        return m_ui->currentPassphraseWidget->password();
    }

    QString ChangePassphraseDialogue::newPassphrase() const
    {
        return m_ui->newPassphraseWidget->password();
    }

    void ChangePassphraseDialogue::setCurrentPassphrase(const QString & passphrase)
    {
        if (passphrase != m_ui->currentPassphraseWidget->password()) {
            m_ui->currentPassphraseWidget->setPassword(passphrase);
            Q_EMIT currentPassphraseChanged(passphrase);
        }
    }

    void ChangePassphraseDialogue::setNewPassphrase(const QString & passphrase)
    {
        if (passphrase != m_ui->newPassphraseWidget->password()) {
            m_ui->newPassphraseWidget->setPassword(passphrase);
            Q_EMIT newPassphraseChanged(passphrase);
        }
    }

    void ChangePassphraseDialogue::setMessageVisible(bool vis)
    {
        m_ui->message->setVisible(vis);
    }
}  // namespace Qonvince
