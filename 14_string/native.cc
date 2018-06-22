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
#include <string.h>

void testUtf8(JSNIEnv* env, JSNICallbackInfo info) {
  JSValueRef value = JSNIGetArgOfCallback(env, info, 0);
  assert(JSNIIsString(env, value));
  const char* native_str = "hello, world!";
  const size_t str_len = strlen(native_str);

  JSValueRef str_val = JSNINewStringFromUtf8(env, native_str, str_len);
  assert(JSNIGetStringUtf8Length(env, str_val) == str_len);

  char get_str[str_len];
  JSNIGetStringUtf8Chars(env, str_val, get_str, str_len);
  for (size_t i = 0; i < str_len; i++) {
    assert(get_str[i] == native_str[i]);
  }

  JSNISetReturnValue(env, info, str_val);
}


int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "testUtf8", testUtf8);
  return JSNI_VERSION_2_0;
}
