#include "WrappingHelpers.h"

using namespace v8;
using namespace std;

std::vector<std::vector<int>> UnWrapPermutation(Local<Array> jsArray){
  vector<vector<int>> permutationArray;
  for (unsigned int i = 0; i < jsArray->Length(); i++) {
    vector<int> innerPermutationArray;
    Local<Array> innerJsArray = Local<Array>::Cast(jsArray->Get(i));
    for (unsigned int k = 0; k < innerJsArray->Length(); k++) {
      Handle<Value> val = innerJsArray->Get(k);
      int numVal = val->NumberValue();
      innerPermutationArray.push_back(numVal);
    }
    permutationArray.push_back(innerPermutationArray);
  }
  return permutationArray;
}

Local<Array> WrapPermutation(std::vector<std::vector<int>> permutation){
  v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(permutation.size());
  for (size_t i = 0; i < permutation.size(); i++) {
    v8::Local<v8::Array> innerJsArray = Nan::New<v8::Array>(permutation.at(i).size());
    for (size_t k = 0; k < permutation.at(i).size(); k++) {
      Nan::Set(innerJsArray, k, Nan::New<Number>(permutation[i][k]));
    }
    Nan::Set(jsArray, i, innerJsArray);
  }
  return jsArray;
}
