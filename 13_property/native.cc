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
#include <stdio.h>

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

void DefineProperty(JSNIEnv* env, JSNICallbackInfo info) {
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

void DefineProperty2(JSNIEnv* env, JSNICallbackInfo info) {
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

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "define_property", DefineProperty);
  JSNIRegisterMethod(env, exports, "define_property_2", DefineProperty2);
  return JSNI_VERSION_2_0;
}
