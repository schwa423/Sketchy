# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: benchmark_virtual.exe

run: all
	./build/Release/benchmark_virtual.exe

perf.xcodeproj: perf.gyp
	gyp --depth=. -f xcode

%.exe: perf.xcodeproj
	xcodebuild -configuration Release -project perf.xcodeproj -target $@
