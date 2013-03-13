# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe

job01_tests: test_generic_worker_01.exe test_queue_01.exe test_ring_01.exe \
		test_padding_01.exe 

job01_core_tests: test_core__queue_01.exe test_core__link_01.exe

job01: job01_tests job01_core_tests

run_job01_tests: job01
	./build/Debug/test_generic_worker_01.exe
	./build/Debug/test_link_01.exe
	./build/Debug/test_queue_01.exe
	./build/Debug/test_ring_01.exe
	./build/Debug/test_padding_01.exe
	./build/Debug/test_core__queue_01.exe

run_tests: run_job01_tests

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

%.exe: job.xcodeproj
	xcodebuild -project job.xcodeproj -target $@
