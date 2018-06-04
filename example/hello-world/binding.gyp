{
  "targets": [
    {
      "target_name": "test",
      "sources": ["test.cc"],
      "include_dirs": [
        "<!@(node -p \"require('jsni').include\")"
      ],
      'dependencies': ["./node_modules/jsni/binding.gyp:jsni"],
      # "ldflags": [
      #   "-Wl,-rpath,'$$ORIGIN'"
      # ],
    }

  ]
}
