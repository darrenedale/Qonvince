#! /bin/bash
BASEDIR=$(dirname "$(readlink -f makepkgs.sh)")
cd "${BASEDIR}"

PKG_VERSION=1.8.1
PKG_RELEASE=2

echo Building package libqonvince ...
cd "${BASEDIR}"/libqonvince
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "glibc qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince ...
cd "${BASEDIR}"/qonvince
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "glibc libqonvince qt5-base qca-qt5 zbar" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince-displayplugins ...
cd "${BASEDIR}"/qonvince-displayplugins
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "libqonvince qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince-displayplugins-extra ...
cd "${BASEDIR}"/qonvince-displayplugins-extra
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "libqonvince qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

