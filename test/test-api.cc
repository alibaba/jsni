// JavaScript Native Interface Release License.
//
// Copyright (c) 2015-2018 Alibaba Group. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Alibaba Group nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <execinfo.h>
#include <cmath>

#include <v8.h>
#include "test-api.h"

// Only for testing use.
void RequestGC() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  isolate->RequestGarbageCollectionForTesting(
    v8::Isolate::kFullGarbageCollection);
}

void print_stack(int stack_count) {
  int count;
  void *stack[stack_count];
  char **symbols;

  count = backtrace(stack, stack_count);
  symbols = backtrace_symbols(stack, count);

  for (int i = 0; i < count; i++)
    puts(symbols[i]);

  free(symbols);
}

void AssertHelper(JSNIEnv* env) {
  if (!JSNIGetLastErrorInfo(env).error_code) {
    fprintf(stderr, "%s\n", "Error is expected!.");
    print_stack(3);
    assert(0);
  }

  if (JSNIGetLastErrorInfo(env).error_code) {
    fprintf(stderr, "%s\n", "Error is not expected!.");
    print_stack(3);
    assert(0);
  }
}

TEST(Version) {
  int version = JSNIGetVersion(env);
  assert(version >= JSNI_VERSION_2_0);
}

TEST(Array) {
  JSValueRef array = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsArray(env, array));
  assert(JSNIGetArrayLength(env, array) == 2);
  JSValueRef obj0 = JSNIGetArrayElement(env, array, 0);
  JSValueRef obj1 = JSNIGetArrayElement(env, array, 1);
  JSValueRef new_array = JSNINewArray(env, 2);
  JSNISetArrayElement(env, new_array, 0, obj0);
  JSNISetArrayElement(env, new_array, 1, obj1);
  JSNISetReturnValue(env, info, new_array);
}

TEST(Boolean) {
  JSValueRef bool_val = JSNINewBoolean(env, true);
  assert(JSNIIsBoolean(env, bool_val));
  bool native_val = JSNIToCBool(env, bool_val);
  assert(native_val);
  JSNISetReturnValue(env, info, bool_val);
}

TEST(BoolCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  bool result = JSNIToCBool(env, arg);
  assert(!result);
  AssertHelper(env);
}

TEST(NumberCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);

  {
    double result = JSNIToCDouble(env, arg);
    assert(std::isnan(result));
    AssertHelper(env);
  }
}

TEST(FunctionCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef result = JSNICallFunction(env, arg, JSNINewNull(env), 0, nullptr);
  AssertHelper(env);
  assert(JSNIIsEmpty(env, result));
}

TEST(ArrayCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  size_t result_len;
  {
    result_len = JSNIGetArrayLength(env, arg);
    assert(result_len == 0);
    AssertHelper(env);
  }

  {
    JSValueRef result = JSNIGetArrayElement(env, arg, 0);
    AssertHelper(env);
    assert(JSNIIsUndefined(env, result));
  }

  {
    JSNISetArrayElement(env, arg, 0, JSNINewUndefined(env));
    AssertHelper(env);
  }

  {
    JsTypedArrayType result = JSNIGetTypedArrayType(env, arg);
    assert(result == JsArrayTypeNone);
    AssertHelper(env);
  }

  {
    void* result = JSNIGetTypedArrayData(env, arg);
    assert(result == 0);
    AssertHelper(env);
  }
}

TEST(StringCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  size_t result_len;
  {
    result_len = JSNIGetStringUtf8Length(env, arg);
    assert(result_len == 0);
    AssertHelper(env);
  }

  {
    char copy[100];
    size_t result = JSNIGetStringUtf8Chars(env, arg, copy, result_len);
    assert(result == 0);
    AssertHelper(env);
  }
}

