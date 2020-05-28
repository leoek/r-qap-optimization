## Emptry Wrapped Solution

```c

v8::Local<v8::Value> argv[argc] = {};
// get a local handle to the solution constructor function
Local<Function> constructorFunc = Nan::New(Solution::constructor)->GetFunction();
// create a new JS instance from arguments
Local<Object> _sol = Nan::NewInstance(constructorFunc, argc, argv).ToLocalChecked();
Solution * sol = Nan::ObjectWrap::Unwrap<Solution>(_sol);

```
