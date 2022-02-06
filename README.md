# Qonvince
A cross-platform OTP generator.

Much like Google's Authenticator app, qonvince generates One-Time Passcodes for your two-factor logins.
It has the following features:
- Works with both time-based (TOTP) and HMAC-based (HOTP) code generation
- Read QR codes
- Secure storage of secrets, encrypted using a passphrase
- Quick copying of the current code to the clipboard
- Clearing of the clipboard after a specified timeout for security
- Customisable code display through plugins (provided with plugins for common six-digit and eight-digit display, and Steam-style 5-character codes)

Qonvince uses Qt5 for its UI, and can be built for Linux, Windows and OSX.

## Requirements
- Qt5 (Core, Widgets, DBus, optionally Network)
- QCA (for settings encryption)
- zbarimg (for reading QR codes)

## Build
```
mkdir build && cd build
cmake ../
cmake --build ./
```
## Install
Follow the build instructions above.
```
cmake --install
```
