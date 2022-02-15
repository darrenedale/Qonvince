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

#ifndef QONVINCE_ALGORITHMS_H
#define QONVINCE_ALGORITHMS_H

#include <QtCore/QHash>

namespace Qonvince
{
	// convenience algorithms to work with STL and QT containers transparently
	template<class ContainerT, typename ValueT = typename ContainerT::value_type>
	bool contains(const ContainerT & container, const ValueT & value)
	{
		const auto & end = container.cend();
		return end != std::find(container.cbegin(), end, value);
	}

	template<class ContainerT, typename ValueT = typename ContainerT::value_type>
	int removeAll(ContainerT & container, const ValueT & value)
	{
		const auto originalEnd = container.end();
		const auto newEnd = std::remove(container.begin(), originalEnd, value);

		if(newEnd == originalEnd) {
			return 0;
		}

		auto ret = static_cast<int>(std::distance(newEnd, originalEnd));
		container.erase(newEnd, originalEnd);
		return ret;
	}

}	// namespace Qonvince

#endif  // QONVINCE_ALGORITHMS_H