TEST(ObjectCheck) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);

  {
    JSValueRef result = JSNIGetProperty(env, arg, "test");
    AssertHelper(env);
    assert(JSNIIsUndefined(env, result));
  }

  {
    bool result = JSNISetProperty(env, arg, "test", JSNINewUndefined(env));
    assert(!result);
    AssertHelper(env);
  }

  {
    bool result = JSNIDeleteProperty(env, arg, "test");
    assert(!result);
    AssertHelper(env);
  }
}

TEST(NewError) {
  JSValueRef error = JSNINewError(env, "error!!!");
  assert(JSNIIsError(env, error));
  JSNIThrowErrorObject(env, error);
}

TEST(NewTypeError) {
  JSValueRef error = JSNINewTypeError(env, "type error!!!");
  assert(JSNIIsError(env, error));
  JSNIThrowErrorObject(env, error);
}

TEST(NewRangeError) {
  JSValueRef error = JSNINewRangeError(env, "range error!!!");
  assert(JSNIIsError(env, error));
  JSNIThrowErrorObject(env, error);
}

TEST(ThrowTypeError) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, arg)) {
    JSNIThrowTypeErrorException(env, "Wrong parameter type.");
  }
}

TEST(ThrowRangeError) {
  if (JSNIGetArgsLengthOfCallback(env, info) > 0) {
    JSNIThrowRangeErrorException(env, "Range is out of limit.");
  }
}

TEST(ThrowError) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, arg)) {
    JSNIThrowErrorException(env, "Error.");
  }
}

TEST(HasPendingException) {
  JSValueRef func = JSNIGetArgOfCallback(env, info, 0);
  JSNICallFunction(env, func, JSNINewNull(env), 0, NULL);
  assert(JSNIHasException(env) == true);
}

TEST(ClearPendingException) {
  JSValueRef func = JSNIGetArgOfCallback(env, info, 0);
  JSNICallFunction(env, func, JSNINewNull(env), 0, NULL);
  assert(JSNIHasException(env) == true);
  JSNIClearException(env);
}

TEST(NativeFunction) {
  if (JSNIGetArgsLengthOfCallback(env, info) < 1) {
    // JSNIThrowRangeErrorException(env, "Arguments should be more than 1.");
  }
  JSValueRef val = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, val)) {
    // JSNIThrowErrorException(env, "Should be number.");
  }
  JSNISetReturnValue(env, info, val);
}

TEST(NewNativeFunction) {
  JSValueRef func = JSNINewFunction(env, TestNativeFunction);
  JSNISetReturnValue(env, info, func);
}

TEST(IsFunction) {
  if (JSNIGetArgsLengthOfCallback(env, info) < 1) {
    // JSNIThrowRangeErrorException(env, "Arguments should be more than 0.");
  }
  JSValueRef val = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsFunction(env, val));
}

TEST(CallFunction) {
  if (JSNIGetArgsLengthOfCallback(env, info) < 1) {
    // JSNIThrowRangeErrorException(env, "Arguments should be more than 0.");
  }
  JSValueRef val = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsFunction(env, val)) {
    // JSNIThrowErrorException(env, "Should be function.");
  }
  JSValueRef arg = JSNINewNumber(env, 200);
  JSValueRef result = JSNICallFunction(env, val, JSNINewNull(env), 1, &arg);
  JSNISetReturnValue(env, info, result);
}

TEST(GetThis) {
  JSValueRef this_value = JSNIGetThisOfCallback(env, info);
  JSNISetReturnValue(env, info, this_value);
}

TEST(Global) {
  JSValueRef num = JSNINewNumber(env, 100);
  JSGlobalValueRef num_global = JSNINewGlobalValue(env, num);

  JSValueRef num_from_global = JSNIGetGlobalValue(env, num_global);
  assert(JSNIIsNumber(env, num_from_global));
  double num_native = JSNIToCDouble(env, num_from_global);
  assert(num_native == 100);
  JSNISetReturnValue(env, info, num_from_global);

  JSNIDeleteGlobalValue(env, num_global);
}

