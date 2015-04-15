{
  'conditions': [
    ['OS=="mac"', {
      'variables': {
        'CCPATH': '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin',
        'CC_LD_FLAGS': [
          '-std=c++11',
          '-stdlib=libc++',
          '-I.',
        ],
      },
      'configurations': {
        'Debug': {
          'xcode_settings': {
            'OTHER_CFLAGS': [ '<@(CC_LD_FLAGS)', '-O0', '-DDEBUG=1'],
            'OTHER_LDFLAGS': [ '<@(CC_LD_FLAGS)', ],
          },
        },
        'Release': {
          'xcode_settings': {
            'OTHER_CFLAGS': [ '<@(CC_LD_FLAGS)', ],
            'OTHER_LDFLAGS': [ '<@(CC_LD_FLAGS)', ],
          },
        },
        'SDKROOT': 'macosx10.8',
      },
    }],
  ],
  # TODO: why doesn't this do anything?  (I had to copy into eg: job.gyp)
  'target_defaults': {
    'include_dirs': [
      '../job',
      '../mem',
    ],
    'defines': [
#      'BREAK_COMPILE=1',
    ],
    'conditions': [ ],
    'configurations': {
      'Debug': { },
      'Release': { },
    }
  },
}
