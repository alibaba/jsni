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

#include "jsni-env-ext.h"

#include <assert.h>
#include <cmath>
#include <limits>

#define LOG_E printf

#define CHECK(x) assert(x)
using namespace v8;

enum ERROR_CODE {
  NOERR,
  BOOERR,
  NUMERR,
  FUNCERR,
  ARRERR,
  STRERR,
  OBJERR,
  // Scope error
  SCOERR,
};

const char* error_messages[] =
               {"OK",
                "A Boolean value is expected",
                "A Number value is expected",
                "A Function value is expected",
                "An Array value is expected",
                "A String value is expected",
                "An Object value is expected",
                "LocalScope is out of range"
               };

namespace v8 {
/* ----------------------
   Implementation.
----------------------  */

class JSNI {
 public:
  static const int kDataIndex = 0;
  static const int kGetterIndex = 1;
  static const int kSetterIndex = 2;
  static const int kAccessorFieldCount = 3;
  // JSNINewGlobalValue is created with kInitialReferenceCount = 1.
  static const size_t kInitialReferenceCount = 1;

  class JSNITryCatch : public v8::TryCatch {
   public:
    explicit JSNITryCatch(JSNIEnvExt* env)
      : v8::TryCatch(env->isolate_), env_(env) {}
    ~JSNITryCatch() {
      if (HasCaught()) {
        env_->last_exception.Reset(env_->isolate_, Exception());
      }
    }

   private:
    JSNIEnvExt* env_;
  };

  class JSRef {
   public:
    static JSRef* New(JSNIEnv* env, JSValueRef ref) {
      Isolate* isolate = JSNI::GetIsolate(env);
      Local<Value> val = JSNI::ToV8LocalValue(ref);
      return new JSRef(isolate, val);
    }

    static void Delete(JSRef* ref) {
      if (ref == nullptr) {
        return;
      }
      if (ref->callback_ != nullptr) {
        ref->persistent_.SetWeak(ref, FakeGCCallback, WeakCallbackType::kParameter);
      } else {
        delete ref;
      }
    }

    // If SetWeak, to delete *this* should be delayed in the fininalize callback.
    // else, when count_ is zero or JSNIDeleteGlobalValue is called, delete *this*
    // immediately.
    void SetWeak(void* data, JSNIGCCallback callback) {
      data_ = data;
      callback_ = callback;
    }

    void* GetData() {
      return data_;
    }

    JSNIGCCallback GetCallback() {
      return callback_;
    }

    JSValueRef Get(JSNIEnv* env) {
      Isolate* isolate = JSNI::GetIsolate(env);
      if (persistent_.IsEmpty()) {
        return JSNI::ToJSNIValue(Local<Value>());
      } else {
        return JSNI::ToJSNIValue(Local<Value>::New(isolate, persistent_));
      }
    }

    size_t Count() {
      return count_;
    }

    size_t Ref() {
      return ++count_;
    }

    size_t UnRef() {
      if (count_ == 0) {
        LOG_E("Error: decreasing ref count when ref count is 0.\n");
        return 0;
      }
      if (--count_ == 0) {
        Delete(this);
      }
      return count_;
    }

   private:
    JSRef(Isolate* isolate, Local<Value> val)
         : persistent_(isolate, val),
           callback_(nullptr),
           count_(kInitialReferenceCount) {
    }

    ~JSRef() {
      persistent_.Reset();
    }

    static void FakeGCCallback(const WeakCallbackInfo<JSRef>& info) {
      Isolate* isolate = info.GetIsolate();
      JSNIEnv* env = JSNI::GetEnv(isolate);
      JSNI::JSRef* ref =
        reinterpret_cast<JSNI::JSRef*>(info.GetParameter());
      assert(ref != nullptr);
      JSNIGCCallback callback = ref->GetCallback();

      if (callback != nullptr) {
        callback(env, ref->GetData());
      }
      // Reset and delete the global.
      delete ref;
    }

    Persistent<Value> persistent_;
    void* data_;
    JSNIGCCallback callback_;
    size_t count_;
  };

  // TODO(jiny) FunctionCallback should use this JSNICallbackInfoWrap.
  class JSNICallbackInfoWrap {
   public:
    typedef enum {
      kDefault = 0,
      kFunction = kDefault,
      kGetterProperty = 1 << 1,
      kSetterProperty = 1 << 2,
      kWeakGC = 1 << 3,
    } CallbackInfoType;
    JSNICallbackInfoWrap(void* info, CallbackInfoType type)
                    :info_(info), type_(type) {}

    JSNICallbackInfoWrap(void* info, CallbackInfoType type, JSValueRef value)
                    :info_(info), type_(type), value_(value) {}

    CallbackInfoType type() { return type_;}
    void* info() { return info_;}
    JSValueRef value() { return value_;}
   private:
    void* info_;
    CallbackInfoType type_;
    JSValueRef value_;
  };

  static void SetErrorCode(JSNIEnv* env, int error_code) {
    JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
    jsni_env_ext->error_code = error_code;
  }

