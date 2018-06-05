#ifndef QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
#define QONVINCE_OTPDISPLAYPLUGIN_STEAM_H

#include "otpdisplayplugin.h"

class SteamOtpDisplayPlugin
: public LibQonvince::OtpDisplayPlugin {
public:
	LIBQONVINCE_OTPDISPLAYPLUGIN

	QString codeDisplayString(const QByteArray & hmac) const override;
};


#endif  // QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
