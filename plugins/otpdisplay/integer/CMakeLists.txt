cmake_minimum_required(VERSION 3.1)

find_package(Qt5Core)

add_library(sixdigits_display_plugin SHARED src/sixdigits.cpp)
add_library(eightdigits_display_plugin SHARED src/eightdigits.cpp)

set_target_properties(sixdigits_display_plugin eightdigits_display_plugin PROPERTIES
	CXX_STANDARD 14
	PREFIX ""
	SUFFIX ".displayplugin"
	LIBRARY_OUTPUT_DIRECTORY ".."
	)

set_target_properties(sixdigits_display_plugin PROPERTIES
	OUTPUT_NAME "sixdigits"
	)

set_target_properties(eightdigits_display_plugin PROPERTIES
	OUTPUT_NAME "eightdigits"
	)

target_include_directories(sixdigits_display_plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../libqonvince/src)
target_include_directories(eightdigits_display_plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../libqonvince/src)
target_link_libraries(sixdigits_display_plugin PUBLIC libqonvince_shared Qt5::Core)
target_link_libraries(eightdigits_display_plugin PUBLIC libqonvince_shared Qt5::Core)
