#ifndef QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
#define QONVINCE_OTPDISPLAYPLUGIN_STEAM_H

#include "otpdisplayplugin.h"

#include <QString>
#include <QByteArray>

class QString;


class SteamOtpDisplayPlugin
    : public LibQonvince::OtpDisplayPlugin {
	public:
		LIBQONVINCE_ANALYSIS_OTPDISPLAYPLUGIN

		virtual QString codeDisplayString(const QByteArray & hmac) const override;
	};


#endif  // QONVINCE_OTPDISPLAYPLUGIN_STEAM_H
