/*
 * Copyright 2015 - 2017 Darren Edale
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
#include "src/algorithms.h"


int main(int argc, char * argv[]) {
	{
		std::string allLower = "abcdefghijklmnopqrstuvwxyz1234567890!\"£$%^&*()";

		std::cout << "All lower case: \"" << allLower << "\"\n";
		Qonvince::toUpper(allLower);
		std::cout << "Converted     : \"" << allLower << "\"\n";
	}

	{
		std::string allUpper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!\"£$%^&*()";
		std::cout << "All upper case: \"" << allUpper << "\"\n";
		Qonvince::toLower(allUpper);
		std::cout << "Converted     : \"" << allUpper << "\"\n";
	}

	return 0;
}
