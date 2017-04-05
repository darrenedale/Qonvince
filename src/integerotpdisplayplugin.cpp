#include "integerotpdisplayplugin.h"

#include <QString>
#include <QObject>


using namespace Qonvince;


int IntegerOtpDisplayPlugin::DefaultDigits = 6;


IntegerOtpDisplayPlugin::IntegerOtpDisplayPlugin( int digits )
:	m_digits(DefaultDigits) {
	setDigits(digits);
}


bool IntegerOtpDisplayPlugin::setDigits( int digits ) {
	if(0 < digits) {
		m_digits = digits;
		return true;
	}

	return false;
}


IntegerOtpDisplayPlugin::~IntegerOtpDisplayPlugin( void ) {
}


QString IntegerOtpDisplayPlugin::pluginName( void ) const {
	return QString("%1-digit number").arg(m_digits);
}


QString IntegerOtpDisplayPlugin::pluginDescription( void ) const {
	return QObject::tr("Display the generated passcode as a %1-digit number.").arg(m_digits);
}


QString IntegerOtpDisplayPlugin::pluginAuthor( void ) const {
	return {"Darren Edale"};
}


QString IntegerOtpDisplayPlugin::displayString(const QByteArray & hmac) const {
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
