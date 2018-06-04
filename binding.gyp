{
  "targets": [
    {
      "target_name": "jsni",
      "type": "static_library",
      "sources": ["src/jsni.cc"]
    },
    {
      "target_name": "nativeLoad",
      "sources": ["src/native_load.cc", "src/jsni-internal.cc"]
    }
  ]
}