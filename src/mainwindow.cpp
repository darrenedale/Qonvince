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
  * \todo
  * - modify tooltip if qr-code drag-and-drop is not enabled
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
#include "otpeditor.h"


using namespace Qonvince;


MainWindow::MainWindow( QWidget * parent ) :
	QMainWindow(parent),
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
		qonvinceApp->showMessage(tr("%1 message").arg(Application::applicationDisplayName()), tr("Drag and drop of QR code images is not available. You may need to install additional software to enable this."));
	}

	connect(&(qonvinceApp->settings()), SIGNAL(copyCodeOnClickChanged(bool)), this, SLOT(refreshTooltip()));
	refreshTooltip();
}


MainWindow::~MainWindow( void ) {
}


QString MainWindow::hoveredCode( void ) const {
	return m_ui->codes->hoveredCode();
}


Otp * MainWindow::hoveredCodeSpecification( void ) const {
	return m_ui->codes->hoveredCodeSpecification();
}


OtpListWidget * MainWindow::codeList( void ) const {
	return m_ui->codes;
}


void MainWindow::closeEvent( QCloseEvent * ev ) {
	ev->accept();
	Q_EMIT  closing();
}


void MainWindow::dragEnterEvent( QDragEnterEvent * ev ) {
	if(!m_imageDropEnabled) {
		return;
	}

	if(((Qt::CopyAction | Qt::MoveAction) & ev->proposedAction()) && ev->mimeData()->hasUrls()) {
		for(const QUrl & u : ev->mimeData()->urls()) {
			if("file" == u.scheme()) {
				ev->acceptProposedAction();
				return;
			}
		}
	}
}


void MainWindow::dropEvent( QDropEvent * ev ) {
	if(!m_imageDropEnabled) {
		return;
	}

//	QStringList invalidFiles;

	for(const QUrl & u : ev->mimeData()->urls()) {
		if("file" == u.scheme()) {
qDebug() << "decoding image" << u.path();
			qonvinceApp->readQrCode(u.path());
//			OtpQrCodeReader r(u.path());
//
//			if(r.decode()) {
//				ui->authCodeList->addCode(r.code());
//			}
//			else {
//				invalidFiles << u.path();
//			}
		}
	}

//	if(invalidFiles.count()) {
//		QMessageBox::critical(this, tr("%1 error").arg(Application::applicationName()), tr("The following files did not contain valid OTP authenticator specifications:\n\n%1").arg(invalidFiles.join("\n")));
//	}
}


void MainWindow::refreshTooltip( void ) {
    QString tt(tr("<html><body><p>Double-click an entry to edit its details.</p>"));

    if(qonvinceApp->settings().copyCodeOnClick()) {
        tt += "<p>Click an entry to copy its current code to the clipboard.</p>";
    }

    if(m_imageDropEnabled) {
        tt += "<p>Drop a QR code image on this window to decode it.</p>";
    }

    tt += "</body></html>";
    setToolTip(tt);
    m_ui->codes->setToolTip(tt);
}


void MainWindow::onAddCodeClicked( void ) {
	Otp * code = new Otp(Otp::TotpCode);
	m_ui->codes->addCode(code);
	onEditCodeRequested(code);
	/* TODO call onEditCodeRequested? */
//	OtpCodeEditor * w = new OtpCodeEditor(code);
//	connect(this, &MainWindow::closing, w, &QWidget::close);
//	connect(w, &OtpCodeEditor::closing, w, &QObject::deleteLater);
//	w->show();
}


void MainWindow::onSettingsClicked() {
	qonvinceApp->showSettingsWidget();
}


void MainWindow::onEditCodeRequested( Otp * code ) {
	Q_ASSERT(code);
	OtpEditor * w = new OtpEditor(code);
	connect(this, &MainWindow::closing, w, &QWidget::close);
	connect(w, &OtpEditor::closing, w, &QObject::deleteLater);
	w->show();
}


void MainWindow::onCodeClicked( Otp * code ) {
	Q_ASSERT(code);
	const Settings & settings = qonvinceApp->settings();

	if(settings.copyCodeOnClick()) {
		QApplication::clipboard()->setText(code->code());

		/* set a timer for the code on the clipboard to expire */
		if(settings.clearClipboardAfterInterval() && 0 < settings.clipboardClearInterval()) {
			QTimer::singleShot(1000 * settings.clipboardClearInterval(), qonvinceApp, &Application::clearClipboard);
		}

		if(settings.hideOnCodeCopyClick()) {
			/* show a notification so that it doesn't look like the application
			 * just closed */
			QString label;

			switch(settings.codeLabelDisplayStyle()) {
				default:
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

			qonvinceApp->showMessage(tr("The OTP code for <b>%1</b> was copied to the clipboard.").arg(label), 3000);
			hide();
		}
	}
}


void MainWindow::readSettings( const QSettings & settings ) {
	QPoint pos(settings.value("position").toPoint());
	QSize size(settings.value("size").toSize());

	if(!pos.isNull()) {
		move(pos);
	}

	if(!size.isNull()) {
		resize(size);
	}
}


void MainWindow::writeSettings( QSettings & settings ) const {
	settings.setValue("position", pos());
	settings.setValue("size", size());
}