  static void ClearErrorCode(JSNIEnv* env) {
    JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
    jsni_env_ext->error_code = NOERR;
  }

  static Local<Value> WrapAccessorData(JSNIEnv* env,
                                JSNICallback getter,
                                JSNICallback setter,
                                void* data) {
    Isolate* isolate = GetIsolate(env);
    Local<Context> context = isolate->GetCurrentContext();
    Local<ObjectTemplate> temp = ObjectTemplate::New(isolate);
    temp->SetInternalFieldCount(kAccessorFieldCount);
    Local<Object> external = temp->NewInstance(context).ToLocalChecked();

    if (getter != nullptr) {
      external->SetInternalField(
          JSNI::kGetterIndex,
          External::New(isolate, reinterpret_cast<void*>(getter)));
    }

    if (setter != nullptr) {
      external->SetInternalField(
          JSNI::kSetterIndex,
          External::New(isolate, reinterpret_cast<void*>(setter)));
    }

    external->SetInternalField(
        JSNI::kDataIndex,
        External::New(isolate, data));
    return external;
  }

  // Getter wrap.
  static void WrapGetter(Local<Name> property,
                         const PropertyCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    JSNIEnv* env = JSNI::GetEnv(isolate);
    Local<Object> external = info.Data().As<Object>();
    JSNICallback getter =
      (JSNICallback)external->GetInternalField(kGetterIndex)
      .As<External>()->Value();

    JSNICallbackInfoWrap jsni_info(
      reinterpret_cast<void*>(const_cast<PropertyCallbackInfo<Value>*>(&info)),
      JSNICallbackInfoWrap::kGetterProperty);

    getter(env, (JSNICallbackInfo)&jsni_info);
  }
  // Setter wrap.
  static void WrapSetter(Local<Name> property,
                         Local<Value> value,
                         const PropertyCallbackInfo<void>& info) {
    Isolate* isolate = info.GetIsolate();
    JSNIEnv* env = JSNI::GetEnv(isolate);
    Local<Object> external = info.Data().As<Object>();
    JSNICallback getter =
      (JSNICallback)external->GetInternalField(kSetterIndex)
      .As<External>()->Value();

    JSValueRef jsni_value = ToJSNIValue(value);
    JSNICallbackInfoWrap jsni_info(reinterpret_cast<void*>(
                                const_cast<PropertyCallbackInfo<void>*>(
                                  &info)),
                               JSNICallbackInfoWrap::kSetterProperty,
                               jsni_value);

    getter(env, (JSNICallbackInfo)&jsni_info);
  }

  // A fake func to call native callback.
  static void FakeJSNICallback(
                const FunctionCallbackInfo<Value>& info) {
    Isolate* isolate = info.GetIsolate();
    JSNICallback nativeFunc =
      reinterpret_cast<JSNICallback>(
          info.Data().As<External>()->Value());

    JSNIEnvExt* env = reinterpret_cast<JSNIEnvExt*>(JSNI::GetEnv(isolate));

    JSNICallbackInfoWrap jsni_info(reinterpret_cast<void*>(
                               const_cast<FunctionCallbackInfo<Value>*>(&info)),
      JSNICallbackInfoWrap::kFunction);

    nativeFunc(env, (JSNICallbackInfo)&jsni_info);
    // exception is caught by us. If not empty, throw it here.
    if (!env->last_exception.IsEmpty()) {
      isolate->ThrowException(
          v8::Local<v8::Value>::New(isolate, env->last_exception));
      env->last_exception.Reset();
    }
  }

  static Local<Value> ToV8LocalValue(JSValueRef val) {
    return *reinterpret_cast<Local<Value>*>(&val);
  }

  static JSValueRef ToJSNIValue(Local<Value> val) {
    return reinterpret_cast<JSValueRef>(*val);
  }

  static void JSNIAbort(const char* jsni_function_name, const char* msg) {
    LOG_E("\n#\n# JSNI fatal error in %s\n# %s\n#\n\n",
                         jsni_function_name, msg);
    abort();
  }

  static void JSNICheck(bool condition,
                        const char* location,
                        const char* message) {
    if (!condition) JSNIAbort(location, message);
  }

  static JSValueRef RawNewUndefined(JSNIEnv* env) {
    Local<Primitive> undefined_value = Undefined(GetIsolate(env));
    return ToJSNIValue(undefined_value);
  }

  static Isolate* GetIsolate(JSNIEnv* env) {
    JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
    return jsni_env_ext->isolate_;
  }

  static JSNIEnv* GetEnv(Isolate* isolate) {
    auto context = isolate->GetCurrentContext();
    auto global = context->Global();
    auto jsni_name = String::NewFromUtf8(
                     isolate,
                     "__jsni_env__", NewStringType::kNormal
                     ).ToLocalChecked();
    Local<External> jsni = global->Get(context, jsni_name).ToLocalChecked().As<External>();
    JSNIEnv* env = reinterpret_cast<JSNIEnv*>(jsni->Value());
    return env;
  }
};


}  // namespace v8


