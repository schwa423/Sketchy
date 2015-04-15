# Simple makefile for building from within Sublime Text.
# Gyp and XCode do the real work.

# Read preference about whether to build in Debug or Release mode.
BUILDCONFIG_FILE := .schwa_BUILDCONFIG
BUILDCONFIG := $(shell if [ -f ${BUILDCONFIG_FILE} ]; then cat ${BUILDCONFIG_FILE}; else echo Debug; fi)

all: web00

web00: test_web00_boss.exe test_web00_worker.exe


web.ninja: web.gyp
	gyp -DOS=mac --debug=all --depth=. -f ninja

# Subsequent invocations of "make" will compile and run in Debug mode.
Debug:
	`echo Debug > .schwa_BUILDCONFIG`

# Subsequent invocations of "make" will compile and run in Release mode.
Release:
	`echo Release > .schwa_BUILDCONFIG`

# Build an executable target.
#%.asmjs: web.ninja

%.exe: web.ninja
	ninja -C out/${BUILDCONFIG}
