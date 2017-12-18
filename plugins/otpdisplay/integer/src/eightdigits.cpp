#include "eightdigits.h"
#include "integercodedisplaystring.h"

DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN(EightDigitsPlugin, "EightDigits", "Display the code as eight decimal digits.", "Darren Edale", "1.0.0")

QString EightDigitsPlugin::codeDisplayString(const QByteArray & hmac) const {
	return integerCodeDisplayString<8>(hmac);
}
