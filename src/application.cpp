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

/** \file application.cpp
  * \brief Implementation of the Application class.
  *
  * \todo
  * - load plugins
  * - Current system tray, application and default code icons are from iconfinder
  *   (via google). The iconfinder site is https-only but Chrome says its cert
  *   expired 8th April 2017 so I haven't yet been able to check its licence status.
  *   Don't release the app until this is resolved.
  */
#include "application.h"

#include <QDebug>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QClipboard>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QChar>

#include <cstring>

#include "crypt.h"
#include "mainwindow.h"
#include "passworddialogue.h"
#include "settingswidget.h"
#include "aboutdialogue.h"
#include "otplistwidget.h"
#include "otp.h"
#include "otpqrcodereader.h"
#include "otpdisplayplugin.h"
#include "integerotpdisplayplugin.h"
#include "steamotpdisplayplugin.h"


using namespace Qonvince;


/* just a random-ish string to identify the shared memory that is in place
 * if the application is already running */
#define QONVINCE_APPLICATION_RUNCHECK_KEY "blarglefangledungle"


Application * Application::s_instance = nullptr;


Application::Application( int & argc, char ** argv )
:	QApplication(argc, argv),
	m_runChecker{new SingleInstanceGuard{QONVINCE_APPLICATION_RUNCHECK_KEY}},
	m_mainWindow{nullptr},
	m_settingsWidget{nullptr},
	m_aboutDialogue{nullptr},
	m_trayIcon{nullptr},
	m_trayIconMenu{nullptr},
	m_quitAction{nullptr},
	m_loadQrImageAction{nullptr},
	m_mainWindowAction{nullptr},
	m_settingsAction{nullptr},
	m_qcaInit{} {
	Q_ASSERT(nullptr == s_instance);
	s_instance = this;

	setOrganizationName("Équit");
	setOrganizationDomain("equituk.net");
	setApplicationName("Qonvince");
	setApplicationDisplayName("Qonvince");
	setApplicationVersion("1.8.0");
	setQuitOnLastWindowClosed(false);
	QSettings::setDefaultFormat(QSettings::IniFormat);

	/* add the built-in code display plugins */
	OtpDisplayPlugin * plugin = new IntegerOtpDisplayPlugin(6);
	m_codeDisplayPlugins[plugin->pluginName()] = plugin;
	plugin = new IntegerOtpDisplayPlugin(8);
	m_codeDisplayPlugins[plugin->pluginName()] = plugin;
	plugin = new SteamOtpDisplayPlugin();
	m_codeDisplayPlugins[plugin->pluginName()] = plugin;

	m_trayIcon = new QSystemTrayIcon(QIcon::fromTheme("qonvince", QIcon(":/icons/systray")), this);
	m_trayIcon->setToolTip(tr("%1: One-Time passcode generator.").arg(applicationDisplayName()));
	m_trayIconMenu = new QMenu(tr("Qonvince"));

	m_quitAction = new QAction(QIcon::fromTheme("application-exit", QIcon(":/icons/app/quit")), tr("Quit Qonvince"), this);
	m_mainWindowAction = new QAction(tr("Show main window"), this);
	m_settingsAction = new QAction(QIcon::fromTheme("system-settings", QIcon(":/icons/app/settings")), tr("Settings..."), this);

	connect(m_quitAction, SIGNAL(triggered(bool)), this, SLOT(quit()));
	m_trayIconMenu->addAction(m_mainWindowAction);
	m_trayIconMenu->addAction(m_settingsAction);

	if(OtpQrCodeReader::isAvailable()) {
		m_loadQrImageAction = new QAction(QIcon::fromTheme("image-png", QIcon(":/icons/app/readqrcode")), tr("Read a QR code image"), this);
		connect(m_loadQrImageAction, SIGNAL(triggered(bool)), this, SLOT(readQrCode()));
		m_trayIconMenu->addSeparator();
		m_trayIconMenu->addAction(m_loadQrImageAction);
	}

	m_trayIconMenu->addSeparator();
	m_trayIconMenu->addAction(tr("About %1").arg(applicationDisplayName()), this, &Application::aboutQonvince);
	m_trayIconMenu->addAction(tr("About Qt"), this, &Application::aboutQt);
	m_trayIconMenu->addAction(m_quitAction);

	m_trayIcon->setContextMenu(m_trayIconMenu);
	m_mainWindow = new MainWindow();

	readApplicationSettings();
	onSettingsChanged();

	m_settingsWidget = new SettingsWidget(m_settings);

	connect(m_trayIcon, &QSystemTrayIcon::activated, this, &Application::onTrayIconActivated);
	connect(m_mainWindowAction, SIGNAL(triggered(bool)), m_mainWindow, SLOT(show()));
	connect(m_settingsAction, SIGNAL(triggered(bool)), this, SLOT(showSettingsWidget()));
	connect(&m_settings, &Settings::changed, this, &Application::onSettingsChanged);
	connect(this, &Application::aboutToQuit, this, &Application::writeSettings);
	connect(m_mainWindow->codeList(), &OtpListWidget::codeAdded, this, &Application::codeAdded);
}


