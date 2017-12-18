#include "sixdigits.h"
#include "integercodedisplaystring.h"

DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN(SixDigitsPlugin, "SixDigits", "Display the code as six decimal digits.", "Darren Edale", "1.0.0")

QString SixDigitsPlugin::codeDisplayString(const QByteArray & hmac) const {
	return integerCodeDisplayString<6>(hmac);
}
