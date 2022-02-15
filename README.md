# Qonvince
A cross-platform OTP generator.

Much like Google's Authenticator app, qonvince generates One-Time Passcodes for your two-factor logins.
It has the following features:
- Generate both time-based (TOTP) and HMAC-based (HOTP) codes
- Read QR codes
- Secure storage of secrets, encrypted using a passphrase
- One-click copy of the current code to the clipboard
- Clear the code from the clipboard after a timeout for security
- Extensible through plugins to display codes in different ways (plugins for common six-digit and eight-digit display, and Steam-style 5-character codes are included)

Qonvince uses [Qt5](https://www.qt.io/ "Visit the Qt website") for its UI, and can be built for Linux, Windows and OSX.

## Requirements
- C++20 compiler
- [Qt5](https://doc.qt.io/qt-5/ "Visit the Qt5 API documentation on the web") (Core, Widgets, DBus, optionally Network)
- [QCA](https://api.kde.org/qca/html/ "Visit the QCA KDE website") (for settings encryption)
- [nlohman::json](https://github.com/nlohmann/json "View repository on Github")
- [zbarimg](http://zbar.sourceforge.net/ "Visit the ZBar website on SourceForge") (for reading QR codes, linux only)
- [CMake](https://cmake.org/ "Visit the CMake website") (to build)

## Build
```
mkdir build && cd build
cmake ../
cmake --build ./
```

When building the `qonvince` target, the following options are available:
- `WITH_NETWORK_ACCESS` Enable downloading of QR codes/icons from URLs. Run CMake with the argument `-DWITH_NETWORK_ACCESS`. This defaults to `ON`. Set it to `OFF` to turn off this feature.
- `WITH_DBUS_NOTIFICATIONS` Use _DBus_ to show notifications to the user. Not available on Windows or MacOS (so ignored). Run CMake with the argument `-DWITH_DBUS_NOTIFICATIONS`. This defaults to `ON`. Set it to `OFF` to turn off this feature.

## Install
Follow the build instructions above.
```
cmake --install
```