Application::~Application( void ) {
	/* it's safe to delete nullptr */
	delete m_runChecker;
	delete m_trayIcon;
	delete m_trayIconMenu;
	delete m_mainWindow;
	delete m_settingsWidget;
	delete m_aboutDialogue;
	m_runChecker = nullptr;
	m_trayIcon = nullptr;
	m_trayIconMenu = nullptr;
	m_mainWindow = nullptr;
	m_settingsWidget = nullptr;
	m_aboutDialogue = nullptr;

	for(auto plugin : m_codeDisplayPlugins) {
		delete plugin;
	}

	m_codeDisplayPlugins.clear();
}


Application::DesktopEnvironment Application::desktopEnvironment( void ) {
	static DesktopEnvironment ret = DesktopEnvironment::Unknown;
	static bool done(false);

	if(!done) {
#if defined(Q_OS_UNIX)
		QByteArray wm(qgetenv("XDG_CURRENT_DESKTOP"));

		if("Unity" == wm) {
			ret = DesktopEnvironment::Unity;
		}
		else if("XFCE" == wm) {
			ret = DesktopEnvironment::Xfce;
		}
		else if("KDE" == wm) {
			ret = DesktopEnvironment::Kde;
		}
		else if("LXDE" == wm) {
			ret = DesktopEnvironment::Lxde;
		}
		else if("" == wm) {
			wm = qgetenv("GDMSESSION");

			if("kde-plasma" == wm) {
				ret = DesktopEnvironment::Kde;
			}
		}
#elif defined(Q_OS_WIN32)
		ret = DesktopEnvironment::WindowsDesktop;
#elif defined(Q_OS_ANDROID)
		ret = DesktopEnvironment::Android;
#elif defined(Q_OS_IOS)
		ret = DesktopEnvironment::Ios;
#elif defined(Q_OS_MAC)
		ret = DesktopEnvironment::MacOSx;
#elif defined(Q_OS_WINPHONE)
		ret = DesktopEnvironment::WindowsPhone;
#endif
	}

	return ret;
}


bool Application::ensureDirectory( const QStandardPaths::StandardLocation & location, const QString & path ) {
	static QVector<QChar> s_validChars = {'-', '_', ' ', '.', '~'};

	QString rootPath = QStandardPaths::writableLocation(location);

	if(rootPath.isEmpty()) {
		return false;
	}

	/* check that the path is not malicious */
	if(rootPath != QDir(rootPath % "/" % path).absolutePath().left(rootPath.length())) {
		/* the path contains some ".." components that move it out of the
		 * root data directory - this is not allowed */
		qWarning() << path << "is not a valid subdirectory";
		return false;
	}

	/* ensure the path is sane */
	if(path.isEmpty() || path.trimmed().isEmpty()) {
		qWarning() << "path" << path << "is enmpty or contains only whitespace";
	}

	for(QChar c : path) {
		for(QChar v : s_validChars) {
			if(c == v) {
				continue;
			}
		}

		if(c.isLetterOrNumber()) {
			continue;
		}

		qWarning() << "path" << path << "contains the invalid character" << c;
		return false;
	}

	/* mkpath returns true if the path already exists */
	return QDir(rootPath).mkpath(path);
}