#define PREPARE_API_CALL(env)         \
  JSNI::ClearErrorCode(env);          \
  JSNI::JSNITryCatch try_catch(reinterpret_cast<JSNIEnvExt*>(env))

int JSNIGetVersion(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  return JSNI_VERSION_2_1;
}

bool JSNIRegisterMethod(JSNIEnv* env, JSValueRef recv,
                    const char* name,
                    JSNICallback callback) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> ctx = isolate->GetCurrentContext();
  HandleScope handle_scope(isolate);

  // Create a fake function to keep opaque.
  Local<FunctionTemplate> t =
    FunctionTemplate::New(
      isolate,
      JSNI::FakeJSNICallback,
      External::New(isolate, reinterpret_cast<void*>(callback)));

  Local<Function> fn = t->GetFunction(ctx).ToLocalChecked();
  Local<String> fn_name = String::NewFromUtf8(isolate, name,
                                  NewStringType::kNormal).ToLocalChecked();
  fn->SetName(fn_name);

  return (*reinterpret_cast<Local<Value>*>(&recv))
          ->ToObject(ctx).ToLocalChecked()
          ->Set(ctx, fn_name, fn).FromJust();
}

int JSNIGetArgsLengthOfCallback(JSNIEnv* env, JSNICallbackInfo info) {
  PREPARE_API_CALL(env);
  JSNI::JSNICallbackInfoWrap* jsni_info =
    reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  switch (type) {
    case JSNI::JSNICallbackInfoWrap::kGetterProperty:
      return 0;
    case JSNI::JSNICallbackInfoWrap::kSetterProperty:
      return 1;
    case JSNI::JSNICallbackInfoWrap::kFunction:
    {
      const FunctionCallbackInfo<Value>* v8_info =
                reinterpret_cast<FunctionCallbackInfo<Value>*>(
                  jsni_info->info());
      return v8_info->Length();
    }
    default:
      JSNI::JSNIAbort(__func__, "UNREACHABLE.");
  }
  return 0;
}

JSValueRef JSNIGetArgOfCallback(JSNIEnv* env,  JSNICallbackInfo info, int id) {
  PREPARE_API_CALL(env);
  JSNI::JSNICallbackInfoWrap* jsni_info =
    reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  switch (type) {
    case JSNI::JSNICallbackInfoWrap::kGetterProperty:
      return NULL;
    case JSNI::JSNICallbackInfoWrap::kSetterProperty:
      if (id == 0) {
        return jsni_info->value();
      } else {
        return NULL;
      }
    case JSNI::JSNICallbackInfoWrap::kFunction:
    {
      const FunctionCallbackInfo<Value>* v8_info =
                reinterpret_cast<FunctionCallbackInfo<Value>*>(
                  jsni_info->info());
      return JSNI::ToJSNIValue((*v8_info)[id]);
    }
    default:
      JSNI::JSNIAbort(__func__, "UNREACHABLE.");
  }
  return NULL;
}

JSValueRef JSNIGetThisOfCallback(JSNIEnv* env,  JSNICallbackInfo info) {
  PREPARE_API_CALL(env);
  JSNI::JSNICallbackInfoWrap* jsni_info =
    reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  switch (type) {
    case JSNI::JSNICallbackInfoWrap::kGetterProperty:
    case JSNI::JSNICallbackInfoWrap::kSetterProperty:
    {
      const PropertyCallbackInfo<Value>* v8_info =
            reinterpret_cast<PropertyCallbackInfo<Value>*>(jsni_info->info());
      return JSNI::ToJSNIValue(v8_info->This());
    }
    case JSNI::JSNICallbackInfoWrap::kFunction:
    {
      const FunctionCallbackInfo<Value>* v8_info =
            reinterpret_cast<FunctionCallbackInfo<Value>*>(jsni_info->info());
      return JSNI::ToJSNIValue(v8_info->This());
    }
    default:
      JSNI::JSNIAbort(__func__, "UNREACHABLE.");
  }
  return NULL;
}

void JSNISetReturnValue(JSNIEnv* env,
                                  JSNICallbackInfo info,
                                  JSValueRef val) {
  PREPARE_API_CALL(env);
  JSNI::JSNICallbackInfoWrap* jsni_info =
    reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  Local<Value> v8_val = *reinterpret_cast<Local<Value>*>(&val);

  switch (type) {
    case JSNI::JSNICallbackInfoWrap::kSetterProperty:
      JSNI::JSNIAbort("JSNISetReturnValueOfCallback",
                      "JSNISetReturnValueOfCallback can not"
                      "be called in setter property callback.");
    case JSNI::JSNICallbackInfoWrap::kGetterProperty:
    {
      const PropertyCallbackInfo<Value>* v8_info =
            reinterpret_cast<PropertyCallbackInfo<Value>*>(jsni_info->info());
      v8_info->GetReturnValue().Set(v8_val);
      break;
    }
    case JSNI::JSNICallbackInfoWrap::kFunction:
    {
      const FunctionCallbackInfo<Value>* v8_info =
                reinterpret_cast<FunctionCallbackInfo<Value>*>(
                  jsni_info->info());
      v8_info->GetReturnValue().Set(v8_val);
      break;
    }
    default:
      JSNI::JSNIAbort(__func__, "UNREACHABLE.");
  }
}

