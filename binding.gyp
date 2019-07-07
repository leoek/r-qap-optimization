{
  "targets": [
    {
      "target_name": "nativeaddon",
      "sources": [ "src/nativeaddon.cc", "src/async.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    },
    {
      "target_name": "agentaddon",
      "sources": [ "src/index.cc", "src/Agent.cc", "src/Solution.cc", "src/Fabric.cc", "src/Machine.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    }
  ]
}