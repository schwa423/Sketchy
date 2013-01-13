{
  'conditions': [
    ['OS=="mac"', {
      'variables': {
        'CCPATH': '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin',
      },
      'xcode_settings': {
        'OTHER_CFLAGS': [
          "-std=c++11",
          "-stdlib=libc++",
          "-I.",
        ],
        'OTHER_LDFLAGS': [
          "-std=c++11",
          "-stdlib=libc++",
          "-I.",
        ],
        'SDKROOT': 'macosx10.8',
      },
    }],
  ],
  'target_defaults': {
    'include_dirs': [ ],
    'defines': [ ],
    'conditions': [ ],
    'configurations': {
      'Debug': { },
      'Release': { },
    }
  },
}
