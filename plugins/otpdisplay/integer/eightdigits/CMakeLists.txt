cmake_minimum_required(VERSION 3.1)

find_package(Qt5Core)

add_library(eightdigits_display_plugin SHARED src/eightdigits.cpp)

set_target_properties(eightdigits_display_plugin PROPERTIES
	CXX_STANDARD 14
	PREFIX ""
	SUFFIX ".displayplugin"
	OUTPUT_NAME "eightdigits"
	LIBRARY_OUTPUT_DIRECTORY ".."
	)

target_include_directories(eightdigits_display_plugin PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../../libqonvince/src)
target_link_libraries(eightdigits_display_plugin PUBLIC libqonvince_shared Qt5::Core)

if(WIN32)
	install(TARGETS eightdigits_display_plugin RUNTIME DESTINATION usr/share/Equit/Qonvince/plugins/otpdisplay COMPONENT extradisplayplugins OPTIONAL)
else()
	install(TARGETS eightdigits_display_plugin LIBRARY DESTINATION usr/share/Equit/Qonvince/plugins/otpdisplay COMPONENT extradisplayplugins OPTIONAL)
endif()
