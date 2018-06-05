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

/// \file mainwindow.cpp
/// \brief Implementation of the MainWindow class.
///
/// \todo Don't include markup in notifications on Win10

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <algorithm>

#include <QString>
#include <QIcon>
#include <QPoint>
#include <QSize>
#include <QClipboard>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QCloseEvent>
#include <QSettings>
#include <QTimer>
#include <QMimeData>
#include <QUrl>
#include <QStringBuilder>
#include <QTemporaryFile>

#if defined(WITH_NETWORK_ACCESS)
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#endif

#include "application.h"
#include "otp.h"
#include "otplistview.h"
#include "otplistmodel.h"
#include "otplistitemdelegate.h"
#include "otpqrcodereader.h"
#include "otpeditordialogue.h"
#include "functions.h"


namespace Qonvince {


	MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent),
	  m_ui(std::make_unique<Ui::MainWindow>()) {
		m_ui->setupUi(this);

		if(OtpQrCodeReader::isAvailable()) {
			setAcceptDrops(true);
		}

		m_ui->addCode->setIcon(QIcon::fromTheme(QStringLiteral("list-add"), QIcon(QStringLiteral(":/icons/mainwindow/add"))));
		m_ui->settings->setIcon(QIcon::fromTheme(QStringLiteral("configure-shortcuts"), QIcon(QStringLiteral(":/icons/mainwindow/settings"))));

		connect(m_ui->addCode, &QPushButton::clicked, this, &MainWindow::onAddOtpClicked);
		connect(m_ui->settings, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
		connect(m_ui->otpList, &OtpListView::codeClicked, this, &MainWindow::onOtpClicked);
		connect(m_ui->otpList, &OtpListView::codeDoubleClicked, this, &MainWindow::onOtpDoubleClicked);
		connect(m_ui->otpList, &OtpListView::editCodeRequested, this, &MainWindow::onEditOtpRequested);

		connect(&(qonvinceApp->settings()), qOverload<bool>(&Settings::copyCodeOnClickChanged), this, &MainWindow::refreshTooltip);
	}


	MainWindow::~MainWindow() = default;


	void MainWindow::closeEvent(QCloseEvent * ev) {
		ev->accept();
		Q_EMIT closing();
	}


	void MainWindow::dragEnterEvent(QDragEnterEvent * ev) {
		if(!OtpQrCodeReader::isAvailable()) {
			return;
		}

		if(((Qt::CopyAction | Qt::MoveAction) & ev->proposedAction()) && ev->mimeData()->hasUrls()) {
			// prevent deep copy of QList
			const auto & urls = ev->mimeData()->urls();

			for(const auto & url : urls) {
				if(url.isLocalFile()) {
					ev->acceptProposedAction();
					return;
				}
			}
		}
	}


	void MainWindow::dropEvent(QDropEvent * ev) {
		if(!OtpQrCodeReader::isAvailable()) {
			return;
		}

		// prevent deep copy of QList
		const auto & urls = ev->mimeData()->urls();

		for(const auto & url : urls) {
			if(url.isLocalFile()) {
				qonvinceApp->readQrCodeFrom(url.toLocalFile());
			}
#if defined(WITH_NETWORK_ACCESS)
			else {
				auto * reply = m_netManager.get(QNetworkRequest(url));
				connect(reply, &QNetworkReply::finished, this, &MainWindow::onRemoteQrCodeImageDownloadFinished);
			}
#endif
		}
	}


	void MainWindow::refreshTooltip() {
		// looks a bit odd, but should make translation simpler - no HTML required
		QString tt = QStringLiteral("<html><body><p>%1</p>%2%3</body></html>").arg(tr("Double-click an entry to edit its details."), (qonvinceApp->settings().copyCodeOnClick() ? QStringLiteral("<p>%1</p>").arg(tr("Click an entry to copy its current code to the clipboard.")) : QStringLiteral("")), (OtpQrCodeReader::isAvailable() ? QStringLiteral("<p>%1</p>").arg(tr("Drop a QR code image on this window to decode it.")) : QStringLiteral("")));

		setToolTip(tt);
		m_ui->otpList->setToolTip(tt);
	}


	void MainWindow::onAddOtpClicked() {
		auto otp = std::make_unique<Otp>(OtpType::Totp);
		onEditOtpRequested(otp.get());
		qonvinceApp->addOtp(std::move(otp));
	}


	void MainWindow::onSettingsClicked() {
		qonvinceApp->showSettingsWidget();
	}


	void MainWindow::onEditOtpRequested(Otp * otp) {
		Q_ASSERT_X(otp, __PRETTY_FUNCTION__, "null OTP object");
		auto * editor = new OtpEditorDialogue(otp, this);
		editor->setAttribute(Qt::WA_DeleteOnClose, true);
		editor->show();
	}


	void MainWindow::onOtpClicked(Otp * otp) {
		Q_ASSERT_X(otp, __PRETTY_FUNCTION__, "null OTP object");
		const Settings & settings = qonvinceApp->settings();

		if(settings.copyCodeOnClick()) {
			QApplication::clipboard()->setText(otp->code());

			if(settings.clearClipboardAfterInterval() && 0 < settings.clipboardClearInterval()) {
				QTimer::singleShot(1000 * settings.clipboardClearInterval(), qonvinceApp, &Application::clearClipboard);
			}

			if(settings.hideOnCodeCopyClick()) {
				// notify so it doesn't look like the application closed
				qonvinceApp->showNotification(tr("The OTP code for <b>%1</b> was copied to the clipboard.").arg(otpLabel(otp)), 3000);
				hide();
			}
		}
	}


#if defined(WITH_NETWORK_ACCESS)
	void MainWindow::onRemoteQrCodeImageDownloadFinished() {
		auto * reply = qobject_cast<QNetworkReply *>(sender());
		Q_ASSERT_X(reply, __PRETTY_FUNCTION__, "sender is not a QNetworkReply object");
		QTemporaryFile imageFile;

		if(!imageFile.open()) {
			qonvinceApp->showNotification(tr("Error"), tr("A temporary file for the downloaded QR image could not be created."));
		}
		else if(reply->bytesAvailable() != imageFile.write(reply->readAll())) {
			qonvinceApp->showNotification(tr("Error"), tr("The downloaded QR image could not be saved to a temporary file."));
		}
		else {
			qonvinceApp->readQrCodeFrom(imageFile.fileName());
		}

		reply->deleteLater();
	}
#endif


	void MainWindow::readSettings(const QSettings & settings) {
		QPoint pos(settings.value(QStringLiteral("position")).toPoint());
		QSize size(settings.value(QStringLiteral("size")).toSize());

		if(!pos.isNull()) {
			move(pos);
		}

		if(!size.isNull()) {
			resize(size);
		}
	}


	void MainWindow::writeSettings(QSettings & settings) const {
		settings.setValue(QStringLiteral("position"), pos());
		settings.setValue(QStringLiteral("size"), size());
	}


}  // namespace Qonvince
