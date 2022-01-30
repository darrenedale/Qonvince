#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-convert-member-functions-to-static"
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

/// \file application.cpp
/// \brief Implementation of the Application class.
///
/// \todo investigate segfault (mingw-w64):
/// - remove all settings files
/// - start qonvince
/// - add (the first) new code
/// - enter some settings for new code
/// - close code editor
/// - exit
/// - should segfault

#include "application.h"

#include <array>
#include <iostream>
#include <iterator>
#include <random>
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
#include <QScreen>
#include <QStandardPaths>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QtDBus/QDBusPendingReply>

#if defined(WITH_DBUS_NOTIFICATIONS)
#include <QtDBus/QDBusReply>
#endif

#include "mainwindow.h"
#include "passworddialogue.h"
#include "settingswidget.h"
#include "aboutdialogue.h"
#include "otplistview.h"
#include "otp.h"
#include "otpqrcodereader.h"
#include "pluginfactory.h"

namespace Qonvince
{

    namespace
    {
        // Based on RunGuard code from
        // http://stackoverflow.com/questions/5006547/qt-best-practice-for-a-single-instance-app-protection
        class SingleInstanceGuard final
        {
        public:
            explicit SingleInstanceGuard(const QString & key)
                    : m_key(key),
                      m_memLockKey(QCryptographicHash::hash(key.toUtf8().append(QByteArrayLiteral("_memLockKey")),
                                                            QCryptographicHash::Sha1).toHex()),
                      m_sharedmemKey(QCryptographicHash::hash(key.toUtf8().append(QByteArrayLiteral("_sharedmemKey")),
                                                              QCryptographicHash::Sha1).toHex()),
                      m_sharedMem(m_sharedmemKey),
                      m_memLock(m_memLockKey, 1)
            {
                m_memLock.acquire();

                {
                    QSharedMemory fix(m_sharedmemKey);  // Fix for *nix: http://habrahabr.ru/post/173281/
                    fix.attach();
                }

                m_memLock.release();
            }

            ~SingleInstanceGuard()
            {
                release();
            }

            SingleInstanceGuard(const SingleInstanceGuard &) = delete;

            SingleInstanceGuard(SingleInstanceGuard &&) = delete;

            void operator=(const SingleInstanceGuard &) = delete;

            void operator=(SingleInstanceGuard &&) = delete;

            bool isAnotherRunning()
            {
                if (m_sharedMem.isAttached()) {
                    return false;
                }

                m_memLock.acquire();
                const bool isRunning = m_sharedMem.attach();

                if (isRunning) {
                    m_sharedMem.detach();
                }

                m_memLock.release();
                return isRunning;
            }

            bool tryToRun()
            {
                if (isAnotherRunning()) {  // Extra check
                    return false;
                }

                m_memLock.acquire();
                const bool result = m_sharedMem.create(sizeof(quint64));
                m_memLock.release();

                if (!result) {
                    release();
                    return false;
                }

                return true;
            }

