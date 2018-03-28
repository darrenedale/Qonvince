QT       += core
TEMPLATE = app
CONFIG += c++14

INCLUDEPATH += ../../qonvince/src
INCLUDEPATH += ../../libqonvince/src

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../libqonvince/release/ -lqonvince
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../libqonvince/debug/ -lqonvince
else:unix: LIBS += -L$$OUT_PWD/../../libqonvince/ -lqonvince
