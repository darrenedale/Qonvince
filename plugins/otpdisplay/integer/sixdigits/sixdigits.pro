TEMPLATE = lib
QT += core
CONFIG += c++14 plugin

include(../../../../common.pri)
include(../../../../libqonvince.pri)

LIBS += -L$$OUT_PWD/../../../../libqonvince/release -lqonvince

TARGET = sixdigits.displayplugin
SOURCES += src/sixdigits.cpp
HEADERS += src/sixdigits.h
