// JavaScript Native Interface Release License.
//
// Copyright (c) 2015-2017 Alibaba Group. All rights reserved.
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

#include <jsni.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>
#include <cmath>

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

void BoolCheck(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  bool result = JSNIToCBool(env, arg);
  assert(!result);
  AssertHelper(env);
}

void NumberCheck(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);

  {
    double result = JSNIToCDouble(env, arg);
    assert(std::isnan(result));
    AssertHelper(env);
  }
}

void FunctionCheck(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  JSValueRef result = JSNICallFunction(env, arg, JSNINewNull(env), 0, nullptr);
  AssertHelper(env);
  assert(JSNIIsEmpty(env, result));
}

void ArrayCheck(JSNIEnv* env, JSNICallbackInfo info) {
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

void StringCheck(JSNIEnv* env, JSNICallbackInfo info) {
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

void ObjectCheck(JSNIEnv* env, JSNICallbackInfo info) {
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


int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "BoolCheck", BoolCheck);
  JSNIRegisterMethod(env, exports, "NumberCheck", NumberCheck);
  JSNIRegisterMethod(env, exports, "FunctionCheck", FunctionCheck);
  JSNIRegisterMethod(env, exports, "ArrayCheck", ArrayCheck);
  JSNIRegisterMethod(env, exports, "StringCheck", StringCheck);
  JSNIRegisterMethod(env, exports, "ObjectCheck", ObjectCheck);
  return JSNI_VERSION_2_0;
}