// Primitive Operations
bool JSNIIsUndefined(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsUndefined();
}

JSValueRef JSNINewUndefined(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  return JSNI::RawNewUndefined(env);
}

bool JSNIIsNull(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsNull();
}

JSValueRef JSNINewNull(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  return JSNI::ToJSNIValue(Null(JSNI::GetIsolate(env)));
}

bool JSNIIsBoolean(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsBoolean();
}

bool JSNIToCBool(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  if (!JSNI::ToV8LocalValue(val)->IsBoolean()) {
    JSNI::SetErrorCode(env, BOOERR);
    return false;
  }
  // Will crash if failed.
  return (reinterpret_cast<Value*>(val))->BooleanValue(context).FromJust();
}

JSValueRef JSNINewBoolean(JSNIEnv* env, bool val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  return reinterpret_cast<JSValueRef>(*(Boolean::New(isolate, val)));
}

bool JSNIIsNumber(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsNumber();
}

JSValueRef JSNINewNumber(JSNIEnv* env, double val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  return reinterpret_cast<JSValueRef>(*(Number::New(isolate, val)));
}

double JSNIToCDouble(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  // Will crash if failed.
  if (!JSNI::ToV8LocalValue(val)->IsNumber()) {
    JSNI::SetErrorCode(env, NUMERR);
    return std::numeric_limits<double>::quiet_NaN();
  }
  return (reinterpret_cast<Value*>(val))->NumberValue(context).FromJust();
}

int32_t JSNIToInt32(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  // Will crash if failed.
  if (!JSNI::ToV8LocalValue(val)->IsNumber()) {
    JSNI::SetErrorCode(env, NUMERR);
    return std::numeric_limits<int32_t>::quiet_NaN();
  }

  return (reinterpret_cast<Value*>(val))->Int32Value(context).FromJust();
}

uint32_t JSNIToUint32(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  // Will crash if failed.
  if (!JSNI::ToV8LocalValue(val)->IsNumber()) {
    JSNI::SetErrorCode(env, NUMERR);
    return std::numeric_limits<uint32_t>::quiet_NaN();
  }

  return (reinterpret_cast<Value*>(val))->Uint32Value(context).FromJust();
}

int64_t JSNIToInt64(JSNIEnv* env, JSValueRef val){
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  // Will crash if failed.
  if (!JSNI::ToV8LocalValue(val)->IsNumber()) {
    JSNI::SetErrorCode(env, NUMERR);
    return std::numeric_limits<int64_t>::quiet_NaN();
  }

  return (reinterpret_cast<Value*>(val))->IntegerValue(context).FromJust();
}

bool JSNIIsSymbol(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsSymbol();
}

JSValueRef JSNINewSymbol(JSNIEnv* env, JSValueRef name) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (name != NULL) {
    Local<String> v8_name = JSNI::ToV8LocalValue(name).As<String>();
    return JSNI::ToJSNIValue(Symbol::New(isolate, v8_name));
  } else {
    return JSNI::ToJSNIValue(Symbol::New(isolate));
  }
}

bool JSNIIsString(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsString();
}

JSValueRef JSNINewStringFromUtf8(JSNIEnv* env, const char* src, size_t length) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<String> str = String::NewFromUtf8(isolate, src,
                      NewStringType::kNormal, length).ToLocalChecked();
  return reinterpret_cast<JSValueRef>(*(scope.Escape(str)));
}

size_t JSNIGetStringUtf8Length(JSNIEnv* env, JSValueRef string) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(string)->IsString()) {
    JSNI::SetErrorCode(env, STRERR);
    return 0;
  }
  return String::Cast(reinterpret_cast<Value*>(string))->Utf8Length();
}

size_t JSNIGetStringUtf8Chars(JSNIEnv* env,
                              JSValueRef string,
                              char* copy,
                              size_t length) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(string)->IsString()) {
    JSNI::SetErrorCode(env, STRERR);
    return 0;
  }
  String* s = reinterpret_cast<String*>(string);
  if (s == nullptr) {
    return 0;
  }

  assert(copy != nullptr);
  int result = s->WriteUtf8(copy, length);
  return result;
}

JSValueRef JSNINewString(JSNIEnv* env, const uint16_t* src, size_t length) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<String> str =
      String::NewFromTwoByte(isolate, src,
                             NewStringType::kNormal, length)
          .ToLocalChecked();
  return reinterpret_cast<JSValueRef>(*str);
}

size_t JSNIGetStringLength(JSNIEnv* env, JSValueRef string) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(string)->IsString()) {
    JSNI::SetErrorCode(env, STRERR);
    return 0;
  }
  return String::Cast(reinterpret_cast<Value*>(string))->Length();
}