int Application::exec( void ) {
	Application * app(qonvinceApp);

	if(app->m_settings.singleInstance()) {
		if(!app->m_runChecker->tryToRun()) {
			qDebug() << "Qonvince is already running.";
			return 0;
		}
	}

	/* read command-line args */
	bool forceStartMinimised(false);

	for(const QString & arg : arguments()) {
		if(arg == "-m" || arg == "--minimised") {
			forceStartMinimised = true;
		}
	}

	{
		PasswordDialogue dlg(tr("Enter the passphrase used to encrypt your settings. This ensures that your OTP seeds cannot be stolen from your settings file."));

		while(true) {
			if(QDialog::Accepted != dlg.exec()) {
				/* if the user refuses to enter his/her passphrase, exit the app */
				return 0;
			}

			QString pw(dlg.password());

			if(pw.isEmpty()) {
				qWarning() << "passphrase is empty" << pw;
			}
			else if(8 > pw.length()) {
				qWarning() << "passphrase is too short" << pw;
			}
			else {
				app->m_cryptPassphrase = pw;

				if(app->readCodeSettings()) {
					/* if reading succeeds, passphrase was correct, so
					 * exit loop and continue app execution */
					break;
				}

				app->m_cryptPassphrase = QString();
				qWarning() << "failed to read code settings - likely incorrect passphrase";
				/* sleep for a short period before allowing another attempt */
			}

			/* we can only get here if the passphrase was not correct */
			QThread::msleep(1000);
			dlg.setMessage(tr("The passphrase you entered is not correct. Enter the passphrase used to encrypt your settings."));
		}
	}

	app->onSettingsChanged();
	app->m_trayIcon->show();

	if(!forceStartMinimised && !app->m_settings.startMinimised()) {
		app->m_mainWindow->show();
	}
//	else if(QX11Info::isPlatformX11()){
//		/* haven't mapped a main window, so manually inform the system that
//		 * we're up and running */
//	}

	return QApplication::exec();
}


void Application::showMessage( const QString & title, const QString & message, int timeout ) {
	if(!QSystemTrayIcon::supportsMessages()) {
		QMessageBox::information(m_mainWindow, title, message, QMessageBox::StandardButtons(QMessageBox::Ok));
	}
	else {
		m_trayIcon->showMessage(title, message, QSystemTrayIcon::Information, timeout);
	}
}


void Application::showMessage( const QString & message, int timeout ) {
	if(!QSystemTrayIcon::supportsMessages()) {
		QMessageBox::information(m_mainWindow, tr("%1 message").arg(applicationName()), message, QMessageBox::StandardButtons(QMessageBox::Ok));
	}
	else {
		m_trayIcon->showMessage(tr("%1 message").arg(applicationName()), message, QSystemTrayIcon::Information, timeout);
	}
}


void Application::readQrCode( void ) {
	QString fileName = QFileDialog::getOpenFileName(m_mainWindow, tr("Open QR code image"));

	if(fileName.isEmpty()) {
		return;
	}

	readQrCode(fileName);
}


void Application::readQrCode( const QString & fileName ) {
	OtpQrCodeReader r(fileName);

	if(!r.decode()) {
		showMessage(tr("The file <strong>%1</strong> could not be read as a QR code."));
		return;
	}

	m_mainWindow->codeList()->addCode(r.code());
}


bool Application::readApplicationSettings( void ) {
	QSettings settings;

	if(!!m_mainWindow) {
		settings.beginGroup("mainwindow");
		m_mainWindow->readSettings(settings);
		settings.endGroup();

		m_mainWindow->codeList()->clear();
		settings.beginGroup("application");
		m_settings.read(settings);
	}

	return true;
}


