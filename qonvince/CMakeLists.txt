cmake_minimum_required(VERSION 3.1)

option(QONVINCE_QT4 "Build and link against Qt4" OFF)
option(QONVINCE_QT5 "Build and link against Qt5" ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

find_package(QCA REQUIRED)

set(qonvince_sources src/passwordwidget.cpp
	src/passworddialogue.cpp
	src/aboutdialogue.cpp
	src/application.cpp
	src/base32.cpp
	src/crypt.cpp
	src/libqrencode.cpp
	src/main.cpp
	src/mainwindow.cpp
	src/otpqrcodereader.cpp
	src/qrcodecreator.cpp
	src/qrcodereader.cpp
	src/settings.cpp
	src/settingswidget.cpp
	src/sharedlibrary.cpp
	src/otp.cpp
	src/otpdisplayplugin.cpp
	src/otpeditor.cpp
	src/otplistwidget.cpp
	src/otplistwidgetitem.cpp
	src/steamotpdisplayplugin.cpp
	src/integerotpdisplayplugin.cpp
	src/iconselectbutton.cpp)

if(QONVINCE_QT4)
	message("configuring build for Qt4")
	find_package(Qt4 REQUIRED COMPONENTS QtCore QtGui)
	add_executable(qonvince-qt4 ${qonvince_sources})
	target_link_libraries(qonvince-qt4 Qt4::QtCore Qt4::QtGui "${QCA_LIBRARY}" dl)

	set_target_properties(qonvince-qt4 PROPERTIES
		OUTPUT_NAME qonvince
		RUNTIME_OUTPUT_DIRECTORY qt4
		AUTOMOC ON
		AUTOUIC ON
		AUTOUIC_SEARCH_PATHS "ui"
		AUTORCC ON
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)
endif(QONVINCE_QT4)

if(QONVINCE_QT5)
	message("configuring build for Qt5")
	find_package(Qt5 REQUIRED COMPONENTS Core Widgets)
	add_executable(qonvince-qt5 ${qonvince_sources})
	target_link_libraries(qonvince-qt5 Qt5::Core Qt5::Widgets "${QCA_LIBRARY}" dl)

	set_target_properties(qonvince-qt5 PROPERTIES
		OUTPUT_NAME qonvince
		RUNTIME_OUTPUT_DIRECTORY qt5
		AUTOMOC ON
		AUTOUIC ON
		AUTOUIC_SEARCH_PATHS "ui"
		AUTORCC ON
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF)
endif(QONVINCE_QT5)