size_t JSNIGetString(JSNIEnv* env, JSValueRef string, uint16_t* copy, size_t length) {
  PREPARE_API_CALL(env);
  assert(copy != nullptr);

  if (!JSNI::ToV8LocalValue(string)->IsString()) {
    JSNI::SetErrorCode(env, STRERR);
    return 0;
  }

  String* s = reinterpret_cast<String*>(string);
  if (s == nullptr) {
    return 0;
  }

  int result = s->Write(copy, 0, length);
  return result;
}

// Object Operations
bool JSNIIsObject(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsObject();
}

bool JSNIIsEmpty(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return val == nullptr;
}

JSValueRef JSNINewObject(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Value> val = Object::New(isolate);
  return reinterpret_cast<JSValueRef>(*val);
}

JSValueRef JSNINewObjectWithInternalField(JSNIEnv* env, int count) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<ObjectTemplate> result = ObjectTemplate::New(isolate);
  result->SetInternalFieldCount(count);
  Local<Object> obj =
    result->NewInstance(isolate->GetCurrentContext()).ToLocalChecked();
  return JSNI::ToJSNIValue(scope.Escape(obj));
}

bool JSNIHasProperty(JSNIEnv* env, JSValueRef object, const char* name) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = (reinterpret_cast<Value*>(object))->ToObject(context)
    .ToLocalChecked();
  Maybe<bool> t =
    obj->Has(context,
             String::NewFromUtf8(isolate,
                                 name,
                                 NewStringType::kNormal).ToLocalChecked());
  return t.FromJust();
}

JSValueRef JSNIGetProperty(JSNIEnv* env, JSValueRef object, const char* name) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(object)->IsObject()) {
    JSNI::SetErrorCode(env, OBJERR);
    return JSNI::RawNewUndefined(env);
  }
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = (reinterpret_cast<Value*>(object))->ToObject(context)
    .ToLocalChecked();
  MaybeLocal<Value> p =
    obj->Get(context,
            String::NewFromUtf8(isolate,
                                name,
                                NewStringType::kNormal).ToLocalChecked());
  return JSNI::ToJSNIValue(scope.Escape(p.ToLocalChecked()));
}

bool JSNISetProperty(JSNIEnv* env,
                     JSValueRef object,
                     const char* name,
                     JSValueRef property) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(object)->IsObject()) {
    JSNI::SetErrorCode(env, OBJERR);
    return false;
  }
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = (reinterpret_cast<Value*>(object))->ToObject(context)
    .ToLocalChecked();
  Maybe<bool> t =
    obj->Set(context,
             String::NewFromUtf8(isolate,
                                 name,
                                 NewStringType::kNormal).ToLocalChecked(),
                                 JSNI::ToV8LocalValue(property));
  return t.FromJust();
}

bool JSNIDeleteProperty(JSNIEnv* env, JSValueRef object, const char* name) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(object)->IsObject()) {
    JSNI::SetErrorCode(env, OBJERR);
    return false;
  }
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = (reinterpret_cast<Value*>(object))->ToObject(context)
    .ToLocalChecked();
  Maybe<bool> t =
    obj->Delete(context,
                String::NewFromUtf8(isolate,
                                    name,
                                    NewStringType::kNormal).ToLocalChecked());
  return t.FromJust();
}

JSValueRef JSNIGetPrototype(JSNIEnv* env, JSValueRef object) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Object> o = JSNI::ToV8LocalValue(object).As<Object>();
  Local<Value> proto = o->GetPrototype();
  return reinterpret_cast<JSValueRef>(*(scope.Escape(proto)));
}

int JSNIInternalFieldCount(JSNIEnv* env, JSValueRef object) {
  PREPARE_API_CALL(env);
  return Object::Cast(reinterpret_cast<Value*>(object))->InternalFieldCount();
}

void JSNISetInternalField(JSNIEnv* env,
                          JSValueRef object,
                          int index,
                          void* field) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<External> ext = External::New(isolate, field);
  Object::Cast(reinterpret_cast<Value*>(object))->SetInternalField(index, ext);
}

void* JSNIGetInternalField(JSNIEnv* env, JSValueRef object, int index) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<External> ext = Local<External>::Cast(
    Object::Cast(reinterpret_cast<Value*>(object))->GetInternalField(index));
  void* field = ext->Value();
  return field;
}

bool JSNIIsFunction(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
return (reinterpret_cast<Value*>(val))->IsFunction();
}

JSValueRef JSNINewFunction(JSNIEnv* env, JSNICallback nativeFunc) {
  PREPARE_API_CALL(env);
  // Create a fack function to keep opaque.
  // Note: V8 and jsni ABI is not the same, so we need the fake function to call
  // the real function.
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<FunctionTemplate> temp =
    FunctionTemplate::New(
      isolate,
      JSNI::FakeJSNICallback,
      External::New(isolate, reinterpret_cast<void*>(nativeFunc)));
  Local<Function> function = temp->GetFunction(context).ToLocalChecked();
  return reinterpret_cast<JSValueRef>(*(scope.Escape(function)));
}

