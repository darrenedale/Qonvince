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

#ifndef QONVINCE_APPLICATION_H
#define QONVINCE_APPLICATION_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <QApplication>
#include <QString>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QObject>
#include <QStandardPaths>

#include <Qca-qt5/QtCrypto/QtCrypto>

#include "otp.h"
#include "algorithms.h"
#include "mainwindow.h"
#include "settings.h"
#include "settingswidget.h"
#include "aboutdialogue.h"
#include "otpdisplayplugin.h"
#include "pluginfactory.h"
#include "qtstdhash.h"

class QAction;

#define qonvinceApp (Qonvince::Application::qonvince())

namespace Qonvince {

	class MainWindow;
	class SettingsWidget;
	class AboutDialogue;

	class Application
	: public QApplication {
		Q_OBJECT

	public:
		enum class DesktopEnvironment {
			Unknown,
			Kde,
			Gnome,
			Unity,
			Xfce,
			Lxde,
			WindowsDesktop,
			MacOSx,
			Android,
			WindowsPhone,
		};

		Application(int & argc, char ** argv);
		virtual ~Application();

		static DesktopEnvironment desktopEnvironment();

		inline static bool ensureDataDirectory(const QString & path) {
			return ensureDirectory(QStandardPaths::AppLocalDataLocation, path);
		}

		inline static bool ensureConfigDirectory(const QString & path) {
			return ensureDirectory(QStandardPaths::AppConfigLocation, path);
		}

		inline static Application * qonvince() {
			return static_cast<Application *>(QApplication::instance());
		}

		inline Settings & settings() {
			return m_settings;
		}

		inline const Settings & settings() const {
			return m_settings;
		}

		inline int otpCount() const {
			return static_cast<int>(m_otpList.size());
		}

		Otp * otp(int index) const {
			if(0 > index || m_otpList.size() <= static_cast<std::size_t>(index)) {
				return nullptr;
			}

			return m_otpList[static_cast<std::size_t>(index)].get();
		}

		int addOtp(std::unique_ptr<Otp> &&);

		// Application takes ownership of otp
		inline int addOtp(Otp * otp) {
			const auto end = m_otpList.cend();

			if(end != std::find_if(m_otpList.cbegin(), end, [otp](const auto & listOtp) {
					return listOtp.get() == otp;
				})) {
				return -1;
			}

			return addOtp(std::unique_ptr<Otp>(otp));
		}

		bool removeOtp(int);
		bool removeOtp(Otp *);

		inline LibQonvince::OtpDisplayPlugin * otpDisplayPluginByName(const QString & name) {
			return m_displayPluginFactory.pluginByName(name);
		}

		inline std::vector<LibQonvince::OtpDisplayPlugin *> otpDisplayPlugins() {
			return m_displayPluginFactory.loadedPlugins();
		}

		static int exec();

	Q_SIGNALS:
		void otpAdded(Otp *);
		void otpAdded(int, Otp *);

		// Otp object has been removed but not (yet) destroyed
		void otpRemoved(Otp *);
		void otpRemoved(int);

		void otpChanged(Otp *);
		void otpChanged(int);

	public Q_SLOTS:
		void showNotification(const QString & title, const QString & message, int timeout = 10000);
		void showNotification(const QString & message, int timeout = 10000);
		void readQrCode();
		void readQrCodeFrom(const QString & fileName);
		bool readApplicationSettings();
		bool readCodeSettings();
		void writeSettings() const;
		void showAboutDialogue();
		void showSettingsWidget();
		void clearClipboard();

	private Q_SLOTS:
		void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
		void onSettingsChanged();

	private:
		using DisplayPluginFactory = PluginFactory<LibQonvince::OtpDisplayPlugin>;
		static bool ensureDirectory(const QStandardPaths::StandardLocation & location, const QString & path);

		Settings m_settings;
		MainWindow m_mainWindow;
		QSystemTrayIcon m_trayIcon;
		QMenu m_trayIconMenu;
		QMetaObject::Connection m_quitOnMainWindowClosedConnection;
		std::vector<std::unique_ptr<Otp>> m_otpList;

		DisplayPluginFactory m_displayPluginFactory;

		QCA::SecureArray m_cryptPassphrase;
	};

}  // namespace Qonvince

#endif  // QONVINCE_APPLICATION_H
