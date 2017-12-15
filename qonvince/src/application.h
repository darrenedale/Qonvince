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

#include <QtCrypto/QtCrypto>

#include "algorithms.h"
#include "mainwindow.h"
#include "settings.h"
#include "aboutdialogue.h"
#include "otpdisplayplugin.h"

class QAction;

#define qonvinceApp (Qonvince::Application::qonvince())

namespace Qonvince {

	class MainWindow;
	class SettingsWidget;
	class AboutDialogue;
	class Otp;

	using PluginPtr = std::shared_ptr<OtpDisplayPlugin>;
	using PluginMap = std::unordered_map<QString, PluginPtr, Qonvince::QtHash<QString>>;
	using PluginArray = std::vector<PluginPtr>;

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
			return s_instance;
		}

		inline Settings & settings() {
			return m_settings;
		}

		inline const Settings & settings() const {
			return m_settings;
		}

		PluginPtr codeDisplayPluginByName(const QString & name) const;
		PluginArray codeDisplayPlugins() const;

		static int exec();

	public Q_SLOTS:
		void showNotification(const QString & title, const QString & message, int timeout = 10000);
		void showNotification(const QString & message, int timeout = 10000);
		void readQrCode();
		void readQrCodeFrom(const QString & fileName);
		bool readApplicationSettings();
		bool readCodeSettings();
		void writeSettings() const;
		void showAboutDialogue();
		void clearClipboard();
		void showSettingsWidget();

	private Q_SLOTS:
		void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
		void onSettingsChanged();

	private:
		static bool ensureDirectory(const QStandardPaths::StandardLocation & location, const QString & path);

		static Application * s_instance;

		Settings m_settings;
		std::unique_ptr<MainWindow> m_mainWindow;
		std::unique_ptr<SettingsWidget> m_settingsWidget;
		std::unique_ptr<AboutDialogue> m_aboutDialogue;
		QSystemTrayIcon m_trayIcon;
		QMenu m_trayIconMenu;
		QMetaObject::Connection m_quitOnMainWindowClosedConnection;

		PluginMap m_codeDisplayPlugins;

		QCA::Initializer m_qcaInit;
		QCA::SecureArray m_cryptPassphrase;
	};

}  // namespace Qonvince

#endif  // QONVINCE_APPLICATION_H
