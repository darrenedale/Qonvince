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

#ifndef QONVINCE_SETTINGSWIDGET_H
#define QONVINCE_SETTINGSWIDGET_H

#include <memory>

#include <QWidget>

#include "types.h"
#include "settings.h"

namespace Qonvince {

	namespace Ui {
		class SettingsWidget;
	}

	class SettingsWidget
	: public QWidget {
		Q_OBJECT

	public:
		explicit SettingsWidget(Settings &, QWidget * = nullptr);
		~SettingsWidget() override;

		[[nodiscard]] bool singleInstance() const;
		[[nodiscard]] bool quitOnMainWindowClose() const;
		[[nodiscard]] bool startMinimised() const;
		[[nodiscard]] bool copyCodeOnClick() const;
		[[nodiscard]] bool hideOnCodeCopyClick() const;
		[[nodiscard]] bool clearClipboardAfterInterval() const;
		[[nodiscard]] int clipboardClearInterval() const;
		[[nodiscard]] CodeLabelDisplayStyle codeLabelDisplayStyle() const;
		[[nodiscard]] int codeRevealTimeout() const;

	public Q_SLOTS:
		void setSingleInstance(bool close);
		void setQuitOnMainWindowClose(bool close);
		void setStartMinimised(bool minimised);
		void setCopyCodeOnClick(bool copy);
		void setHideOnCodeCopyClick(bool hide);
		void setClearClipboardAfterInterval(bool clear);
		void setClipboardClearInterval(int interval);
		void setCodeLabelDisplayStyle(Qonvince::CodeLabelDisplayStyle style);
		void setCodeRevealTimeout(int timeout);

	private Q_SLOTS:
		void resyncWithSettings();
		void onDisplayStyleWidgetChanged();

	private:
		std::unique_ptr<Ui::SettingsWidget> m_ui;
		Settings & m_settings;
	};
}  // namespace Qonvince

#endif  // QONVINCE_SETTINGSWIDGET_H
