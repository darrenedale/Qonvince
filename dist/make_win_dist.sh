#! /usr/bin/env bash

# build zip file for standalone (i.e. no installer) qonvince on
# windows
BASEDIR=$(dirname "$(readlink -f "$0")")
VERSION=$(cat version.txt)
PLATFORM=mingw
ARCH=32
TMPDIR=/tmp


function usage() {
	echo >&2 "$(basename "$0"): create a stand-alone distribution zip archive for Windows."
	echo
	echo >&2 "Usage:"
	echo
	echo >&2 "$(basename "$0") <platform> <architecture> [<version]"
	echo
	echo >&2 "<platform>     is the build platform - either mingw or msvc2013. This is"
	echo >&2 "               currently very picky - it is case sensitive and must be one of the"
	echo >&2 "               two options specified above. Other options are possible and easy"
	echo >&2 "               to add, but I only have these two compilers set up and therefore"
	echo >&2 "               I only need these two options."
	echo
	echo >&2 "<architecture> is the CPU architecture. Currently only 32 and 64 are supported,"
	echo >&2 "               representing x86 32- and 64-bit architectures respectively. This"
	echo >&2 "               parameter is a little more tolerant - it will accept 32 and 64"
	echo >&2 "               both with and without the 'bit' suffix."
	echo
	echo >&2 "<version>      is the version of the program. This is used only to include in"
	echo >&2 "               the name of the created zip file. It can be anything you want, but"
	echo >&2 "               make sure it's suitable for use in a file name. If this argument"
	echo >&2 "               is absent, the content of version.txt will be used."
}


function removeTempDir() {
	rm -rf "${TMPDIR}/${DEST}"
}


if [ "msvc2013" == "$1" ]; then
	PLATFORM=$1
elif [ "mingw" == "$1" ]; then
	PLATFORM=mingw
else
	echo >&2 "Unrecognised platform '$1'."
	usage
	exit 1
fi

if [ "32" == "$2" ] || [ "32bit" == "$2" ] || [ "32-bit" == "$2" ]; then
	ARCH=32
elif [ "64" == "$2" ] || [ "64bit" == "$2" ] || [ "64-bit" == "$2" ]; then
	ARCH=64
else
	echo >&2 "Unrecognised architecture '$2'."
	usage
	exit 2
fi

if [ "z" != "z$3" ]; then
	VERSION="_$3"
fi

SOURCE="win-${PLATFORM}-${ARCH}bit"
DEST="qonvince${VERSION}"

cp -arf "${SOURCE}" "${TMPDIR}/${DEST}"

if [ 0 -ne $? ]; then
	echo "failed to create temporary directory."
	exit 3
fi

CWD=$(pwd)
cd "${TMPDIR}"
zip >/dev/null 2>&1 -r "${DEST}.zip" ${DEST}/*

if [ 0 -ne $? ]; then
	cd "${CWD}"
	echo >&2 "failed to create zip file."

	if [! removeTempDir ]; then
		echo >&2 "warning: temporary directory for zip file contents (${TMPDIR}/${DEST}) could not be removed."
	fi

	exit 4
fi

RET=0
cd "${CWD}"
mv "${TMPDIR}/${DEST}.zip" ./

if [ 0 -ne $? ]; then
	echo >&2 "failed to move created zip file to distribution directory."
	RET=5
fi

if [ !removeTempDir ]; then
	echo >&2 "warning: temporary directory for zip file contents (${TMPDIR}/${DEST}) could not be removed."
fi

exit ${RET}
