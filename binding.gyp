{
  "targets": [
    {
      "target_name": "nativeaddon",
      "sources": [ "src/nativeaddon.cc", "src/async.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ]
    },
    {
      "target_name": "agentaddon",
      "sources": [ "src/index.cc", "src/Agent.cc", "src/Solution.cc", "src/Factory.cc", "src/Machine.cc", "src/helperFunctions.cc", "src/Matrix.cc", "src/WrappingHelpers.cc" ],
      "include_dirs": [ "<!(node -e \"require('nan')\")" ],
      "cflags": [ "-std=c++11", "-fexceptions" ],
      'cflags!': [ '-fno-exceptions' ],
      'cflags_cc!': [ '-fno-exceptions' ]
    }
  ]
}