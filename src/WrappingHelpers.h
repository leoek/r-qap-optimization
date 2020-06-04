#ifndef AGENT_ADDON_WRAPPING_HELPERS_H_
#define AGENT_ADDON_WRAPPING_HELPERS_H_

#include <nan.h>
#include "Solution.h"

std::vector<std::vector<int>> UnWrapPermutation(v8::Local<v8::Array>);
v8::Local<v8::Array> WrapPermutation(std::vector<std::vector<int>>);

#endif  // AGENT_ADDON_WRAPPING_HELPERS_H_
