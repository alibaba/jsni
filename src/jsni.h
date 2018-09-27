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


/** @defgroup jsni JavaScript Native Interface */

/*! @addtogroup jsni
 *   @{
 */
/** @file jsni.h */

#ifndef INCLUDE_JSNI_H_
#define INCLUDE_JSNI_H_

// For size_t.
#include <stddef.h>
// For bool.
#include <stdbool.h>
// For uint16_t.
#include <stdint.h>

/*! \typedef JSValueRef
    \brief Reference type.
*/
typedef struct _JSValueRef* JSValueRef;

/*! \typedef JSNIEnv
    \brief JSNI environment.
*/
typedef struct _JSNIEnv JSNIEnv;

/*! \typedef JSNIGCCallback
    \brief GCCallback helper type.
*/
typedef void (*JSNIGCCallback)(JSNIEnv*, void*);

/*! \typedef JSNICallbackInfo
    \brief Callback helper type.
*/
typedef struct _JSNICallbackInfo* JSNICallbackInfo;

/*! \typedef JSNICallback
    \brief Callback helper type.
*/
typedef void (*JSNICallback)(JSNIEnv*, const JSNICallbackInfo);

/*! \typedef JSGlobalValueRef
    \brief Global reference type.
*/
typedef struct _JSGlobalValueRef* JSGlobalValueRef;

/*! \enum JsTypedArrayType
    \brief The type of a typed JavaScript array.
*/
typedef enum {
  /*! Do not have a type. */
  JsArrayTypeNone,
  /*! An int8 array. */
  JsArrayTypeInt8,
  /*! An uint8 array. */
  JsArrayTypeUint8,
  /*! An uint8 clamped array. JsArrayTypeUint8Clamped represents an array
      of 8-bit unsigned integers clamped to 0-255.
  */
  JsArrayTypeUint8Clamped,
  /*! An int16 array. */
  JsArrayTypeInt16,
  /*! An uint16 array. */
  JsArrayTypeUint16,
  /*! An int32 array. */
  JsArrayTypeInt32,
  /*! An uint32 array. */
  JsArrayTypeUint32,
  /*! An float32 array. */
  JsArrayTypeFloat32,
  /*! An float64 array. */
  JsArrayTypeFloat64
} JsTypedArrayType;

/*! \enum JSNIPropertyAttributes */
typedef enum {
  /*! JSNINone means writable, enumerable and configurable */
  JSNINone       = 0,
  /*! Read-only property*/
  JSNIReadOnly   = 1 << 0,
  /*! Not-enumerable property*/
  JSNIDontEnum   = 1 << 1,
  /*! Not-configurable property*/
  JSNIDontDelete = 1 << 2
} JSNIPropertyAttributes;

/*! \struct JSNIDataPropertyDescriptor */
typedef struct {
  /*! The value associated with the property */
  JSValueRef value;
  /*! Property attributes*/
  JSNIPropertyAttributes attributes;
} JSNIDataPropertyDescriptor;

/*! \struct JSNIAccessorPropertyDescriptor */
typedef struct {
  /*! Accessor callback of getter */
  JSNICallback getter;
  /*! Accessor callback of setter */
  JSNICallback setter;
  /*! Property attributes */
  JSNIPropertyAttributes attributes;
  /*! The pointer of data passed to accessor getter and setter */
  void* data;
} JSNIAccessorPropertyDescriptor;

/*! \struct JSNIPropertyDescriptor */
typedef struct {
  /*! Data property descriptor */
  JSNIDataPropertyDescriptor* data_attributes;
  /*! Accessor property descriptor */
  JSNIAccessorPropertyDescriptor* accessor_attributes;
} JSNIPropertyDescriptor;

/*! \enum JSNIErrorCode */
typedef enum {
  /*! No error */
  JSNIOK,
  /*! Generic error */
  JSNIERR
} JSNIErrorCode;

/*! \struct JSNIErrorInfo */
typedef struct {
  /*! Error message */
  const char* msg;
  /*! Error code */
  JSNIErrorCode error_code;
} JSNIErrorInfo;


