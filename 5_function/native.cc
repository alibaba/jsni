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

void NativeFunction(JSNIEnv* env, JSNICallbackInfo info) {
  if (JSNIGetArgsLengthOfCallback(env, info) < 1) {
    // JSNIThrowRangeErrorException(env, "Arguments should be more than 1.");
  }
  JSValueRef val = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, val)) {
    // JSNIThrowErrorException(env, "Should be number.");
  }
  JSNISetReturnValue(env, info, val);
}

void newNativeFunction(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef func = JSNINewFunction(env, NativeFunction);
  JSNISetReturnValue(env, info, func);
}

void testIsFunction(JSNIEnv* env, JSNICallbackInfo info) {
  if (JSNIGetArgsLengthOfCallback(env, info) < 1) {
    // JSNIThrowRangeErrorException(env, "Arguments should be more than 0.");
  }
  JSValueRef val = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsFunction(env, val));
}

void callFunction(JSNIEnv* env, JSNICallbackInfo info) {
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

void getThis(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef this_value = JSNIGetThisOfCallback(env, info);
  JSNISetReturnValue(env, info, this_value);
}

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "newNativeFunction", newNativeFunction);
  JSNIRegisterMethod(env, exports, "testIsFunction", testIsFunction);
  JSNIRegisterMethod(env, exports, "callFunction", callFunction);
  JSNIRegisterMethod(env, exports, "getThis", getThis);
  return JSNI_VERSION_2_0;
}
