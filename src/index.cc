#include <nan.h>
#include "Agent.h"
#include "Solution.h"
#include "Factory.h"
#include "Machine.h"
#include "Matrix.h"

NAN_MODULE_INIT(InitModule) {
  Agent::Init(target);
  Solution::Init(target);
  Machine::Init(target);
  Factory::Init(target);
  Matrix::Init(target);
}

NODE_MODULE(agentaddon, InitModule);