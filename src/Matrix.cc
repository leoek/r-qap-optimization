#include "Matrix.h"

using namespace v8;
using namespace std;

Nan::Persistent<v8::FunctionTemplate> Matrix::constructor;

NAN_MODULE_INIT(Matrix::Init) {
  v8::Local<v8::FunctionTemplate> ctor = Nan::New<v8::FunctionTemplate>(Matrix::New);
  constructor.Reset(ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("Matrix").ToLocalChecked());

  // link our getters and setter to the object property
  // The matrix is read-only from js
  Nan::SetAccessor(ctor->InstanceTemplate(), Nan::New("matrix").ToLocalChecked(), Matrix::HandleGetters);

  Nan::SetPrototypeMethod(ctor, "getValue", _GetValue);

  target->Set(Nan::New("Matrix").ToLocalChecked(), ctor->GetFunction());
}

NAN_METHOD(Matrix::New) {
  // throw an error if constructor is called without new keyword
  if(!info.IsConstructCall()) {
    return Nan::ThrowError(Nan::New("Matrix::New - called without new keyword").ToLocalChecked());
  }

  // expect exactly 1 argument
  if(info.Length() != 1) {
    return Nan::ThrowError(Nan::New("Matrix::New - expected 1 argument").ToLocalChecked());
  }

  // create a new instance and wrap our javascript instance
  Matrix* self = new Matrix();
  self->Wrap(info.Holder());

  // initialize it's values
  vector<vector<int>> temp;
  if(info[0]->IsArray()) {
    Local<Array> jsArray = Local<Array>::Cast(info[0]);
    temp.reserve(jsArray->Length());
    for (unsigned int i = 0; i < jsArray->Length(); i++)
    {
      Handle<Value> val = jsArray->Get(i);
      vector<int> innerTemp;
      if (val->IsArray()){
        Local<Array> innerJsArray = Local<Array>::Cast(val);
        innerTemp.reserve(innerJsArray->Length());
        for (unsigned int j = 0; j < innerJsArray->Length(); j++){
          Handle<Value> innerVal = innerJsArray->Get(j);
          int numVal = innerVal->NumberValue();
          innerTemp.push_back(numVal);
        }
      }
      temp.push_back(innerTemp);
    }
  }
  self->matrix = temp;

  // return the wrapped javascript instance
  info.GetReturnValue().Set(info.Holder());
}

NAN_GETTER(Matrix::HandleGetters) {
  Matrix* self = Nan::ObjectWrap::Unwrap<Matrix>(info.This());

  std::string propertyName = std::string(*Nan::Utf8String(property));
  if (propertyName == "matrix"){
    v8::Local<v8::Array> jsArray = Nan::New<v8::Array>(self->matrix.size());
    for (size_t i = 0; i < self->matrix.size(); i++)
    {
      v8::Local<v8::Array> innerJsArray = Nan::New<v8::Array>(self->matrix.size());
      for (size_t k = 0; k < self->matrix[i].size(); k++){
        Nan::Set(innerJsArray, k, Nan::New<Number>(self->matrix[i][k]));
      }
      Nan::Set(jsArray, i, innerJsArray);
    }
    info.GetReturnValue().Set(jsArray);
  }
}

int Matrix::GetValue(int a, int b, bool check){
  // omit the out of bounds check for better performance
  if (check){
    if(a >= matrix.size() ) {
      Nan::ThrowError(Nan::New("Matrix::GetValue(int a, int b) a was out of bounds").ToLocalChecked());
    }
    if(b >= matrix.size()) {
      Nan::ThrowError(Nan::New("Matrix::GetValue(int a, int b) b was out of bounds").ToLocalChecked());
    }
  }
  return matrix[a][b];
}

int Matrix::GetValue(int a, int b){
  return GetValue(a, b, True);
}

NAN_METHOD(Matrix::_GetValue) {
  // unwrap this Solution
  Matrix * self = Nan::ObjectWrap::Unwrap<Matrix>(info.This());

  if(!info[0]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Matrix::GetValuie - expected argument 0 to be an integer").ToLocalChecked());
  }
  if(!info[1]->IsNumber()) {
    return Nan::ThrowError(Nan::New("Matrix::GetValue - expected argument 1 to be an integer").ToLocalChecked());
  }
  int a = info[0]->IntegerValue();
  int b = info[1]->IntegerValue();

  info.GetReturnValue().Set(self->GetValue(a, b));
}