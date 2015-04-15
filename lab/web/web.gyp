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
    ],
  },
  'targets': [

    # schwa::web00 ============================================================

    # TARGET worker.asmjs
    {
      'target_name': 'test_web00_worker.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'web00/worker.cc',
      ],
    },
    # TARGET boss.asmjs
    {
      'target_name': 'test_web00_boss.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'web00/boss.cc',
      ],
    },
  ],
}