#! /bin/bash
BASEDIR=$(dirname "$(readlink -f "$0")")
cd "${BASEDIR}"

PKG_VERSION_FILE="${BASEDIR}/../version.txt"
PKG_RELEASE_FILE="${BASEDIR}/../package-release.txt"

PKG_VERSION=$(cat "${PKG_VERSION_FILE}")
PKG_RELEASE=$(cat "${PKG_RELEASE_FILE}")

while [ "" != "$1" ]; do
  case "$1" in
    "--bump-release")
      PKG_RELEASE=$((${PKG_RELEASE} + 1))
      echo >"${PKG_RELEASE_FILE}" ${PKG_RELEASE}
      ;;

    "--reset-release")
      PKG_RELEASE=1
      echo >"${PKG_RELEASE_FILE}" ${PKG_RELEASE}
      ;;

    *)
      echo >&2 Unrecognised argument "\"$1\""
  esac

  shift
done

echo Building package libqonvince ${PKG_VERSION}-${PKG_RELEASE} ...
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

echo Building package qonvince ${PKG_VERSION}-${PKG_RELEASE} ...
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

echo Building package qonvince-displayplugins ${PKG_VERSION}-${PKG_RELEASE} ...
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

echo Building package qonvince-displayplugins-extra ${PKG_VERSION}-${PKG_RELEASE} ...
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

