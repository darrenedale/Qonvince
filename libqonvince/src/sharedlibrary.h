/*
 * Copyright 2015 - 2022 Darren Edale
 *
 * This file is part of the Équit library.
 *
 * The Équit library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Équit library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Équit library. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBQONVINCE_SHAREDLIBRARY_H
#define LIBQONVINCE_SHAREDLIBRARY_H

/**
 * @file sharedlibrary.h
 * @author Darren Edale
 * @version 0.5
 * @date September 2017
 *
 * @brief Definition of the SharedLibrary class.
 */

#include <string>

#if defined(_WIN32)
#include <windows.h>
#endif

namespace LibQonvince
{

    class SharedLibrary final
    {
#if defined(_WIN32)

    public:
        using Symbol = FARPROC;

    private:
        using LibraryHandle = HINSTANCE;
        static constexpr const LibraryHandle NullLibraryHandle = static_cast<HINSTANCE>(nullptr);

#elif defined(__unix)

    public:
        using Symbol = void *;

    private:
        using LibraryHandle = void *;
        static constexpr const LibraryHandle NullLibraryHandle = nullptr;

#else

#error Unsupported platform.

#endif

    public:
        explicit SharedLibrary(const std::string & path) noexcept;
        SharedLibrary(const SharedLibrary & other) = delete;
        SharedLibrary(SharedLibrary && other) noexcept;
        void operator=(const SharedLibrary & other) = delete;
        SharedLibrary & operator=(SharedLibrary && other) noexcept;
        ~SharedLibrary();

        [[nodiscard]] inline bool isOpen() const
        {
            return NullLibraryHandle != m_lib;
        }

        bool open(const std::string & path);
        bool close();
        [[nodiscard]] bool hasSymbol(const std::string & sym) const;
        bool symbol(const std::string & sym, Symbol * receiver) const;

        [[nodiscard]] std::string lastError() const;

    private:
        LibraryHandle m_lib;
    };

}  // namespace LibQonvince

#endif  // LIBQONVINCE_SHAREDLIBRARY_H
