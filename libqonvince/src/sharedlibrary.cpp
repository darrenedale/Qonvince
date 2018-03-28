/**
 * \file sharedlibrary.cpp
 * \author Darren Edale
 * \version 0.5
 * \date September 2017
 *
 * \brief Implementation of the SharedLibrary class.
 */
#include "sharedlibrary.h"

#include <iostream>
#include <string>


#if defined(_WIN32)

#warning Building SharedLibrary class for Windows.

#include <vector>
#include <winbase.h>

namespace {
	/* convert UTF8-encoded std::string to std::wstring */
	inline std::wstring stringToWString(const std::string & str) {
		auto strLength = static_cast<int>(str.length()) + 1;
		auto bufferSize = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strLength, nullptr, 0);
		std::vector<wchar_t> buffer(static_cast<std::string::size_type>(bufferSize));
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), strLength, buffer.data(), bufferSize);
		return buffer.data();
	}
}  // namespace

#define LIBQONVINCE_SL_DLOPEN(path) LoadLibrary(static_cast<LPCWSTR>(stringToWString(path).c_str()))
#define LIBQONVINCE_SL_DLSYM(lib, sym) GetProcAddress((lib), (sym))
#define LIBQONVINCE_SL_DLCLOSE(lib) FreeLibrary((lib))
#define LIBQONVINCE_SL_DLERROR ""

#elif defined(__unix)

#warning Building SharedLibrary class for Unix.

#include <dlfcn.h>

#if !defined(NDEBUG)
/* for debugging LAZY binding can cause some inscrutable bugs
 * in some circumstances, so use NOW for this purpose */
#define LIBQONVINCE_SL_DLOPEN(path) dlopen((path).c_str(), RTLD_NOW)
#else
#define LIBQONVINCE_SL_DLOPEN(path) dlopen((path).c_str(), RTLD_LAZY)
#endif

#define LIBQONVINCE_SL_DLSYM(lib, sym) dlsym((lib), (sym))
#define LIBQONVINCE_SL_DLCLOSE(lib) (0 == dlclose((lib)))
#define LIBQONVINCE_SL_DLERROR dlerror()

#else

#error Unsupported platform.

#endif


namespace LibQonvince {

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
	SharedLibrary::SharedLibrary(const std::string & path)
	: m_lib(nullptr) {
#ifndef NDEBUG
		if(open(path)) {
			std::cerr << "successfully opened \"" << path << "\"\n";
		}
		else {
			std::cerr << "failed to open \"" << path << "\"\n";
		}
#else
		open(path);
#endif
	}


	/**
	 * \brief Move constructor.
	 *
	 * \param other The SharedLibrary from which to move.
	 *
	 * The constructed object takes the moved object's library handle.
	 * The moved-from object will act as if not open, regardless of whether
	 * it was open previously.
	 */
	SharedLibrary::SharedLibrary(SharedLibrary && other)
	: m_lib(other.m_lib) {
		other.m_lib = nullptr;
	}


	/**
	 * \brief Move constructor.
	 *
	 * \param other The SharedLibrary from which to move.
	 *
	 * This object takes the moved object's library handle. The moved-from
	 * object will act as if not open, regardless of whether it was open
	 * previously.
	 */
	SharedLibrary & SharedLibrary::operator=(SharedLibrary && other) {
		std::swap(m_lib, other.m_lib);
		return *this;
	}


	/** \brief Destructor.
	 *
	 * The library is closed if it was open and resources used by it are
	 * dispensed with.
	 */
	SharedLibrary::~SharedLibrary() {
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
	bool SharedLibrary::open(const std::string & path) {
		close();
		m_lib = LIBQONVINCE_SL_DLOPEN(path);
		return isOpen();
	}


	/** \brief Close the shared library.
	 *
	 * If the library is open, it is closed. It is safe to call this
	 * method if the library is not open - in such cases, the object
	 * will do nothing and report success.
	 *
	 * \return \b true if the library was closed, \b false if not.
	 */
	bool SharedLibrary::close() {
		bool ret = true;

		if(m_lib) {
			ret = LIBQONVINCE_SL_DLCLOSE(m_lib);
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
	bool SharedLibrary::hasSymbol(const std::string & sym) const {
		return (0 != LIBQONVINCE_SL_DLSYM(m_lib, sym.c_str()));
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
	bool SharedLibrary::symbol(const std::string & sym, Symbol * receiver) const {
		bool ret = false;
		Symbol myReceiver;
		myReceiver = LIBQONVINCE_SL_DLSYM(m_lib, sym.c_str());

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
	std::string SharedLibrary::lastError() const {
		return LIBQONVINCE_SL_DLERROR;
	}


};  // namespace LibQonvince

#undef LIBQONVINCE_SL_DLOPEN
#undef LIBQONVINCE_SL_DLSYM
#undef LIBQONVINCE_SL_DLCLOSE
#undef LIBQONVINCE_SL_DLERROR
