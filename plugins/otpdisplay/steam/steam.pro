TEMPLATE = lib
QT += core
CONFIG += c++14 plugin

include(../../../common.pri)
include(../../../libqonvince.pri)

LIBS += -L$$OUT_PWD/../../../libqonvince/release -lqonvince

TARGET = steam.displayplugin
SOURCES += src/steam.cpp
HEADERS += src/steam.h
DISTFILES += CMakeLists.txt
