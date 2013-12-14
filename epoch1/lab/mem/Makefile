# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: mem00_tests

mem00_tests: test_utils_00.exe test_align_00.exe

run_mem00_tests: mem00_tests
	./build/Debug/test_utils_00.exe
	./build/Debug/test_align_00.exe

run_tests: run_mem00_tests

dev_test: run_tests

mem.xcodeproj: mem.gyp
	gyp --depth=. -f xcode

%.exe: mem.xcodeproj
	xcodebuild -project mem.xcodeproj -target $@
