#!/bin/bash

# This script is useful for building "fat" Cap'n Proto libraries on OS X.
# It builds Cap'n Proto libraries for all specified platforms using the
# standard './configure; make' procedure, and using the 'lipo' tool to
# merge the resulting single-platform versions of each library into a
# single "fat" one.

echo "$(tput setaf 2)"
echo "###################################################################"
echo "# Preparing to build Cap'n Proto"
echo "# (Thanks Bennett Smith ...."
echo "#     https://gist.github.com/BennettSmith/9487468ae3375d0db0cc)"
echo "###################################################################"
echo "$(tput sgr0)"

# The results will be stored relative to the location
# where you stored this script, **not** relative to
# the location of the Cap'n Proto git repo.
PREFIX=`pwd`/capnproto
if [ -d ${PREFIX} ]
then
    rm -rf "${PREFIX}"
fi
mkdir -p "${PREFIX}/platform"

# A "YES" value will build the latest code from GitHub on the master branch.
# A "NO" value will use the 2.6.0 tarball downloaded from googlecode.com.
USE_GIT_MASTER=NO

CAPNPROTO_GIT_URL=https://github.com/sandstorm-io/capnproto.git
CAPNPROTO_GIT_DIRNAME=capnproto
CAPNPROTO_VERSION=0.5.2
CAPNPROTO_RELEASE_URL=https://github.com/sandstorm-io/capnproto/archive/v${CAPNPROTO_VERSION}.tar.gz
CAPNPROTO_RELEASE_DIRNAME=capnproto-${CAPNPROTO_VERSION}

BUILD_MACOSX_X86_64=YES

BUILD_I386_IOSSIM=NO
BUILD_X86_64_IOSSIM=NO

BUILD_IOS_ARMV7=NO
BUILD_IOS_ARMV7S=NO
BUILD_IOS_ARM64=YES

CAPNPROTO_SRC_DIR=/tmp/capnproto

# TODO(josh): perhaps concatenate 'uname -s' and 'uname -r'
DARWIN=darwin14.3.0

XCODEDIR=`xcode-select --print-path`
IOS_SDK_VERSION=`xcrun --sdk iphoneos --show-sdk-version`
MIN_SDK_VERSION=7.1

MACOSX_PLATFORM=${XCODEDIR}/Platforms/MacOSX.platform
MACOSX_SYSROOT=${MACOSX_PLATFORM}/Developer/MacOSX10.9.sdk

IPHONEOS_PLATFORM=`xcrun --sdk iphoneos --show-sdk-platform-path`
IPHONEOS_SYSROOT=`xcrun --sdk iphoneos --show-sdk-path`

IPHONESIMULATOR_PLATFORM=`xcrun --sdk iphonesimulator --show-sdk-platform-path`
IPHONESIMULATOR_SYSROOT=`xcrun --sdk iphonesimulator --show-sdk-path`

# Uncomment if you want to see more information about each invocation
# of clang as the builds proceed.
# CLANG_VERBOSE="--verbose"

CC=clang
CXX=clang

CFLAGS="${CLANG_VERBOSE} -DNDEBUG -g -O0 -pipe -fPIC -fcxx-exceptions"
CXXFLAGS="${CLANG_VERBOSE} ${CFLAGS} -std=c++11 -stdlib=libc++"

LDFLAGS="-stdlib=libc++"
LIBS="-lc++ -lc++abi"

echo "PREFIX ..................... ${PREFIX}"
echo "USE_GIT_MASTER ............. ${USE_GIT_MASTER}"
echo "CAPNPROTO_GIT_URL ........... ${CAPNPROTO_GIT_URL}"
echo "CAPNPROTO_GIT_DIRNAME ....... ${CAPNPROTO_GIT_DIRNAME}"
echo "CAPNPROTO_VERSION ........... ${CAPNPROTO_VERSION}"
echo "CAPNPROTO_RELEASE_URL ....... ${CAPNPROTO_RELEASE_URL}"
echo "CAPNPROTO_RELEASE_DIRNAME ... ${CAPNPROTO_RELEASE_DIRNAME}"
echo "BUILD_MACOSX_X86_64 ........ ${BUILD_MACOSX_X86_64}"
echo "BUILD_I386_IOSSIM .......... ${BUILD_I386_IOSSIM}"
echo "BUILD_X86_64_IOSSIM ........ ${BUILD_X86_64_IOSSIM}"
echo "BUILD_IOS_ARMV7 ............ ${BUILD_IOS_ARMV7}"
echo "BUILD_IOS_ARMV7S ........... ${BUILD_IOS_ARMV7S}"
echo "BUILD_IOS_ARM64 ............ ${BUILD_IOS_ARM64}"
echo "CAPNPROTO_SRC_DIR ........... ${CAPNPROTO_SRC_DIR}"
echo "DARWIN ..................... ${DARWIN}"
echo "XCODEDIR ................... ${XCODEDIR}"
echo "IOS_SDK_VERSION ............ ${IOS_SDK_VERSION}"
echo "MIN_SDK_VERSION ............ ${MIN_SDK_VERSION}"
echo "MACOSX_PLATFORM ............ ${MACOSX_PLATFORM}"
echo "MACOSX_SYSROOT ............. ${MACOSX_SYSROOT}"
echo "IPHONEOS_PLATFORM .......... ${IPHONEOS_PLATFORM}"
echo "IPHONEOS_SYSROOT ........... ${IPHONEOS_SYSROOT}"
echo "IPHONESIMULATOR_PLATFORM ... ${IPHONESIMULATOR_PLATFORM}"
echo "IPHONESIMULATOR_SYSROOT .... ${IPHONESIMULATOR_SYSROOT}"
echo "CC ......................... ${CC}"
echo "CFLAGS ..................... ${CFLAGS}"
echo "CXX ........................ ${CXX}"
echo "CXXFLAGS ................... ${CXXFLAGS}"
echo "LDFLAGS .................... ${LDFLAGS}"
echo "LIBS ....................... ${LIBS}"

