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

#ifndef QONVINCE_MAINWINDOW_H
#define QONVINCE_MAINWINDOW_H

#include <memory>

#include <QMainWindow>

class QShowEvent;
class QCloseEvent;
class QDropEvent;
class QDragEnterEvent;
class QSettings;

#if defined(WITH_NETWORK_ACCESS)
#include <QNetworkAccessManager>
class QNetworkReply;
#endif

namespace Qonvince {

	namespace Ui {
		class MainWindow;
	}

	class Otp;

	class MainWindow
	: public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(QWidget * = nullptr);
		~MainWindow();

		void writeSettings(QSettings &) const;
		void readSettings(const QSettings &);

	Q_SIGNALS:
		void closing();

	protected:
		void closeEvent(QCloseEvent *) override;
		void dragEnterEvent(QDragEnterEvent *) override;
		void dropEvent(QDropEvent *) override;

	private:
		void refreshTooltip();
		void createOtpEditor(Otp *);

	private:
		std::unique_ptr<Ui::MainWindow> m_ui;
#if defined(WITH_NETWORK_ACCESS)
	private:
		QNetworkAccessManager m_netManager;

	private Q_SLOTS:
		void onRemoteQrCodeImageDownloadFinished();
#endif
	};

}  // namespace Qonvince

#endif  // QONVINCE_MAINWINDOW_H
