#ifndef QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
#define QONVINCE_OTPDISPLAYPLUGIN_STEAM_H

#include "otpdisplayplugin.h"
#include "securestring.h"

using LibQonvince::SecureString;

class SteamOtpDisplayPlugin
        : public LibQonvince::OtpDisplayPlugin
{
public:
LIBQONVINCE_OTPDISPLAYPLUGIN

	SecureString codeDisplayString(const SecureString & hmac) const override;
};

#endif  // QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
