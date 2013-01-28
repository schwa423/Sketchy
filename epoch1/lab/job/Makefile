# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

#all: job_pool_00.exe generic_pool_00.exe

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe
job01: worker_01.exe

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

%.exe: job.xcodeproj
	xcodebuild -project job.xcodeproj -target $@
