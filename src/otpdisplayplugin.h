#ifndef QONVINCE_OTPDISPLAYPLUGIN_H
#define QONVINCE_OTPDISPLAYPLUGIN_H

#include <QString>

namespace Qonvince {
	class OtpDisplayPlugin {
		public:
			OtpDisplayPlugin( void );
			virtual ~OtpDisplayPlugin( void );

			virtual QString pluginName( void ) const = 0;
			virtual QString pluginDescription( void ) const = 0;
			virtual QString pluginAuthor( void ) const = 0;

			virtual QString codeDisplayString( const QByteArray & hmac ) const = 0;
	};
}

#endif // QONVINCE_OTPDISPLAYPLUGIN_H
