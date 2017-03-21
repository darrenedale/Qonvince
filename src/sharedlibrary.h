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

#ifndef EQ_WEBFRAMEWORK_SHAREDLIBRARY_H
#define EQ_WEBFRAMEWORK_SHAREDLIBRARY_H

/** \file sharedlibrary.h
  * \author Darren Edale
  * \version 0.5
  * \date March 2015
  *
  * \brief Definition of the SharedLibrary class.
  */

#include <QString>

#if defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace Qonvince {
	class SharedLibrary {

#if defined(Q_OS_WIN)
		public:
			typedef FARPROC Symbol;

		private:
			typedef HINSTANCE LibraryHandle;
#elif defined(Q_OS_UNIX)
		public:
			typedef void * Symbol;

		private:
			typedef void * LibraryHandle;
#else
#error Unsupported platform.
#endif

		public:
			explicit SharedLibrary( const QString & path );
			virtual ~SharedLibrary( void );

			inline bool isOpen( void ) const {
				return !!m_lib;
			}

			bool open( const QString & path );
			bool close( void );
			bool hasSymbol( const QString & sym ) const;
			bool symbol( const QString & sym, Symbol * receiver ) const;

			QString lastError( void ) const;

		private:
			LibraryHandle m_lib;
	};
}

#endif
