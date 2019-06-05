#include <nan.h>
#include "async.h"

using namespace v8;

using v8::FunctionTemplate;
using v8::Handle;
using v8::Object;
using v8::String;
using Nan::GetFunction;
using Nan::New;
using Nan::Set;

NAN_METHOD(Test) {
    printf("test");
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("test").ToLocalChecked(),
      Nan::GetFunction(Nan::New<FunctionTemplate>(Test)).ToLocalChecked());
  Nan::Set(target, New<String>("createSolution").ToLocalChecked(),
    GetFunction(New<FunctionTemplate>(CreateSolution)).ToLocalChecked());
}

NODE_MODULE(nativeaddon, Init)