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
    # TARGET benchmark_virtual.exe
    {
      'target_name': 'benchmark_virtual.exe',
      'type': 'executable',
      'defines': [ '<@(COMMON_DEFINES)', ],
      'sources': [
        'benchmark_virtual.cc',
      ],
    },
  ],
}