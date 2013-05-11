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

    # TARGET test_boss_01.exe
    {
      'target_name': 'test_boss_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_boss.cpp',
        'job01/boss.cpp',
      ],
    },
    # TARGET test_generic_worker_01.exe
    {
      'target_name': 'test_generic_worker_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_generic_worker.cpp',
        'job01/generic_worker.cpp',
      ],
    },
    # TARGET test_jobqueue_01.exe
    {
      'target_name': 'test_jobqueue_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_jobqueue.cpp',
      ],
    },
 

    # schwa::job01::core ======================================================

    # TARGET test_core__link_01.exe
    {
      'target_name': 'test_core__link_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_link.cpp',
      ],
    },
    # TARGET test_core__padding_01.exe
    {
      'target_name': 'test_core__padding_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_padding.cpp',
      ],
    },  
    # TARGET test_core__queue_01.exe
    {
      'target_name': 'test_core__queue_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_queue.cpp',
      ],
    },
    # TARGET test_core__ring_01.exe
    {
      'target_name': 'test_core__ring_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/core/tests/test_ring.cpp',
      ],
    },
  ],
}