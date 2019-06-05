#include <nan.h>
#include "async.h"  // NOLINT(build/include)

using v8::Function;
using v8::Local;
using v8::Number;
using v8::Value;
using Nan::AsyncQueueWorker;
using Nan::AsyncWorker;
using Nan::Callback;
using Nan::HandleScope;
using Nan::New;
using Nan::Null;
using Nan::To;

class SolutionWorker : public AsyncWorker {
 public:
  SolutionWorker(Callback *callback, int runs)
    : AsyncWorker(callback), runs(runs), estimate(0) {}
  ~SolutionWorker() {}

  // Executed inside the worker-thread.
  // It is not safe to access V8, or V8 data structures
  // here, so everything we need for input and output
  // should go on `this`.
  void Execute () {
    estimate = 1.5;
  }

  // Executed when the async work is complete
  // this function will be run inside the main event loop
  // so it is safe to use V8 again
  void HandleOKCallback () {
    HandleScope scope;

    Local<Value> argv[] = {
        Null()
      , New<Number>(estimate)
    };

    callback->Call(2, argv, async_resource);
  }

 private:
  int runs;
  double estimate;
};

// Asynchronous access to the `Estimate()` function
NAN_METHOD(CreateSolution) {
  int runs = To<int>(info[0]).FromJust();
  Callback *callback = new Callback(To<Function>(info[1]).ToLocalChecked());

  AsyncQueueWorker(new SolutionWorker(callback, runs));
}