            void release()
            {
                m_memLock.acquire();

                if (m_sharedMem.isAttached()) {
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
    }  // namespace

    Application::Application(int & argc, char ** argv)
    : QApplication(argc, argv),
      m_settings(),
      m_trayIcon(QIcon::fromTheme(QStringLiteral("qonvince"), QIcon(QStringLiteral(":/icons/systray")))),
      m_trayIconMenu(tr("Qonvince")),
      m_clipboardClearTimer(),
      m_clipboardContent(),
      m_notificationsInterface(QStringLiteral("org.freedesktop.Notifications"),
                               QStringLiteral("/org/freedesktop/Notifications"),
                               QStringLiteral("org.freedesktop.Notifications")),
      m_displayPluginFactory(QStringLiteral(".displayplugin"))
    {
        m_clipboardClearTimer.setSingleShot(true);

        setOrganizationName(QStringLiteral("Equit"));
        setOrganizationDomain(QStringLiteral("equituk.net"));
        setApplicationName(QStringLiteral("Qonvince"));
        setApplicationDisplayName(QStringLiteral("Qonvince"));
        setApplicationVersion(QStringLiteral("1.8.2"));
        setQuitOnLastWindowClosed(false);
        QSettings::setDefaultFormat(QSettings::IniFormat);

        processCommandLineArguments();
        loadPlugins();
        setUpTrayIcon();
        readApplicationSettings();
        onSettingsChanged();

        connect(&m_trayIcon, &QSystemTrayIcon::activated, this, &Application::onTrayIconActivated);
        connect(&m_clipboardClearTimer, &QTimer::timeout, this, &Application::clearOtpFromClipboard);
        connect(&m_settings, &Settings::changed, this, &Application::onSettingsChanged);
        connect(this, &Application::aboutToQuit, this, &Application::writeSettings);
    }

    Application::~Application() = default;

    DesktopEnvironment Application::desktopEnvironment()
    {
        static DesktopEnvironment ret = DesktopEnvironment::Unknown;
        static bool done = false;

        if (!done) {
#if defined(Q_OS_UNIX)
            QByteArray wm(qgetenv("XDG_CURRENT_DESKTOP"));

            if ("Unity" == wm) {
                ret = DesktopEnvironment::Unity;
            } else if ("XFCE" == wm) {
                ret = DesktopEnvironment::Xfce;
            } else if ("KDE" == wm) {
                ret = DesktopEnvironment::Kde;
            } else if ("LXDE" == wm) {
                ret = DesktopEnvironment::Lxde;
            } else if ("" == wm) {
                wm = qgetenv("GDMSESSION");

                if ("kde-plasma" == wm) {
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

    int Application::addOtp(std::unique_ptr<Otp> && otp)
    {
        if (!otp || contains(m_otpList, otp)) {
            return -1;
        }

        auto index = static_cast<int>(m_otpList.size());
        auto * otpPtr = otp.get();
        m_otpList.push_back(std::move(otp));
        Q_EMIT otpAdded(otpPtr);
        Q_EMIT otpAdded(index, otpPtr);

        // clazy warning about 3arg lambda connnect() call is OK because the
        // lambda asserts that the Otp ptr is valid; since it must check this
        // in order to compute the index to emit in the signal in any case,
        // there is no gain in using a context argument
        connect(otpPtr, &Otp::changed, [this, otp = otpPtr]() {
            const auto otpBegin = m_otpList.cbegin();
            const auto otpIt = std::find_if(otpBegin, m_otpList.cend(), [otp](const auto & listOtp) {
                return listOtp.get() == otp;
            });

            Q_ASSERT_X(m_otpList.cend() != otpIt, "Application::addOtp",
                       "Otp object that sent changed() signal is not owned by Application instance");

            Q_EMIT otpChanged(otp);
            Q_EMIT otpChanged(static_cast<int>(std::distance(otpBegin, otpIt)));
        });

        return index;
    }

    bool Application::removeOtp(int index)
    {
        if (0 > index || m_otpList.size() <= static_cast<std::size_t>(index)) {
            return false;
        }

        return removeOtp(m_otpList[static_cast<std::size_t>(index)].get());
    }

    bool Application::removeOtp(Otp * otp)
    {
        auto begin = m_otpList.begin();
        auto end = m_otpList.end();
        auto otpIter = std::find_if(begin, end, [&otp](const auto & listOtp) {
            return listOtp.get() == otp;
        });

        if (otpIter == end) {
            return false;
        }

        auto index = static_cast<int>(std::distance(begin, otpIter));

        // need this to keep the Otp alive while the signals are emitted
        // NOTE this means that otpRemoved can only be connected using
        // direct connections (queued connections won't work)
        auto myOtp = std::move(*otpIter);
        m_otpList.erase(otpIter);
        //		std::cout << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): emitting otpRemoved(" << static_cast<void *>(otp) << ")\n";
        //		Q_EMIT otpRemoved(otp);
        std::cout << __PRETTY_FUNCTION__ << " (" << __LINE__ << "): emitting otpRemoved(" << index << ")\n";
        Q_EMIT otpRemoved(index);
        return true;
    }

    void Application::copyOtpToClipboard(Otp * otp)
    {
        Q_ASSERT_X(otp, __PRETTY_FUNCTION__, "can't copy code for null OTP");

        if (m_clipboardClearTimer.isActive()) {
            m_clipboardClearTimer.stop();
        }

        m_clipboardContent = otp->code();
        clipboard()->setText(m_clipboardContent);

        if (settings().clearClipboardAfterInterval() && 0 < settings().clipboardClearInterval()) {
            m_clipboardClearTimer.start(1000 * settings().clipboardClearInterval());
        }
    }

    bool Application::ensureDirectory(QStandardPaths::StandardLocation location, const QString & path)
    {
        QString rootPath = QStandardPaths::writableLocation(location);

        if (rootPath.isEmpty()) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__
                      << "]: failed to find writable location for standard location of type "
                      << static_cast<int>(location) << "\n";
            return false;
        }

        // check that the path is not malicious
        if (rootPath != QDir(rootPath % QStringLiteral("/") % path).absolutePath().left(rootPath.length())) {
            // path contains ".." components that move it out of root data directory
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: \"" << qPrintable(path)
                      << "\" is not a valid subdirectory\n";
            return false;
        }

        if (path.isEmpty() || path.trimmed().isEmpty()) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: path \"" << qPrintable(path)
                      << "\" is enmpty or contains only whitespace\n";
        }

        auto isValidPathChar = [](QChar ch) -> bool {
            static std::array<QChar, 6> s_validChars = {{'-', '_', ' ', '.', '~', '/'}};
            return ch.isLetterOrNumber() ||
                   (s_validChars.cend() != std::find(s_validChars.cbegin(), s_validChars.cend(), ch));
        };

        const auto end = path.cend();
        const auto invalidChar = std::find_if_not(path.cbegin(), end, isValidPathChar);

        if (end != invalidChar) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: path \"" << qPrintable(path)
                      << "\" contains the invalid character " << *invalidChar << "\n";
            return false;
        }

        std::cout << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: creating path \"" << qPrintable(path) << "\" in \""
                  << qPrintable(rootPath) << "\"\n";

        if (!QDir(rootPath).mkpath(path)) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to create path \"" << qPrintable(path)
                      << "\" in \"" << qPrintable(rootPath) << "\"\n";
            return false;
        }

        return true;
    }

    int Application::exec()
    {
        Application * app = qonvinceApp;

        if (app->m_settings.singleInstance()) {
            if (!SingleInstanceGuard{QStringLiteral("blarglefangledungle")}.tryToRun()) {
                std::cerr << "Qonvince is already running.\n";
                return 0;
            }
        }

        bool forceStartMinimised = false;
        auto args = arguments();
        auto argsEnd = args.cend();

        if (argsEnd != std::find_if(args.cbegin(), argsEnd, [](const auto & arg) {
            return QStringLiteral("-m") == arg || QStringLiteral("--minimised") == arg;
        })) {
            forceStartMinimised = true;
        }

        // TODO if settings file does not exist, ask user for passphrase to create a new one
        {
            PasswordDialogue dlg(tr("Enter the passphrase used to encrypt your settings."));

            while (true) {
                if (QDialog::Accepted != dlg.exec()) {
                    /* if the user refuses to enter his/her passphrase, exit the app */
                    return 0;
                }

                QCA::SecureArray pw = dlg.password().toUtf8();

                if (pw.isEmpty()) {
                    std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: passphrase is empty\n";
                } else if (8 > pw.size()) {
                    std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: passphrase is too short\n";
                } else {
                    qonvinceApp->m_cryptPassphrase = pw;

                    if (app->readCodeSettings()) {
                        // if reading succeeds, passphrase was correct, so
                        // exit loop and continue app execution
                        break;
                    }

                    qonvinceApp->m_cryptPassphrase.clear();
                    std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__
                              << "]: failed to read code settings - likely incorrect passphrase\n";
                }

                // we can only get here if the passphrase was not correct
                QThread::msleep(1000);
                dlg.setMessage(
                        tr("The passphrase you entered is not correct. Enter the passphrase used to encrypt your settings."));
            }
        }

        app->onSettingsChanged();
        app->m_trayIcon.show();

        if (!OtpQrCodeReader::isAvailable()) {
            qonvinceApp->showNotification(tr("%1 message").arg(Application::applicationDisplayName()),
                                          tr("Drag and drop of QR code images is not available. You may need to install additional software to enable this."));
        }

        if (!forceStartMinimised && !app->m_settings.startMinimised()) {
            app->m_mainWindow.show();
        }
        //	else if(QX11Info::isPlatformX11()){
        //		/* haven't mapped a main window, so manually inform the system that
        //		 * we're up and running */
        //	}

        return QApplication::exec();
    }

    void Application::showNotification(const QString & title, const QString & message, int timeout)
    {
#if defined(WITH_DBUS_NOTIFICATIONS)
        if (m_notificationsInterface.isValid()) {
             QDBusPendingReply reply = m_notificationsInterface.asyncCall(
                QLatin1String("Notify"),
                applicationDisplayName(),                   // app_name
                0,                                          // replaces_id
                QLatin1String("qonvince"),               // app_icon
                title,                                      // summary
                message,                                    // body
                QStringList(),                              // actions
                QVariantMap(),                              // hints
                timeout                                     // timeout
            );

            if (reply.isValid()) {
                return;
            }
        }
#endif

        if (QSystemTrayIcon::supportsMessages()) {
            m_trayIcon.showMessage(title, message, QSystemTrayIcon::Information, timeout);
        } else {
            QMessageBox::information(&m_mainWindow, title, message, QMessageBox::StandardButtons(QMessageBox::Ok));
        }
    }

    void Application::showNotification(const QString & message, int timeout)
    {
        showNotification(tr("%1 message").arg(applicationDisplayName()), message, timeout);
    }

    void Application::readQrCode()
    {
        QString fileName = QFileDialog::getOpenFileName(&m_mainWindow, tr("Open QR code image"));

        if (fileName.isEmpty()) {
            return;
        }

        readQrCodeFrom(fileName);
    }

    void Application::readQrCodeFrom(const QString & fileName)
    {
        OtpQrCodeReader reader(fileName);

        if (!reader.decode()) {
            showNotification(tr("The file <strong>%1</strong> could not be read as a QR code."));
            return;
        }

        addOtp(reader.createOtp());
    }

    bool Application::readApplicationSettings()
    {
        QSettings settings;

        settings.beginGroup(QStringLiteral("mainwindow"));
        m_mainWindow.readSettings(settings);
        settings.endGroup();

        m_otpList.clear();
        settings.beginGroup(QStringLiteral("application"));
        m_settings.read(settings);

        return true;
    }

    bool Application::readCodeSettings()
    {
        QSettings settings;

        // read the crypt_check value. if decryption indicates an error, the passphrase
        // is wrong; if it indicates success the passphrase is right. this determines
        // whether to continue reading the file or not. if the passphrase not correct,
        // it won't successfully decrypt the seeds and subsequently when writing the
        // codes back they will be encrypted with the erroneous passphrase and will
        // effectively become inaccessible. if the passphrase is correct, or someone
        // manages to trick this check, the passphrase must still correctly decrypt the
        // seeds. in other words, bypassing this check does not grant access to seeds
        if (settings.contains(QStringLiteral("crypt_check"))) {
            if (!QCA::isSupported("aes256-cbc")) {
                std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: AES256 is not supported\n";
                showNotification(
                        tr("AES256 encryption is required to keep your OTP seeds safe. This encryption algorithm is not available, therefore your settings cannot be read."));
                return false;
            }

            QCA::SecureArray value = QCA::hexToArray(settings.value(QStringLiteral("crypt_check")).toString());

            QCA::Cipher cipher(QStringLiteral("aes256"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Decode,
                               QCA::SymmetricKey(m_cryptPassphrase),
                               QCA::InitializationVector(value.toByteArray().left(16)));
            cipher.process(value.toByteArray().mid(16));

            if (!cipher.ok()) {
                std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__
                          << "]: decryption failure - incorrect passphrase\n";
                return false;
            }
        }

        m_otpList.clear();

        settings.beginGroup(QStringLiteral("codes"));
        int n = settings.value(QStringLiteral("code_count"), 0).toInt();

        for (int i = 0; i < n; ++i) {
            settings.beginGroup(QStringLiteral("code-%1").arg(i));
            std::unique_ptr<Otp> otp = Otp::fromSettings(settings, m_cryptPassphrase);

            if (otp) {
                connect(otp.get(), &Otp::changed, this, &Application::writeSettings);
                addOtp(std::move(otp));
            } else {
                std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: failed to read code" << i << "\n";
            }

            settings.endGroup();
        }

        return true;
    }

    void Application::writeSettings()
    {
        QSettings settings;
        bool writeOtpDetails = QCA::isSupported("aes256-cbc");

        if (!writeOtpDetails) {
            std::cerr << __PRETTY_FUNCTION__ << " [" << __LINE__ << "]: AES256 is not supported\n";
            showNotification(
                    tr("AES256 encryption is required to keep your OTP seeds safe. This encryption algorithm is not available, therefore your OTP settings cannot be saved."));
        } else {
            // this "random" string in the settings will, when read, indicate whether the crypt key is correct
            {
                // use length of passphrase so that a truncated passphrase can never pass the check
                int l = (2 * m_cryptPassphrase.size()) + (std::random_device()() % 20);
                QByteArray random(l, 0);

                while (0 < l) {
                    l--;
                    random[l] = 'a' + (std::random_device()() % 26);
                }

                QCA::SymmetricKey key(m_cryptPassphrase);
                QCA::InitializationVector initVec(16);
                QCA::Cipher cipher(QStringLiteral("aes256"), QCA::Cipher::CBC, QCA::Cipher::DefaultPadding, QCA::Encode,
                                   key, initVec);
                settings.setValue(QStringLiteral("crypt_check"),
                                  QCA::arrayToHex(initVec.toByteArray() + cipher.process(random).toByteArray()));

                settings.beginGroup(QStringLiteral("codes"));
                settings.remove(QStringLiteral(
                                        ""));  // remove all settings in "codes" group to ensure file doesn't contain lingering old codes
                settings.setValue(QStringLiteral("code_count"), static_cast<int>(m_otpList.size()));

                auto i = 0;
                for (const auto & otp : m_otpList) {
                    Q_ASSERT_X(otp, __PRETTY_FUNCTION__, "found null OTP in OTP list");
                    settings.beginGroup(QStringLiteral("code-%1").arg(i));
                    otp->writeSettings(settings, m_cryptPassphrase);
                    settings.endGroup();
                    ++i;
                }

                settings.endGroup();
            }
        }

        settings.beginGroup(QStringLiteral("application"));
        m_settings.write(settings);
        settings.endGroup();

        settings.beginGroup(QStringLiteral("mainwindow"));
        m_mainWindow.writeSettings(settings);
        settings.endGroup();
    }

    void Application::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
    {
        if (QSystemTrayIcon::Trigger == reason) {
            if (m_mainWindow.isHidden() || !m_mainWindow.isActiveWindow()) {
                m_mainWindow.show();
                m_mainWindow.raise();
                m_mainWindow.activateWindow();
            } else {
                m_mainWindow.hide();
            }
        }
    }

    void Application::onSettingsChanged()
    {
        if (m_settings.quitOnMainWindowClosed()) {
            if (!m_quitOnMainWindowClosedConnection) {
                m_quitOnMainWindowClosedConnection = connect(&m_mainWindow, &MainWindow::closing, this,
                                                             &QApplication::quit, Qt::UniqueConnection);
            }
        } else if (m_quitOnMainWindowClosedConnection) {
            disconnect(m_quitOnMainWindowClosedConnection);
            m_quitOnMainWindowClosedConnection = {};
        }
    }

    void Application::showAboutDialogue()
    {
        static AboutDialogue aboutDialogue;

        aboutDialogue.show();
        aboutDialogue.raise();
        aboutDialogue.activateWindow();
    }

    void Application::clearOtpFromClipboard()
    {
        auto * clip = clipboard();

        if (!m_clipboardContent.isEmpty() && m_clipboardContent == clip->text()) {
            clip->clear();
            // clear() doesn't actually clear the content sometimes (KDE5, Manjaro, Qt5.11)
            clip->setText(QStringLiteral());
        }
    }

    void Application::showSettingsWidget()
    {
        static SettingsWidget settingsWidget(m_settings);
        settingsWidget.show();
        settingsWidget.activateWindow();
        settingsWidget.raise();
    }

    void Application::processCommandLineArguments()
    {
        const QStringList args = arguments();

        for (int idx = 1, argCount = args.size(); idx < argCount; ++idx) {
            const QString & arg = args[idx];

            if ("--plugin-path" == arg) {
                ++idx;

                if (argCount <= idx) {
                    std::cerr << "missing argument for --plugin-path option";
                    break;
                }

                m_displayPluginFactory.addSearchPath(args[idx]);
                break;
            }
        }
    }

    void Application::loadPlugins()
    {
        // set the search paths for the display plugin factory
        const auto locations = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);

        for (const auto & pathRoot : locations) {
            m_displayPluginFactory.addSearchPath(pathRoot + QStringLiteral("/plugins/otpdisplay"));
        }

#ifndef NDEBUG
        // search in the build dir
        m_displayPluginFactory.addSearchPath(QStringLiteral("../plugins/otpdisplay"));
#endif

        m_displayPluginFactory.loadAllPlugins();
    }

    void Application::setUpTrayIcon()
    {

        m_trayIcon.setToolTip(tr("%1: One-Time passcode generator.").arg(applicationDisplayName()));

        m_trayIconMenu.addAction(tr("Show main window"), &m_mainWindow, &MainWindow::show);
        m_trayIconMenu.addAction(
                QIcon::fromTheme(QStringLiteral("system-settings"), QIcon(QStringLiteral(":/icons/app/settings"))),
                tr("Settings..."), this, &Application::showSettingsWidget);

        if (OtpQrCodeReader::isAvailable()) {
            m_trayIconMenu.addSeparator();
            m_trayIconMenu.addAction(
                    QIcon::fromTheme(QStringLiteral("image-png"), QIcon(QStringLiteral(":/icons/app/readqrcode"))),
                    tr("Read a QR code image"), this, &Application::readQrCode);
        }

        m_trayIconMenu.addSeparator();
        m_trayIconMenu.addAction(tr("About %1").arg(applicationDisplayName()), this, &Application::showAboutDialogue);
        m_trayIconMenu.addAction(
                QIcon::fromTheme(QStringLiteral("application-exit"), QIcon(QStringLiteral(":/icons/app/quit"))),
                tr("Quit Qonvince"), this, &Application::quit);

        m_trayIcon.setContextMenu(&m_trayIconMenu);
    }
}  // namespace Qonvince

#pragma clang diagnostic pop
