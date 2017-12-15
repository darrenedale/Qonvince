cmake_minimum_required(VERSION 3.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# at least 5.7 for qOverload<>() (and probably more)
find_package(Qt5 5.7 REQUIRED COMPONENTS Core Widgets)
find_package(QCA REQUIRED)

add_executable(qonvince
	src/passwordwidget.cpp
	src/passworddialogue.cpp
	src/aboutdialogue.cpp
	src/application.cpp
	src/crypt.cpp
	src/libqrencode.cpp
	src/main.cpp
	src/mainwindow.cpp
	src/otpqrcodereader.cpp
	src/qrcodecreator.cpp
	src/qrcodereader.cpp
	src/settings.cpp
	src/settingswidget.cpp
	src/otp.cpp
	src/otpdisplayplugin.cpp
	src/otpeditor.cpp
	src/otplistwidget.cpp
	src/otplistwidgetitem.cpp
	src/steamotpdisplayplugin.cpp
	src/integerotpdisplayplugin.cpp
	src/iconselectbutton.cpp
	)

set_target_properties(qonvince PROPERTIES
	OUTPUT_NAME qonvince
	AUTOMOC ON
	AUTOUIC ON
	AUTOUIC_SEARCH_PATHS "ui"
	AUTORCC ON
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	)

add_dependencies(qonvince libqonvince_shared)
target_include_directories(qonvince PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../libqonvince/src")
target_link_libraries(qonvince Qt5::Core Qt5::Widgets "${QCA_LIBRARY}" dl libqonvince_shared)
