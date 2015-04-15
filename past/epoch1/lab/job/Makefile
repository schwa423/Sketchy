# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

# Read preference about whether to build in Debug or Release mode.
BUILDCONFIG_FILE := .schwa_BUILDCONFIG
BUILDCONFIG := $(shell if [ -f ${BUILDCONFIG_FILE} ]; then cat ${BUILDCONFIG_FILE}; else echo Debug; fi)

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe

job01_tests: test_job01__boss.exe test_job01__generic_worker.exe \
	test_job01__jobqueue.exe test_job01__worker.exe

job01_examples: example_job01__threadloop_signalhandler.exe

job01_impl_tests: test_job01_impl__jobpool.exe

job01_core_tests: test_job01_core__link.exe test_job01_core__meta.exe \
	test_job01_core__padding.exe test_job01_core__queue.exe \
	test_job01_core__ring.exe

job01_mem_tests:

job01_obj_tests: test_job01_obj__obj.exe

job01_thread_tests: test_job01_thread__turntaker.exe 	test_job01_thread__threadloop.exe

job01: job01_tests job01_impl_tests job01_core_tests job01_mem_tests job01_obj_tests job01_thread_tests

run_job01_tests: job01
	./build/${BUILDCONFIG}/test_job01__boss.exe
	./build/${BUILDCONFIG}/test_job01__generic_worker.exe
	./build/${BUILDCONFIG}/test_job01__jobqueue.exe
	./build/${BUILDCONFIG}/test_job01_impl__jobpool.exe
	./build/${BUILDCONFIG}/test_job01_core__link.exe
	./build/${BUILDCONFIG}/test_job01_core__meta.exe
	./build/${BUILDCONFIG}/test_job01_core__padding.exe
	./build/${BUILDCONFIG}/test_job01_core__queue.exe
	./build/${BUILDCONFIG}/test_job01_core__ring.exe
	./build/${BUILDCONFIG}/test_job01_obj__obj.exe

run_tests: run_job01_tests

# Run the thing we're currently developing.
dev_test: test_job01_thread__threadloop.exe
	./build/${BUILDCONFIG}/test_job01_thread__threadloop.exe

job.xcodeproj: job.gyp
	gyp --depth=. -f xcode

# Subsequent invocations of "make" will compile and run in Debug mode.
Debug:
	`echo Debug > .schwa_BUILDCONFIG`

# Subsequent invocations of "make" will compile and run in Release mode.
Release:
	`echo Release > .schwa_BUILDCONFIG`

# Build an executable target.
%.exe: job.xcodeproj
	xcodebuild -configuration ${BUILDCONFIG} -project job.xcodeproj -target $@
