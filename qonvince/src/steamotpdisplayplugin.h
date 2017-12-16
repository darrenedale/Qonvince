#ifndef QONVINCE_STEAMOTPDISPLAYPLUGIN_H
#define QONVINCE_STEAMOTPDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

#include <QByteArray>

class QString;

namespace Qonvince {

	class SteamOtpDisplayPlugin
	: public OtpDisplayPlugin {
		public:
			SteamOtpDisplayPlugin();
			virtual ~SteamOtpDisplayPlugin();

			virtual QString pluginName() const override;
			virtual QString pluginDescription() const override;
			virtual QString pluginAuthor() const override;
			virtual QString displayString(const QByteArray & hmac) const override;
	};

} // namespace Qonvince

#endif // QONVINCE_STEAMOTPDISPLAYPLUGIN_H
