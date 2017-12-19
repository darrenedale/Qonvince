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
#include <QNetworkAccessManager>

class QShowEvent;
class QCloseEvent;
class QDropEvent;
class QDragEnterEvent;
class QSettings;
class QNetworkReply;

namespace Qonvince {

	namespace Ui {
		class MainWindow;
	}

	class Otp;
	class OtpListView;
	class OtpListModel;
	class OtpListItemDelegate;

	class MainWindow
	: public QMainWindow {
		Q_OBJECT

	public:
		explicit MainWindow(QWidget * = nullptr);
		~MainWindow();

		OtpListView * otpList() const;

		void writeSettings(QSettings &) const;
		void readSettings(const QSettings &);

	Q_SIGNALS:
		void closing();

	protected:
		virtual void closeEvent(QCloseEvent *) override;
		virtual void dragEnterEvent(QDragEnterEvent *) override;
		virtual void dropEvent(QDropEvent *) override;

	private Q_SLOTS:
		void refreshTooltip();
		void onAddOtpClicked();
		void onSettingsClicked();

		inline void onOtpDoubleClicked(Otp * otp) {
			onEditOtpRequested(otp);
		}

		void onEditOtpRequested(Otp *);
		void onOtpClicked(Otp *);

		void onRemoteQrCodeImageDownloadFinished();

	private:
		std::unique_ptr<Ui::MainWindow> m_ui;
		std::unique_ptr<OtpListModel> m_model;
		std::unique_ptr<OtpListItemDelegate> m_delegate;
		QNetworkAccessManager m_netManager;
		bool m_imageDropEnabled;
	};

}  // namespace Qonvince

#endif  // QONVINCE_MAINWINDOW_H
