{
  "targets": [
    {
      "target_name": "nativeaddon",
      "sources": [ "src/nativeaddon.cc", "src/async.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}