bool Application::readCodeSettings( void ) {
	QSettings settings;

	/* read the crypt_check value. if decrypt() indicates an error, the passphrase
	 * is wrong; if it indicates success the passphrase is either right or is wrong
	 * but hits the right checksum by chance */
	if(settings.contains("crypt_check")) {
		/* temporarily support reading files with old crypto as fallback for
		 * transitional purposes */
		{
			QCA::SecureArray value{QCA::hexToArray(settings.value("crypt_check").toString())};
			QCA::SymmetricKey key{m_cryptPassphrase.toUtf8()};
			QCA::InitializationVector initVec{value.toByteArray().left(16)};
			QCA::Cipher cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode, key, initVec);

			if(cipher.process(value.toByteArray().mid(16)).isEmpty()) {
				Crypt c(m_cryptPassphrase.toUtf8());
				Crypt::ErrorCode err;
				c.decrypt(settings.value("crypt_check").toString(), &err);

				switch(err) {
					case Crypt::ErrOk:
						break;

					default:
						qDebug() << "unkown error code from Crypt::decrypt()";
						return false;

					case Crypt::ErrKeyTooShort:				/*!< The key provided contained fewer than 8 bytes */
						qDebug() << "decryption failure - key is too short";
						return false;

					case Crypt::ErrNoKeySet:				/*!< No key was set. You can not encrypt or decrypt without a valid key. */
						qDebug() << "decryption failure - no passphrase";
						return false;

					case Crypt::ErrUnknownVersion:			/*!< The version of this data is unknown, or the data is otherwise not valid. */
						qDebug() << "decryption failure - unrecognised algorithm version";
						return false;

					case Crypt::ErrUuidMismatch:			/*!< The UUID in the encrypted data does not match the UUID of the machine decrypting it */
						/* this should never happen - the UUID is not in use */
						qDebug() << "decryption failure - UUID mismatch";
						return false;

					case Crypt::ErrIntegrityCheckFailed:	/*!< The integrity check of the data failed. Perhaps the wrong key was used. */
						qDebug() << "decryption integrity check failure - probably incorrect passphrase";
						return false;
				}
			}
		}
	}

	if(!!m_mainWindow) {
		m_mainWindow->codeList()->clear();

		settings.beginGroup("codes");
		int n = settings.value("code_count", 0).toInt();

		for(int i = 0; i < n; ++i) {
			settings.beginGroup(QString("code-%1").arg(i));
			Otp * code = Otp::fromSettings(settings, m_cryptPassphrase);

			if(!!code) {
				m_mainWindow->codeList()->addCode(code);
				connect(code, &Otp::changed, this, &Application::writeSettings);
			}
			else {
				qWarning() << "failed to read code" << i;
			}

			settings.endGroup();
		}
	}

	return true;
}


void Application::writeSettings( void ) const {
	QSettings settings;

	/* write a random string to the settings which, when read, will indicate
	 * whether the crypt key is correct */
	{
		/* we use the length of the passphrase so that entry of a truncated
		 * passphrase can never pass this check */
		int l = (2 * m_cryptPassphrase.toUtf8().length()) + (qrand() % 20);
		QByteArray random(l, 0);

		while(0 < l) {
			l--;
			random[l] = 'a' + (qrand() % 26);
		}

		QCA::SymmetricKey key{m_cryptPassphrase.toUtf8()};
		QCA::InitializationVector initVec(16);
		QCA::Cipher cipher("aes128", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode, key, initVec);
		settings.setValue("crypt_check", QCA::arrayToHex(initVec.toByteArray() + cipher.process(random).toByteArray()));
	}

	if(!!m_mainWindow) {
		OtpListWidget * list = m_mainWindow->codeList();
		int n = list->count();

		settings.beginGroup("application");
		m_settings.write(settings);
		settings.endGroup();

		settings.beginGroup("mainwindow");
		m_mainWindow->writeSettings(settings);
		settings.endGroup();

		settings.beginGroup("codes");
		settings.setValue("code_count", n);

		for(int i = 0; i < n; ++i) {
			Otp * code = list->code(i);

			if(!code) {
				qWarning() << "code #" << i << "is null!";
				continue;
			}

			settings.beginGroup(QString("code-%1").arg(i));
			code->writeSettings(settings, m_cryptPassphrase);
			settings.endGroup();
		}

		settings.endGroup();
	}
}


void Application::onTrayIconActivated( QSystemTrayIcon::ActivationReason reason ) {
	if(QSystemTrayIcon::Trigger == reason) {
		if(m_mainWindow->isHidden() || !m_mainWindow->isActiveWindow()) {
			m_mainWindow->show();
			m_mainWindow->raise();
			m_mainWindow->activateWindow();
		}
		else {
			m_mainWindow->hide();
		}
	}
}