TEST(GlobalGC) {
  JSNIPushEscapableLocalScope(env);
  JSValueRef num = JSNINewNumber(env, 100);
  JSGlobalValueRef num_global = JSNINewGlobalValue(env, num);
  JSNIPopEscapableLocalScope(env, NULL);
  RequestGC();

  JSValueRef num_from_global = JSNIGetGlobalValue(env, num_global);
  assert(JSNIIsNumber(env, num_from_global));
  double num_native = JSNIToCDouble(env, num_from_global);
  assert(num_native == 100);

  JSNIDeleteGlobalValue(env, num_global);
  RequestGC();
}

int global_native_1 = 200;
void nativeGCCallback(JSNIEnv* env, void* info) {
  int get_global_native = *reinterpret_cast<int*>(info);
  assert(get_global_native == 200);
  global_native_1 += 1;
}

TEST(GCCallback) {
  JSNIPushEscapableLocalScope(env);
  JSValueRef str = JSNINewStringFromUtf8(env, "it's a string", -1);
  JSGlobalValueRef str_global = JSNINewGlobalValue(env, str);
  JSNISetGCCallback(env, str_global, &global_native_1, nativeGCCallback);
  JSNIDeleteGlobalValue(env, str_global);
  JSNIPopEscapableLocalScope(env, NULL);

  RequestGC();
  assert(global_native_1 == 201);
}

int global_native_2 = 200;
void nativeGCCallback_2(JSNIEnv* env, void* info) {
  int get_global_native = *reinterpret_cast<int*>(info);
  assert(get_global_native == 200);
  global_native_2 += 1;
}

TEST(AcquireRelease) {
  int version = JSNIGetVersion(env);
  // Keep compatible with old version of vm implementation of jsni.
  if (version >= JSNI_VERSION_2_1) {
    JSNIPushEscapableLocalScope(env);
    JSValueRef str = JSNINewStringFromUtf8(env, "it's a string", -1);
    JSGlobalValueRef str_global = JSNINewGlobalValue(env, str);
    JSNIAcquireGlobalValue(env, str_global);
    JSNISetGCCallback(env, str_global, &global_native_2, nativeGCCallback_2);
    JSNIReleaseGlobalValue(env, str_global);
    JSNIReleaseGlobalValue(env, str_global);
    JSNIPopEscapableLocalScope(env, NULL);

    RequestGC();
    assert(global_native_2 == 201);
  } else {
    printf("JSNI version of implementation should not be less than JSNI_VERSION_2_1\n");
  }
}

TEST(Hidden) {
  int field_int = 123;
  JSValueRef object = JSNINewObjectWithInternalField(env, 1);
  assert(JSNIInternalFieldCount(env, object) == 1);
  JSNISetInternalField(env, object, 0, reinterpret_cast<void*>(&field_int));
  void* field_ptr = JSNIGetInternalField(env, object, 0);
  int field_int_get = *reinterpret_cast<int*>(field_ptr);
  assert(field_int_get == field_int);
}

// This function is just for testing the correctness of JSNI API.
int getNumHandlesInternal() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  int num_handles = v8::HandleScope::NumberOfHandles(isolate);
  return num_handles;
}

TEST(LocalScope) {
  int num_handles = 0;
  JSValueRef obj;
  JSNIPushLocalScope(env);
  num_handles = getNumHandlesInternal();
  JSNIPopLocalScope(env);
  assert(getNumHandlesInternal() == num_handles);

  num_handles = 0;
  JSNIPushLocalScope(env);
  num_handles = getNumHandlesInternal();
  obj = JSNINewNumber(env, 100);
  JSNIPopLocalScope(env);
  assert(getNumHandlesInternal() == num_handles);


  // DEBUGV8("v8 begin\n");
  num_handles = 0;
  num_handles = getNumHandlesInternal();
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  {
    v8::EscapableHandleScope ehs(isolate);
    v8::Local<v8::Number> n(v8::Integer::New(isolate, 42));
    ehs.Escape(n);
  }
  assert(getNumHandlesInternal() == num_handles + 1);
  // DEBUGV8("v8 end\n");


  num_handles = 0;
  num_handles = getNumHandlesInternal();
  JSNIPushEscapableLocalScope(env);
  obj = JSNINewNumber(env, 200);
  assert(JSNIIsNumber(env, obj));
  JSValueRef save = JSNIPopEscapableLocalScope(env, obj);
  assert(getNumHandlesInternal() == num_handles + 1);

  isolate->RequestGarbageCollectionForTesting(
    v8::Isolate::kFullGarbageCollection);

  assert(JSNIIsNumber(env, save));
  JSNISetReturnValue(env, info, save);
}

