# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe

job01: test_generic_worker_01.exe test_link_01.exe test_queue_01.exe

test_job01: job01
	./build/Debug/test_generic_worker_01.exe
	./build/Debug/test_link_01.exe
	./build/Debug/test_queue_01.exe

test: test_job01

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

%.exe: job.xcodeproj
	xcodebuild -project job.xcodeproj -target $@
