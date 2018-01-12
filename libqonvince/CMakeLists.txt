cmake_minimum_required(VERSION 3.1)

set(libqonvince_sources src/sharedlibrary.cpp src/otpdisplayplugin.cpp)

add_library(libqonvince_shared SHARED ${libqonvince_sources})
add_library(libqonvince_static STATIC ${libqonvince_sources})

find_package(Qt5 COMPONENTS Core)

set_target_properties(libqonvince_shared libqonvince_static PROPERTIES
	CXX_STANDARD 14
	LIBRARY_OUTPUT_NAME qonvince
	ARCHIVE_OUTPUT_NAME qonvince
	SOVERSION 1.0.0
#	CXX_CLANG_TIDY clang-tidy-5.0
)

set_target_properties(libqonvince_shared PROPERTIES
	PROJECT_LABEL "LibQonvince (Shared)"
)

set_target_properties(libqonvince_static PROPERTIES
	PROJECT_LABEL "LibQonvince (Static)"
)

# TODO -ldl is only required on unix-like OS
target_link_libraries(libqonvince_shared dl Qt5::Core)
target_link_libraries(libqonvince_static dl Qt5::Core)

install(TARGETS libqonvince_shared LIBRARY DESTINATION /lib/x86_64-linux-gnu COMPONENT libraries)
install(TARGETS libqonvince_static ARCHIVE DESTINATION /lib/x86_64-linux-gnu COMPONENT libraries)
