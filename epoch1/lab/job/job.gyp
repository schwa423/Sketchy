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
  ],
}