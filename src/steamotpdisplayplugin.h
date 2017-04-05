#ifndef QONVINCE_STEAMOTPDISPLAYPLUGIN_H
#define QONVINCE_STEAMOTPDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

#include <QByteArray>
class QString;

namespace Qonvince {

	class SteamOtpDisplayPlugin
	:	public OtpDisplayPlugin {
		public:
			SteamOtpDisplayPlugin( void );
			virtual ~SteamOtpDisplayPlugin( void );

			virtual QString pluginName( void ) const override;
			virtual QString pluginDescription( void ) const override;
			virtual QString pluginAuthor( void ) const override;
			virtual QString displayString( const QByteArray & hmac ) const override;

		private:
			static QByteArray Alphabet;
			static int CodeDigits;
	};

} // namespace Qonvince

#endif // QONVINCE_STEAMOTPDISPLAYPLUGIN_H
