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
    # TARGET test_utils_00.exe
    {
      'target_name': 'test_utils_00.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'mem00/tests/test_utils.cpp',
      ],
    },
    # TARGET test_align_00.exe
    {
      'target_name': 'test_align_00.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'mem00/tests/test_align.cpp',
      ],
    },
  ],
}