#!/bin/sh

set -e
set -x

host="$1"
flavour="$2"
INSTALL_DIR=install
shift 2

mkdir deps build

case "${host}" in
	(*-w64-mingw32)
		export CC=${host}-gcc
		export CXX=${host}-g++
		set -- \
			-D CMAKE_TOOLCHAIN_FILE=$(pwd)/CMakeModules/Toolchains/${host}.cmake \
			"$@"
		;;

	(i?86-linux-gnu)
		set -- \
			-D CMAKE_TOOLCHAIN_FILE=$(pwd)/CMakeModules/Toolchains/linux-i686.cmake \
			"$@"
		;;

	(native)
		;;

	(*)
		set +x
		echo "Error: don't know how to cross-compile for ${host} host"
		exit 1
		;;
esac

set -- -D CMAKE_BUILD_TYPE="$flavour" "$@"

( cd build && cmake \
	-D CMAKE_INSTALL_PREFIX=${INSTALL_DIR} \
	"$@" .. )
make -C build VERBOSE=1
make -C build install
( cd $(pwd)/build/${INSTALL_DIR} && find . -ls )
