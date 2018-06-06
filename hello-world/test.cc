#include <jsni.h>

void Method(JSNIEnv* env, const JSNICallbackInfo info) {
  JSValueRef str = JSNINewStringFromUtf8(env, "hello", -1);
  JSNISetReturnValue(env, info, str);
}

int JSNIInit(JSNIEnv* env, JSValueRef exports) {
  JSNIRegisterMethod(env, exports, "hello", Method);
  return JSNI_VERSION_2_0;
}

