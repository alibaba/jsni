{
  "targets": [
    {
      "target_name": "test",
      "sources": ["test.cc"],
      "include_dirs": [
        "<!@(node -p \"require('../index').include\")"
      ],
      'dependencies': ["../binding.gyp:jsni"],
      # "ldflags": [
      #   "-Wl,-rpath,'$$ORIGIN'"
      # ],
    }

  ]
}
