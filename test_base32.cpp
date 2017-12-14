#include "src/base32.h"


int main(int argc, char ** argv) {
	if(2 > argc) {
		std::cout << "Usage: " << argv[0] << " <data> [<data>...]\n";
		return 1;
	}

	using StdBase32 = LibQonvince::Base32<>;
	StdBase32 b32;

	for(int i = 1; i < argc; ++i) {
		b32.setPlain(argv[i]);

		std::cout << "\"" << b32.plain() << "\" => \"" << b32.encoded() << "\"\n";
	}
}
