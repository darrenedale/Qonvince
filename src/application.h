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

#include <QApplication>
#include <QSystemTrayIcon>
#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QStandardPaths>

#include "settings.h"

class QAction;
class QMenu;

#define qonvinceApp (Qonvince::Application::qonvince())

namespace Qonvince {
	class MainWindow;
	class SettingsWidget;
	class AboutDialogue;
	class Otp;
	class OtpDisplayPlugin;

	template<int digits>
	class IntegerOtpCodeDisplayPlugin;

	class Application
	:	public QApplication {
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

			Application( int & argc, char ** argv );
			virtual ~Application( void );

			static DesktopEnvironment desktopEnvironment( void );

			static inline constexpr int doubleClickInterval( void ) {
				return 200;
			}

			inline static bool ensureDataDirectory( const QString & path ) {
				return ensureDirectory(QStandardPaths::AppLocalDataLocation, path);
			}

			inline static bool ensureConfigDirectory( const QString & path ) {
				return ensureDirectory(QStandardPaths::AppConfigLocation, path);
			}

			static inline Application * qonvince( void ) {
				return s_instance;
			}

			inline MainWindow * mainWindow( void ) {
				return m_mainWindow;
			}

			inline QSystemTrayIcon * trayIcon( void ) const {
				return m_trayIcon;
			}

			inline Settings & settings( void ) {
				return m_settings;
			}

			inline OtpDisplayPlugin * codeDisplayPluginByName( const QString & name ) const {
				return m_codeDisplayPlugins.value(name);
			}

			inline QList<OtpDisplayPlugin *> codeDisplayPlugins( void ) const {
				return m_codeDisplayPlugins.values();
			}

//			/* note Don't make this return a const reference for security reasons. returning
//			 * a reference could allow a malicious client to cast away the const-ness and therefore
//			 * alter the stored passphrase, rendering the settings unreadable to the actual
//			 * owner once they've been saved. */
//			inline QString cryptPassphrase( void ) const {
//				return m_cryptPassphrase;
//			}

			static int exec( void );

		public Q_SLOTS:
			void showMessage( const QString & title, const QString & message, int timeout = 10000 );
			void showMessage( const QString & message, int timeout = 10000 );
			void readQrCode( void );
			void readQrCode( const QString & fileName );
			void readSettings( void );
			void writeSettings( void ) const;
			void aboutQonvince( void );
			void clearClipboard( void );
			void showSettingsWidget( void );

		private Q_SLOTS:
			void onTrayIconActivated( QSystemTrayIcon::ActivationReason reason );
			void onSettingsChanged( void );
			void codeAdded( Otp * code );
			void codeDestroyed( QObject * code );
			void updateSystemTrayIconTooltip( void );

		private:
			/* RunGuard code from
			 * http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
			 */
			class SingleInstanceGuard {
				public:
					SingleInstanceGuard( const QString & key );
					~SingleInstanceGuard( void );

					bool isAnotherRunning( void );
					bool tryToRun( void );
					void release( void );

				private:
					const QString m_key;
					const QString m_memLockKey;
					const QString m_sharedmemKey;

					QSharedMemory m_sharedMem;
					QSystemSemaphore m_memLock;

					Q_DISABLE_COPY(SingleInstanceGuard)
			};

			static bool ensureDirectory( const QStandardPaths::StandardLocation & location, const QString & path );

			static Application * s_instance;

			SingleInstanceGuard * m_runChecker;
			Settings m_settings;
			MainWindow * m_mainWindow;
			SettingsWidget * m_settingsWidget;
			AboutDialogue * m_aboutDialogue;
			QSystemTrayIcon * m_trayIcon;
			QMenu * m_trayIconMenu;
			QAction * m_quitAction, * m_loadQrImageAction, * m_mainWindowAction, * m_settingsAction;
			QMetaObject::Connection m_quitConnection;

			QHash<QString, OtpDisplayPlugin *> m_codeDisplayPlugins;

			QString m_cryptPassphrase;
	};
}

#endif // QONVINCE_APPLICATION_H
