# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

# Read preference about whether to build in Debug or Release mode.
BUILDCONFIG_FILE := .schwa_BUILDCONFIG
BUILDCONFIG := $(shell if [ -f ${BUILDCONFIG_FILE} ]; then cat ${BUILDCONFIG_FILE}; else echo Debug; fi)

all: job00 job01

job00: job_pool_00.exe generic_pool_00.exe

job01_tests: test_boss_01.exe test_generic_worker_01.exe test_jobqueue_01.exe

job01_impl_tests: test_job01_impl__jobpool.exe

job01_core_tests: test_core__link_01.exe test_job01_core__meta.exe \
	test_core__padding_01.exe test_core__queue_01.exe test_core__ring_01.exe

job01_mem_tests: test_job01_mem__blocks.exe test_job01_mem__ref.exe

job01_obj_tests: test_job01_obj__obj.exe

job01: job01_tests job01_impl_tests job01_core_tests job01_mem_tests job01_obj_tests

run_job01_tests: job01
	./build/${BUILDCONFIG}/test_boss_01.exe
	./build/${BUILDCONFIG}/test_generic_worker_01.exe
	./build/${BUILDCONFIG}/test_jobqueue_01.exe
	./build/${BUILDCONFIG}/test_core__link_01.exe
	./build/${BUILDCONFIG}/test_core__padding_01.exe
	./build/${BUILDCONFIG}/test_core__queue_01.exe
	./build/${BUILDCONFIG}/test_core__ring_01.exe
	./build/${BUILDCONFIG}/test_job01_core__meta.exe	
	./build/${BUILDCONFIG}/test_job01_mem__blocks.exe
	./build/${BUILDCONFIG}/test_job01_impl__jobpool.exe	

run_tests: run_job01_tests

# Run the thing we're currently developing.
dev_test: test_job01_obj__obj.exe
	./build/${BUILDCONFIG}/test_job01_obj__obj.exe
#dev_test: test_job01_mem__ref.exe
#	./build/${BUILDCONFIG}/test_job01_mem__ref.exe
#dev_test: test_job01_mem__blocks.exe test_job01_impl__jobpool.exe	
#   ./build/${BUILDCONFIG}/test_job01_mem__blocks.exe
#	./build/${BUILDCONFIG}/test_job01_impl__jobpool.exe

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
