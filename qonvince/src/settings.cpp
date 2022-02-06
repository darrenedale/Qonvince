/*
 * Copyright 2015 - 2020 Darren Edale
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

/** \file settings.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the Settings class.
  */

#include "settings.h"

#include <iostream>

#include <QObject>
#include <QSettings>

#include "types.h"

namespace Qonvince
{
    static constexpr const int DefaultClipboardClearInterval = 30;
    static constexpr const int DefaultCodeRevealTimeout = 5;

    Settings::Settings(QObject * parent)
            : QObject(parent),
              m_codeLabelDisplayStyle(CodeLabelDisplayStyle::IssuerAndName),
              m_clipboardClearInterval(DefaultClipboardClearInterval),
              m_quitOnMainWindowClosed(true),
              m_startMinimised(false),
              m_copyCodeOnClick(false),
              m_hideOnCodeCopyClick(false),
              m_clearClipboardAfterInterval(false)
    {
    }

    void Settings::read(const QSettings & settings)
    {
        setSingleInstance(settings.value(QStringLiteral("single_instance"), false).toBool());
        setQuitOnMainWindowClosed(settings.value(QStringLiteral("quit_on_window_close"), true).toBool());
        setStartMinimised(settings.value(QStringLiteral("start_minimised"), false).toBool());
        setCopyCodeOnClick(settings.value(QStringLiteral("copy_code_on_click"), false).toBool());
        setHideOnCodeCopyClick(settings.value(QStringLiteral("hide_on_code_copy_click"), false).toBool());
        setClearClipboardAfterInterval(settings.value(QStringLiteral("clear_clipboard_after_interval"), false).toBool());

        bool ok;
        int i = settings.value(QStringLiteral("clipboard_clear_interval"), DefaultClipboardClearInterval).toInt(&ok);

        if (!ok) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"clear_clipboard_after_interval\" setting - using default\n";
        } else {
            setClipboardClearInterval(i);
        }

        i = settings.value(QStringLiteral("code_reveal_timeout"), DefaultCodeRevealTimeout).toInt(&ok);

        if (!ok) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: invalid \"code_reveal_timeout\" setting - using default\n";
        } else {
            setCodeRevealTimeout(i);
        }

        QString styleStr = settings.value(QStringLiteral("code_label_display_style"), QStringLiteral("IssuerAndName")).toString();

        if (QStringLiteral("IssuerAndName") == styleStr) {
            setCodeLabelDisplayStyle(CodeLabelDisplayStyle::IssuerAndName);
        } else if (QStringLiteral("NameOnly") == styleStr) {
            setCodeLabelDisplayStyle(CodeLabelDisplayStyle::NameOnly);
        } else if (QStringLiteral("IssuerOnly") == styleStr) {
            setCodeLabelDisplayStyle(CodeLabelDisplayStyle::IssuerOnly);
        } else {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << R"(]: (invalid or missing "code_label_display_style" setting (")" << qPrintable(styleStr)
                      << "\") - using default\n";
        }
    }

    void Settings::write(QSettings & settings) const
    {
        settings.setValue(QStringLiteral("single_instance"), singleInstance());
        settings.setValue(QStringLiteral("quit_on_window_close"), quitOnMainWindowClosed());
        settings.setValue(QStringLiteral("start_minimised"), startMinimised());
        settings.setValue(QStringLiteral("copy_code_on_click"), copyCodeOnClick());
        settings.setValue(QStringLiteral("hide_on_code_copy_click"), hideOnCodeCopyClick());
        settings.setValue(QStringLiteral("clear_clipboard_after_interval"), clearClipboardAfterInterval());
        settings.setValue(QStringLiteral("clipboard_clear_interval"), clipboardClearInterval());
        settings.setValue(QStringLiteral("code_reveal_timeout"), codeRevealTimeout());

        switch (codeLabelDisplayStyle()) {
            case CodeLabelDisplayStyle::IssuerAndName:
                settings.setValue(QStringLiteral("code_label_display_style"), QStringLiteral("IssuerAndName"));
                break;

            case CodeLabelDisplayStyle::NameOnly:
                settings.setValue(QStringLiteral("code_label_display_style"), QStringLiteral("NameOnly"));
                break;

            case CodeLabelDisplayStyle::IssuerOnly:
                settings.setValue(QStringLiteral("code_label_display_style"), QStringLiteral("IssuerOnly"));
                break;
        }
    }
}  // namespace Qonvince
