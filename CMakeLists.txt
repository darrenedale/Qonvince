cmake_minimum_required(VERSION 3.1)

project(Qonvince VERSION 1.8.0 DESCRIPTION "An OTP code generator" LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "/usr")

add_subdirectory(qonvince)
add_subdirectory(libqonvince)
add_subdirectory(plugins)
#add_subdirectory(tests)

# cpack config
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "DEB")
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

set(CPACK_PACKAGE_CONTACT "darren -at- equituk -dot- net")

set(CPACK_COMPONENTS_ALL application libraries displayplugins extradisplayplugins)

set(CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "Qonvince")
set(CPACK_COMPONENT_APPLICATION_DEPENDS "libraries")
set(CPACK_DEBIAN_APPLICATION_PACKAGE_DEPENDS "zbar-tools (>=0), libqt5gui5 (>=5), libqt5widgets5 (>=5), libqt5network5 (>=5),  libqt5core5a (>=5),  libqca-qt5-2 (>=2)")
set(CPACK_DEBIAN_APPLICATION_PACKAGE_RECOMMENDS "qonvince-displayplugins, libqrencode3 (>=3)")
set(CPACK_DEBIAN_APPLICATION_PACKAGE_SECTION "utilities")
set(CPACK_DEBIAN_APPLICATION_PACKAGE_REPLACES "qonvince-qt5, qonvince-qt4")

set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Support libraries for generating OTP codes")
set(CPACK_DEBIAN_LIBRARIES_PACKAGE_SECTION "libraries")

set(CPACK_COMPONENT_DISPLAYPLUGINS_DISPLAY_NAME "Display plugins")
set(CPACK_COMPONENT_DISPLAYPLUGINS_DESCRIPTION "Core OTP code display plugins for Qonvince")
set(CPACK_DEBIAN_DISPLAYPLUGINS_PACKAGE_SECTION "plugins")
set(CPACK_DEBIAN_DISPLAYPLUGINS_PACKAGE_ENHANCES "qonvince")

set(CPACK_COMPONENT_EXTRADISPLAYPLUGINS_DISPLAY_NAME "Optional Extra Display plugins")
set(CPACK_COMPONENT_EXTRADISPLAYPLUGINS_DESCRIPTION "Optional extra OTP code display plugins for Qonvince")
set(CPACK_DEBIAN_EXTRADISPLAYPLUGINS_PACKAGE_SECTION "plugins")
set(CPACK_DEBIAN_EXTRADISPLAYPLUGINS_PACKAGE_ENHANCES "qonvince")

set(CPACK_PACKAGE_VENDOR "Équit")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/dist/package-description.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/dist/gplv3.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "8")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Qonvince ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")


if(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/dist\\\\application_icon_32.png")
  set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\Qonvince.exe")
  set(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} An OTP code generator")
  set(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
  set(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
  set(CPACK_NSIS_CONTACT "me@my-personal-home-page.com")
  set(CPACK_NSIS_MODIFY_PATH ON)
else(WIN32 AND NOT UNIX)
  set(CPACK_STRIP_FILES "bin/qonvince")
  set(CPACK_SOURCE_STRIP_FILES "")
endif(WIN32 AND NOT UNIX)

set(CPACK_PACKAGE_EXECUTABLES "qonvince" "Qonvince")
include(CPack)
