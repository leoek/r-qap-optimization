#include <nan.h>
#include "Agent.h"
#include "Solution.h"
#include "Fabric.h"
#include "Machine.h"

NAN_MODULE_INIT(InitModule) {
  Agent::Init(target);
  Solution::Init(target);
  Machine::Init(target);
  Fabric::Init(target);
}

NODE_MODULE(agentaddon, InitModule);