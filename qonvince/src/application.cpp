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
  * \todo consider whether Application should manage a list of OTPs and
  * this list should be used as a model for list widget
  */
#include "application.h"

#include <array>
#include <iostream>

#include <QDebug>
#include <QString>
#include <QStringBuilder>
#include <QChar>
#include <QThread>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "mainwindow.h"
#include "passworddialogue.h"
#include "settingswidget.h"
#include "aboutdialogue.h"
#include "otplistwidget.h"
#include "otp.h"
#include "otpqrcodereader.h"
#include "pluginfactory.h"


namespace Qonvince {


	namespace Detail {
		// Based on RunGuard code from
		// http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
		class SingleInstanceGuard final {
		public:
			explicit SingleInstanceGuard(const QString & key)
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

			~SingleInstanceGuard() {
				release();
			}

			SingleInstanceGuard(const SingleInstanceGuard &) = delete;
			SingleInstanceGuard(SingleInstanceGuard &&) = delete;
			void operator=(const SingleInstanceGuard &) = delete;
			void operator=(SingleInstanceGuard &&) = delete;

			bool isAnotherRunning() {
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

			bool tryToRun() {
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

			void release() {
				m_memLock.acquire();

				if(m_sharedMem.isAttached()) {
					m_sharedMem.detach();
				}

				m_memLock.release();
			}

		private:
			const QString m_key;
			const QString m_memLockKey;
			const QString m_sharedmemKey;

			QSharedMemory m_sharedMem;
			QSystemSemaphore m_memLock;
		};


		static SingleInstanceGuard runChecker(QStringLiteral("blarglefangledungle"));
		static QCA::Initializer qcaInitializer;
	}  // namespace Detail


	Application::Application(int & argc, char ** argv)
	: QApplication(argc, argv),
	  // random-ish string identifying the shared memory that is in place
	  // if application is already running
	  m_settings(),
	  //	  m_mainWindow(nullptr),
	  m_trayIcon(QIcon::fromTheme("qonvince", QIcon(":/icons/systray"))),
	  m_trayIconMenu(tr("Qonvince")),
	  m_displayPluginFactory(".displayplugin") {
		setOrganizationName("Équit");
		setOrganizationDomain("equituk.net");
		setApplicationName("Qonvince");
		setApplicationDisplayName("Qonvince");
		setApplicationVersion("1.8.0");
		setQuitOnLastWindowClosed(false);
		QSettings::setDefaultFormat(QSettings::IniFormat);

		// set the search paths for the display plugin factory
		for(const auto & pathRoot : QStandardPaths::standardLocations(QStandardPaths::AppDataLocation)) {
			m_displayPluginFactory.addSearchPath(pathRoot + "/plugins/otpdisplay");
		}

#ifndef NDEBUG
		// search in the build dir
		m_displayPluginFactory.addSearchPath("../plugins/otpdisplay");
#endif

		m_displayPluginFactory.loadAllPlugins();
		// add the built-in code display plugins
		//		std::shared_ptr<OtpDisplayPlugin> plugin = std::make_shared<IntegerOtpDisplayPlugin>(6);
		//		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});
		//		plugin = std::make_shared<IntegerOtpDisplayPlugin>(8);
		//		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});
		//		plugin = std::make_shared<SteamOtpDisplayPlugin>();
		//		m_codeDisplayPlugins.insert({plugin->pluginName(), plugin});

		//		m_mainWindow = std::make_unique<MainWindow>();
		m_trayIcon.setToolTip(tr("%1: One-Time passcode generator.").arg(applicationDisplayName()));

		m_trayIconMenu.addAction(tr("Show main window"), &m_mainWindow, &MainWindow::show);
		m_trayIconMenu.addAction(QIcon::fromTheme("system-settings", QIcon(":/icons/app/settings")), tr("Settings..."), this, &Application::showSettingsWidget);

		if(OtpQrCodeReader::isAvailable()) {
			m_trayIconMenu.addSeparator();
			m_trayIconMenu.addAction(QIcon::fromTheme("image-png", QIcon(":/icons/app/readqrcode")), tr("Read a QR code image"), this, &Application::readQrCode);
		}

		m_trayIconMenu.addSeparator();
		m_trayIconMenu.addAction(tr("About %1").arg(applicationDisplayName()), this, &Application::showAboutDialogue);
		m_trayIconMenu.addAction(QIcon::fromTheme("application-exit", QIcon(":/icons/app/quit")), tr("Quit Qonvince"), this, &Application::quit);

		m_trayIcon.setContextMenu(&m_trayIconMenu);

		readApplicationSettings();
		onSettingsChanged();

		connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &Application::onTrayIconActivated);
		connect(&m_settings, &Settings::changed, this, &Application::onSettingsChanged);
		connect(this, &Application::aboutToQuit, this, &Application::writeSettings);
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


