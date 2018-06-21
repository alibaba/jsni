{
  "targets": [
    {
      "target_name": "test",
      "sources": ["test.cc"],
      "include_dirs": [
        "<!@(node -p \"require('../index').include\")"
      ],
      "libraries": [
        "<(module_root_dir)/../build/<!@(node -p \"process.config.target_defaults.default_configuration\")/jsni.a"
      ],
    }

  ]
}
