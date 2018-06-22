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

void throwTypeError(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, arg)) {
    JSNIThrowTypeErrorException(env, "Wrong parameter type.");
  }
}

void throwRangeError(JSNIEnv* env, JSNICallbackInfo info) {
  if (JSNIGetArgsLengthOfCallback(env, info) > 0) {
    JSNIThrowRangeErrorException(env, "Range is out of limit.");
  }
}

void throwError(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef arg = JSNIGetArgOfCallback(env, info, 0);
  if (!JSNIIsNumber(env, arg)) {
    JSNIThrowErrorException(env, "Error.");
  }
}

void hasPendingException(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef func = JSNIGetArgOfCallback(env, info, 0);
  JSNICallFunction(env, func, JSNINewNull(env), 0, NULL);
  assert(JSNIHasException(env) == true);
}

void clearPendingException(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef func = JSNIGetArgOfCallback(env, info, 0);
  JSNICallFunction(env, func, JSNINewNull(env), 0, NULL);
  assert(JSNIHasException(env) == true);
  JSNIClearException(env);
}

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "throwTypeError", throwTypeError);
  JSNIRegisterMethod(env, exports, "throwRangeError", throwRangeError);
  JSNIRegisterMethod(env, exports, "throwError", throwError);
  JSNIRegisterMethod(env, exports, "hasPendingException", hasPendingException);
  JSNIRegisterMethod(env, exports, "clearPendingException",
    clearPendingException);
  return JSNI_VERSION_2_0;
}