TEST(Null) {
  JSValueRef null_val = JSNINewNull(env);
  assert(JSNIIsNull(env, null_val));
  JSNISetReturnValue(env, info, null_val);
}

TEST(NewNumber) {
  JSValueRef number_val = JSNINewNumber(env, 1);
  assert(JSNIIsNumber(env, number_val));
  JSNISetReturnValue(env, info, number_val);
}

TEST(IsNumber) {
  assert(JSNIIsNumber(env, JSNIGetArgOfCallback(env, info, 0)));
}

TEST(Number2Native) {
  JSValueRef double_val = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIToCDouble(env, double_val) == 100.1);
}

TEST(Int32) {
  JSValueRef number = JSNIGetArgOfCallback(env, info, 0);
  API_ASSERT(JSNIToInt32(env, number) == 100, "JSNIToInt32");
}

TEST(Uint32) {
  JSValueRef number = JSNIGetArgOfCallback(env, info, 0);
  API_ASSERT(JSNIToUint32(env, number) == 100, "JSNIToUint32");
}

TEST(Int64) {
  JSValueRef number = JSNIGetArgOfCallback(env, info, 0);
  API_ASSERT(JSNIToInt64(env, number) == 100, "JSNIToInt64");
}

TEST(Object) {
  JSValueRef obj = JSNINewObject(env);
  assert(JSNIIsObject(env, obj));
  assert(!JSNIIsEmpty(env, obj));

  JSValueRef property1 = JSNINewNumber(env, 100);
  JSNISetProperty(env, obj, "property1", property1);
  assert(JSNIHasProperty(env, obj, "property1"));

  JSValueRef get_property1 = JSNIGetProperty(env, obj, "property1");
  assert(JSNIIsNumber(env, get_property1));
  assert(JSNIToCDouble(env, get_property1) == 100);

  JSNIDeleteProperty(env, obj, "property1");
  assert(!JSNIHasProperty(env, obj, "property1"));
}

TEST(GetProto) {
  JSValueRef obj = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef proto = JSNIGetPrototype(env, obj);
  JSNISetReturnValue(env, info, proto);
}


int data = 100;

void getter(JSNIEnv* env, JSNICallbackInfo info) {
  int args_length = JSNIGetArgsLengthOfCallback(env, info);
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef g_this = JSNIGetThisOfCallback(env, info);
  int* callback_data_p = reinterpret_cast<int*>(
    JSNIGetDataOfCallback(env, info));
  assert(arg == NULL);
  assert(args_length == 0);
  JSValueRef str = JSNINewStringFromUtf8(env, "getter is set.", -1);
  JSNISetProperty(env, g_this, "getter", str);

  int get_data = *callback_data_p + 1;
  JSValueRef number = JSNINewNumber(env,
    static_cast<double>(get_data));
  JSNISetReturnValue(env, info, number);
}

void setter(JSNIEnv* env, JSNICallbackInfo info) {
  int args_length = JSNIGetArgsLengthOfCallback(env, info);
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef g_this = JSNIGetThisOfCallback(env, info);
  int* callback_data_p = reinterpret_cast<int*>(
    JSNIGetDataOfCallback(env, info));
  assert(arg != NULL);
  assert(args_length == 1);
  assert(*callback_data_p == 100);
  JSValueRef str = JSNINewStringFromUtf8(env, "setter is set.", -1);
  JSNISetProperty(env, g_this, "setter", str);

  double value = JSNIToCDouble(env, arg);
  *callback_data_p = static_cast<int>(value);
}

