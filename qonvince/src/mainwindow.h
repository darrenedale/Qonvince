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

#ifndef QONVINCE_MAINWINDOW_H
#define QONVINCE_MAINWINDOW_H

#include <memory>

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QString>

class QShowEvent;
class QCloseEvent;
class QDropEvent;
class QDragEnterEvent;
class QSettings;

namespace Qonvince {

	namespace Ui {
		class MainWindow;
	}

	class Otp;
	class OtpListWidget;

	class MainWindow
	: public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(QWidget * parent = nullptr);
		~MainWindow();

		OtpListWidget * codeList() const;

		void writeSettings(QSettings & settings) const;
		void readSettings(const QSettings & settings);

	Q_SIGNALS:
		void closing();

	protected:
		virtual void closeEvent(QCloseEvent * ev) override;
		virtual void dragEnterEvent(QDragEnterEvent * ev) override;
		virtual void dropEvent(QDropEvent * ev) override;

	private Q_SLOTS:
		void refreshTooltip();
		void onAddCodeClicked();
		void onSettingsClicked();

		inline void onCodeDoubleClicked(Otp * code) {
			onEditCodeRequested(code);
		}

		void onEditCodeRequested(Otp * code);
		void onCodeClicked(Otp * code);

	private:
		std::unique_ptr<Ui::MainWindow> m_ui;
		bool m_imageDropEnabled;
	};

}  // namespace Qonvince

#endif  // QONVINCE_MAINWINDOW_H