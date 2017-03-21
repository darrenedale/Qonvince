#-------------------------------------------------
#
# Project created by QtCreator 2015-12-08T11:16:01
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

TARGET = Qonvince
TEMPLATE = app

# TODO msvc uses different command line options
QMAKE_CXXFLAGS += -O2 -std=c++11

win32 {
}

unix {
    LIBS += -ldl
}

SOURCES +=\
    src/passwordwidget.cpp \
    src/aboutdialogue.cpp \
    src/application.cpp \
    src/base32.cpp \
    src/crypt.cpp \
    src/libqrencode.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/otpqrcodereader.cpp \
    src/qrcodecreator.cpp \
    src/qrcodereader.cpp \
    src/settings.cpp \
    src/settingswidget.cpp \
    src/sharedlibrary.cpp \
    src/otp.cpp \
    src/otpdisplayplugin.cpp \
    src/otpeditor.cpp \
    src/otplistwidget.cpp \
    src/otplistwidgetitem.cpp

HEADERS  += \
    src/passwordwidget.h \
    src/aboutdialogue.h \
    src/application.h \
    src/base32.h \
    src/crypt.h \
    src/libqrencode.h \
    src/mainwindow.h \
    src/otpqrcodereader.h \
    src/qrcodecreator.h \
    src/qrcodereader.h \
    src/settings.h \
    src/settingswidget.h \
    src/sharedlibrary.h \
    src/integerotpcodedisplayplugin.h \
    src/otp.h \
    src/otpdisplayplugin.h \
    src/otpeditor.h \
    src/otplistwidget.h \
    src/otplistwidgetitem.h

FORMS    += \
    ui/passwordwidget.ui \
    ui/aboutdialogue.ui \
    ui/mainwindow.ui \
    ui/settingswidget.ui \
    ui/otpeditor.ui

RESOURCES += \
    icons.qrc

DEFINES += "QONVINCE_OTPCODE_SEED_CRYPT_KEY=0x5629f6d32a50003a"

DISTFILES += \
    dist/doc/qonvince/copyright.txt \
    dist/doc/qonvince/changelog.Debian \
    dist/man/qonvince.1 \
    dist/qonvince-qt5.desktop \
    dist/qonvince-qt4.desktop \
    dist/installer/config/config.xml \
    dist/installer/packages/com.equit.qonvince/meta/package.xml \
    dist/installer/packages/com.equit.qonvince/meta/license.txt \
    todo.txt \
    dist/gplv3.txt \
    Qonvince-Qt5.mkpx

