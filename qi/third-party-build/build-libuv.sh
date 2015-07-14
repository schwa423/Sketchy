#!/bin/bash

. ./common-build-settings.sh

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LIPO="xcrun -sdk iphoneos lipo"
STRIP="xcrun -sdk iphoneos strip"

COMPONENT_NAME=libuv
PREFIX=`pwd`/${COMPONENT_NAME}

ISDK=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain
ISDKP=/usr/bin/

LIBUV_VERSION=1.6.1

RELEASE_URL=https://github.com/libuv/libuv/archive/v${LIBUV_VERSION}.tar.gz


SRCDIR=/tmp/qi-build/${COMPONENT_NAME}-${LIBUV_VERSION}
# TODO: ${DESTDIR} and ${PREFIX} are redundant
DESTDIR=$DIR/${COMPONENT_NAME}

# download
if [ ! -d ${SRCDIR} ]
then
  mkdir -p ${SRCDIR}
  cd /tmp/qi-build
  curl --location ${RELEASE_URL} --output libuv-v${LIBUV_VERSION}.tar.gz
  tar xvf libuv-v${LIBUV_VERSION}.tar.gz
  cd ${SRCDIR}
else
  cd ${SRCDIR}
fi

echo "about to run autogen.sh in " `pwd`
./autogen.sh
autoreconf -i

# TODO: necessary?  or will "platforms/" be auto-created if necessary?
rm -fr ${DESTDIR}/platforms
mkdir -p ${DESTDIR}/platforms

CC=clang
CXX=clang

CFLAGS="${CLANG_VERBOSE} -DNDEBUG -g -O0 -pipe -fPIC"
CXXFLAGS="${CLANG_VERBOSE} ${CFLAGS} -std=c++11 -stdlib=libc++ -fcxx-exceptions"

LDFLAGS="-stdlib=libc++"
LIBS="-lc++ -lc++abi"

# TODO(josh): perhaps concatenate 'uname -s' and 'uname -r'
DARWIN=darwin14.3.0

MIN_SDK_VERSION=7.1

IPHONEOS_PLATFORM=`xcrun --sdk iphoneos --show-sdk-platform-path`
IPHONEOS_SYSROOT=`xcrun --sdk iphoneos --show-sdk-path`

IPHONESIMULATOR_PLATFORM=`xcrun --sdk iphonesimulator --show-sdk-platform-path`
IPHONESIMULATOR_SYSROOT=`xcrun --sdk iphonesimulator --show-sdk-path`

# TODO: If we build this then we end up with two x86_64 libuv.a, and lipo refuses to put
#       them into the same archive.  Not sure why this seems to work in build-capnproto.sh.
# echo
# echo "#####################################################################"
# echo "######### Building for OS X x86_64"
# echo "#####################################################################"
# make clean
# ./configure --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platforms/x86_64-mac "CC=${CC}" "CFLAGS=${CFLAGS} -arch x86_64" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch x86_64" "LDFLAGS=${LDFLAGS}" "LIBS=${LIBS}"
# make clean
# make
# make install
# if [ $? -ne 0 ]; then exit 1; fi
# mkdir -p ${DESTDIR}/platforms/x86_64-mac
# mv ${SRCDIR}/src/libuv.a ${DESTDIR}/platforms/x86_64-mac


for ARCH in armv7 armv7s arm64 x86_64 i386;
do
echo
echo "#####################################################################"
echo "######### Building for iOS ${ARCH}"
echo "#####################################################################"
make clean
# For whatever reason, arm64 doesn't fit in the nice naming scheme
# (Perhaps there's a separate way to specify ARMv8-32bit?  Not that we care...)
if [ $ARCH = "arm64" ]; then
  HOST="arm"
else
  HOST="${ARCH}-apple-${DARWIN}"
fi
# All "arm" targets are real devices, all "non-arm" targets are simulators.
if [[ ${HOST} = arm* ]]; then
  TARGET_PLATFORM=${IPHONEOS_PLATFORM}
  TARGET_SYSROOT=${IPHONEOS_SYSROOT}
else
  TARGET_PLATFORM=${IPHONESIMULATOR_PLATFORM}
  TARGET_SYSROOT=${IPHONESIMULATOR_SYSROOT}
fi
./configure --build=x86_64-apple-${DARWIN} --host=${HOST} --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platforms/${ARCH}-ios "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch ${ARCH} -isysroot ${TARGET_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch ${ARCH} -isysroot ${TARGET_SYSROOT}" LDFLAGS="-arch ${ARCH} -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
make
make install
mkdir -p ${DESTDIR}/platforms/${ARCH}-ios
mv ${SRCDIR}/src/libuv.a ${DESTDIR}/platforms/${ARCH}-ios
done

echo
echo "#####################################################################"
echo "######### Create Universal Library"
echo "#####################################################################"
make clean
(
    cd ${DESTDIR}
    rm -fr lib/
    mkdir lib/
    lipo ./platforms/*/lib/libuv.a -create -output lib/libuv.a
    rm -fr platforms/
)
