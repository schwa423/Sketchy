# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: job_pool_00.exe generic_pool_00.exe

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

%.exe: job.xcodeproj
	xcodebuild -project job.xcodeproj -target $@
