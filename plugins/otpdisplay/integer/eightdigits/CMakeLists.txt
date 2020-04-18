cmake_minimum_required(VERSION 3.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 5.7 REQUIRED COMPONENTS Core)

add_library(eightdigits SHARED src/eightdigits.cpp)

set_target_properties(eightdigits PROPERTIES
        OUTPUT_NAME eightdigits
        PREFIX ""
        SUFFIX ".displayplugin"
        AUTORCC ON
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	)

add_dependencies(eightdigits libqonvince_shared)
target_include_directories(eightdigits PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../../../libqonvince/src)

target_link_libraries(eightdigits Qt5::Core libqonvince_shared)

if(WIN32)
    install(TARGETS eightdigits RUNTIME DESTINATION usr/share/Equit/Qonvince/plugins/otpdisplay COMPONENT qonvince-displayplugins)
else()
    install(TARGETS eightdigits LIBRARY DESTINATION usr/local/share/Equit/Qonvince/plugins/otpdisplay COMPONENT qonvince-displayplugins)
endif()