	int Application::addOtp(std::unique_ptr<Otp> && otp) {
		if(!otp || contains(m_otpList, otp)) {
			return -1;
		}

		int index = static_cast<int>(m_otpList.size());
		m_otpList.push_back(std::move(otp));
		Q_EMIT otpAdded(otp.get());
		Q_EMIT otpAdded(index, otp.get());
		return index;
	}


	bool Application::removeOtp(int index) {
		if(0 > index || m_otpList.size() <= static_cast<std::size_t>(index)) {
			return false;
		}

		return removeOtp(m_otpList[static_cast<std::size_t>(index)].get());
	}


	bool Application::removeOtp(Otp * otp) {
		auto begin = m_otpList.begin();
		auto end = m_otpList.end();
		auto otpIter = std::find_if(begin, end, [&otp](const auto & listOtp) {
			return listOtp.get() == otp;
		});

		if(otpIter == end) {
			return false;
		}

		int index = static_cast<int>(std::distance(begin, otpIter));
		auto myOtp = std::move(*otpIter);
		m_otpList.erase(otpIter);
		Q_EMIT otpRemoved(otp);
		Q_EMIT otpRemoved(index);
		return true;
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
			if(!Detail::runChecker.tryToRun()) {
				std::cerr << "Qonvince is already running.\n";
				return 0;
			}
		}

		bool forceStartMinimised = false;
		auto args = arguments();
		auto argsEnd = args.cend();

		if(argsEnd != std::find_if(args.cbegin(), argsEnd, [](const auto & arg) {
				return "-m" == arg || "--minimised" == arg;
			})) {
			forceStartMinimised = true;
		}

		{
			PasswordDialogue dlg(tr("Enter the passphrase used to encrypt your settings."));

			while(true) {
				if(QDialog::Accepted != dlg.exec()) {
					/* if the user refuses to enter his/her passphrase, exit the app */
					return 0;
				}

				QCA::SecureArray pw = dlg.password().toUtf8();

				if(pw.isEmpty()) {
					qWarning() << "passphrase is empty";
				}
				else if(8 > pw.size()) {
					qWarning() << "passphrase is too short";
				}
				else {
					qonvinceApp->m_cryptPassphrase = pw;

					if(app->readCodeSettings()) {
						// if reading succeeds, passphrase was correct, so
						// exit loop and continue app execution
						break;
					}

					qonvinceApp->m_cryptPassphrase.clear();
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
			app->m_mainWindow.show();
		}
		//	else if(QX11Info::isPlatformX11()){
		//		/* haven't mapped a main window, so manually inform the system that
		//		 * we're up and running */
		//	}

		return QApplication::exec();
	}


	void Application::showNotification(const QString & title, const QString & message, int timeout) {
		if(!QSystemTrayIcon::supportsMessages()) {
			QMessageBox::information(&m_mainWindow, title, message, QMessageBox::StandardButtons(QMessageBox::Ok));
		}
		else {
			m_trayIcon.showMessage(title, message, QSystemTrayIcon::Information, timeout);
		}
	}


	void Application::showNotification(const QString & message, int timeout) {
		if(!QSystemTrayIcon::supportsMessages()) {
			QMessageBox::information(&m_mainWindow, tr("%1 message").arg(applicationName()), message, QMessageBox::StandardButtons(QMessageBox::Ok));
		}
		else {
			m_trayIcon.showMessage(tr("%1 message").arg(applicationName()), message, QSystemTrayIcon::Information, timeout);
		}
	}


	void Application::readQrCode() {
		QString fileName = QFileDialog::getOpenFileName(&m_mainWindow, tr("Open QR code image"));

		if(fileName.isEmpty()) {
			return;
		}

		readQrCodeFrom(fileName);
	}


	void Application::readQrCodeFrom(const QString & fileName) {
		OtpQrCodeReader reader(fileName);

		if(!reader.decode()) {
			showNotification(tr("The file <strong>%1</strong> could not be read as a QR code."));
			return;
		}

		m_mainWindow.otpList()->addOtp(std::unique_ptr<Otp>(reader.createOtp()));
	}


	bool Application::readApplicationSettings() {
		QSettings settings;

		settings.beginGroup("mainwindow");
		m_mainWindow.readSettings(settings);
		settings.endGroup();

		m_mainWindow.otpList()->clear();
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
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode, QCA::SymmetricKey(m_cryptPassphrase), QCA::InitializationVector(value.toByteArray().left(16)));
			cipher.process(value.toByteArray().mid(16));

			if(!cipher.ok()) {
				qDebug() << "decryption failure - incorrect passphrase";
				return false;
			}
		}

