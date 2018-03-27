QT       += core widgets

lessThan(QT_MAJOR_VERSION, 5) {
    error("Qt version 5 is required")
}

TARGET = qonvince
TEMPLATE = app
CONFIG += c++14

WITH_NETWORK_ACCESS {
    QT += network
    DEFINES += WITH_NETWORK_ACCESS
}
else {
    DEFINES -= WITH_NETWORK_ACCESS
}

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
            -lqca-qt5

    INCLUDEPATH += /usr/include/Qca-qt5
    DEPENDPATH += /usr/include/Qca-qt5
}

macx {
	TARGET = Qonvince
}

INCLUDEPATH += ../libqonvince/src

SOURCES +=\
	src/passwordwidget.cpp \
	src/passworddialogue.cpp \
	src/aboutdialogue.cpp \
	src/application.cpp \
	src/libqrencode.cpp \
	src/main.cpp \
	src/mainwindow.cpp \
	src/otpqrcodereader.cpp \
	src/qrcodecreator.cpp \
	src/qrcodereader.cpp \
	src/settings.cpp \
	src/settingswidget.cpp \
	src/otp.cpp \
	src/otpeditor.cpp \
	src/otpeditordialogue.cpp \
	src/otplistview.cpp \
	src/otplistitemdelegate.cpp \
	src/iconselectbutton.cpp \
	src/qtiostream.cpp \
	src/otpdisplaypluginchooser.cpp \
	src/otplistmodel.cpp \

HEADERS  += \
	src/aboutdialogue.h \
	src/algorithms.h \
	src/application.h \
	src/iconselectbutton.h \
	src/libqrencode.h \
	src/mainwindow.h \
	src/otpcodeitemdelegate.h \
	src/otpdisplaypluginchooser.h \
	src/otpeditordialogue.h \
	src/otpeditor.h \
	src/otp.h \
	src/otplistitemdelegate.h \
	src/otplistmodel.h \
	src/otplistview.h \
	src/otplistwidgetitem.h \
	src/otpqrcodereader.h \
	src/passworddialogue.h \
	src/passwordwidget.h \
	src/pluginfactory.h \
	src/qrcodecreator.h \
	src/qrcodereader.h \
	src/qtiostream.h \
	src/qtstdhash.h \
	src/settings.h \
	src/settingswidget.h \

FORMS += \
 	ui/aboutdialogue.ui \
 	ui/iconselectbutton.ui \
 	ui/mainwindow.ui \
 	ui/otpeditordialogue.ui \
 	ui/otpeditor.ui \
 	ui/passworddialogue.ui \
 	ui/passwordwidget.ui \
 	ui/settingswidget.ui \

RESOURCES += \
    resources/icons.qrc \

DISTFILES += \
    CMakeLists.txt \
    cmake/FindQCA.cmake \
    cmake/COPYING-CMAKE-SCRIPTS