JSValueRef JSNICallFunction(JSNIEnv* env, JSValueRef func, JSValueRef recv,
                            int argc, JSValueRef* argv) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  if (!JSNI::ToV8LocalValue(func)->IsFunction()) {
    JSNI::SetErrorCode(env, FUNCERR);
  }
  // We do not need new args to pass. Just modify the primary.
  Local<Value>* new_argv = reinterpret_cast<Local<Value>*>(argv);
  for (int i = 0; i < argc; i++) {
    new_argv[i] = JSNI::ToV8LocalValue(argv[i]);
  }
  MaybeLocal<Value> ret =
    Function::Cast(reinterpret_cast<Value*>(func))
      ->Call(context, JSNI::ToV8LocalValue(recv),
                                argc, new_argv);
  return JSNI::ToJSNIValue(scope.Escape(
    ret.FromMaybe(Local<Value>())));
}

bool JSNIIsArray(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsArray();
}

size_t JSNIGetArrayLength(JSNIEnv* env, JSValueRef array) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(array)->IsArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return 0;
  }
  HandleScope scope(isolate);
  Local<Array> v8_arr = JSNI::ToV8LocalValue(array).As<Array>();
  return v8_arr->Length();
}

JSValueRef JSNINewArray(JSNIEnv* env, size_t initial_length) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Array> v8_arr = Array::New(isolate);
  return JSNI::ToJSNIValue(v8_arr);
}

JSValueRef JSNIGetArrayElement(JSNIEnv* env, JSValueRef array, size_t index) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(array)->IsArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return JSNI::RawNewUndefined(env);
  }
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = JSNI::ToV8LocalValue(array)->ToObject(context)
    .ToLocalChecked();
  if (index > UINT32_MAX) {
    JSNI::SetErrorCode(env, OBJERR);
  }
  Local<Value> val = obj->Get(context, static_cast<uint32_t>(index))
                     .ToLocalChecked();
  return JSNI::ToJSNIValue(scope.Escape(val));
}

void JSNISetArrayElement(JSNIEnv* env,
                         JSValueRef array,
                         size_t index,
                         JSValueRef value) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  if (!JSNI::ToV8LocalValue(array)->IsArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return;
  }
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Object> obj = JSNI::ToV8LocalValue(array)->ToObject(context)
                        .ToLocalChecked();
  Local<Value> val = JSNI::ToV8LocalValue(value);
  if (index > UINT32_MAX) {
    JSNI::SetErrorCode(env, OBJERR);
  }
  bool success = (obj->Set(context, static_cast<uint32_t>(index), val))
                 .FromMaybe(false);
  if (!success) {
    JSNI::SetErrorCode(env, OBJERR);
  }
}

bool JSNIIsTypedArray(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  return (reinterpret_cast<Value*>(val))->IsTypedArray();
}

JSValueRef JSNINewTypedArray(JSNIEnv* env,
                             JsTypedArrayType type,
                             void* data,
                             size_t length) {
  // Only uint8 supported yet.
  if (type != JsArrayTypeUint8) {
    assert(0 && "Only uint8 supported yet.");
  }
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<ArrayBuffer> abuf =
    ArrayBuffer::New(isolate, data, length);
  Local<Uint8Array> uarr = Uint8Array::New(abuf, 0, length);
  return reinterpret_cast<JSValueRef>(*(scope.Escape(uarr)));
}

JsTypedArrayType JSNIGetTypedArrayType(JSNIEnv* env, JSValueRef typed_array) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(typed_array)->IsTypedArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return JsArrayTypeNone;
  }
  if (TypedArray::Cast(reinterpret_cast<Value*>(typed_array))->IsUint8Array()) {
    return JsArrayTypeUint8;
  }
  return JsArrayTypeNone;
}

void* JSNIGetTypedArrayData(JSNIEnv* env, JSValueRef typed_array) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(typed_array)->IsTypedArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return nullptr;
  }
  ArrayBuffer::Contents ab_c =
    TypedArray::Cast(reinterpret_cast<Value*>(typed_array))
      ->Buffer()->GetContents();
  return static_cast<char *>(ab_c.Data())
    + TypedArray::Cast(reinterpret_cast<Value*>(typed_array))->ByteOffset();
}

size_t JSNIGetTypedArrayLength(JSNIEnv* env, JSValueRef typed_array) {
  PREPARE_API_CALL(env);
  if (!JSNI::ToV8LocalValue(typed_array)->IsTypedArray()) {
    JSNI::SetErrorCode(env, ARRERR);
    return 0;
  }
  Local<TypedArray> array =
    JSNI::ToV8LocalValue(typed_array).As<TypedArray>();
  return array->Length();
}

// Reference
void JSNIPushLocalScope(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  JsLocalScope* local_scope = new JsLocalScope(isolate);
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
  jsni_env_ext->stacked_local_scope.push_back(local_scope);
}

void JSNIPopLocalScope(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);

  // Set error code and return early
  // if the number of JSNIPopLocalScope used is more than JSNIPushLocalScope.
  if (jsni_env_ext->stacked_local_scope.empty()) {
    JSNI::SetErrorCode(env, SCOERR);
    return;
  }

  JsLocalScope* local_scope = reinterpret_cast<JsLocalScope*>(
    jsni_env_ext->stacked_local_scope.back());
  jsni_env_ext->stacked_local_scope.pop_back();

  delete local_scope;
}