while true; do
    read -p "Proceed with build? (y/n) " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

echo "$(tput setaf 2)"
echo "###################################################################"
echo "# Fetch Cap'n Proto source code"
echo "###################################################################"
echo "$(tput sgr0)"

(
    if [ -d ${CAPNPROTO_SRC_DIR} ]
    then
        rm -rf ${CAPNPROTO_SRC_DIR}
    fi

    cd `dirname ${CAPNPROTO_SRC_DIR}`

    if [ "${USE_GIT_MASTER}" == "YES" ]
    then
        git clone ${CAPNPROTO_GIT_URL}
    else
        if [ -d ${CAPNPROTO_RELEASE_DIRNAME} ]
        then
            rm -rf "${CAPNPROTO_RELEASE_DIRNAME}"
        fi
        curl --location ${CAPNPROTO_RELEASE_URL} --output ${CAPNPROTO_RELEASE_DIRNAME}.tar.gz
        tar xvf ${CAPNPROTO_RELEASE_DIRNAME}.tar.gz
        mv "${CAPNPROTO_RELEASE_DIRNAME}" "${CAPNPROTO_SRC_DIR}"
        rm ${CAPNPROTO_RELEASE_DIRNAME}.tar.gz

        # Remove the version of Google Test included with the release.
        # We will replace it with version 1.7.0 in a later step.
# XX    if [ -d "${CAPNPROTO_SRC_DIR}/gtest" ]
# XX    then
# XX        rm -r "${CAPNPROTO_SRC_DIR}/gtest"
# XX    fi
    fi

    echo "SCHWA: entering Cap'n Proto C++ directory"
    cd ${CAPNPROTO_SRC_DIR}/c++
    echo "SCHWA: ${PWD}"
    autoreconf -i
)

#  XX echo "$(tput setaf 2)"
#  XX echo "###################################################################"
#  XX echo "# Fetch Google Test & Prepare the Configure Script"
#  XX echo "#   (note: This section is lifted from autogen.sh)"
#  XX echo "###################################################################"
#  XX echo "$(tput sgr0)"
#  XX
#  XX (
#  XX    cd ${CAPNPROTO_SRC_DIR}
#  XX
#  XX    # Check that we're being run from the right directory.
#  XX    if test ! -f src/google/capnproto/stubs/common.h
#  XX    then
#  XX        cat >&2 << __EOF__
#  XX        Could not find source code.  Make sure you are running this script from the
#  XX        root of the distribution tree.
#  XX        __EOF__
#  XX        exit 1
#  XX    fi
#  XX
#  XX    # Check that gtest is present. Older versions of Cap'n Proto were stored in SVN
#  XX    # and the gtest directory was setup as an SVN external.  Now, Cap'n Proto is
#  XX    # stored in GitHub and the gtest directory is not included. The commands
#  XX    # below will grab the latest version of gtest. Currently that is 1.7.0.
#  XX    if test ! -e gtest
#  XX    then
#  XX        echo "Google Test not present.  Fetching gtest-1.7.0 from the web..."
#  XX        curl --location http://googletest.googlecode.com/files/gtest-1.7.0.zip --output gtest-1.7.0.zip
#  XX        unzip gtest-1.7.0.zip
#  XX        rm gtest-1.7.0.zip
#  XX        mv gtest-1.7.0 gtest
#  XX    fi
#  XX
#  XX    autoreconf -f -i -Wall,no-obsolete
#  XX    rm -rf autom4te.cache config.h.in~
#  XX )

###################################################################
# This section contains the build commands to create the native
# Cap'n Proto library for Mac OS X.  This is done first so we have
# a copy of the protoc compiler.  It will be used in all of the
# susequent iOS builds.
###################################################################

echo "$(tput setaf 2)"
echo "###################################################################"
echo "# x86_64 for Mac OS X"
echo "###################################################################"
echo "$(tput sgr0)"

