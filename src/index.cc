#include <nan.h>
#include "Agent.h"
#include "Solution.h"

NAN_MODULE_INIT(InitModule) {
  Agent::Init(target);
  Solution::Init(target);
}

NODE_MODULE(agentaddon, InitModule);