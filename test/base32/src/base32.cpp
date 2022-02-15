/*
 * Copyright 2015 - 2022 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * Qonvince is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Qonvince is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Qonvince. If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "base32.h"


using Base32 = LibQonvince::Base32<std::string>;


int main(int argc, char * argv[]) {
	(void) argc;
	(void) argv;

	Base32::ByteArray data = reinterpret_cast<const Base32::ByteArray::value_type *>("what have you done for me lately?\n");
	Base32 encoder(data);

	std::cout << "Plain: \"" << data.data() << "\"\n";
	// expected: "O5UGC5BANBQXMZJAPFXXKIDEN5XGKIDGN5ZCA3LFEBWGC5DFNR4T6CQ="
	std::cout << "Base32: \"" << encoder.encoded().data() << "\"\n";

	encoder.setEncoded(encoder.encoded());
	std::cout << "Plain: \"" << encoder.plain().data() << "\"\n";
	return 0;
}
