TEMPLATE = lib
QT += core
CONFIG += c++14 plugin

include(../../../../common.pri)
include(../../../../libqonvince.pri)

LIBS += -L$$OUT_PWD/../../../../libqonvince/release -lqonvince

TARGET = eightdigits.displayplugin
SOURCES += src/eightdigits.cpp
HEADERS += src/eightdigits.h