#if defined(__cplusplus)
extern "C" {
#endif
/**
 * JSNI functions.
 */


/*! \fn int JSNIGetVersion(JSNIEnv* env)
    \brief Returns the version of the JSNI.
    \param env The JSNI environment pointer.
    \return Returns the version of the JSNI.
*/
int JSNIGetVersion(JSNIEnv* env);

/*! \fn bool JSNIRegisterMethod(JSNIEnv* env, const JSValueRef recv, const char* name, JSNICallback callback)
    \brief Registers a native callback function.
    \param env The JSNI environment pointer. registered JS function associated with the callback.
    \param recv The method receiver. It is passed through JSNIInit to receive the
    \param name A function name.
    \param callback A native callback function to be registered.
    \return Returns true on success.
*/
bool JSNIRegisterMethod(JSNIEnv* env, const JSValueRef recv, const char* name, JSNICallback callback);

/*! \fn int JSNIGetArgsLengthOfCallback(JSNIEnv* env, JSNICallbackInfo info)
    \brief Returns the number of arguments for the callback.
    \param env The JSNI environment pointer.
    \param info The callback info.
    \return Returns the number of arguments.
*/
int JSNIGetArgsLengthOfCallback(JSNIEnv* env, JSNICallbackInfo info);

/*! \fn JSValueRef JSNIGetArgOfCallback(JSNIEnv* env, JSNICallbackInfo info, int id)
    \brief Returns the argument for the callback.
    \param env The JSNI environment pointer.
    \param info The callback info.
    \param id The index of arguments.
    \return Returns the argument.
*/
JSValueRef JSNIGetArgOfCallback(JSNIEnv* env, JSNICallbackInfo info, int id);

/*! \fn JSValueRef JSNIGetThisOfCallback(JSNIEnv* env, JSNICallbackInfo info)
    \brief Returns "this" JavaScript object of the JavaScript function
which current callback associated with.
    \param env The JSNI environment pointer.
    \param info The callback info.
    \return Returns "this" JavaScript object..
*/
JSValueRef JSNIGetThisOfCallback(JSNIEnv* env, JSNICallbackInfo info);

/*! \fn void* JSNIGetDataOfCallback(JSNIEnv* env, JSNICallbackInfo info)
    \brief Get the raw data which is passed to this callback.
    \param env The JSNI environment pointer.
    \param info The callback info.
    \return Returns the data.
*/
void* JSNIGetDataOfCallback(JSNIEnv* env, JSNICallbackInfo info);

/*! \fn void JSNISetReturnValue(JSNIEnv* env, JSNICallbackInfo info, JSValueRef val)
    \brief Sets the JavaScript return value for the callback.
    \param env The JSNI environment pointer.
    \param info The callback info.
    \param val The JavaScript value to return to JavaScript.
    \return None.
*/
void JSNISetReturnValue(JSNIEnv* env, JSNICallbackInfo info, JSValueRef val);

/*! \fn bool JSNIIsUndefined(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is undefined.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is undefined.
*/
bool JSNIIsUndefined(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewUndefined(JSNIEnv* env)
    \brief Constructs a new Undefined JavaScript value.
    \param env The JSNI environment pointer.
    \return Returns an Undefined JavaScript value.
*/
JSValueRef JSNINewUndefined(JSNIEnv* env);

/*! \fn bool JSNIIsNull(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is Null.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is Null.
*/
bool JSNIIsNull(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewNull(JSNIEnv* env)
    \brief Constructs a new Null JavaScript value.
    \param env The JSNI environment pointer.
    \return Returns a Null JavaScript value.
*/
JSValueRef JSNINewNull(JSNIEnv* env);

/*! \fn bool JSNIIsBoolean(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is Boolean.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is Boolean.
*/
bool JSNIIsBoolean(JSNIEnv* env, JSValueRef val);

/*! \fn bool JSNIToCBool(JSNIEnv* env, JSValueRef val)
    \brief Converts the JavaScript value to C bool.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns the C bool value.
*/
bool JSNIToCBool(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewBoolean(JSNIEnv* env, bool val)
    \brief Constructs a new Boolean JavaScript value.
    \param env The JSNI environment pointer.
    \param val A bool value.
    \return Returns a Boolean JavaScript value.
*/
JSValueRef JSNINewBoolean(JSNIEnv* env, bool val);

/*! \fn bool JSNIIsNumber(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is Number.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is Number.
*/
bool JSNIIsNumber(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewNumber(JSNIEnv* env, double val)
    \brief Constructs a new Number JavaScript value.
    \param env The JSNI environment pointer.
    \param val A double value.
    \return Returns a Number JavaScript value.
*/
JSValueRef JSNINewNumber(JSNIEnv* env, double val);

/*! \fn double JSNIToCDouble(JSNIEnv* env, JSValueRef val)
    \brief Converts the JavaScript value to C double.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns the C double value.
*/
double JSNIToCDouble(JSNIEnv* env, JSValueRef val);

/*! \fn int32_t JSNIToInt32(JSNIEnv* env, JSValueRef val)
    \brief Converts the JavaScript value to int32.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns the int32 value.
    \since JSNI 2.2.
*/
int32_t JSNIToInt32(JSNIEnv* env, JSValueRef val);

/*! \fn uint32_t JSNIToUint32(JSNIEnv* env, JSValueRef val)
    \brief Converts the JavaScript value to uint32.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns the uint32 value.
    \since JSNI 2.2.
*/
uint32_t JSNIToUint32(JSNIEnv* env, JSValueRef val);

/*! \fn int64_t JSNIToInt64(JSNIEnv* env, JSValueRef val)
    \brief Converts the JavaScript value to int64.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns the int64 value.
    \since JSNI 2.2.
*/
int64_t JSNIToInt64(JSNIEnv* env, JSValueRef val);

/*! \fn bool JSNIIsSymbol(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is Symbol.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is Symbol.
*/
bool JSNIIsSymbol(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewSymbol(JSNIEnv* env, JSValueRef val)
    \brief Constructs a new Symbol JavaScript value
    \param env The JSNI environment pointer.
    \param val The description of the symbol.
    \return Returns a Symbol JavaScript value.
*/
JSValueRef JSNINewSymbol(JSNIEnv* env, JSValueRef val);

/*! \fn bool JSNIIsString(JSNIEnv* env, JSValueRef val)
    \brief Tests whether the JavaScript value is String.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if the value is String.
*/
bool JSNIIsString(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewStringFromUtf8(JSNIEnv* env, const char* src, size_t length)
    \brief Constructs a new String value from an array of characters
in UTF-8 encoding.
    \param env The JSNI environment pointer.
    \param src The pointer to a UTF-8 string.
    \param length The length of string to be created. It should exclude the null terminator.
    If length equals -1, it will use the length of src.
    \return Returns a String value, or NULL if the string can not be constructed.
*/
JSValueRef JSNINewStringFromUtf8(JSNIEnv* env, const char* src, size_t length);

/*! \fn size_t JSNIGetStringUtf8Length(JSNIEnv* env, JSValueRef string)
    \brief Returns the length in bytes of the UTF-8 representation of a string.
    \param env The JSNI environment pointer.
    \param string A JavaScript string value.
    \return Returns the UTF-8 length of the string, excluding the null-terminator.
*/
size_t JSNIGetStringUtf8Length(JSNIEnv* env, JSValueRef string);

/*! \fn size_t JSNIGetStringUtf8Chars(JSNIEnv* env, JSValueRef string, char* copy, size_t length)
    \brief Copyies a JavaScript string into a UTF-8 encoding string buffer.
    \param env The JSNI environment pointer.
    \param string A JavaScript string value.
    \param copy The buffer copied to.
    \param length The length copied to copy.
    It should be greater than the length of the string considering for string null terminator.
    If length equals -1, it will use the length of string.
    \return Returns the size of copied.
*/
size_t JSNIGetStringUtf8Chars(JSNIEnv* env, JSValueRef string, char* copy, size_t length);

/*! \fn JSValueRef JSNINewString(JSNIEnv* env, const uint16_t* src, size_t length)
    \brief Constructs a new String value from an array of characters in two bytes.
    \param env The JSNI environment pointer.
    \param src The pointer to a two bytes string.
    \param length The length of string to be created. It should exclude the null terminator.
    If length equals -1, it will use the length of src.
    \return Returns a String value, or NULL if the string can not be constructed.
    \since JSNI 2.2.
*/
JSValueRef JSNINewString(JSNIEnv* env, const uint16_t* src, size_t length);

/*! \fn size_t JSNIGetStringUtf8Length(JSNIEnv* env, JSValueRef string)
    \brief Returns the number of charaters in this string.
    \param env The JSNI environment pointer.
    \param string A JavaScript string value.
    \return Returns the number of charaters in this string.
    \since JSNI 2.2.
*/
size_t JSNIGetStringLength(JSNIEnv* env, JSValueRef string);

/*! \fn size_t JSNIGetString(JSNIEnv* env, JSValueRef string, char* copy, size_t length)
    \brief Copyies a JavaScript string into a two bytes string buffer.
    \param env The JSNI environment pointer.
    \param string A JavaScript string value.
    \param copy The buffer copied to.
    \param length The length to copy.
    If length equals -1, it will use the length of string.
    \return Returns the size of copied.
    \since JSNI 2.2.
*/
size_t JSNIGetString(JSNIEnv* env, JSValueRef string, uint16_t* copy, size_t length);

/*! \fn bool JSNIIsObject(JSNIEnv* env, JSValueRef val)
    \brief Tests whether a JavaScript value is a JavaScript object.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if val is a JavaScript object.
*/
bool JSNIIsObject(JSNIEnv* env, JSValueRef val);

/*! \fn bool JSNIIsEmpty(JSNIEnv* env, JSValueRef val)
    \brief Tests whether a JavaScript value is empty.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if val is empty.
*/
bool JSNIIsEmpty(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewObject(JSNIEnv* env)
    \brief Constructs a JavaScript object.
    \param env The JSNI environment pointer.
    \return Returns a JavaScript object.
*/
JSValueRef JSNINewObject(JSNIEnv* env);

/*! \fn bool JSNIHasProperty(JSNIEnv* env, JSValueRef object, const char* name)
    \brief Tests whether a JavaScript object has a property named name.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param name A property name.
    \return Returns true if object has property named name.
*/
bool JSNIHasProperty(JSNIEnv* env, JSValueRef object, const char* name);

/*! \fn JSValueRef JSNIGetProperty(JSNIEnv* env, JSValueRef object, const char* name)
    \brief Returns the property of the JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param name A property name.
    \return Returns the property of the JavaScript object.
*/
JSValueRef JSNIGetProperty(JSNIEnv* env, JSValueRef object, const char* name);

/*! \fn bool JSNISetProperty(JSNIEnv* env, JSValueRef object, const char* name, JSValueRef property)
    \brief Sets a property of a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param name A property name.
    \param property A JavaScript value.
    \return Returns true if the operation succeeds.
*/
bool JSNISetProperty(JSNIEnv* env, JSValueRef object, const char* name, JSValueRef property);

/*! \fn bool JSNIDefineProperty(JSNIEnv* env, JSValueRef object, const char* name, const JSNIPropertyDescriptor descriptor)
    \brief Defines a new property directly on an object, or modifies
an existing property on an object.
    \param env The JSNI environment pointer.
    \param object The object on which to define the property.
    \param name The name of the property to be defined or modified.
    \param descriptor The descriptor for the property being defined or modified.
    \return Returns true on success.
*/
bool JSNIDefineProperty(JSNIEnv* env, JSValueRef object, const char* name, const JSNIPropertyDescriptor descriptor);

/*! \fn bool JSNIDeleteProperty(JSNIEnv* env, JSValueRef object, const char* name)
    \brief Deletes the property of a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param name A property name.
    \return Returns true if the operation succeeds.
*/
bool JSNIDeleteProperty(JSNIEnv* env, JSValueRef object, const char* name);

/*! \fn JSValueRef JSNIGetPrototype(JSNIEnv* env, JSValueRef object)
    \brief Returns a prototype of a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \return Returns a JavaScript value.
*/
JSValueRef JSNIGetPrototype(JSNIEnv* env, JSValueRef object);

/*! \fn JSValueRef JSNINewObjectWithInternalField(JSNIEnv* env, int count)
    \brief Constructs a JavaScript object with internal field.
Internal field is a raw C pointer which can not be
get in JavaScript.
    \param env The JSNI environment pointer.
    \param count A number of internal fields.
    \return Returns a JavaScript object.
*/
JSValueRef JSNINewObjectWithInternalField(JSNIEnv* env, int count);

/*! \fn int JSNIInternalFieldCount(JSNIEnv* env, JSValueRef object)
    \brief Gets the number of the internal field fo a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \return Returns the number of the internal internal field.
*/
int JSNIInternalFieldCount(JSNIEnv* env, JSValueRef object);

/*! \fn void JSNISetInternalField(JSNIEnv* env, JSValueRef object, int index, void* field)
    \brief Sets an internal field of a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param index Index of an internal field. and it will not be freed when object is Garbage Collected.
    \param field An internal field. This is a raw C pointer,
    \return None.
*/
void JSNISetInternalField(JSNIEnv* env, JSValueRef object, int index, void* field);

/*! \fn void* JSNIGetInternalField(JSNIEnv* env, JSValueRef object, int index)
    \brief Gets an internal field of a JavaScript object.
    \param env The JSNI environment pointer.
    \param object A JavaScript object.
    \param index Index of an internal field.
    \return an internal field.
*/
void* JSNIGetInternalField(JSNIEnv* env, JSValueRef object, int index);

/*! \fn bool JSNIIsFunction(JSNIEnv* env, JSValueRef val)
    \brief Tests whether a JavaScript value is Function.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if val is Function.
*/
bool JSNIIsFunction(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewFunction(JSNIEnv* env, JSNICallback callback)
    \brief Constructs a JavaScript function with callback.
    \param env The JSNI environment pointer.
    \param callback A native callback function.
    \return Returns a JavaScript function.
*/
JSValueRef JSNINewFunction(JSNIEnv* env, JSNICallback callback);

/*! \fn JSValueRef JSNICallFunction(JSNIEnv* env, JSValueRef func, JSValueRef recv, int argc, JSValueRef* argv)
    \brief Calls a JavaScript function.
    \param env The JSNI environment pointer.
    \param func A JavaScript funciton.
    \param recv The receiver the func belongs to.
    \param argc The arguments number.
    \param argv A pointer to an array of JavaScript value.
    \return Returns the JavaScript value returned from calling func.
*/
JSValueRef JSNICallFunction(JSNIEnv* env, JSValueRef func, JSValueRef recv, int argc, JSValueRef* argv);

/*! \fn bool JSNIIsArray(JSNIEnv* env, JSValueRef val)
    \brief Tests whether a JavaScript value is Array.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if val is Array.
*/
bool JSNIIsArray(JSNIEnv* env, JSValueRef val);

/*! \fn size_t JSNIGetArrayLength(JSNIEnv* env, JSValueRef array)
    \brief Returns the number of elements in the array.
    \param env The JSNI environment pointer.
    \param array A JavaScript array.
    \return Returns the length of the array.
*/
size_t JSNIGetArrayLength(JSNIEnv* env, JSValueRef array);

/*! \fn JSValueRef JSNINewArray(JSNIEnv* env, size_t initial_length)
    \brief Constructs a JavaScript array with initial length: initial_length.
    \param env The JSNI environment pointer.
    \param initial_length Initial array size.
    \return Returns a JavaScript array object, or NULL if the operation fails.
*/
JSValueRef JSNINewArray(JSNIEnv* env, size_t initial_length);

/*! \fn JSValueRef JSNIGetArrayElement(JSNIEnv* env, JSValueRef array, size_t index)
    \brief Returns an element of a JavaScript array.
    \param env The JSNI environment pointer.
    \param array A JavaScript array.
    \param index Array index.
    \return Returns a JavaScript value.
*/
JSValueRef JSNIGetArrayElement(JSNIEnv* env, JSValueRef array, size_t index);

/*! \fn void JSNISetArrayElement(JSNIEnv* env, JSValueRef array, size_t index, JSValueRef value)
    \brief Sets an element of a JavaScript array.
    \param env The JSNI environment pointer.
    \param array A JavaScript array.
    \param index A array index.
    \param value A new value.
    \return None.
*/
void JSNISetArrayElement(JSNIEnv* env, JSValueRef array, size_t index, JSValueRef value);

/*! \fn bool JSNIIsTypedArray(JSNIEnv* env, JSValueRef val)
    \brief Tests whether a JavaScript value is TypedArray.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns true if val is TypedArray.
*/
bool JSNIIsTypedArray(JSNIEnv* env, JSValueRef val);

/*! \fn JSValueRef JSNINewTypedArray(JSNIEnv* env, JsTypedArrayType type, void* data, size_t length)
    \brief Constructs a JavaScript TypedArray object.
    \param env The JSNI environment pointer.
    \param type The type of the array.
    \param data The pointer to the data buffer of the array.
    \param length The length of the array.
    \return Returns a JavaScript TypedArray object.
*/
JSValueRef JSNINewTypedArray(JSNIEnv* env, JsTypedArrayType type, void* data, size_t length);

/*! \fn JsTypedArrayType JSNIGetTypedArrayType(JSNIEnv* env, JSValueRef typed_array)
    \brief Returns the type of the JavaScript TypedArray value.
    \param env The JSNI environment pointer.
    \param typed_array A JavaScript TypedArray value.
    \return Returns the type of the JavaScript TypedArray value.
*/
JsTypedArrayType JSNIGetTypedArrayType(JSNIEnv* env, JSValueRef typed_array);

/*! \fn void* JSNIGetTypedArrayData(JSNIEnv* env, JSValueRef typed_array)
    \brief Returns the pointer to the buffer of TypedArray data.
    \param env The JSNI environment pointer.
    \param typed_array A JavaScript TypedArray value.
    \return Returns the pointer to the buffer of TypedArray data.
*/
void* JSNIGetTypedArrayData(JSNIEnv* env, JSValueRef typed_array);

/*! \fn size_t JSNIGetTypedArrayLength(JSNIEnv* env, JSValueRef typed_array)
    \brief Returns the number of elements in the TypedArray value.
    \param env The JSNI environment pointer.
    \param typed_array A JavaScript TypedArray value.
    \return Returns the length of the TypedArray value.
*/
size_t JSNIGetTypedArrayLength(JSNIEnv* env, JSValueRef typed_array);

/*! \fn void JSNIPushLocalScope(JSNIEnv* env)
    \brief Creates a local reference scope, and then all local references will
be allocated within this reference scope until the reference scope
is deleted using PoplocalScope() or another local reference scope
is created.
    \param env The JSNI environment pointer.
    \return None.
*/
void JSNIPushLocalScope(JSNIEnv* env);

/*! \fn void JSNIPopLocalScope(JSNIEnv* env)
    \brief Pops off the current local reference scope, frees all the local
references in the local reference scope.
    \param env The JSNI environment pointer.
    \return None.
*/
void JSNIPopLocalScope(JSNIEnv* env);

/*! \fn void JSNIPushEscapableLocalScope(JSNIEnv* env)
    \brief Creates a local reference scope, and then all local references will
be allocated within this reference scope until the reference scope
is deleted using PopEscapableLocalScope() or another local reference scope
is created.
    \param env The JSNI environment pointer.
    \return None.
*/
void JSNIPushEscapableLocalScope(JSNIEnv* env);

/*! \fn JSValueRef JSNIPopEscapableLocalScope(JSNIEnv* env, JSValueRef val)
    \brief Pops off the current local reference scope, frees all the local
references in the local reference scope, and returns a local reference
in the previous local scope for the given val JavaScript value.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns a JavaScript value.
*/
JSValueRef JSNIPopEscapableLocalScope(JSNIEnv* env, JSValueRef val);

/*! \fn JSGlobalValueRef JSNINewGlobalValue(JSNIEnv* env, JSValueRef val)
    \brief Creates a new global reference to the JavaScript value referred to by
the val argument. The global value must be explicitly disposed of by
calling JSNIDeleteGlobalValue() or JSNIReleaseGlobalValue(). The global value will be alive
untile calling JSNIDeleteGlobalValue() or JSNIReleaseGlobalValue() to dispose it.
JSNIDeleteGlobalValue() dispose it immediately, whereas JSNIReleaseGlobalValue()
dispose it when decrease the reference count to zero.
    \param env The JSNI environment pointer.
    \param val A JavaScript value.
    \return Returns a global value.
*/
JSGlobalValueRef JSNINewGlobalValue(JSNIEnv* env, JSValueRef val);

/*! \fn void JSNIDeleteGlobalValue(JSNIEnv* env, JSGlobalValueRef val)
    \brief Deletes the global reference pointed by val.
    \param env The JSNI environment pointer.
    \param val A JSGlobalValueRef value.
    \return None.
*/
void JSNIDeleteGlobalValue(JSNIEnv* env, JSGlobalValueRef val);

/*! \fn size_t JSNIAcquireGlobalValue(JSNIEnv* env, JSGlobalValueRef val)
    \brief Acquire the val means to increase the reference count of the val.
Once the reference count of the val equals to zero, the val is available to
be garbage collected.
    \param env The JSNI environment pointer.
    \param val A JSGlobalValueRef value.
    \return The reference count.
    \since JSNI 2.1.
*/
size_t JSNIAcquireGlobalValue(JSNIEnv* env, JSGlobalValueRef val);

/*! \fn size_t JSNIReleaseGlobalValue(JSNIEnv* env, JSGlobalValueRef val)
    \brief Release the val means to decrease the reference count of the val.
Once the reference count of the val equals to zero, the val is available to
be garbage collected.
    \param env The JSNI environment pointer.
    \param val A JSGlobalValueRef value.
    \return The reference count.
    \since JSNI 2.1.
*/
size_t JSNIReleaseGlobalValue(JSNIEnv* env, JSGlobalValueRef val);

/*! \fn JSValueRef JSNIGetGlobalValue(JSNIEnv* env, JSGlobalValueRef val)
    \brief Returns a local JSValueRef value from a JSGlobalValueRef value.
    \param env The JSNI environment pointer.
    \param val A JSGlobalValueRef value.
    \return Returns a local JSValueRef value.
*/
JSValueRef JSNIGetGlobalValue(JSNIEnv* env, JSGlobalValueRef val);

/*! \fn void JSNISetGCCallback(JSNIEnv* env, JSGlobalValueRef val, void* args, JSNIGCCallback callback)
    \brief Sets a callback which will be called when the JavaScript value pointed by val is freed.
The developer can pass an argument to callback by args. JSNISetGCCallback() is only valid when reference count
is bigger than zero.
    \param env The JSNI environment pointer.
    \param val A JSGlobalValueRef value.
    \param args A pointer to an argument passed to callback.
    \param callback A function callback.
    \return None.
*/
void JSNISetGCCallback(JSNIEnv* env, JSGlobalValueRef val, void* args, JSNIGCCallback callback);

/*! \fn void JSNIThrowErrorException(JSNIEnv* env, const char* errmsg)
    \brief Constructs an error object with the message specified by errmsg
and causes that error to be thrown. It throws a JavaScript Exception.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return None.
*/
void JSNIThrowErrorException(JSNIEnv* env, const char* errmsg);

/*! \fn void JSNIThrowTypeErrorException(JSNIEnv* env, const char* errmsg)
    \brief Constructs an type error object with the message specified by errmsg
and causes that type error to be thrown. It throws a JavaScript Exception.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return None.
*/
void JSNIThrowTypeErrorException(JSNIEnv* env, const char* errmsg);

/*! \fn void JSNIThrowRangeErrorException(JSNIEnv* env, const char* errmsg)
    \brief Constructs an range error object with the message specified by errmsg
and causes that type error to be thrown. It throws a JavaScript Exception.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return None.
*/
void JSNIThrowRangeErrorException(JSNIEnv* env, const char* errmsg);

/*! \fn JSNIErrorInfo JSNIGetLastErrorInfo(JSNIEnv* env)
    \brief Tests whether there is error occured during pervious JSNI call. After
calling JSNIGetLastErrorInfo(), if there is error occured, the error
will be cleared.
    \param env The JSNI environment pointer.
    \return Returns true if there is error occured during previous JSNI call.
*/
JSNIErrorInfo JSNIGetLastErrorInfo(JSNIEnv* env);

/*! \fn bool JSNIHasException(JSNIEnv* env)
    \brief Tests whether a JavaScript exception is being thrown.
It's different with error get from JSNIGetLastErrorInfo.
Eorror is defined by JSNI, whereas this is a JavaScript exception.
    \param env The JSNI environment pointer.
    \return Returns true if a JavaScript exception is being thrown.
*/
bool JSNIHasException(JSNIEnv* env);

/*! \fn void JSNIClearException(JSNIEnv* env)
    \brief Clear a JavaScript exception that is being thrown. If there
is no exception, this routine has no effect.
    \param env The JSNI environment pointer.
    \return None.
*/
void JSNIClearException(JSNIEnv* env);

/*! \fn
    \brief Constructs a JavaScript Error object.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return JavaScript Error Object.
    \since JSNI 2.3.
*/
JSValueRef JSNINewError(JSNIEnv* env, const char* errmsg);

/*! \fn
    \brief Constructs a JavaScript TypeError object.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return JavaScript TypeError Object.
    \since JSNI 2.3.
*/
JSValueRef JSNINewTypeError(JSNIEnv* env, const char* errmsg);

/*! \fn
    \brief Constructs a JavaScript RangeError object.
    \param env The JSNI environment pointer.
    \param errmsg An error message.
    \return JavaScript RangeError Object.
    \since JSNI 2.3.
*/
JSValueRef JSNINewRangeError(JSNIEnv* env, const char* errmsg);

/*! \fn
    \brief Throw JavaScript Error object.
    \param env The JSNI environment pointer.
    \param error JavaScript Error Object.
    \return None.
    \since JSNI 2.3.
*/
void JSNIThrowErrorObject(JSNIEnv* env, JSValueRef error);

/*! \fn
    \brief Tests whether a JavaScript value is Error object.
    \param env The JSNI environment pointer.
    \param val A JavaScript Value.
    \return Returns true if val is an Error object.
    \since JSNI 2.3.
*/
bool JSNIIsError(JSNIEnv* env, JSValueRef val);

/*! \fn
*/
JSValueRef JSNINewInstance(JSNIEnv* env, JSValueRef constructor, int argc, JSValueRef* argv);

/*! \fn
*/
bool JSNIInstanceOf(JSNIEnv* env, JSValueRef left, JSValueRef right);

/*! \fn
*/
JSValueRef JSNIGetNewTarget(JSNIEnv* env, JSNICallbackInfo info);

/*! \fn
*/
bool JSNIStrictEquals(JSNIEnv* env, JSValueRef left, JSValueRef right);



#if defined(__cplusplus)
}
#endif

/*! \struct _JSNIEnv
    \brief JSNI environment structure.
*/
struct _JSNIEnv {
  /*! Reserved pointer */
  void* reserved;
};

// JSNI Versions.
/*! \def JSNI_VERSION_1_0
    \brief JSNI version 1.0.
    \deprecated Version 1.0 is deprecated.
*/
#define JSNI_VERSION_1_0 0x00010000
/*! \def JSNI_VERSION_1_1
    \brief JSNI version 1.1.
    \deprecated Version 1.1 is deprecated.
*/

#define JSNI_VERSION_1_1 0x00010001


/*! \def JSNI_VERSION_2_0
    \brief JSNI version 2.0.

  2.0 is not compatibale with 1.1.
  Since 2.0, we should be compatible with old versions.
*/
#define JSNI_VERSION_2_0 0x00020000

/*! \def JSNI_VERSION_2_1
    \brief JSNI version 2.1.
*/
#define JSNI_VERSION_2_1 0x00020001

/*! \def JSNI_VERSION_2_2
    \brief JSNI version 2.2.
*/
#define JSNI_VERSION_2_2 0x00020002

/*! \def JSNI_VERSION_2_3
    \brief JSNI version 2.3.
*/
#define JSNI_VERSION_2_3 0x00020003

#if defined(__cplusplus)
extern "C" {
#endif
/*! \fn int JSNIInit(JSNIEnv* env, JSValueRef exports)
    \brief This function is called by JSNI, not part of JSNI.
*/
int JSNIInit(JSNIEnv* env, JSValueRef exports);

#if defined(__cplusplus)
}
#endif

#endif  // INCLUDE_JSNI_H_

/*! @} */
