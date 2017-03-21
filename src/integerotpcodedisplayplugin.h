#ifndef QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
#define QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

namespace Qonvince {
	template <int digits>
	class IntegerOtpCodeDisplayPlugin
	:	public OtpDisplayPlugin 	{
		public:
			IntegerOtpCodeDisplayPlugin() {
			}

			virtual ~IntegerOtpCodeDisplayPlugin() {
			}

			virtual QString pluginName( void ) const override {
				static QString ret(QString("%1-digit number").arg(digits));
				return ret;
			}

			virtual QString pluginDescription( void ) const override {
				static QString ret(QString("Display the generated passcode as a %1-digit number.").arg(digits));
				return ret;
			}

			virtual QString pluginAuthor( void ) const override {
				static QString ret("Darren Edale");
				return ret;
			}

			virtual QString codeDisplayString( const QByteArray & hmac ) const override {
				/* calculate offset and read value from 4 bytes at offset */
				int offset = ((char) hmac[19]) & 0xf;
				quint32 ret = (hmac[offset] & 0x7f) << 24 | (hmac[offset + 1] & 0xff) << 16 | (hmac[offset + 2] & 0xff) << 8 | (hmac[offset + 3] & 0xff);

				/* convert value to requested number of digits */
				int mod = 1;

				for(int i = 0; i < digits; ++i) {
					mod *= 10;
				}

				return QString::number(ret % mod).rightJustified(digits, '0');
			}
	};
}

#endif // QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
