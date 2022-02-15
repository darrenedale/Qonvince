#! /bin/bash
BASEDIR=$(dirname "$(readlink -f makepkgs.sh)")
cd "${BASEDIR}"

PKG_VERSION=$(cat $(realpath $(dirname "$0"))/version.txt)
PKG_RELEASE=2

echo Building package libqonvince ${PKG_VERSION} ...
cd "${BASEDIR}"/libqonvince
strip pkg/usr/lib/libqonvince.so.*
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "glibc qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince ${PKG_VERSION} ...
cd "${BASEDIR}"/qonvince
strip pkg/usr/bin/qonvince
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "glibc libqonvince-git qt5-base qca-qt5 zbar" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince-displayplugins ${PKG_VERSION} ...
cd "${BASEDIR}"/qonvince-displayplugins
strip pkg/usr/share/Equit/Qonvince/plugins/otpdisplay/*.displayplugin
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "libqonvince-git qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

echo Building package qonvince-displayplugins-extra ${PKG_VERSION} ...
cd "${BASEDIR}"/qonvince-displayplugins-extra
strip pkg/usr/share/Equit/Qonvince/plugins/otpdisplay/*.displayplugin
. "${BASEDIR}"/makepkg.sh --pkg-version $PKG_VERSION --pkg-release $PKG_RELEASE --pkg-depends "libqonvince-git qt5-base" --pkg-makedepends "cmake qt5-tools"

if [ 0 -eq $? ]; then
    echo "OK"
else
    echo "failed"
	exit 1
fi

cp *.pkg.tar.xz "${BASEDIR}"/

