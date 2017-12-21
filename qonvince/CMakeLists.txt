cmake_minimum_required(VERSION 3.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

option(WITH_NETWORK_ACCESS "include network access features (e.g. read remote QR code images)" ON)

# at least 5.7 for qOverload<>() (and probably more)
find_package(Qt5 5.7 REQUIRED COMPONENTS Core Widgets)
find_package(QCA REQUIRED)

set(QONVICE_QT_LIBS Qt5::Core Qt5::Widgets)

if(WITH_NETWORK_ACCESS)
	find_package(Qt5 5.7 COMPONENTS Network)
	set(QONVICE_QT_LIBS ${QONVICE_QT_LIBS} Qt5::Network)
endif(WITH_NETWORK_ACCESS)

add_executable(qonvince
	src/passwordwidget.cpp
	src/passworddialogue.cpp
	src/aboutdialogue.cpp
	src/application.cpp
	src/libqrencode.cpp
	src/main.cpp
	src/mainwindow.cpp
	src/otpqrcodereader.cpp
	src/qrcodecreator.cpp
	src/qrcodereader.cpp
	src/settings.cpp
	src/settingswidget.cpp
	src/otp.cpp
	src/otpeditor.cpp
	src/otpeditordialogue.cpp
	src/otplistview.cpp
	src/otplistitemdelegate.cpp
	src/iconselectbutton.cpp
	src/qtiostream.cpp
	src/otpdisplaypluginchooser.cpp
	src/otplistmodel.cpp
	resources/icons.qrc
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
target_link_libraries(qonvince ${QONVICE_QT_LIBS} ${QCA_LIBRARY} dl libqonvince_shared)
