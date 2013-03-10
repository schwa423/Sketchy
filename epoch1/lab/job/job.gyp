{
  'includes': [
    '../common.gypi',
  ],
  'variables': {
    'COMMON_DEFINES': [
#      'BREAK_COMPILE=1',
    ],
  },
  'targets': [
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
    # TARGET test_link_01.exe
    {
      'target_name': 'test_link_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_link.cpp',
      ],
    },
    # TARGET test_queue_01.exe
    {
      'target_name': 'test_queue_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_queue.cpp',
      ],
    },
    # TARGET test_ring_01.exe
    {
      'target_name': 'test_ring_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_ring.cpp',
      ],
    },  
    # TARGET test_padding_01.exe
    {
      'target_name': 'test_padding_01.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'job01/tests/test_padding.cpp',
      ],
    },    
  ],
}