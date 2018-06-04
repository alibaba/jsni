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

#include <node.h>
#include "jsni.h"
#include "jsni-internal.h"

#include <dlfcn.h>
#include <string>
#include <unordered_map>

using namespace v8;
using namespace std;

unordered_map<int, string> jsni_version_map = {
  {JSNI_VERSION_1_0, "JSNI_VERSION_1_0"},
  {JSNI_VERSION_1_1, "JSNI_VERSION_1_1"},
  {JSNI_VERSION_2_0, "JSNI_VERSION_2_0"},
  {JSNI_VERSION_2_1, "JSNI_VERSION_2_1"},
};

void JSNIEnvGCCallback(const WeakCallbackInfo<JSNIEnvExt>& info) {
  JSNIEnvExt* jsni_env = reinterpret_cast<JSNIEnvExt*>(info.GetParameter());
  delete jsni_env;
}

void NativeLoad(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Local<Value> native_exports = args[0];
  Local<Value> filename_resolved = args[1];
  String::Utf8Value filename(filename_resolved);
  void* handle = dlopen(*filename, RTLD_LAZY);

  if (handle == nullptr) {
    char errmsg[1024];
    snprintf(errmsg,
             sizeof(errmsg),
             "Can not open native module: %s. %s",
             *filename, dlerror());
    isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, errmsg)));
    return;
  }

  // Call JSNIInit to Register native methods.
  typedef int (*JSNIInitFn) (void*, JSValueRef);
  void* ptr = nullptr;
  ptr = dlsym(handle, "JSNIInit");

  if (ptr != nullptr) {

    // Prepare JSNIEnv* env
    // env is set in global. First check, then use.
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> global = context->Global();
    Local<String> jsni_name = String::NewFromUtf8(
                              isolate,
                              "__jsni_env__", NewStringType::kNormal
                              ).ToLocalChecked();
    Local<External> jsni = global->Get(context, jsni_name).ToLocalChecked().As<External>();
    JSNIEnvExt* jsni_env = nullptr;
    if (jsni->IsUndefined()) {
      jsni_env = JSNIEnvExt::Create(isolate);
      Local<External> jsni_external = External::New(isolate, jsni_env);
      global->Set(jsni_name, jsni_external);
      Persistent<Value> jsni_persistent(isolate, jsni_external);
      jsni_persistent.SetWeak(jsni_env, JSNIEnvGCCallback, WeakCallbackType::kParameter);
    } else {
      jsni_env = reinterpret_cast<JSNIEnvExt*>(jsni->Value());
    }
    JSNIInitFn jsni_init = reinterpret_cast<JSNIInitFn>(ptr);
    JSValueRef exports = reinterpret_cast<JSValueRef>(*native_exports);
    int version = jsni_init(jsni_env, exports);
    if (version < JSNI_VERSION_2_0) {
      char errmsg[1024];
      const char* version_string;
      if (jsni_version_map.find(version) != jsni_version_map.end()) {
        version_string = jsni_version_map[version].c_str();
      } else {
        version_string = "UNKNOW VERSION";
      }
      snprintf(errmsg,
               sizeof(errmsg),
               "%s: Native module version mismatch. Expected >= %s, got %s.",
               *filename, jsni_version_map[JSNI_VERSION_2_0].c_str(), version_string);
      dlclose(handle);
      isolate->ThrowException(v8::Exception::Error(String::NewFromUtf8(isolate, errmsg)));
      return;
    }
  } else {
    printf("%s: Native module has no declared entry point: JSNIInit.", *filename);
    return;
  }
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "nativeLoad", NativeLoad);
}

NODE_MODULE(binding, init);