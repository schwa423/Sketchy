{
  'includes': [
    '../common.gypi',
  ],
  'variables': {
    'COMMON_DEFINES': [
#      'BREAK_COMPILE=1',
    ],
  },
  # TODO: Why can't I just put this in common.gypi ?
  'target_defaults': {
    'include_dirs': [
      '../job',
      '../mem',
    ],
  },
  'targets': [

    # schwa::job00 ============================================================

    # TARGET generic_pool_00.exe
    {
      'target_name': 'generic_pool_00.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job00/test_generic_pool.cpp',
      ],
    },
    # TARGET job_pool_00.exe
    {
      'target_name': 'job_pool_00.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job00/test_job_pool.cpp',
        'job00/job_pool.cpp',
      ],
    },


    # schwa::job01 ============================================================

    # TARGET test_job01__boss.exe
    {
      'target_name': 'test_job01__boss.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_boss.cpp',
        'job01/boss.cpp',
        'job01/worker.cc',
        'job01/obj/obj.cc',
      ],
    },
    # TARGET test_job01__generic_worker.exe
    {
      'target_name': 'test_job01__generic_worker.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_generic_worker.cpp',
        'job01/generic_worker.cpp',
      ],
    },
    # TARGET test_job01__jobqueue.exe
    {
      'target_name': 'test_job01__jobqueue.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_jobqueue.cpp',
      ],
    },
    # TARGET test_job01__worker.exe
    {
      'target_name': 'test_job01__worker.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_worker.cc',
        'job01/worker2.cc',
        'job01/thread/threadloop.cc',
      ],
    },
    # TARGET example_job01__threadloop_signalhandler.exe
    {
      'target_name': 'example_job01__threadloop_signalhandler.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/examples/example_threadloop_signalhandler.cc',
        'job01/thread/threadloop.cc',
      ],
    },


    # schwa::job01::impl ======================================================

    # TARGET test_job01_impl__jobpool.exe
    {
      'target_name': 'test_job01_impl__jobpool.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/impl/tests/test_jobpool.cc',
        'job01/obj/obj.cc',
      ],
    },


    # schwa::job01::core ======================================================

    # TARGET test_job01_core__link.exe
    {
      'target_name': 'test_job01_core__link.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_link.cpp',
      ],
    },
    # TARGET test_job01_core__padding.exe
    {
      'target_name': 'test_job01_core__padding.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_padding.cpp',
      ],
    },
    # TARGET test_job01_core__queue.exe
    {
      'target_name': 'test_job01_core__queue.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_queue.cpp',
      ],
    },
    # TARGET test_job01_core__ring.exe
    {
      'target_name': 'test_job01_core__ring.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_ring.cpp',
      ],
    },
    # TARGET test_job01_core__meta.exe
    {
      'target_name': 'test_job01_core__meta.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_meta.cc',
      ],
    },


    # schwa::job01::obj =======================================================

    # TARGET test_job01_obj__obj.exe
    {
      'target_name': 'test_job01_obj__obj.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/obj/tests/test_obj.cc',
        'job01/obj/obj.cc',
      ],
    },
  ],
}
