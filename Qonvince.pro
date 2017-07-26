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
}

unix {
    LIBS += -ldl \
            -L/usr/lib/x86_64-linux-gnu/ \
            -lqca-qt5
}

INCLUDEPATH += /usr/include/Qca-qt5/QtCrypto
DEPENDPATH += /usr/include/Qca-qt5/QtCrypto

SOURCES +=\
    src/passwordwidget.cpp \
    src/passworddialogue.cpp \
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
    src/otplistwidgetitem.cpp \
    src/steamotpdisplayplugin.cpp \
    src/integerotpdisplayplugin.cpp

HEADERS  += \
    src/passwordwidget.h \
    src/passworddialogue.h \
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
    src/otp.h \
    src/otpdisplayplugin.h \
    src/otpeditor.h \
    src/otplistwidget.h \
    src/otplistwidgetitem.h \
    src/integerotpdisplayplugin.h \
    src/steamotpdisplayplugin.h

FORMS    += \
    ui/passwordwidget.ui \
    ui/passworddialogue.ui \
    ui/aboutdialogue.ui \
    ui/mainwindow.ui \
    ui/settingswidget.ui \
    ui/otpeditor.ui

RESOURCES += \
    icons.qrc

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

