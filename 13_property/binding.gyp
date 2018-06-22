{
  "targets": [
    {
      "target_name": "native",
      "sources": ["native.cc"],
      "include_dirs": [
        "<!@(node -p \"require('jsni').include\")"
      ],
      'dependencies': ["./node_modules/jsni/binding.gyp:jsni"],
    }

  ]
}
