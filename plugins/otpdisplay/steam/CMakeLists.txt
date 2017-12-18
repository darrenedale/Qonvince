cmake_minimum_required(VERSION 3.1)

find_package(Qt5Core)

add_library(steam_display_plugin SHARED src/steam.cpp)

set_target_properties(steam_display_plugin PROPERTIES
	CXX_STANDARD 14
	PREFIX ""
	SUFFIX ".displayplugin"
	OUTPUT_NAME "steam"
	LIBRARY_OUTPUT_DIRECTORY ".."
	)

target_include_directories(steam_display_plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../libqonvince/src)
target_link_libraries(steam_display_plugin PUBLIC libqonvince_shared Qt5::Core)
