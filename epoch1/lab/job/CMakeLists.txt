cmake_minimum_required (VERSION 2.8.6)

project (schwa_job CXX)

# Engine-wide library properties
set (DYNAMIC_LIB_PROPS COMPILE_DEFINITIONS SCHWA_BUILD_DYNAMIC COMPILE_DEFINITIONS_DEBUG SCHWA_DEBUG DEBUG_POSTFIX _d)
set (STATIC_LIB_PROPS COMPILE_DEFINITIONS SCHWA_BUILD_STATIC COMPILE_DEFINITIONS_DEBUG SCHWA_DEBUG DEBUG_POSTFIX d)

# Assign the SCHWA_PLATFORM_X value, used throughout Schwa for consistency
if (ANDROID)
  set (SCHWA_PLATFORM_ANDROID ON CACHE BOOL "Schwa for Android")

  option (INSTALL_ANDROID_APP "Install just the libs for use by an android application" OFF)
endif (ANDROID)
if (UNIX)
  set (SCHWA_PLATFORM_POSIX ON CACHE BOOL "Schwa for unix platforms")
  if (APPLE)
    set (SCHWA_PLATFORM_OSX ON CACHE BOOL "Schwa for OSX")
    if (IOS)
      set (SCHWA_PLATFORM_IOS ON CACHE BOOL "Schwa for iOS")
    endif (IOS)
  endif (APPLE)
elseif (EMSCRIPTEN)
  set (SCHWA_PLATFORM_EMSCRIPTEN ON CACHE BOOL "Schwa for emscripten")
endif (UNIX)

# Schwa options
option (SCHWA_RUN_MEMCHECK "Run memory check operations on unit tests" OFF)
option (SCHWA_BUILD_DYNAMIC "Build dynamic libraries" OFF)
option (SCHWA_BUILD_STATIC "Build static libraries" ON)
option (SCHWA_64_BIT "Build 64-bit libraries (32-bit otherwise" ON)

# Change from ON/OFF to true/false (so we can substitute this value into header
# files).  Also, define SCHWA_32_BIT to be the opposite.
if (SCHWA_64_BIT)
set (SCHWA_64_BIT true)
set (SCHWA_32_BIT false)
else (SCHWA_64_BIT)
set (SCHWA_64_BIT false)
set (SCHWA_32_BIT true)
endif (SCHWA_64_BIT)

# Set compile flags
set (SCHWA_COMPILER_FLAGS "-std=c++11 -stdlib=libc++")
if (SCHWA_64_BIT)
set (SCHWA_COMPILER_FLAGS "${SCHWA_COMPILER_FLAGS} -m64")
else (SCHWA_64_BIT)
set (SCHWA_COMPILER_FLAGS "${SCHWA_COMPILER_FLAGS} -m32")
endif (SCHWA_64_BIT)

set (DYNAMIC_LIB_FLAGS "${DYNAMIC_LIB_FLAGS} ${SCHWA_COMPILER_FLAGS}")
set (STATIC_LIB_FLAGS "${STATIC_LIB_FLAGS} ${SCHWA_COMPILER_FLAGS}")
set (CMAKE_CFLAGS ${SCHWA_COMPILER_FLAGS})
set (CMAKE_CXX_FLAGS ${SCHWA_COMPILER_FLAGS})

# TODO: integrate somewhere
set (CMAKE_EXE_LINKER_FLAGS "-std=c++11 -lc++")


# Add compile flags to library properties.
set (DYNAMIC_LIB_PROPS ${DYNAMIC_LIB_PROPS} COMPILE_FLAGS ${DYNAMIC_LIB_FLAGS})
set (STATIC_LIB_PROPS ${STATIC_LIB_PROPS} COMPILE_FLAGS ${STATIC_LIB_FLAGS})

set (LIB_INSTALL_DIR lib)

# Build Schwa
add_subdirectory (job01)
