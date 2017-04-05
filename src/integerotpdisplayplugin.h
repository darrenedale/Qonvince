#ifndef QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
#define QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

class QString;

namespace Qonvince {
	class IntegerOtpDisplayPlugin
	:	public OtpDisplayPlugin {
		public:
			IntegerOtpDisplayPlugin( int digits );
			virtual ~IntegerOtpDisplayPlugin( void );

			virtual QString pluginName( void ) const override;
			virtual QString pluginDescription( void ) const override;
			virtual QString pluginAuthor( void ) const override;
			virtual QString displayString( const QByteArray & hmac ) const override;

			inline int digits( void ) const {
				return m_digits;
			}

			bool setDigits( int digits );

		private:
			static int DefaultDigits;
			int m_digits;
	};
}

#endif // QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
