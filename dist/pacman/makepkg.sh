#!/bin/sh --
# Basic makepkg which just creates a package for qonvince
#
# just bail if any command fails for any reason.
set +o errexit

startdir=$PWD
pkgdir=$startdir/pkg
pkgname=$(basename "${startdir}")
pkgnamesuffix=-git
pkgurl=https://bitcraft.eu
pkgdesc=
pkgrel=1
arch=$(uname -m)
pkgver=0
pkgdepends=
pkgmakedepends=

while [ "" != "$1" ]; do
    case "$1" in
	    "--pkg-name")
		    shift
			pkgname=$1;
			;;

        "--pkg-description")
		    shift
			pkgdesc=$1;
			;;

        "--pkg-version")
		    shift
			pkgver=$1;
			;;

        "--pkg-release")
		    shift
			pkgrel="$1";
			;;

        "--pkg-arch")
		    shift
			arch="$1";
			;;

        "--pkg-subdir")
		    shift
			pkgdir="$startdir/$1";
			;;

        "--pkg-depends")
		    shift
			pkgdepends="${pkgdepends} $1"
			;;

        "--pkg-makedepends")
		    shift
			pkgmakedepends="${pkgmakedepends} $1"
			;;

		"--dist")
			pkgnamesuffix=
			;;
	esac

    shift
done

# check for invalid args
if ! [ -e "$pkgdir" ] || ! [ -d "$pkgdir" ]; then
    echo >&2 "Package directory \"$pkgdir\" does not exist."
	exit 1
fi

if ! [[ $pkgrel =~ ^[1-9][0-9]*$ ]]; then
    echo >&2 "Package release \"$pkgrel\" is not valid - it must be an integer > 0."
	exit 1
fi

if ! [[ $pkgname =~ ^[0-9a-z@_+][0-9a-z@_+\.\-]*$ ]]; then
    echo >&2 "Package name \"$pkgname\" is not valid - it must start with a-z, @, _ or + and be composed entirely of a-z, @, _, +, . and - characters."
	exit 1
fi

if ! [[ $pkgver =~ ^[0-9a-zA-Z\.]+$ ]]; then
    echo >&2 "Package version \"$pkgver\" is not valid - it must contain letters, digits . and - only."
	exit 1
fi

if [ "" != "$pkgdepends" ]; then
    for dep in $pkgdepends; do
	    if ! [[ $dep =~ ^[0-9a-z@_+][0-9a-z@_+\.\\-]*$ ]]; then
		    echo >&2 "Dependency name \"$dep\" is not valid - it must start with a-z, @, _ or + and be composed entirely of a-z, @, _, +, . and - characters."
			exit 1
		fi
	done
fi

if [ "" != "$pkgmakedepends" ]; then
    for dep in $pkgmakedepends; do
	    if ! [[ $dep =~ ^[0-9a-z@_+][0-9a-z@_+\.\-]*$ ]]; then
		    echo >&2 "Make dependency name \"$dep\" is not valid - it must start with a-z, @, _ or + and be composed entirely of a-z, @, _, +, . and - characters."
			exit 1
		fi
	done
fi

# Package.
cd "$pkgdir"
cat <<! > .PKGINFO
pkgname = ${pkgname}${pkgnamesuffix}
pkgver = $pkgver-$pkgrel
pkgdesc = $pkgdesc
url = $pkgurl
builddate = $(date -u +%s)
packager = Unknown Packager
size = $(du -sb --apparent-size "$pkgdir" | awk '{print $1}')
arch = $arch
license = GPL3
conflict = $pkgname
provides = $pkgname
!

if [ "" != "$pkgdepends" ]; then
    for dep in $pkgdepends; do
	    echo "depend = $dep" | cat >>.PKGINFO
	done
fi

if [ "" != "$pkgmakedepends" ]; then
    for dep in $pkgmakedepends; do
	    echo "makedepend = $dep" | cat >>.PKGINFO
	done
fi

fakeroot -- env LANG=C bsdtar -czf .MTREE --format=mtree --options='!all,use-set,type,uid,gid,mode,time,size,md5,sha256,link' .PKGINFO *
fakeroot -- env LANG=C bsdtar -cf - .MTREE .PKGINFO * | xz -c -z - > "$startdir"/${pkgname}${pkgnamesuffix}-"$pkgver"-"$pkgrel"-"$arch".pkg.tar.xz

# Test.
cd "$startdir"
namcap ${pkgname}${pkgnamesuffix}-"$pkgver"-"$pkgrel"-"$arch".pkg.tar.xz
