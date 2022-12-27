#include "steam.h"

#include <array>
#include "securestring.h"

using LibQonvince::SecureString;

DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN(SteamOtpDisplayPlugin, "Steam code", "Display the code as a Steam-type 5-character code.", "Darren Edale", "1.0.0")

namespace
{
    constexpr const std::array<char, 26> Alphabet = {
            {'2', '3', '4', '5', '6', '7', '8', '9', 'B', 'C', 'D', 'F', 'G', 'H', 'J', 'K', 'M', 'N', 'P', 'Q', 'R', 'T', 'V', 'W', 'X', 'Y'}};
    constexpr const int CodeDigits = 5;
}  // namespace

// heavily influenced by WinAuth's Steam code generator
SecureString SteamOtpDisplayPlugin::codeDisplayString(const SecureString & hmac) const
{
    // the last 4 bits of the mac say where the code starts
    // (e.g. if last 4 bit are 1100, we start at byte 12)
    int i = hmac[19] & 0x0f;

    // TODO check endianness and reverse if necessary

    // extract those 4 bytes
    std::array<char, 4> bytes = {{hmac[i], hmac[i + 1], hmac[i + 2], hmac[i + 3]}};
    uint32_t fullcode = *(reinterpret_cast<uint32_t *>(&bytes[0])) & 0x7fffffff;

    // build the alphanumeric code
    SecureString code(CodeDigits, '\0');

    for (i = 0; i < CodeDigits; ++i) {
        code[i] = Alphabet[fullcode % Alphabet.size()];
        fullcode /= Alphabet.size();
    }

    return code;
}
