set (JOB01_SRC
  boss.cpp
  generic_worker.cpp
  worker.cc
  worker2.cc
)

set (JOB01_OBJ_SRC
  obj/obj.cc
)

set (JOB01_THREAD_SRC
  thread/threadloop.cc
)

set (JOB01_HDR
  boss.h
  generic_worker.h
  jobdescription.h
  jobqueue.h
  jobqueue2.h
  worker.h
  worker2.h
)

set (JOB01_CORE_HDR
  core/link.h
  core/meta.h
  core/padding.h
  core/queue.h
  core/ring.h
  core/schwassert.h
  core/stack.h
)

set (JOB01_DBG_HDR
  dbg/dbg.h
)

set (JOB01_HOST_HDR
  host/host.h
)

set (JOB01_IMPL_HDR
  impl/boss_impl.h
  impl/job_impl.h
  impl/jobpool.h
  impl/typedjobx.h
)

set (JOB01_MEM_HDR
  mem/align.h
  mem/util.h
)

set (JOB01_OBJ_HDR
  obj/obj.h
  obj/obj_defs.h
  obj/obj_includes.h
)

set (JOB01_THREAD_HDR
  thread/synchronization.h
  thread/threadloop.h
)

set (JOB01_TIME_HDR
  time/time_util.h
)

set (JOB01_SRC ${JOB01_SRC} ${JOB01_OBJ_SRC} ${JOB01_THREAD_SRC})
set (JOB01_HDR
  ${JOB01_HDR}
  ${JOB01_CORE_HDR}
  ${JOB01_DBG_HDR}
  ${JOB01_HOST_HDR}
  ${JOB01_IMPL_HDR}
  ${JOB01_MEM_HDR}
  ${JOB01_OBJ_HDR}
  ${JOB01_THREAD_HDR}
  ${JOB01_TIME_HDR}
)

### TODO what is this for?
set (JOB01_INSTALL)

include_directories (../../mem)
include_directories (../)

### Dynamic Library
if (SCHWA_BUILD_DYNAMIC)
  message (FATAL_ERROR "dynamic build not supported")
endif (SCHWA_BUILD_DYNAMIC)

### Static Library
if (SCHWA_BUILD_STATIC)
  add_library (schwa_job01_s STATIC ${JOB01_SRC} ${JOB01_HDR})
  set_target_properties (schwa_job01_s PROPERTIES ${STATIC_LIB_PROPS})
#  target_link_libraries (schwa_job01_s ${JOB01_LIBS})
#  install (TARGETS schwa_job01_s DESTINATION ${LIB_INSTALL_DIR} COMPONENT job01)
#  install (FILES ${JOB01_HDR} DESTINATION include/job01 COMPONENT required)
  set (JOB01_INSTALL ${JOB01_INSTALL} schwa_job01_s)
else (SCHWA_BUILD_STATIC)
  message (FATAL_ERROR "SCHWA_BUILD_STATIC must be specified")
endif (SCHWA_BUILD_STATIC)

### Rule to install only job01
add_custom_target (job01_install
  COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=$<CONFIGURATION> -DCOMPONENT=required -P ${PROJECT_BINARY_DIR}/cmake_install.cmake
  COMMAND ${CMAKE_COMMAND} -DBUILD_TYPE=$<CONFIGURATION> -DCOMPONENT=job01 -P ${PROJECT_BINARY_DIR}/cmake_install.cmake
  DEPENDS ${JOB01_INSTALL}
)

add_subdirectory (tests)