// Reference
void JSNIPushEscapableLocalScope(JSNIEnv* env) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  JsEscapableLocalScope* local_scope = new JsEscapableLocalScope(isolate);
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
  jsni_env_ext->stacked_local_scope.push_back(local_scope);
}

JSValueRef JSNIPopEscapableLocalScope(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  JSNIEnvExt* jsni_env_ext =
    reinterpret_cast<JSNIEnvExt*>(env);

  // Set error code and return early
  // if the number of JSNIPopLocalScope used is more than JSNIPushLocalScope.
  if (jsni_env_ext->stacked_local_scope.empty()) {
    JSNI::SetErrorCode(env, SCOERR);
    return NULL;
  }

  JsLocalScopeBase* scope_base = jsni_env_ext->stacked_local_scope.back();
  CHECK(scope_base->IsEscapable());
  JsEscapableLocalScope* escapable_local_scope =
    reinterpret_cast<JsEscapableLocalScope*>(scope_base);
  jsni_env_ext->stacked_local_scope.pop_back();

  Local<Value> escape =
    escapable_local_scope->Escape(JSNI::ToV8LocalValue(val));
  JSValueRef result = JSNI::ToJSNIValue(escape);
  delete escapable_local_scope;

  return result;
}

JSGlobalValueRef JSNINewGlobalValue(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  JSNI::JSRef* ref = JSNI::JSRef::New(env, val);
  return reinterpret_cast<JSGlobalValueRef>(ref);
}

void JSNIDeleteGlobalValue(JSNIEnv* env, JSGlobalValueRef val) {
  PREPARE_API_CALL(env);
  JSNI::JSRef::Delete(reinterpret_cast<JSNI::JSRef*>(val));
}

size_t JSNIAcquireGlobalValue(JSNIEnv* env, JSGlobalValueRef val) {
  PREPARE_API_CALL(env);
  JSNI::JSRef* ref = reinterpret_cast<JSNI::JSRef*>(val);
  return ref->Ref();
}

size_t JSNIReleaseGlobalValue(JSNIEnv* env, JSGlobalValueRef val) {
  JSNI::JSRef* ref = reinterpret_cast<JSNI::JSRef*>(val);
  return ref->UnRef();
}

JSValueRef JSNIGetGlobalValue(JSNIEnv* env, JSGlobalValueRef val) {
  PREPARE_API_CALL(env);
  JSNI::JSRef* ref = reinterpret_cast<JSNI::JSRef*>(val);
  return ref->Get(env);
}

void JSNISetGCCallback(JSNIEnv* env,
                       JSGlobalValueRef global,
                       void* args,
                       JSNIGCCallback callback) {
  JSNI::JSRef* ref = reinterpret_cast<JSNI::JSRef*>(global);
  ref->SetWeak(args, callback);
}

// Exception
void JSNIThrowErrorException(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  HandleScope scope(isolate);
  isolate->ThrowException(Exception::Error(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked()));
}

void JSNIThrowTypeErrorException(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  HandleScope scope(isolate);
  isolate->ThrowException(Exception::TypeError(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked()));
}

void JSNIThrowRangeErrorException(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  HandleScope scope(isolate);
  isolate->ThrowException(Exception::RangeError(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked()));
}

JSNIErrorInfo JSNIGetLastErrorInfo(JSNIEnv* env) {
  // Do not clear error here.
  // PREPARE_API_CALL(env);
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);

  int err = jsni_env_ext->error_code;
  jsni_env_ext->last_error_info.msg = error_messages[err];
  jsni_env_ext->last_error_info.error_code = (JSNIErrorCode)err;

  if (err > 0) {
    LOG_E("Error occured in JSNI!\n");
    jsni_env_ext->error_code = 0;
  }
  return jsni_env_ext->last_error_info;
}

bool JSNIDefineProperty(JSNIEnv* env,
                        JSValueRef object,
                        const char* name,
                        const JSNIPropertyDescriptor descriptor) {
  Isolate* isolate = JSNI::GetIsolate(env);
  Local<Context> context = isolate->GetCurrentContext();
  JSNIDataPropertyDescriptor* data_attributes =
    descriptor.data_attributes;
  JSNIAccessorPropertyDescriptor* accessor_attributes =
    descriptor.accessor_attributes;
  Local<Object> obj = JSNI::ToV8LocalValue(object).As<Object>();
  Local<Name> pro_name = String::NewFromUtf8(isolate,
                               name,
                               NewStringType::kNormal).ToLocalChecked();

  CHECK(data_attributes == NULL || accessor_attributes == NULL);
  if (accessor_attributes != NULL) {
    CHECK(data_attributes == NULL);
    JSNICallback getter = accessor_attributes->getter;
    JSNICallback setter = accessor_attributes->setter;

    Local<Value> wrap_data =
      JSNI::WrapAccessorData(env, getter, setter, accessor_attributes->data);

    bool result = obj->SetAccessor(
      context,
      pro_name,
      getter ? JSNI::WrapGetter : NULL,
      setter ? JSNI::WrapSetter : NULL,
      wrap_data,
      AccessControl::DEFAULT,
      static_cast<PropertyAttribute>(accessor_attributes->attributes))
        .FromMaybe(false);
    return result;
  } else {
    CHECK(accessor_attributes == NULL);
    PropertyAttribute attributes =
      static_cast<PropertyAttribute>(data_attributes->attributes);
    bool result =
      obj->DefineOwnProperty(context,
                             pro_name,
                             JSNI::ToV8LocalValue(data_attributes->value),
                             attributes).FromMaybe(false);
    return result;
  }
}

