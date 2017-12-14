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
  * \todo load plugins
  */
#include "application.h"

#include <array>

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


#define QONVINCE_APPLICATION_SINGLEINSTANCE_KEY ""


namespace Qonvince {


	Application * Application::s_instance = nullptr;


	Application::Application(int & argc, char ** argv)
	: QApplication(argc, argv),
	  // random-ish string identifying the shared memory that is in place
	  // if application is already running
	  m_runChecker(std::make_unique<SingleInstanceGuard>("blarglefangledungle")),
	  m_settings(),
	  m_mainWindow(nullptr),
	  m_settingsWidget(std::make_unique<SettingsWidget>(m_settings)),
	  m_aboutDialogue(nullptr),
	  m_trayIcon(QIcon::fromTheme("qonvince", QIcon(":/icons/systray"))),
	  m_trayIconMenu(tr("Qonvince")),
	  m_qcaInit() {
		Q_ASSERT_X(nullptr == s_instance, __PRETTY_FUNCTION__, "can't instantiate more than one Qonvince::Application object");
		s_instance = this;

		setOrganizationName("Équit");
		setOrganizationDomain("equituk.net");
		setApplicationName("Qonvince");
		setApplicationDisplayName("Qonvince");
		setApplicationVersion("1.8.0");
		setQuitOnLastWindowClosed(false);
		QSettings::setDefaultFormat(QSettings::IniFormat);

		// add the built-in code display plugins
		std::shared_ptr<OtpDisplayPlugin> plugin = std::make_shared<IntegerOtpDisplayPlugin>(6);
		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});
		plugin = std::make_shared<IntegerOtpDisplayPlugin>(8);
		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});
		plugin = std::make_shared<SteamOtpDisplayPlugin>();
		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});

		m_mainWindow = std::make_unique<MainWindow>();
		//	m_trayIcon = new QSystemTrayIcon(QIcon::fromTheme("qonvince", QIcon(":/icons/systray")), this);
		m_trayIcon.setToolTip(tr("%1: One-Time passcode generator.").arg(applicationDisplayName()));

		m_trayIconMenu.addAction(tr("Show main window"), m_mainWindow.get(), &MainWindow::show);
		m_trayIconMenu.addAction(QIcon::fromTheme("system-settings", QIcon(":/icons/app/settings")), tr("Settings..."), this, &Application::showSettingsWidget);

		if(OtpQrCodeReader::isAvailable()) {
			m_trayIconMenu.addSeparator();
			m_trayIconMenu.addAction(QIcon::fromTheme("image-png", QIcon(":/icons/app/readqrcode")), tr("Read a QR code image"), this, &Application::readQrCode);
		}

		m_trayIconMenu.addSeparator();
		m_trayIconMenu.addAction(tr("About %1").arg(applicationDisplayName()), this, &Application::aboutQonvince);
		//		m_trayIconMenu.addAction(tr("About Qt"), this, &Application::aboutQt);
		m_trayIconMenu.addAction(QIcon::fromTheme("application-exit", QIcon(":/icons/app/quit")), tr("Quit Qonvince"), this, &Application::quit);

		m_trayIcon.setContextMenu(&m_trayIconMenu);

		readApplicationSettings();
		onSettingsChanged();

		//	m_settingsWidget = std::make_unique<SettingsWidget>(m_settings);

		connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &Application::onTrayIconActivated);
		connect(&m_settings, &Settings::changed, this, &Application::onSettingsChanged);
		connect(this, &Application::aboutToQuit, this, &Application::writeSettings);
		connect(m_mainWindow->codeList(), &OtpListWidget::codeAdded, this, &Application::onCodeAdded);
	}


	Application::~Application() = default;


	Application::DesktopEnvironment Application::desktopEnvironment() {
		static DesktopEnvironment ret = DesktopEnvironment::Unknown;
		static bool done = false;

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


	PluginPtr Application::codeDisplayPluginByName(const QString & name) const {
		const auto it = m_codeDisplayPlugins.find(name);

		if(it != m_codeDisplayPlugins.cend()) {
			return it->second;
		}

		return nullptr;
	}


	PluginArray Application::codeDisplayPlugins() const {
		PluginArray ret;
		ret.reserve(m_codeDisplayPlugins.size());

		for(const auto & item : m_codeDisplayPlugins) {
			ret.push_back(item.second);
		}

		return ret;
	}


	bool Application::ensureDirectory(const QStandardPaths::StandardLocation & location, const QString & path) {
		QString rootPath = QStandardPaths::writableLocation(location);

		if(rootPath.isEmpty()) {
			return false;
		}

		// check that the path is not malicious
		if(rootPath != QDir(rootPath % "/" % path).absolutePath().left(rootPath.length())) {
			// path contains ".." components that move it out of root data directory
			qWarning() << path << "is not a valid subdirectory";
			return false;
		}

		if(path.isEmpty() || path.trimmed().isEmpty()) {
			qWarning() << "path" << path << "is enmpty or contains only whitespace";
		}

		auto isValidPathChar = [](const QChar & ch) -> bool {
			static std::array<QChar, 6> s_validChars = {{'-', '_', ' ', '.', '~', '/'}};
			return ch.isLetterOrNumber() || (s_validChars.cend() != std::find(s_validChars.cbegin(), s_validChars.cend(), ch));
		};

		const auto end = path.cend();
		const auto invalidChar = std::find_if_not(path.cbegin(), end, isValidPathChar);

		if(end != invalidChar) {
			qWarning() << "path" << path << "contains the invalid character" << *invalidChar;
			return false;
		}

		return QDir(rootPath).mkpath(path);
	}


	int Application::exec() {
		Application * app(qonvinceApp);

		if(app->m_settings.singleInstance()) {
			if(!app->m_runChecker->tryToRun()) {
				qDebug() << "Qonvince is already running.";
				return 0;
			}
		}

		/* read command-line args */
		bool forceStartMinimised(false);

		// TODO arguments() is slow - find another way
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

				QString pw = dlg.password();

				if(pw.isEmpty()) {
					qWarning() << "passphrase is empty" << pw;
				}
				else if(8 > pw.length()) {
					qWarning() << "passphrase is too short" << pw;
				}
				else {
					app->m_cryptPassphrase = pw;

					if(app->readCodeSettings()) {
						// if reading succeeds, passphrase was correct, so
						// exit loop and continue app execution
						break;
					}

					app->m_cryptPassphrase = QString();
					qWarning() << "failed to read code settings - likely incorrect passphrase";
				}

				// we can only get here if the passphrase was not correct
				QThread::msleep(1000);
				dlg.setMessage(tr("The passphrase you entered is not correct. Enter the passphrase used to encrypt your settings."));
			}
		}

		app->onSettingsChanged();
		app->m_trayIcon.show();

		if(!forceStartMinimised && !app->m_settings.startMinimised()) {
			app->m_mainWindow->show();
		}
		//	else if(QX11Info::isPlatformX11()){
		//		/* haven't mapped a main window, so manually inform the system that
		//		 * we're up and running */
		//	}

		return QApplication::exec();
	}


	void Application::showMessage(const QString & title, const QString & message, int timeout) {
		if(!QSystemTrayIcon::supportsMessages()) {
			QMessageBox::information(m_mainWindow.get(), title, message, QMessageBox::StandardButtons(QMessageBox::Ok));
		}
		else {
			m_trayIcon.showMessage(title, message, QSystemTrayIcon::Information, timeout);
		}
	}


	void Application::showMessage(const QString & message, int timeout) {
		if(!QSystemTrayIcon::supportsMessages()) {
			QMessageBox::information(m_mainWindow.get(), tr("%1 message").arg(applicationName()), message, QMessageBox::StandardButtons(QMessageBox::Ok));
		}
		else {
			m_trayIcon.showMessage(tr("%1 message").arg(applicationName()), message, QSystemTrayIcon::Information, timeout);
		}
	}


	void Application::readQrCode() {
		QString fileName = QFileDialog::getOpenFileName(m_mainWindow.get(), tr("Open QR code image"));

		if(fileName.isEmpty()) {
			return;
		}

		readQrCodeFrom(fileName);
	}


	void Application::readQrCodeFrom(const QString & fileName) {
		OtpQrCodeReader r(fileName);

		if(!r.decode()) {
			showMessage(tr("The file <strong>%1</strong> could not be read as a QR code."));
			return;
		}

		m_mainWindow->codeList()->addCode(r.code());
	}


	bool Application::readApplicationSettings() {
		QSettings settings;

		settings.beginGroup("mainwindow");
		m_mainWindow->readSettings(settings);
		settings.endGroup();

		m_mainWindow->codeList()->clear();
		settings.beginGroup("application");
		m_settings.read(settings);

		return true;
	}


	bool Application::readCodeSettings() {
		QSettings settings;

		// read the crypt_check value. if decryption indicates an error, the passphrase
		// is wrong; if it indicates success the passphrase is right. this determines
		// whether to continue reading the file or not. if the passphrase not correct,
		// it won't successfully decrypt the seeds and subsequently when writing the
		// codes back they will be encrypted with the erroneous passphrase and will
		// effectively become inaccessible. if the passphrase is correct, or someone
		// manages to trick this check, the passphrase must still correctly decrypt the
		// seeds. in other words, bypassing this check does not grant access to seeds
		if(settings.contains("crypt_check")) {
			QCA::SecureArray value = QCA::hexToArray(settings.value("crypt_check").toString());
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode, QCA::SymmetricKey{m_cryptPassphrase.toUtf8()}, QCA::InitializationVector{value.toByteArray().left(16)});
			cipher.process(value.toByteArray().mid(16));

			if(!cipher.ok()) {
				qDebug() << "decryption failure - incorrect passphrase";
			}

			//					Crypt c(m_cryptPassphrase.toUtf8());
			//					Crypt::ErrorCode err;
			//					c.decrypt(settings.value("crypt_check").toString(), &err);

			//					switch(err) {
			//						case Crypt::ErrOk:
			//							break;

			//						case Crypt::ErrKeyTooShort: /*!< The key provided contained fewer than 8 bytes */
			//							qDebug() << "decryption failure - key is too short";
			//							return false;

			//						case Crypt::ErrNoKeySet: /*!< No key was set. You can not encrypt or decrypt without a valid key. */
			//							qDebug() << "decryption failure - no passphrase";
			//							return false;

			//						case Crypt::ErrUnknownVersion: /*!< The version of this data is unknown, or the data is otherwise not valid. */
			//							qDebug() << "decryption failure - unrecognised algorithm version";
			//							return false;

			//						case Crypt::ErrUuidMismatch: /*!< The UUID in the encrypted data does not match the UUID of the machine decrypting it */
			//							/* this should never happen - the UUID is not in use */
			//							qDebug() << "decryption failure - UUID mismatch";
			//							return false;

			//						case Crypt::ErrIntegrityCheckFailed: /*!< The integrity check of the data failed. Perhaps the wrong key was used. */
			//							qDebug() << "decryption integrity check failure - probably incorrect passphrase";
			//							return false;
			//					}
			//				}
		}

		m_mainWindow->codeList()->clear();

		settings.beginGroup("codes");
		int n = settings.value("code_count", 0).toInt();

		for(int i = 0; i < n; ++i) {
			settings.beginGroup(QString("code-%1").arg(i));
			Otp * code = Otp::fromSettings(settings, m_cryptPassphrase);

			if(code) {
				m_mainWindow->codeList()->addCode(code);
				connect(code, &Otp::changed, this, &Application::writeSettings);
			}
			else {
				qWarning() << "failed to read code" << i;
			}

			settings.endGroup();
		}

		return true;
	}


	void Application::writeSettings() const {
		QSettings settings;

		// this "random" string in the settings will, when read, indicate whether the crypt key is correct
		{
			// use length of passphrase so that a truncated passphrase can never pass the check
			int l = (2 * m_cryptPassphrase.toUtf8().length()) + (qrand() % 20);
			QByteArray random(l, 0);

			while(0 < l) {
				l--;
				random[l] = 'a' + (qrand() % 26);
			}

			QCA::SymmetricKey key{m_cryptPassphrase.toUtf8()};
			QCA::InitializationVector initVec(16);
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode, key, initVec);
			settings.setValue("crypt_check", QCA::arrayToHex(initVec.toByteArray() + cipher.process(random).toByteArray()));
		}

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


	void Application::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
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


	void Application::onSettingsChanged() {
		disconnect(m_quitOnMainWindowClosedConnection);

		if(m_settings.quitOnMainWindowClosed()) {
			m_quitOnMainWindowClosedConnection = connect(m_mainWindow.get(), &MainWindow::closing, this, &QApplication::quit);
		}
		else {
			disconnect(m_quitOnMainWindowClosedConnection);
		}
	}


	void Application::onCodeDestroyed(QObject * obj) {
		Otp * code = qobject_cast<Otp *>(obj);

		if(code) {
			code->disconnect(this);
		}
	}


	void Application::onCodeAdded(Otp * code) {
		connect(code, &QObject::destroyed, this, &Application::onCodeDestroyed);
	}


	void Application::aboutQonvince() {
		if(!m_aboutDialogue) {
			m_aboutDialogue = std::make_unique<AboutDialogue>();
		}

		m_aboutDialogue->show();
	}


	void Application::clearClipboard() {
		clipboard()->clear();
	}


	void Application::showSettingsWidget() {
		m_settingsWidget->show();
		m_settingsWidget->activateWindow();
		m_settingsWidget->raise();
	}


	// SingleInstanceGuard is a mostly unmodified copy of the RunGuard code from
	// http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
	Application::SingleInstanceGuard::SingleInstanceGuard(const QString & key)
	: m_key(key),
	  m_memLockKey(QCryptographicHash::hash(key.toUtf8().append("_memLockKey"), QCryptographicHash::Sha1).toHex()),
	  m_sharedmemKey(QCryptographicHash::hash(key.toUtf8().append("_sharedmemKey"), QCryptographicHash::Sha1).toHex()),
	  m_sharedMem(m_sharedmemKey),
	  m_memLock(m_memLockKey, 1) {
		m_memLock.acquire();

		{
			QSharedMemory fix(m_sharedmemKey);  // Fix for *nix: http://habrahabr.ru/post/173281/
			fix.attach();
		}

		m_memLock.release();
	}


	Application::SingleInstanceGuard::~SingleInstanceGuard() {
		release();
	}


	bool Application::SingleInstanceGuard::isAnotherRunning() {
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


	bool Application::SingleInstanceGuard::tryToRun() {
		if(isAnotherRunning()) {  // Extra check
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


	void Application::SingleInstanceGuard::release() {
		m_memLock.acquire();

		if(m_sharedMem.isAttached()) {
			m_sharedMem.detach();
		}

		m_memLock.release();
	}


}  // namespace Qonvince
