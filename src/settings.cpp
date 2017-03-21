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

/** \file settings.cpp
  * \author Darren Edale
  * \date November 2016
  *
  * \brief Implementation of the Settings class.
  *
  * \todo
  */

#include "settings.h"

#include <QObject>
#include <QSettings>
#include <QDebug>


#define DEFAULT_CLIPBOARD_CLEAR_INTERVAL 30
#define DEFAULT_CODE_REVEAL_TIMEOUT 5


using namespace Qonvince;


Settings::Settings( QObject * parent )
:	QObject(parent),
	m_quitOnMainWindowClosed(true),
	m_startMinimised(false),
	m_copyCodeOnClick(false),
	m_hideOnCodeCopyClick(false),
	m_clearClipboardAfterInterval(false),
	m_clipboardClearInterval(DEFAULT_CLIPBOARD_CLEAR_INTERVAL),
	m_codeLabelDisplayStyle(IssuerAndName) {
}


void Settings::read(const QSettings & settings ) {
	setSingleInstance(settings.value("single_instance", false).toBool());
	setQuitOnMainWindowClosed(settings.value("quit_on_window_close", true).toBool());
	setStartMinimised(settings.value("start_minimised", false).toBool());
	setCopyCodeOnClick(settings.value("copy_code_on_click", false).toBool());
	setHideOnCodeCopyClick(settings.value("hide_on_code_copy_click", false).toBool());
	setClearClipboardAfterInterval(settings.value("clear_clipboard_after_interval", false).toBool());

	bool ok;
	int i = settings.value("clipboard_clear_interval", DEFAULT_CLIPBOARD_CLEAR_INTERVAL).toInt(&ok);

	if(!ok) {
        qWarning() << "invalid \"clear_clipboard_after_interval\" setting - using default";
	}
	else {
		setClipboardClearInterval(i);
	}

	i = settings.value("code_reveal_timeout", DEFAULT_CODE_REVEAL_TIMEOUT).toInt(&ok);

	if(!ok) {
        qWarning() << "invalid \"code_reveal_timeout\" setting - using default";
	}
	else {
		setCodeRevealTimeout(i);
	}

	QString s(settings.value("code_label_display_style", "IssuerAndName").toString());

	if("IssuerAndName" == s) {
		setCodeLabelDisplayStyle(IssuerAndName);
	}
	else if("NameOnly" == s) {
		setCodeLabelDisplayStyle(NameOnly);
	}
	else if("IssuerOnly" == s) {
		setCodeLabelDisplayStyle(IssuerOnly);
	}
	else {
        qWarning() << "invalid or missing \"code_label_display_style\" setting (" << s << ") - using default";
	}
}


void Settings::write( QSettings & settings ) const {
	settings.setValue("single_instance", singleInstance());
	settings.setValue("quit_on_window_close", quitOnMainWindowClosed());
	settings.setValue("start_minimised", startMinimised());
	settings.setValue("copy_code_on_click", copyCodeOnClick());
	settings.setValue("hide_on_code_copy_click", hideOnCodeCopyClick());
	settings.setValue("clear_clipboard_after_interval", clearClipboardAfterInterval());
	settings.setValue("clipboard_clear_interval", clipboardClearInterval());
	settings.setValue("code_reveal_timeout", codeRevealTimeout());

	switch(codeLabelDisplayStyle()) {
		case IssuerAndName:
			settings.setValue("code_label_display_style", "IssuerAndName");
			break;

		case NameOnly:
			settings.setValue("code_label_display_style", "NameOnly");
			break;

		case IssuerOnly:
			settings.setValue("code_label_display_style", "IssuerOnly");
			break;
	}
}
