cmake_minimum_required(VERSION 3.1)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

option(WITH_NETWORK_ACCESS "include network access features (e.g. read remote QR code images)" ON)
option(WITH_DBUS_NOTIFICATIONS "use org.freedesktop.Notifications DBus service for notifications, if available" ON)

# at least 5.7 for qOverload<>() (and probably more)
find_package(Qt5 5.7 REQUIRED COMPONENTS Core Widgets DBus)
find_package(QCA REQUIRED)
find_package(nlohmann_json 3.10 REQUIRED)

set(QONVICE_QT_LIBS Qt5::Core Qt5::Widgets Qt5::DBus)

if(WITH_NETWORK_ACCESS)
	find_package(Qt5 5.7 COMPONENTS Network)
	set(QONVICE_QT_LIBS ${QONVICE_QT_LIBS} Qt5::Network)
endif(WITH_NETWORK_ACCESS)

add_executable(qonvince
	src/passphrasedialogue.cpp
	src/changepassphrasedialogue.cpp
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
	src/functions.cpp
	src/otpmimedata.cpp
	resources/icons.qrc
	)

set_target_properties(qonvince PROPERTIES
	OUTPUT_NAME qonvince
	AUTOMOC ON
	AUTOUIC ON
	AUTOUIC_SEARCH_PATHS "ui"
	AUTORCC ON
	CXX_EXTENSIONS OFF
	)

add_dependencies(qonvince libqonvince_shared)
target_compile_features(qonvince PRIVATE cxx_std_20)

target_include_directories(qonvince PUBLIC "${CMAKE_CURRENT_LIST_DIR}/../libqonvince/src" "${QCA_INCLUDE_DIR}")
target_link_libraries(qonvince ${QONVICE_QT_LIBS} ${QCA_LIBRARY} nlohmann_json::nlohmann_json libqonvince_shared)

set(QONVINCE_ICON_INSTALL_DIR "usr/share/icons/hicolor")

install(TARGETS qonvince RUNTIME DESTINATION usr/bin COMPONENT qonvince)
install(FILES "../dist/qonvince.desktop" DESTINATION usr/share/applications
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ GROUP_EXECUTE GROUP_READ WORLD_EXECUTE WORLD_READ
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/application_icon.svg" DESTINATION "${QONVINCE_ICON_INSTALL_DIR}/scalable/apps"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	RENAME qonvince.svg
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/application_icon_32.png" DESTINATION "${QONVINCE_ICON_INSTALL_DIR}/32x32/apps"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	RENAME qonvince.png
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/application_icon_24.png" DESTINATION "${QONVINCE_ICON_INSTALL_DIR}/24x24/apps"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	RENAME qonvince.png
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/application_icon.png" DESTINATION "${QONVINCE_ICON_INSTALL_DIR}/128x128/apps"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	RENAME qonvince.png
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/doc/qonvince/copyright.txt" "../dist/doc/qonvince/changelog" "../dist/doc/qonvince/changelog.Debian" DESTINATION "usr/share/doc/qonvince"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	CONFIGURATIONS Release
	COMPONENT qonvince)
install(FILES "../dist/man/qonvince.1" DESTINATION "usr/share/man/man1"
	PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
	CONFIGURATIONS Release
	COMPONENT qonvince)
