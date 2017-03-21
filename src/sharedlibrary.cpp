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

/** \file sharedlibrary.cpp
 * \author Darren Edale
 * \version 0.5
 * \date March 2015
 * 
 * \brief Implementation of the SharedLibrary class.
 * 
 * \todo
 * 
 */
#include "sharedlibrary.h"

#include <QtGlobal>
#include <QDebug>
#include <QString>

#if defined(Q_OS_WIN)

#include <winbase.h>
#include <string>

#define _DLOPEN(path) LoadLibrary((LPCWSTR)((path).toStdWString().c_str()))
#define _DLSYM(lib, sym) GetProcAddress((lib), (sym))
#define _DLCLOSE(lib) FreeLibrary((lib))
#define _DLERROR ""

#elif defined(Q_OS_UNIX)

#include <dlfcn.h>

#if defined(Q_DEBUG)
/* for debugging LAZY binding can cause some inscrutable bugs
 * in some circumstances, so use NOW for this purpose */
#define _DLOPEN(path) dlopen((path).toUtf8().data(), RTLD_NOW)
#else
#define _DLOPEN(path) dlopen((path).toUtf8().data(), RTLD_LAZY)
#endif

#define _DLSYM(lib, sym) dlsym((lib), (sym))
#define _DLCLOSE(lib) (0 == dlclose((lib)))
#define _DLERROR dlerror()

#else

#error Unsupported platform.

#endif


using namespace Qonvince;


/** \brief Create a new SharedLibrary and open it.
 * 
 * \param path The library to open.
 *
 * The path can be either a full path to open a specific file as
 * a shared library, or just the name of the library to open in which
 * case it will be sought from the system library paths.
 *
 * In either case, the path should include the appropriate suffix for
 * the platform (.so on *nix, .dll on windows).
 */
SharedLibrary::SharedLibrary( const QString & path )
:	m_lib(nullptr) {
	if(open(path)) {
		qDebug() << "successfully opened" << path;
	}
	else {
		qDebug() << "failed to open " << path;
	}
}


/** \brief Destructor.
 * 
 * The library is closed if it was open and resources used by it are
 * dispensed with.
 */
SharedLibrary::~SharedLibrary( void ) {
	close();
}


/** \brief Open a shared library from a file on disk.
 * 
 * \param path The path to the file to open.
 * 
 * If the ShareLibrary object is currently open it is closed (even
 * if it is the same the library). An attempt is then made to open
 * the new library.
 * 
 * \return \b true if the library was opened, \b false otherwise.
 */
bool SharedLibrary::open( const QString & path ) {
	if(m_lib) {
		close();
	}

    m_lib = _DLOPEN(path);
	return !!m_lib;
}


/** \brief Close the shared library.
 * 
 * If the library is open, it is closed. It is safe to call this
 * method if the library is not open - in such cases, the object
 * will do nothing and report success.
 * 
 * \return \b true if the library was closed, \b false if not.
 */
bool SharedLibrary::close( void ) {
	bool ret = true;

	if(m_lib) {
		ret = _DLCLOSE(m_lib);
		m_lib = nullptr;
	}

	return ret;
}


/** \brief Check whether the library contains a named symbol.
 * 
 * \param sym The symbol to check.
 *
 * If the library is open it is checked for the named symbol.
 * 
 * \return \b true if the library is open and contains a matching
 * symbol, \b false otherwise.
 */
bool SharedLibrary::hasSymbol( const QString & sym ) const {
	return (0 != _DLSYM(m_lib, sym.toUtf8().data()));
}


/** \brief Retrieve a symbol from the library.
 * 
 * \param sym The symbol to retrieve.
 * \param receiver The Symbol object that will be assigned the symbol,
 * if found.
 * 
 * If the library is open, it will be searched for the symbol. If the
 * symbol is found, it will be placed into the receiver. If the library
 * is not open or the requested symbol is not found, the receiver will
 * be left unmodified.
 * 
 * It is safe, though somewhat pointless, to provide a null receiver.
 * Doing so will always result in the method returning \b false.
 * 
 * \return \b true if the symbol was found and placed in the receiver,
 * \b false otherwise.
 */
bool SharedLibrary::symbol( const QString & sym, Symbol * receiver ) const {
	bool ret = false;
	Symbol myReceiver;
	myReceiver = _DLSYM(m_lib, sym.toUtf8().data());

	if(myReceiver && receiver) {
		ret = true;
		*receiver = myReceiver;
	}

	return ret;
}


/** \brief Fetch a description of the last error.
 * 
 * The returned string pointer is statically allocated and must not
 * be freed by the calling code.
 * 
 * \note The last error is not available on all platforms.
 * 
 * \return The last error message.
 */
QString SharedLibrary::lastError( void ) const {
	return QString::fromUtf8(_DLERROR);
}

#undef _DLOPEN
#undef _DLSYM
#undef _DLCLOSE
#undef _DLERROR