if [ "${BUILD_MACOSX_X86_64}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/x86_64-mac "CC=${CC}" "CFLAGS=${CFLAGS} -arch x86_64" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch x86_64" "LDFLAGS=${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

###################################################################
# This section contains the build commands for each of the
# architectures that will be included in the universal binaries.
###################################################################

echo "$(tput setaf 2)"
echo "###########################"
echo "# i386 for iPhone Simulator"
echo "###########################"
echo "$(tput sgr0)"

if [ "${BUILD_I386_IOSSIM}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --with-external-capnp --build=x86_64-apple-${DARWIN} --host=i386-apple-${DARWIN} --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/i386-sim "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch i386 -isysroot ${IPHONESIMULATOR_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch i386 -isysroot ${IPHONESIMULATOR_SYSROOT}" LDFLAGS="-arch i386 -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

echo "$(tput setaf 2)"
echo "#############################"
echo "# x86_64 for iPhone Simulator"
echo "#############################"
echo "$(tput sgr0)"

if [ "${BUILD_X86_64_IOSSIM}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --with-external-capnp --build=x86_64-apple-${DARWIN} --host=x86_64-apple-${DARWIN} --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/x86_64-sim "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch x86_64 -isysroot ${IPHONESIMULATOR_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch x86_64 -isysroot ${IPHONESIMULATOR_SYSROOT}" LDFLAGS="-arch x86_64 -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

echo "$(tput setaf 2)"
echo "##################"
echo "# armv7 for iPhone"
echo "##################"
echo "$(tput sgr0)"

if [ "${BUILD_IOS_ARMV7}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --with-external-capnp --build=x86_64-apple-${DARWIN} --host=armv7-apple-${DARWIN} --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/armv7-ios "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch armv7 -isysroot ${IPHONEOS_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch armv7 -isysroot ${IPHONEOS_SYSROOT}" LDFLAGS="-arch armv7 -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

echo "$(tput setaf 2)"
echo "###################"
echo "# armv7s for iPhone"
echo "###################"
echo "$(tput sgr0)"

if [ "${BUILD_IOS_ARMV7S}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --with-external-capnp --build=x86_64-apple-${DARWIN} --host=armv7s-apple-${DARWIN} --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/armv7s-ios "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch armv7s -isysroot ${IPHONEOS_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch armv7s -isysroot ${IPHONEOS_SYSROOT}" LDFLAGS="-arch armv7s -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

echo "$(tput setaf 2)"
echo "##################"
echo "# arm64 for iPhone"
echo "##################"
echo "$(tput sgr0)"

if [ "${BUILD_IOS_ARM64}" == "YES" ]
then
    (
        cd ${CAPNPROTO_SRC_DIR}/c++
        make clean
        ./configure --with-external-capnp --build=x86_64-apple-${DARWIN} --host=arm --disable-shared --prefix=${PREFIX} --exec-prefix=${PREFIX}/platform/arm64-ios "CC=${CC}" "CFLAGS=${CFLAGS} -miphoneos-version-min=${MIN_SDK_VERSION} -arch arm64 -isysroot ${IPHONEOS_SYSROOT}" "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} -arch arm64 -isysroot ${IPHONEOS_SYSROOT}" LDFLAGS="-arch arm64 -miphoneos-version-min=${MIN_SDK_VERSION} ${LDFLAGS}" "LIBS=${LIBS}"
        make
        make install
    )
fi

echo "$(tput setaf 2)"
echo "###################################################################"
echo "# Create Universal Libraries and Finalize the packaging"
echo "###################################################################"
echo "$(tput sgr0)"

(
    cd ${PREFIX}/platform
    mkdir -p universal/lib/
    lipo ./*/lib/libcapnp.a -create -output universal/lib/libcapnp.a
    lipo ./*/lib/libcapnp-rpc.a -create -output universal/lib/libcapnp-rpc.a
    lipo ./*/lib/libcapnpc.a -create -output universal/lib/libcapnpc.a
    lipo ./*/lib/libkj.a -create -output universal/lib/libkj.a
    lipo ./*/lib/libkj-async.a -create -output universal/lib/libkj-async.a
)

(
    cd ${PREFIX}
    mkdir bin
    mkdir lib
    # TODO(josh): do this earlier so that it can be used for --with-external-capnp if
    # Cap'n Proto isn't already installed on this machine.
    cp platform/x86_64-mac/bin/* bin/
    cp -r platform/universal/lib/*.a lib/
    rm -rf platform
    lipo -info lib/libcapnp.a
    lipo -info lib/libcapnp-rpc.a
    lipo -info lib/libcapnpc.a
    lipo -info lib/libkj.a
    lipo -info lib/libkj-async.a
)

# XX if [ "${USE_GIT_MASTER}" == "YES" ]
# XX then
# XX     if [ -d "${PREFIX}-master" ]
# XX     then
# XX         rm -rf "${PREFIX}-master"
# XX     fi
# XX     mv "${PREFIX}" "${PREFIX}-master"
# XX else
# XX     if [ -d "${PREFIX}-${CAPNPROTO_VERSION}" ]
# XX     then
# XX         rm -rf "${PREFIX}-${CAPNPROTO_VERSION}"
# XX     fi
# XX     mv "${PREFIX}" "${PREFIX}-${CAPNPROTO_VERSION}"
# XX fi

echo Done!
