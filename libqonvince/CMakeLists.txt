cmake_minimum_required(VERSION 3.1)

set(libqonvince_sources src/sharedlibrary.cpp src/otpdisplayplugin.cpp)

add_library(libqonvince_shared SHARED ${libqonvince_sources})
add_library(libqonvince_static STATIC ${libqonvince_sources})

find_package(Qt5 COMPONENTS Core)

set_target_properties(libqonvince_shared libqonvince_static PROPERTIES
	CXX_STANDARD 14
	LIBRARY_OUTPUT_NAME qonvince
	ARCHIVE_OUTPUT_NAME qonvince
	VERSION 1.0.0
	SOVERSION 1
#	CXX_CLANG_TIDY clang-tidy-5.0
)

set_target_properties(libqonvince_shared PROPERTIES
	PROJECT_LABEL "LibQonvince (Shared)"
)

set_target_properties(libqonvince_static PROPERTIES
	PROJECT_LABEL "LibQonvince (Static)"
)

if(WIN32)
	target_link_libraries(libqonvince_shared Qt5::Core)
	target_link_libraries(libqonvince_static Qt5::Core)
	install(TARGETS libqonvince_shared RUNTIME DESTINATION "." LIBRARY DESTINATION "." COMPONENT libqonvince)
	install(TARGETS libqonvince_static ARCHIVE DESTINATION "." COMPONENT libqonvince)
else()
	target_link_libraries(libqonvince_shared Qt5::Core)
	target_link_libraries(libqonvince_static Qt5::Core)
	install(TARGETS libqonvince_shared LIBRARY DESTINATION usr/lib COMPONENT libqonvince)
	install(TARGETS libqonvince_static ARCHIVE DESTINATION usr/lib COMPONENT libqonvince)
endif()