		m_mainWindow.otpList()->clear();

		settings.beginGroup("codes");
		int n = settings.value("code_count", 0).toInt();

		for(int i = 0; i < n; ++i) {
			settings.beginGroup(QString("code-%1").arg(i));
			std::unique_ptr<Otp> otp = Otp::fromSettings(settings, m_cryptPassphrase);

			if(otp) {
				connect(otp.get(), &Otp::changed, this, &Application::writeSettings);
				m_mainWindow.otpList()->addOtp(std::move(otp));
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
			int l = (2 * m_cryptPassphrase.size()) + (qrand() % 20);
			QByteArray random(l, 0);

			while(0 < l) {
				l--;
				random[l] = 'a' + (qrand() % 26);
			}

			QCA::SymmetricKey key(m_cryptPassphrase);
			QCA::InitializationVector initVec(16);
			QCA::Cipher cipher("aes256", QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode, key, initVec);
			settings.setValue("crypt_check", QCA::arrayToHex(initVec.toByteArray() + cipher.process(random).toByteArray()));
		}

		auto * list = m_mainWindow.otpList();
		int n = list->count();

		settings.beginGroup("application");
		m_settings.write(settings);
		settings.endGroup();

		settings.beginGroup("mainwindow");
		m_mainWindow.writeSettings(settings);
		settings.endGroup();

		settings.beginGroup("codes");
		settings.setValue("code_count", n);

		for(int i = 0; i < n; ++i) {
			Otp * code = list->otp(i);

			if(!code) {
				qWarning() << "OTP #" << i << "is null!";
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
			if(m_mainWindow.isHidden() || !m_mainWindow.isActiveWindow()) {
				m_mainWindow.show();
				m_mainWindow.raise();
				m_mainWindow.activateWindow();
			}
			else {
				m_mainWindow.hide();
			}
		}
	}


	void Application::onSettingsChanged() {
		if(m_settings.quitOnMainWindowClosed()) {
			if(!m_quitOnMainWindowClosedConnection) {
				m_quitOnMainWindowClosedConnection = connect(&m_mainWindow, &MainWindow::closing, this, &QApplication::quit, Qt::UniqueConnection);
			}
		}
		else if(m_quitOnMainWindowClosedConnection) {
			disconnect(m_quitOnMainWindowClosedConnection);
			m_quitOnMainWindowClosedConnection = {};
		}
	}


	void Application::showAboutDialogue() {
		static AboutDialogue aboutDialogue;

		aboutDialogue.show();
		aboutDialogue.raise();
		aboutDialogue.activateWindow();
	}


	void Application::clearClipboard() {
		clipboard()->clear();
	}


	void Application::showSettingsWidget() {
		static SettingsWidget settingsWidget(m_settings);
		settingsWidget.show();
		settingsWidget.activateWindow();
		settingsWidget.raise();
	}


}  // namespace Qonvince
