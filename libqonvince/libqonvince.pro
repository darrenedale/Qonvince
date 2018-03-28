TEMPLATE = lib
QT       += core

lessThan(QT_MAJOR_VERSION, 5) {
    error("Qt version 5 is required")
}

TARGET = qonvince
CONFIG += c++14

*-g++ {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

*-clang {
    QMAKE_CXXFLAGS_RELEASE += -O3
}

*-msvc* {
    QMAKE_CXXFLAGS_RELEASE += /O3
}

win32 {
    LIBS += -lqca-qt5
}

unix {
    LIBS += -ldl \
            -L/usr/lib/x86_64-linux-gnu/ \
            -lqca-qt5 \

    INCLUDEPATH += /usr/include/Qca-qt5
    DEPENDPATH += /usr/include/Qca-qt5
}

SOURCES +=\
    src/sharedlibrary.cpp \
    src/otpdisplayplugin.cpp \

HEADERS  += \
    src/sharedlibrary.h \
    src/otpdisplayplugin.h \
    src/base32.h \
    src/plugininfo.h \

DISTFILES += \
    CMakeLists.txt \
