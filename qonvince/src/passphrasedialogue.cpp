
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
 * \brief Implementation of the PassphraseDialogue class.
 */
#include "src/passphrasedialogue.h"
#include "ui_passphrasedialogue.h"

namespace Qonvince
{
    PassphraseDialogue::PassphraseDialogue(QWidget * parent)
    : PassphraseDialogue({}, parent)
    {
    }

    PassphraseDialogue::PassphraseDialogue(const QString & msg, QWidget * parent)
    : QDialog(parent),
    m_ui{std::make_unique<Ui::PassphraseDialogue>()}
    {
        m_ui->setupUi(this);
        setMessage(msg);
        setMessageVisible(!msg.isEmpty());
        connect(m_ui->passphrase, &QLineEdit::textEdited, this, &PassphraseDialogue::passphraseChanged);
        adjustSize();
    }

    PassphraseDialogue::~PassphraseDialogue() = default;

    QString PassphraseDialogue::message() const
    {
        return m_ui->message->text();
    }

    void PassphraseDialogue::setMessage(const QString & msg)
    {
        m_ui->message->setText(msg);
    }

    QString PassphraseDialogue::passphrase() const
    {
        return m_ui->passphrase->text();
    }

    void PassphraseDialogue::setPassphrase(const QString & passphrase)
    {
        if (passphrase != m_ui->passphrase->text()) {
            m_ui->passphrase->setText(passphrase);
            Q_EMIT passphraseChanged(passphrase);
        }
    }

    void PassphraseDialogue::setMessageVisible(bool vis)
    {
        m_ui->message->setVisible(vis);
    }
}  // namespace Qonvince