void* JSNIGetDataOfCallback(JSNIEnv* env, JSNICallbackInfo info) {
  JSNI::JSNICallbackInfoWrap* jsni_info =
    reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  Local<Value> external;
  switch (type) {
    case JSNI::JSNICallbackInfoWrap::kFunction:
      JSNI::JSNIAbort("JSNIGetDataOfCallback",
                      "No data should be in function callback.");
    case JSNI::JSNICallbackInfoWrap::kSetterProperty:
    case JSNI::JSNICallbackInfoWrap::kGetterProperty:
      external =
        reinterpret_cast<PropertyCallbackInfo<Value>*>(
          jsni_info->info())
          ->Data();
      return external.As<Object>()->GetInternalField(JSNI::kDataIndex)
                 .As<External>()->Value();
    default:
      JSNI::JSNIAbort(__func__, "UNREACHABLE.");
  }

  return NULL;
}

bool JSNIHasException(JSNIEnv* env) {
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
  return !jsni_env_ext->last_exception.IsEmpty();
}

void JSNIClearException(JSNIEnv* env) {
  JSNIEnvExt* jsni_env_ext = reinterpret_cast<JSNIEnvExt*>(env);
  if (jsni_env_ext->last_exception.IsEmpty()) {
    return;
  } else {
    jsni_env_ext->last_exception.Reset();
  }
}

JSValueRef JSNINewError(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Value> error_obj = Exception::Error(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked());
  return JSNI::ToJSNIValue(scope.Escape(error_obj));
}

JSValueRef JSNINewTypeError(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Value> error_obj = Exception::TypeError(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked());
  return JSNI::ToJSNIValue(scope.Escape(error_obj));
}

JSValueRef JSNINewRangeError(JSNIEnv* env, const char* errmsg) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Value> error_obj = Exception::RangeError(
    String::NewFromUtf8(
      isolate,
      errmsg,
      NewStringType::kNormal).ToLocalChecked());
  return JSNI::ToJSNIValue(scope.Escape(error_obj));
}

void JSNIThrowErrorObject(JSNIEnv* env, JSValueRef error) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  HandleScope scope(isolate);
  isolate->ThrowException(JSNI::ToV8LocalValue(error));
}

bool JSNIIsError(JSNIEnv* env, JSValueRef val) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  HandleScope scope(isolate);
  Local<Value> v8val = JSNI::ToV8LocalValue(val);
  return v8val->IsNativeError();
}

JSValueRef JSNINewInstance(JSNIEnv* env, JSValueRef constructor, int argc, JSValueRef* argv) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> constr = JSNI::ToV8LocalValue(constructor).As<Function>();
  // We do not need new args to pass. Just modify the primary.
  Local<Value>* new_argv = reinterpret_cast<Local<Value>*>(argv);
  for (int i = 0; i < argc; i++) {
    new_argv[i] = JSNI::ToV8LocalValue(argv[i]);
  }
  MaybeLocal<Object> new_instance = constr->NewInstance(context, argc, new_argv);
  return JSNI::ToJSNIValue(
    scope.Escape(new_instance.FromMaybe(Local<Value>())));
}

bool JSNIInstanceOf(JSNIEnv* env, JSValueRef left, JSValueRef right) {
  PREPARE_API_CALL(env);
  Isolate* isolate = JSNI::GetIsolate(env);
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<Value> left_v = JSNI::ToV8LocalValue(left);
  Local<Object> right_v = JSNI::ToV8LocalValue(right).As<Object>();
  return left_v->InstanceOf(context, right_v).FromMaybe(false);
}

JSValueRef JSNIGetNewTarget(JSNIEnv* env, JSNICallbackInfo info) {
  PREPARE_API_CALL(env);
  JSNI::JSNICallbackInfoWrap* jsni_info = reinterpret_cast<JSNI::JSNICallbackInfoWrap*>(info);
  JSNI::JSNICallbackInfoWrap::CallbackInfoType type = jsni_info->type();
  assert(type == JSNI::JSNICallbackInfoWrap::kFunction);
  const FunctionCallbackInfo<Value>* v8_info =
          reinterpret_cast<FunctionCallbackInfo<Value>*>(jsni_info->info());
  return JSNI::ToJSNIValue(v8_info->NewTarget());
}




































































