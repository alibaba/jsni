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

#define DEBUGV8 if (0)printf

// This function is just for testing the correctness of JSNI API.
int getNumHandlesInternal() {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  int num_handles = v8::HandleScope::NumberOfHandles(isolate);
  return num_handles;
}

void testLocalScope(JSNIEnv* env, JSNICallbackInfo info) {
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


  DEBUGV8("v8 begin\n");
  num_handles = 0;
  num_handles = getNumHandlesInternal();
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  {
    v8::EscapableHandleScope ehs(isolate);
    v8::Local<v8::Number> n(v8::Integer::New(isolate, 42));
    ehs.Escape(n);
  }
  assert(getNumHandlesInternal() == num_handles + 1);
  DEBUGV8("v8 end\n");


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

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "testLocalScope", testLocalScope);
  return JSNI_VERSION_2_0;
}
