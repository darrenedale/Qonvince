#include "steamotpdisplayplugin.h"

#include <array>


namespace Qonvince {


static constexpr const std::array<char, 26> Alphabet = {{'2', '3', '4', '5', '6', '7', '8', '9', 'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'M', 'N', 'P', 'Q', 'R', 'T', 'V', 'W', 'X', 'Y'}};
static constexpr const int CodeDigits = 5;


SteamOtpDisplayPlugin::SteamOtpDisplayPlugin() = default;


SteamOtpDisplayPlugin::~SteamOtpDisplayPlugin() = default;



QString SteamOtpDisplayPlugin::pluginName() const {
	return QStringLiteral("5-digit Steam code");
}


QString SteamOtpDisplayPlugin::pluginDescription() const {
	return QStringLiteral("A 5-digit steam guard code.");
}


QString SteamOtpDisplayPlugin::pluginAuthor() const {
	return QStringLiteral("Darren Edale");
}


// heavily influenced by WinAuth's Steam code generator
QString SteamOtpDisplayPlugin::displayString( const QByteArray & hmac ) const {
	// the last 4 bits of the mac say where the code starts
	// (e.g. if last 4 bit are 1100, we start at byte 12)
	int i = hmac[19] & 0x0f;

	// TODO check endianness and reverse if necessary

	// extract those 4 bytes
	std::array<char, 4> bytes = {{hmac[i], hmac[i + 1], hmac[i + 2], hmac[i + 3]}};
	uint32_t fullcode = *(reinterpret_cast<uint32_t *>(&bytes[0])) & 0x7fffffff;

	// build the alphanumeric code
	QString code;

	for(i = 0; i < CodeDigits; ++i) {
		code.push_back(QChar{Alphabet[fullcode % Alphabet.size()]});
		fullcode /= Alphabet.size();
	}

	return code;
}


}	// namespace Qonvince
