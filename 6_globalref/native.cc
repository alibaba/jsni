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

#include <assert.h>
#include <jsni.h>
#include <v8.h>

// Only for testing use.
void RequestGC() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  isolate->RequestGarbageCollectionForTesting(
    v8::Isolate::kFullGarbageCollection);
}

void testGlobal(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef num = JSNINewNumber(env, 100);
  JSGlobalValueRef num_global = JSNINewGlobalValue(env, num);

  JSValueRef num_from_global = JSNIGetGlobalValue(env, num_global);
  assert(JSNIIsNumber(env, num_from_global));
  double num_native = JSNIToCDouble(env, num_from_global);
  assert(num_native == 100);
  JSNISetReturnValue(env, info, num_from_global);

  JSNIDeleteGlobalValue(env, num_global);
}

void testGlobalGC(JSNIEnv* env, JSNICallbackInfo info) {
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

void testGCCallback(JSNIEnv* env, JSNICallbackInfo info) {
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

void testAcquireRelease(JSNIEnv* env, JSNICallbackInfo info) {
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

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "testGlobal", testGlobal);
  JSNIRegisterMethod(env, exports, "testGlobalGC", testGlobalGC);
  JSNIRegisterMethod(env, exports, "testGCCallback", testGCCallback);
  JSNIRegisterMethod(env, exports, "testAcquireRelease", testAcquireRelease);
  return JSNI_VERSION_2_1;
}
