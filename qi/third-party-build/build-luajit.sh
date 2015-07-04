#!/bin/bash

# Build fat luajit.a library for OS X and iOS armv8.
# Adapted from script provided by Raymond W. Ko:
# http://www.freelists.org/post/luajit/cross-compile-luajit-for-iOS-7,5

set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
echo "DIRECTOR = $DIR"

rm -rf include
rm -rf lib
mkdir -p include
mkdir -p lib

# download
mkdir -p src
cd src
if [ ! -d "luajit-2.0" ]
then
  git clone http://luajit.org/git/luajit-2.0.git
fi

cd luajit-2.0
# v2.1 generate bad JIT code and causes a crash on Windows
#git checkout v2.1

IXCODE=`xcode-select -print-path`
ISDK=$IXCODE/Platforms/iPhoneOS.platform/Developer
ISDKVER=iPhoneOS`xcrun --sdk iphoneos --show-sdk-version`.sdk
ISDKP=/usr/bin/

ISDKF="-arch armv7 -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="xcrun gcc -m32 -arch i386" CROSS="$ISDKP" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../../lib/libluajit_arm7.a

ISDKF="-arch armv7s -isysroot $ISDK/SDKs/$ISDKVER"
make clean
make HOST_CC="xcrun gcc -m32 -arch i386" CROSS="$ISDKP" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
cp src/libluajit.a ../../lib/libluajit_arm7s.a

#ISDKF="-arch arm64 -isysroot $ISDK/SDKs/$ISDKVER"
#make clean
#make HOST_CC="xcrun gcc -m32 -arch i386" CROSS="$ISDKP" TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
#cp src/libluajit.a ../../lib/libluajit_arm64.a

# copy includes
cp src/lua.hpp ../../include/

cp src/lauxlib.h ../../include/
cp src/lua.h ../../include/
cp src/luaconf.h ../../include/
cp src/lualib.h ../../include/
cp src/luajit.h ../../include/

# combine lib
cd ../../lib
lipo -create -output libluajit.a libluajit_arm7.a libluajit_arm7s.a