TEST(DefineProperty) {
  int version = JSNIGetVersion(env);
  assert(version >= JSNI_VERSION_2_0);

  JSValueRef obj = JSNIGetArgOfCallback(env, info, 0);

  JSNIPropertyAttributes attr = JSNINone;
  JSNIAccessorPropertyDescriptor accesor =
    {getter, setter, attr, reinterpret_cast<void*>(&data)};
  JSNIPropertyDescriptor des =
    {NULL, &accesor};

  JSNIDefineProperty(env, obj, "abc", des);
}

TEST(DefineProperty2) {
  int version = JSNIGetVersion(env);
  assert(version >= JSNI_VERSION_2_0);

  JSValueRef obj = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef value = JSNINewStringFromUtf8(env, "Hello world!", -1);
  JSNIPropertyAttributes attr = JSNINone;
  JSNIDataPropertyDescriptor data =
    {value, attr};
  JSNIPropertyDescriptor des = {&data, NULL};
  JSNIDefineProperty(env, obj, "abc_2", des);
}

TEST(Utf8) {
  JSValueRef value = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsString(env, value));
  const char* native_str = "hello, world!";
  const size_t str_len = strlen(native_str);

  JSValueRef str_val = JSNINewStringFromUtf8(env, native_str, str_len);
  assert(JSNIGetStringUtf8Length(env, str_val) == str_len);

  char get_str[str_len + 1];
  size_t copied_size = JSNIGetStringUtf8Chars(env, str_val, get_str, str_len + 1);
  assert(copied_size = str_len);
  for (size_t i = 0; i < str_len + 1; i++) {
    assert(get_str[i] == native_str[i]);
  }

  JSNISetReturnValue(env, info, str_val);
}

TEST(String) {
  const uint16_t src[] = {0x0068, 0x0065, 0x006C, 0x006C, 0x006F};
  size_t length = 5;
  JSValueRef string = JSNINewString(env, src, length);
  size_t string_length = JSNIGetStringLength(env, string);
  API_ASSERT(string_length == length, "JSNIGetStringLength");

  uint16_t copy_string[length + 1];
  size_t copied_length = JSNIGetString(env, string, copy_string, length + 1);
  API_ASSERT(copied_length == length, "JSNIGetString");
  for (size_t i = 0; i < length; i++) {
    API_ASSERT(copy_string[i] == src[i], "JSNIGetString");
  }
}

TEST(Symbol) {
  int argc = JSNIGetArgsLengthOfCallback(env, info);
  JSValueRef s;
  if (argc > 0) {
    JSValueRef str = JSNIGetArgOfCallback(env, info, 0);
    s = JSNINewSymbol(env, str);
  } else {
    s = JSNINewSymbol(env, NULL);
  }
  assert(JSNIIsSymbol(env, s));

  JSNISetReturnValue(env, info, s);
}

class External
{
 public:
  External(int a) {
    data = a;
  }
  ~External() {
  }
  int data;
};

TEST(CreateTypedArray) {
  uint8_t a[] = {1, 2, 3};
  uint8_t* new_array = static_cast<uint8_t*>(malloc(sizeof a));
  memcpy(new_array, a, sizeof a);
  JSValueRef js_typed_array =
    JSNINewTypedArray(env, JsArrayTypeUint8,
                      reinterpret_cast<char*>(new_array), sizeof a);

  assert(JSNIIsTypedArray(env, js_typed_array));
  assert(JSNIGetTypedArrayType(env, js_typed_array) == JsArrayTypeUint8);
  uint8_t* data =
    reinterpret_cast<uint8_t*>(JSNIGetTypedArrayData(env, js_typed_array));
  size_t len = JSNIGetTypedArrayLength(env, js_typed_array);
  assert(len = 3);
  assert(data[0] == 1);
  assert(data[1] == 2);
  assert(data[2] == 3);

  JSNISetReturnValue(env, info, js_typed_array);
}