void Application::onSettingsChanged( void ) {
	disconnect(m_quitConnection);

	if(m_settings.quitOnMainWindowClosed()) {
		m_quitConnection = connect(m_mainWindow, &MainWindow::closing, this, &QApplication::quit);
	}
}


void Application::updateSystemTrayIconTooltip() {
	/* query DE and decide on that basis whether to show HTML/plain tooltip */
	bool doHtml = false;

	if(DesktopEnvironment::Kde == qonvinceApp->desktopEnvironment()) {
		doHtml = true;
	}

	if(m_trayIcon) {
		QStringList msgItems;
		int n = m_mainWindow->codeList()->count();

		for(int i = 0; i < n; ++i) {
			Otp * code = m_mainWindow->codeList()->code(i);

			if(!code) {
				continue;
			}

			if(doHtml) {
				msgItems << tr("%1:%2 <b>%3</b>").arg(code->issuer()).arg(code->name()).arg(code->code());
			}
			else {
				msgItems << tr("%1:%2\t%3").arg(code->issuer()).arg(code->name()).arg(code->code());
			}
		}

		if(doHtml) {
			/* TODO work out how to set the "left-click/right-click" text to be smaller */
			m_trayIcon->setToolTip(tr("%1<br />\n<br />\n(Left-click for main window, right-click for menu)").arg(msgItems.join("<br />\n")));
		}
		else {
			m_trayIcon->setToolTip(tr("%1\n\n(Left-click for main window, right-click for menu)").arg(msgItems.join("\n")));
		}
	}
}


void Application::codeDestroyed( QObject * obj ) {
	Otp * code = qobject_cast<Otp *>(obj);

	if(!!code) {
		code->disconnect(this);
	}
}


void Application::codeAdded( Otp * code ) {
	connect(code, &QObject::destroyed, this, &Application::codeDestroyed);
}


void Application::aboutQonvince( void ) {
	if(!m_aboutDialogue) {
		m_aboutDialogue = new AboutDialogue();
	}

	m_aboutDialogue->show();
}


void Application::clearClipboard() {
	clipboard()->clear();
}


void Application::showSettingsWidget( void ) {
	m_settingsWidget->show();
	m_settingsWidget->activateWindow();
	m_settingsWidget->raise();
}


/* SingleInstanceGuard is a mostly unmodified copy of the RunGuard code from
 * http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
 */
Application::SingleInstanceGuard::SingleInstanceGuard( const QString & key )
:	m_key(key),
	m_memLockKey(QCryptographicHash::hash(key.toUtf8().append("_memLockKey"), QCryptographicHash::Sha1).toHex()),
	m_sharedmemKey(QCryptographicHash::hash(key.toUtf8().append("_sharedmemKey"), QCryptographicHash::Sha1).toHex()),
	m_sharedMem(m_sharedmemKey),
	m_memLock(m_memLockKey, 1) {
	m_memLock.acquire();

	{
		QSharedMemory fix(m_sharedmemKey);    // Fix for *nix: http://habrahabr.ru/post/173281/
		fix.attach();
	}

	m_memLock.release();
}


Application::SingleInstanceGuard::~SingleInstanceGuard( void ) {
	release();
}


bool Application::SingleInstanceGuard::isAnotherRunning( void ) {
	if(m_sharedMem.isAttached()) {
		return false;
	}

	m_memLock.acquire();
	const bool isRunning = m_sharedMem.attach();

	if(isRunning) {
		m_sharedMem.detach();
	}

	m_memLock.release();
	return isRunning;
}


bool Application::SingleInstanceGuard::tryToRun( void ) {
	if(isAnotherRunning()) {   // Extra check
		return false;
	}

	m_memLock.acquire();
	const bool result = m_sharedMem.create(sizeof(quint64));
	m_memLock.release();

	if(!result) {
		release();
		return false;
	}

	return true;
}


void Application::SingleInstanceGuard::release( void ) {
	m_memLock.acquire();

	if(m_sharedMem.isAttached()) {
		m_sharedMem.detach();
	}

	m_memLock.release();
}
