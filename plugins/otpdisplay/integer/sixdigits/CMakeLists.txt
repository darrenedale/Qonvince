cmake_minimum_required(VERSION 3.1)

find_package(Qt5Core)

add_library(sixdigits_display_plugin SHARED src/sixdigits.cpp)

set_target_properties(sixdigits_display_plugin PROPERTIES
	CXX_STANDARD 14
	PREFIX ""
	SUFFIX ".displayplugin"
	OUTPUT_NAME "sixdigits"
	LIBRARY_OUTPUT_DIRECTORY ".."
	)

target_include_directories(sixdigits_display_plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../../libqonvince/src)
target_link_libraries(sixdigits_display_plugin PUBLIC libqonvince_shared Qt5::Core)

if(WIN32)
	install(TARGETS sixdigits_display_plugin RUNTIME DESTINATION usr/share/Equit/Qonvince/plugins/otpdisplay COMPONENT qonvince-displayplugins OPTIONAL)
else()
	install(TARGETS sixdigits_display_plugin LIBRARY DESTINATION usr/share/Equit/Qonvince/plugins/otpdisplay COMPONENT qonvince-displayplugins OPTIONAL)
endif()
