QT       += core widgets
TARGET = qonvince
TEMPLATE = app
CONFIG += c++14

include(../common.pri)

WITH_NETWORK_ACCESS {
    QT += network
    DEFINES += WITH_NETWORK_ACCESS
}
else {
    DEFINES -= WITH_NETWORK_ACCESS
}

include(../libqonvince.pri)

LIBS += -lqca-qt5 -lqonvince

win32:CONFIG(release, debug|release) {
    LIBS += -L$$OUT_PWD/../libqonvince/release/ -lqonvince
}
else:win32:CONFIG(debug, debug|release) {
    LIBS += -L$$OUT_PWD/../libqonvince/debug/ -lqonvince
}

unix {
    LIBS += -L$$OUT_PWD/../libqonvince/ \
            -ldl \
            -L/usr/lib/x86_64-linux-gnu/

    INCLUDEPATH += /usr/include/Qca-qt5
}

macx {
    TARGET = Qonvince
}

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
    src/qtendianextra.h

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