TEST(IsArray) {
  JSValueRef check = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsArray(env, check));
}

TEST(IsExternailized) {
  External* ext = new External(123);
  JSNIPushLocalScope(env);
  (void)JSNINewTypedArray(env, JsArrayTypeUint8,
                          ext, sizeof ext);
  JSNIPopLocalScope(env);
  RequestGC();
  RequestGC();
  assert(ext->data == 123);
  delete ext;
}

TEST(Undefined) {
  JSValueRef undefined_val = JSNINewUndefined(env);
  assert(JSNIIsUndefined(env, undefined_val));
  JSNISetReturnValue(env, info, undefined_val);
}

TEST(Instance) {
  JSValueRef constr = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef instance = JSNINewInstance(env, constr, 0, nullptr);
  assert(JSNIInstanceOf(env, instance, constr));
}

TEST(NewTarget) {
  JSValueRef new_target = JSNIGetNewTarget(env, info);
  JSNISetReturnValue(env, info, new_target);
}

TEST(StrictEquals) {
  JSValueRef args_0 = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef args_1 = JSNIGetArgOfCallback(env, info, 1);
  JSValueRef args_2 = JSNIGetArgOfCallback(env, info, 2);
  assert(JSNIStrictEquals(env, args_0, args_1));
  assert(!JSNIStrictEquals(env, args_0, args_2));
}

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  SET_METHOD(Version);
  SET_METHOD(Array);
  SET_METHOD(Boolean);
  // ErrorInfo
  SET_METHOD(BoolCheck);
  SET_METHOD(NumberCheck);
  SET_METHOD(FunctionCheck);
  SET_METHOD(ArrayCheck);
  SET_METHOD(StringCheck);
  SET_METHOD(ObjectCheck);
  // Exception
  SET_METHOD(ThrowTypeError);
  SET_METHOD(ThrowRangeError);
  SET_METHOD(ThrowError);
  SET_METHOD(HasPendingException);
  SET_METHOD(ClearPendingException);
  // Error
  SET_METHOD(NewError);
  SET_METHOD(NewTypeError);
  SET_METHOD(NewRangeError);
  // Function
  SET_METHOD(NewNativeFunction);
  SET_METHOD(IsFunction);
  SET_METHOD(CallFunction);
  SET_METHOD(GetThis);
  // GlobalRef
  SET_METHOD(Global);
  SET_METHOD(GlobalGC);
  SET_METHOD(GCCallback);
  SET_METHOD(AcquireRelease);
  // InternalField
  SET_METHOD(Hidden);
  // LocalScope
  SET_METHOD(LocalScope);
  // Null
  SET_METHOD(Null);
  // Number
  SET_METHOD(NewNumber);
  SET_METHOD(IsNumber);
  SET_METHOD(Number2Native);
  SET_METHOD(Int32);
  SET_METHOD(Uint32);
  SET_METHOD(Int64);
  // Object
  SET_METHOD(Object);
  SET_METHOD(GetProto);
  // Property
  SET_METHOD(DefineProperty);
  SET_METHOD(DefineProperty2);
  // String
  SET_METHOD(Utf8);
  SET_METHOD(String);
  // Symbol
  SET_METHOD(Symbol);
  // TypedArray
  SET_METHOD(CreateTypedArray);
  SET_METHOD(IsArray);
  SET_METHOD(IsExternailized);
  // Undefined
  SET_METHOD(Undefined);
  // Instance
  SET_METHOD(Instance);
  // NewTarget
  SET_METHOD(NewTarget);
  // StrictEquals
  SET_METHOD(StrictEquals);

  return JSNI_VERSION_2_3;
}















