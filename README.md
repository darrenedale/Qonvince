# Qonvince
A cross-platform OTP generator.

Much like Google's Authenticator app, qonvince generates One-Time Passcodes for your two-factor logins.
It has the following features:
- Works with both time-based (TOTP) and HMAC-based (HOTP) code generation
- Read QR codes
- Secure storage of secrets, encrypted using a passphrase
- Quickly copy the current code to the clipboard
- Optionally clears the code from the clipboard after a specified timeout for security
- Extensible through plugins to display code in different ways (plugins for common six-digit and eight-digit display, and Steam-style 5-character codes are included)

Qonvince uses Qt5 for its UI, and can be built for Linux, Windows and OSX.

## Requirements
- Qt5 (Core, Widgets, DBus, optionally Network)
- QCA (for settings encryption)
- zbarimg (for reading QR codes, linux only)
- CMake (to build)

## Build
```
mkdir build && cd build
cmake ../
cmake --build ./
```

When building the qonvince target, the following options are available:
- `WITH_NETWORK_ACCESS` Enable downloading of QR codes/icons from URLs. Run CMAKE with the argument `-DWITH_NETWORK_ACCESS`. This defaults to `ON`. Set it to `OFF` to turn off this feature.
- `WITH_DBUS_NOTIFICATIONS` Use DBus to show notifications to the user. Not available on Windows or MacOS (so ignored). This defaults to `ON`. Set it to `OFF` to turn off this feature.

## Install
Follow the build instructions above.
```
cmake --install
```
