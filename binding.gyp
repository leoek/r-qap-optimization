{
  "targets": [
    {
      "target_name": "nativeaddon",
      "sources": [ "src/nativeaddon.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}