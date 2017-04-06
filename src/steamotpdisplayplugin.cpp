#include "steamotpdisplayplugin.h"

using namespace Qonvince;


QByteArray SteamOtpDisplayPlugin::Alphabet("23456789BCDFGHJKMNPQRTVWXY");
int SteamOtpDisplayPlugin::CodeDigits = 5;


SteamOtpDisplayPlugin::SteamOtpDisplayPlugin( void ) {
}


SteamOtpDisplayPlugin::~SteamOtpDisplayPlugin( void ) {
}



QString SteamOtpDisplayPlugin::pluginName( void ) const {
	return "5-digit Steam code";
}


QString SteamOtpDisplayPlugin::pluginDescription( void ) const {
	return "A 5-digit steam guard code.";
}


QString SteamOtpDisplayPlugin::pluginAuthor( void ) const {
	return "Darren Edale";
}


/* heavily influenced by WinAuth's Steam code generator */
QString SteamOtpDisplayPlugin::displayString( const QByteArray & hmac ) const {
	// the last 4 bits of the mac say where the code starts (e.g. if last 4 bit are 1100, we start at byte 12)
	int i = hmac[19] & 0x0f;

	/* TODO check endianness and reverse if necessary */

	// extract those 4 bytes
	char bytes[4];
	bytes[0] = hmac[i++];
	bytes[1] = hmac[i++];
	bytes[2] = hmac[i++];
	bytes[3] = hmac[i++];

	uint32_t fullcode = *(reinterpret_cast<uint32_t *>(bytes)) & 0x7fffffff;

	// build the alphanumeric code
	QString code;

	for(i = 0; i < CodeDigits; ++i) {
		code.append(QChar(Alphabet[fullcode % Alphabet.length()]));
		fullcode /= Alphabet.length();
	}

	return code;
}
