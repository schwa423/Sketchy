#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LIPO="xcrun -sdk iphoneos lipo"
STRIP="xcrun -sdk iphoneos strip"

SRCDIR=/tmp/luajit-2.0
DESTDIR=$DIR/luajit

ISDK=/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain
ISDKP=/usr/bin/

IPHONEOS_PLATFORM=`xcrun --sdk iphoneos --show-sdk-platform-path`
IPHONEOS_SYSROOT=`xcrun --sdk iphoneos --show-sdk-path`

IPHONESIMULATOR_PLATFORM=`xcrun --sdk iphonesimulator --show-sdk-platform-path`
IPHONESIMULATOR_SYSROOT=`xcrun --sdk iphonesimulator --show-sdk-path`

# download
if [ ! -d ${SRCDIR} ]
then
  mkdir -p ${SRCDIR}
  cd ${SRCDIR}
  git clone http://luajit.org/git/luajit-2.0.git .
  git checkout v2.1
else
  cd ${SRCDIR}
fi

rm -fr ${DESTDIR}/platforms
mkdir -p ${DESTDIR}/platforms
rm -fr ${DESTDIR}/lib
mkdir -p ${DESTDIR}/lib

echo "######### Building for iOS arm64"
make clean
ISDKF="-arch arm64 -isysroot $IPHONEOS_SYSROOT"
make HOST_CC="xcrun gcc -m32 -arch x86_64" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
if [ $? -ne 0 ]; then exit 1; fi
mkdir -p ${DESTDIR}/platforms/ios-arm64
mv ${SRCDIR}/src/libluajit.a ${DESTDIR}/platforms/ios-arm64

echo "######### Building for iOS armv7"
make clean
ISDKF="-arch armv7 -isysroot $IPHONEOS_SYSROOT"
make HOST_CC="xcrun gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
if [ $? -ne 0 ]; then exit 1; fi
mkdir -p ${DESTDIR}/platforms/ios-armv7
mv ${SRCDIR}/src/libluajit.a ${DESTDIR}/platforms/ios-armv7

echo "######### Building for iOS armv7s"
make clean
ISDKF="-arch armv7s -isysroot $IPHONEOS_SYSROOT"
make HOST_CC="xcrun gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
if [ $? -ne 0 ]; then exit 1; fi
mkdir -p ${DESTDIR}/platforms/ios-armv7s
mv ${SRCDIR}/src/libluajit.a ${DESTDIR}/platforms/ios-armv7s

# echo "######### Building for iOS armv8"
# make clean
# ISDKF="-arch armv8 -isysroot $IPHONEOS_SYSROOT"
# make HOST_CC="xcrun gcc -m64 -arch x86_64" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS
# if [ $? -ne 0 ]; then exit 1; fi
# mv "$SRCDIR"/src/libluajit.a "$DESTDIR"/libluajit-armv8.a

echo "######### Building for OS X x86_64"
make clean
make CC="xcrun gcc -m32 -arch x86_64" clean all
mkdir -p ${DESTDIR}/platforms/osx-x86_64
mv ${SRCDIR}/src/libluajit.a ${DESTDIR}/platforms/osx-x86_64

lipo ${DESTDIR}/platforms/*/libluajit.a -create -output ${DESTDIR}/lib/libluajit.a
rm -fr ${DESTDIR}/platforms

# $LIPO -create "$DESTDIR"/libluajit-*.a -output "$DESTDIR"/libluajit.a
# $STRIP -S "$DESTDIR"/libluajit.a
# $LIPO -info "$DESTDIR"/libluajit.a

# rm "$DESTDIR"/libluajit-*.a

# make clean
