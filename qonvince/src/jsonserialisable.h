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

#ifndef QONVINCE_JSONSERIALISABLE_H
#define QONVINCE_JSONSERIALISABLE_H

#include <nlohmann/json.hpp>

namespace Qonvince
{
    using nlohmann::json;

    /**
     * Interface for classes that can be serialised to JSON.
     */
    class JsonSerialisable
    {
    public:
        virtual json toJson() const = 0;
        virtual std::string toJsonString() const = 0;
    };
}

#endif // QONVINCE_JSONSERIALISABLE_H
