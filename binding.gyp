{
  'targets': [
    {
      'target_name': 'jsni',
      'type': 'static_library',
      'sources': ['src/jsni.cc'],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '-std=c++11',
              '-stdlib=libc++'
            ],
          },
        }],
      ],
    },
    {
      'target_name': 'nativeLoad',
      'sources': ['src/native_load.cc', 'src/jsni-internal.cc'],
      'conditions': [
        ['OS=="mac"', {
          'xcode_settings': {
            'OTHER_CFLAGS': [
              '-std=c++11',
              '-stdlib=libc++'
            ],
          },
        }],
      ],
    }
  ]
}