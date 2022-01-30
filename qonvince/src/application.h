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

#ifndef QONVINCE_APPLICATION_H
#define QONVINCE_APPLICATION_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtDBus/QDBusInterface>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSystemTrayIcon>
#include <QtWidgets/QMenu>
#include <QScreen>

#include <QtCrypto>

#include "types.h"
#include "otp.h"
#include "algorithms.h"
#include "mainwindow.h"
#include "settings.h"
#include "settingswidget.h"
#include "aboutdialogue.h"
#include "otpdisplayplugin.h"
#include "pluginfactory.h"
#include "qtstdhash.h"

#define qonvinceApp (Qonvince::Application::qonvince())

namespace Qonvince
{
	class Application
	: public QApplication {
		Q_OBJECT

	public:
        // globally-accessible constant to assist with UI adaptation to screens with different DPIs
        static constexpr const auto ReferencePixelDensity = 96;

		Application(int &, char **);
		~Application() override;

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

        template<typename ValueType>
        ValueType referencePxToScreenPx(ValueType px, const QScreen * screen = nullptr)
        {
            if (!screen) {
                screen = m_mainWindow.screen();
            }

            return (screen->physicalDotsPerInchY() / ReferencePixelDensity) * px;
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

		void copyOtpToClipboard(Otp *);

		static int exec();

	Q_SIGNALS:
		void otpAdded(Otp *);
		void otpAdded(int, Otp *);

		// Otp object has been removed. if the slot is connected using a direct connection
		// the Otp object has not yet been destroyed; if it is used in a queued connection
		// it has been destroyed and MUST NOT BE DEREFERENCED
		//		void otpRemoved(Otp *);
		void otpRemoved(int);

		void otpChanged(Otp *);
		void otpChanged(int);

	public Q_SLOTS:
		void showNotification(const QString & title, const QString & message, int timeout = 10000);
		void showNotification(const QString & message, int timeout = 10000);

		// TODO requires DBus interface to listen for response call
//		void askQuestion(const QString & question, const QStringList & options, int timeout = 10000);
		void readQrCode();
		void readQrCodeFrom(const QString & fileName);
		bool readApplicationSettings();
		bool readCodeSettings();
		void writeSettings();
		void showAboutDialogue();
		void showSettingsWidget();
		void clearOtpFromClipboard();

	private Q_SLOTS:
		void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
		void onSettingsChanged();

	private:
		using DisplayPluginFactory = PluginFactory<LibQonvince::OtpDisplayPlugin>;

		static bool ensureDirectory(QStandardPaths::StandardLocation location, const QString & path);
        void processCommandLineArguments();
        void loadPlugins();

		QCA::Initializer m_qcaInitializer;
		Settings m_settings;
		MainWindow m_mainWindow;
		QSystemTrayIcon m_trayIcon;
		QMenu m_trayIconMenu;
		QTimer m_clipboardClearTimer;
		QString m_clipboardContent;
		QDBusInterface m_notificationsInterface;
		QMetaObject::Connection m_quitOnMainWindowClosedConnection;
		std::vector<std::unique_ptr<Otp>> m_otpList;

		DisplayPluginFactory m_displayPluginFactory;

		QCA::SecureArray m_cryptPassphrase;

        void setUpTrayIcon();
    };

}  // namespace Qonvince

#endif  // QONVINCE_APPLICATION_H
