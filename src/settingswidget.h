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

#ifndef QONVINCE_SETTINGSWIDGET_H
#define QONVINCE_SETTINGSWIDGET_H

#include <QWidget>

#include "settings.h"

namespace Qonvince {

	namespace Ui {
		class SettingsWidget;
	}

	class SettingsWidget
	: public QWidget {
			Q_OBJECT

		public:
			explicit SettingsWidget( Settings & settings, QWidget * parent = nullptr );
			~SettingsWidget( void );

			bool singleInstance( void ) const;
			bool quitOnMainWindowClose( void ) const;
			bool startMinimised( void ) const;
			bool copyCodeOnClick( void ) const;
			bool hideOnCodeCopyClick( void ) const;
			bool clearClipboardAfterInterval( void ) const;
			int clipboardClearInterval( void ) const;
			Settings::CodeLabelDisplayStyle codeLabelDisplayStyle( void ) const;
			int codeRevealTimeout( void ) const;

		public Q_SLOTS:
			void setSingleInstance( bool close );
			void setQuitOnMainWindowClose( bool close );
			void setStartMinimised( bool minimised );
			void setCopyCodeOnClick( bool copy );
			void setHideOnCodeCopyClick( bool hide );
			void setClearClipboardAfterInterval( bool clear );
			void setClipboardClearInterval( int interval );
			void setCodeLabelDisplayStyle( Settings::CodeLabelDisplayStyle style );
			void setCodeRevealTimeout( int timeout );

		private Q_SLOTS:
			void resyncWithSettings( void );
			void onDisplayStyleWidgetChanged( void );

		private:

			Ui::SettingsWidget * m_ui;
			Settings & m_settings;
	};
}

#endif // QONVINCE_SETTINGSWIDGET_H
