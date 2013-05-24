# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe

job01_tests: test_boss_01.exe test_generic_worker_01.exe test_jobqueue_01.exe

job01_core_tests: test_core__link_01.exe test_core__padding_01.exe \
	test_core__queue_01.exe test_core__ring_01.exe

job01_mem_tests: test_job01_mem__blocks.exe

job01: job01_tests job01_core_tests job01_mem_tests

run_job01_tests: job01
	./build/Debug/test_boss_01.exe
	./build/Debug/test_generic_worker_01.exe
	./build/Debug/test_jobqueue_01.exe
	./build/Debug/test_core__link_01.exe
	./build/Debug/test_core__padding_01.exe
	./build/Debug/test_core__queue_01.exe
	./build/Debug/test_core__ring_01.exe
	./build/Debug/test_job01_mem__blocks.exe	

run_tests: run_job01_tests

# Run the thing we're currently developing.
dev_test: test_job01_mem__blocks.exe
	./build/Debug/test_job01_mem__blocks.exe

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

%.exe: job.xcodeproj
	xcodebuild -configuration Debug -project job.xcodeproj -target $@
