#include "../../src/base32.h"

#include <QByteArray>

int main(int argc, char ** argv) {
	if(2 > argc) {
		std::cout << "Usage: " << argv[0] << " <data> [<data>...]\n";
		return 1;
	}

	std::cout << "using default Base32\n\n";

	{
		using MyBase32 = LibQonvince::Base32<>;
		MyBase32 b32;

		for(int i = 1; i < argc; ++i) {
			b32.setPlain(argv[i]);
			auto encoded = b32.encoded();
			std::cout << "\"" << b32.plain() << "\" => \"" << encoded << "\"\n";
			b32.setEncoded(encoded);
			std::cout << "\"" << encoded << "\" => \"" << b32.plain() << "\"\n\n";
		}
	}

	std::cout << "\nusing Base32<QByteArray>\n\n";

	{
		using MyBase32 = LibQonvince::Base32<QByteArray>;
		MyBase32 b32;

		for(int i = 1; i < argc; ++i) {
			b32.setPlain(argv[i]);
			auto encoded = b32.encoded();
			std::cout << "\"" << b32.plain().data() << "\" => \"" << encoded.data() << "\"\n";
			b32.setEncoded(encoded);
			std::cout << "\"" << encoded.data() << "\" => \"" << b32.plain().data() << "\"\n\n";
		}
	}
}
