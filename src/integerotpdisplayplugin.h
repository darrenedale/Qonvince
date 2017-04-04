#ifndef QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
#define QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H

#include "otpdisplayplugin.h"

namespace Qonvince {
	class IntegerOtpDisplayPlugin
	:	public OtpDisplayPlugin {
		public:
			IntegerOtpDisplayPlugin( int digits )
			:	m_digits(6) {
				setDigits(digits);
			}

			int digits( void ) const {
				return m_digits;
			}

			bool setDigits( int digits ) {
				if(0 < digits) {
					m_digits = digits;
					return true;
				}

				return false;
			}

			virtual ~IntegerOtpDisplayPlugin( void ) {
			}

			virtual QString pluginName( void ) const override {
				return QString("%1-digit number").arg(m_digits);
			}

			virtual QString pluginDescription( void ) const override {
				return QObject::tr("Display the generated passcode as a %1-digit number.").arg(m_digits);
			}

			virtual QString pluginAuthor( void ) const override {
				return {"Darren Edale"};
			}

			virtual QString displayString( const QByteArray & hmac ) const override {
				/* calculate offset and read value from 4 bytes at offset */
				int offset = ((char) hmac[19]) & 0xf;
				quint32 ret = (hmac[offset] & 0x7f) << 24 | (hmac[offset + 1] & 0xff) << 16 | (hmac[offset + 2] & 0xff) << 8 | (hmac[offset + 3] & 0xff);

				/* convert value to requested number of digits */
				int mod = 1;

				for(int i = 0; i < m_digits; ++i) {
					mod *= 10;
				}

				return QString::number(ret % mod).rightJustified(m_digits, '0');
			}

		private:
			int m_digits;
	};
}

#endif // QONVINCE_INTEGEROTPCODEDISPLAYPLUGIN_H
