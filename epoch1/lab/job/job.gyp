{
  'targets': [

    # TARGET job00.exe
    {
      'target_name': 'job00.exe',
      'type': 'executable',
      'defines': [
        'FOOBAR=23',
      ],
      'sources': [
        'job00/job.cpp'
      ],
    },

    # TARGET job01.exe
    {
      'target_name': 'job01.exe',
      'type': 'executable',
      'defines': [
        'FOOBAR=13',
      ],
      'sources': [
        'job00/job.cpp'
      ],
    },


  ],
} 