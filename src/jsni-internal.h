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

#ifndef SRC_V8_JSNI_INTERNAL_H_
#define SRC_V8_JSNI_INTERNAL_H_

#include "jsni.h"
#include "v8.h"

#include <vector>

namespace v8 {

struct V8_EXPORT JSNIEnvExt : public _JSNIEnv {
  static JSNIEnvExt* Create(Isolate* isolate);
  Isolate* GetIsolate();

  explicit JSNIEnvExt(Isolate* isolate);
  Isolate* const isolate_;
  // To push/pop local frame.
  std::vector<void*> stacked_local_scope;
  // For error check.
  int error_code;
  JSNIErrorInfo last_error_info;
  Persistent<Value> last_exception;
};

}  // namespace v8

#endif  // SRC_V8_JSNI_INTERNAL_H_

