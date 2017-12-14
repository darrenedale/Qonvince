TEMPLATE = subdirs

SUBDIRS = qonvince \
          test

DISTFILES += \
    todo.txt \
    .gitignore \
    .clang-format \
    dist/doc/qonvince/copyright.txt \
    dist/doc/qonvince/changelog.Debian \
    dist/man/qonvince.1 \
    dist/qonvince-qt5.desktop \
    dist/qonvince-qt4.desktop \
    dist/installer/config/config.xml \
    dist/installer/packages/com.equit.qonvince/meta/package.xml \
    dist/installer/packages/com.equit.qonvince/meta/license.txt \
    dist/gplv3.txt \
    dist/Qonvince-Qt5.mkpx \
    CMakeLists.txt \
    libqonvince/CMakeLists.txt \
    tests/CMakeLists.txt

