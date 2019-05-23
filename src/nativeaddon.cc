#include <nan.h>

using namespace v8;

NAN_METHOD(Test) {
    printf("test");
}

NAN_MODULE_INIT(Init) {
  Nan::Set(target, Nan::New("test").ToLocalChecked(),
      Nan::GetFunction(Nan::New<FunctionTemplate>(Test)).ToLocalChecked());
}

NODE_MODULE(myaddon, Init)