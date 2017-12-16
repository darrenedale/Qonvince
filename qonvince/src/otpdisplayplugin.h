#ifndef QONVINCE_OTPDISPLAYPLUGIN_H
#define QONVINCE_OTPDISPLAYPLUGIN_H

#include <QString>

namespace Qonvince {

	class OtpDisplayPlugin {
	public:
		OtpDisplayPlugin();
		virtual ~OtpDisplayPlugin();

		virtual QString pluginName() const = 0;
		virtual QString pluginDescription() const = 0;
		virtual QString pluginAuthor() const = 0;

		virtual QString displayString(const QByteArray & hmac) const = 0;
	};

}  // namespace Qonvince

#endif  // QONVINCE_OTPDISPLAYPLUGIN_H
