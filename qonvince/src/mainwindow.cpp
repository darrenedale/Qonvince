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

/** \file mainwindow.cpp
  * \brief Implementation of the MainWindow class.
  *
  * \todo modify tooltip if qr-code drag-and-drop is not enabled
  */
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QClipboard>
#include <QShowEvent>
#include <QCloseEvent>
#include <QSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QMimeData>
#include <QUrl>
#include <QMessageBox>
#include <QStringBuilder>

#include "application.h"
#include "otplistwidgetitem.h"
#include "otp.h"
#include "otpqrcodereader.h"
#include "otpeditordialogue.h"


namespace Qonvince {


	MainWindow::MainWindow(QWidget * parent)
	: QMainWindow(parent),
	  m_ui{std::make_unique<Ui::MainWindow>()},
	  m_imageDropEnabled(OtpQrCodeReader::isAvailable()) {
		m_ui->setupUi(this);

		m_ui->codes->setCountdownWarningColour(QColor(160, 160, 92));
		m_ui->codes->setCountdownCriticalColour(QColor(220, 78, 92));

		m_ui->addCode->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/mainwindow/add")));
		m_ui->settings->setIcon(QIcon::fromTheme("configure-shortcuts", QIcon(":/icons/mainwindow/settings")));

		connect(m_ui->codes, &OtpListWidget::codeClicked, this, &MainWindow::onCodeClicked);
		connect(m_ui->codes, &OtpListWidget::codeDoubleClicked, this, &MainWindow::onCodeDoubleClicked);
		connect(m_ui->codes, &OtpListWidget::editCodeRequested, this, &MainWindow::onEditCodeRequested);

		if(m_imageDropEnabled) {
			setAcceptDrops(true);
		}
		else {
			qonvinceApp->showNotification(tr("%1 message").arg(Application::applicationDisplayName()), tr("Drag and drop of QR code images is not available. You may need to install additional software to enable this."));
		}

		connect(&(qonvinceApp->settings()), &Settings::copyCodeOnClickChanged, this, &MainWindow::refreshTooltip);
	}


	MainWindow::~MainWindow() = default;


	OtpListWidget * MainWindow::codeList() const {
		return m_ui->codes;
	}


	void MainWindow::closeEvent(QCloseEvent * ev) {
		ev->accept();
		Q_EMIT closing();
	}


	void MainWindow::dragEnterEvent(QDragEnterEvent * ev) {
		if(!m_imageDropEnabled) {
			return;
		}

		if(((Qt::CopyAction | Qt::MoveAction) & ev->proposedAction()) && ev->mimeData()->hasUrls()) {
			for(const auto & url : ev->mimeData()->urls()) {
				if(url.isLocalFile()) {
					ev->acceptProposedAction();
					return;
				}
			}
		}
	}


	void MainWindow::dropEvent(QDropEvent * ev) {
		if(!m_imageDropEnabled) {
			return;
		}

		// TODO consider reading remote URLs?
		for(const auto & url : ev->mimeData()->urls()) {
			if(url.isLocalFile()) {
				qonvinceApp->readQrCodeFrom(url.toLocalFile());
			}
		}
	}


	void MainWindow::refreshTooltip() {
		// looks a bit odd, but should make translation simpler - no HTML required
		QString tt = QStringLiteral("<html><body><p>%1</p>%2%3</body></html>").arg(tr("Double-click an entry to edit its details."), (qonvinceApp->settings().copyCodeOnClick() ? QStringLiteral("<p>%1</p>").arg(tr("Click an entry to copy its current code to the clipboard.")) : ""), (m_imageDropEnabled ? QStringLiteral("<p>%1</p>").arg(tr("Drop a QR code image on this window to decode it.")) : ""));

		setToolTip(tt);
		m_ui->codes->setToolTip(tt);
	}


	void MainWindow::onAddCodeClicked() {
		auto otp = new Otp(Otp::CodeType::Totp);
		onEditCodeRequested(otp);
		m_ui->codes->addCode(otp);
	}


	void MainWindow::onSettingsClicked() {
		qonvinceApp->showSettingsWidget();
	}


	void MainWindow::onEditCodeRequested(Otp * code) {
		Q_ASSERT_X(code, __PRETTY_FUNCTION__, "null code");
		auto * editor = new OtpEditorDialogue(code, this);
		editor->setAttribute(Qt::WA_DeleteOnClose, true);
		editor->show();
	}


	void MainWindow::onCodeClicked(Otp * code) {
		Q_ASSERT_X(code, __PRETTY_FUNCTION__, "null code");
		const Settings & settings = qonvinceApp->settings();

		if(settings.copyCodeOnClick()) {
			QApplication::clipboard()->setText(code->code());

			if(settings.clearClipboardAfterInterval() && 0 < settings.clipboardClearInterval()) {
				QTimer::singleShot(1000 * settings.clipboardClearInterval(), qonvinceApp, &Application::clearClipboard);
			}

			if(settings.hideOnCodeCopyClick()) {
				// notify so it doesn't look like the application closed
				QString label;

				switch(settings.codeLabelDisplayStyle()) {
					case Settings::IssuerAndName:
						label = code->issuer() % ": " % code->name();
						break;

					case Settings::IssuerOnly:
						label = code->issuer();
						break;

					case Settings::NameOnly:
						label = code->name();
						break;
				}

				qonvinceApp->showNotification(tr("The OTP code for <b>%1</b> was copied to the clipboard.").arg(label), 3000);
				hide();
			}
		}
	}


	void MainWindow::readSettings(const QSettings & settings) {
		QPoint pos(settings.value("position").toPoint());
		QSize size(settings.value("size").toSize());

		if(!pos.isNull()) {
			move(pos);
		}

		if(!size.isNull()) {
			resize(size);
		}
	}


	void MainWindow::writeSettings(QSettings & settings) const {
		settings.setValue("position", pos());
		settings.setValue("size", size());
	}


}  // namespace Qonvince
