#! /usr/bin/python
#
# JavaScript Native Interface Release License.
#
# Copyright (c) 2018 Alibaba Group. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the Alibaba Group nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os
import sys
import re

# If IS_PRINT_COMMAND is True, every replace command will be printed.
# Otherwise it will not be printed.
IS_PRINT_COMMAND = False

# If INSERT_OBJECT_WRAP is True, jsni_wrap_src_code will be inserted
# to your source file.
INSERT_OBJECT_WRAP = True

# If BACKUP_ORIGINAL is True, v8tojsni.py will cp original_file
# to original_file.v8tojsni.back
BACKUP_ORIGINAL = True
BACKUP_ORIGINAL_POSTFIX = ".v8tojsni.back"

value_name_pattern = "[a-zA-Z0-9_]*"

# The keywords are replaced in order. Don't mess up the order.
# V8 api list to be replaced.
v8s = [
"(const.*FunctionCallbackInfo<.*Value>.*\&",
" const.*FunctionCallbackInfo<.*Value>.*\&",
"v8_",
"v8",
"V8",
"v8value::",
"v8object::",
"Local<Object>",
"args.GetReturnValue().Set(",
"Null(isolate",
"Object::New(isolate",
"Number::New(isolate",
"Persistent<[a-zA-Z0-9_]*>",
# Node api.
#"MakeCallback(isolate",
]

# Jsni api list to replace.
jsnis = [
"(JSNIEnv* env, const JSNICallbackInfo ",
" const JSNICallbackInfo ",
"jsni_",
"jsni",
"JSNI",
"jsnivalue::",
"jsniobject::",
"JSValueRef",
"JSNISetReturnValue(env, info, ",
"JSNINewNull(env",
"JSNINewObject(env",
"JSNINewNumber(env",
"JSGlobalValueRef",
# For node api.
#"JSNICallFunction(env",
]

# This is a wrapper class in replace of node::ObjectWrap.
# Because jsniObjectWrap is not the standard api
# of jsni, we place it here to replace node::ObjectWrap.
jsni_wrap_src_code = \
"#ifndef CLASS_JSNI_OBJECT_WRAP\\n"\
"#define CLASS_JSNI_OBJECT_WRAP\\n"\
"class jsniObjectWrap\\n"\
"{\\n"\
"public:\\n"\
"    jsniObjectWrap() {}\\n"\
"    // Must be virtual deconstructor becuase this is base class.\\n"\
"    virtual ~jsniObjectWrap() {}\\n"\
"    template <class T>\\n"\
"    static inline T* Unwrap(JSNIEnv* env, JSValueRef handle) {\\n"\
"        assert(!JSNIIsEmpty(env, handle));\\n"\
"        JSValueRef wrap_object = JSNIGetProperty(env, handle, \\\"__wrap_object__\\\");\\n"\
"        void* ptr = JSNIGetInternalField(env, wrap_object, 0);\\n"\
"        jsniObjectWrap* wrap = static_cast<jsniObjectWrap*>(ptr);\\n"\
"        return static_cast<T*>(wrap);\\n"\
"    }\\n"\
"    void Wrap(JSNIEnv* env, JSValueRef handle) {\\n"\
"        assert(global_hanlde_ == nullptr);\\n"\
"        // Use jsni push/pop to save handles using.\\n"\
"        // It may happen that the *handle* has no internal field, so use wrap again.\\n"\
"        JSValueRef wrap_object = JSNINewObjectWithInternalField(env, 1);\\n"\
"        JSNISetInternalField(env, wrap_object, 0, this);\\n"\
"        JSNISetProperty(env, handle, \\\"__wrap_object__\\\", wrap_object);\\n"\
"\\n"\
"        global_hanlde_ = JSNINewGlobalValue(env, wrap_object);\\n"\
"        JSNISetGCCallback(env, global_hanlde_, this, WeakCallback);\\n"\
"    }\\n"\
"    JSValueRef handle(JSNIEnv* env) {\\n"\
"        assert(global_hanlde_ != nullptr);\\n"\
"        return JSNIGetGlobalValue(env, global_hanlde_);\\n"\
"    }\\n"\
"private:\\n"\
"    static void WeakCallback(JSNIEnv* env, void* data) {\\n"\
"        jsniObjectWrap* wrap = reinterpret_cast<jsniObjectWrap*>(data);\\n"\
"        // No need to release global_hanlde_ here, because jsni has handled it for you.\\n"\
"        delete wrap;\\n"\
"    }\\n"\
"    JSGlobalValueRef global_hanlde_ = nullptr;\\n"\
"};\\n"\
"#endif\\n"\


def print_usage():
  print "# Usage:"
  print "# python v8tojsni.py $file"

def execute_sed(pattern, src_file):
  execute_in_shell(sed_command(pattern, src_file))

def execute_in_shell(command):
  if IS_PRINT_COMMAND == True:
    print command
  os.system(command)

def compose_command(command_array):
  command = ""
  for x in command_array:
    command += x + " ";
  return command

def sed_command(pattern, file):
  return compose_command(["sed -i", '\"' + pattern + '\"', file])

def execute_patterns(patterns, src_file):
  for pattern in patterns:
    command = sed_command(pattern, src_file)
    execute_in_shell(command)

# For apis in list v8s/jsnis, directly do replacing here.
def replace_keywords(src_file):
  length = len(v8s)
  assert(length == len(jsnis))
  for i in xrange(0, length):
    v8 = v8s[i]
    jsni = jsnis[i]
    pattern = "s/" + v8 + "/" + jsni + "/g"
    command = sed_command(pattern, src_file)
    execute_in_shell(command)

# For v8 keywords: Local<Value>/Handle<Value>.
def replace_Local(src_file):
  pattern = "s/\(Local<\)[a-zA-Z0-9_]*\(>\) /JSValueRef /g"
  command = sed_command(pattern, src_file)
  execute_in_shell(command)
  pattern = "s/\(Handle<\)[a-zA-Z0-9_]*\(>\) /JSValueRef /g"
  command = sed_command(pattern, src_file)
  execute_in_shell(command)

# Convert v8 api with argument number 0.
def convert_arg_zero(src_file, src_list, dest, is_dot):
  if is_dot:
    call_str = "."
  else:
    call_str = "->"
  postfix = "\)()/" + dest + "(env, \\1)/g"
  patterns = []

  prefix2 = "s/\([a-zA-Z0-9_]\+([a-zA-Z0-9_]\+)\)" + call_str +  "\("
  for x in src_list:
    pattern = prefix2 + x + postfix
    patterns.append(pattern)

  prefix = "s/\([a-zA-Z0-9_]*\)" + call_str +  "\("
  for x in src_list:
    pattern = prefix + x + postfix
    patterns.append(pattern)
  execute_patterns(patterns, src_file)

# convert v8 api beginning with dot with argument number 0.
def convert_dot_arg_zero(src_file, src_list, dest):
  convert_arg_zero(src_file, src_list, dest, True)

# convert v8 api beginning with array with argument number 0.
# e.g. JSNIARGS(1)->BooleanValue();
# with JSNIARGS(1) matches \1; BooleanValue matches \2
# to   JSNITOCBool(JSNIARGS(1));
def convert_arrow_arg_zero(src_file, src_list, dest):
  convert_arg_zero(src_file, src_list, dest, False)

def convert_boolean(src_file):
  booleans = ["BooleanValue"]
  dest = "JSNIToCBool"
  convert_arrow_arg_zero(src_file, booleans, dest)

def convert_number(src_file):
  numbers = ["Int32Value", "NumberValue",\
             "IntegerValue", "Uint32Value",\
            ]
  dest = "JSNIToCDouble"
  convert_arrow_arg_zero(src_file, numbers, dest)

# For v8 api: IsInt32/IsUint32.
def replace_is_type_need_convert(src_file):
  types = ["IsInt32", "IsUint32"]
  dest = "JSNIIsNumber"
  convert_arrow_arg_zero(src_file, types, dest)

def convert_is_empty(src_file):
  isEmptys = ["IsEmpty"]
  dest = "JSNI\\2"
  convert_dot_arg_zero(src_file, isEmptys, dest)

# For common v8 Is#Type apis. These api can be converted directly.
def replace_is_type(src_file):
  # eg. jsniPos->IsObject()
  #     JSNIARGS(0)->IsObject()
  # "s/\([a-zA-Z0-9_]*\)->Is\([a-zA-Z]*\)()/JSNIIs\\2(env, \\1)/g",\
  #
  Types = ["Object", "Symbol", "Undefined", "Null",\
           "Boolean", "Number", "String",\
           "Function", "Array", "TypedArray",\
          ]
  isTypes = [ "Is" + x for x in Types ]
  dest = "JSNI\\2"
  convert_arrow_arg_zero(src_file, isTypes, dest)

# Replace args which declared as FunctionCallbackInfo args.
def replace_args(src_file):
  # sed -i s/\(args\[\)\([0-9]\+\)]/DEFINE(\2)/g
  pattern = "s/\(args\[\)\([0-9a-zA-Z]\+\)]/JSNIARGS(\\2)/g"
  command = sed_command(pattern, src_file)
  execute_in_shell(command)
  pattern2 = "/JSNIARGS/i #define JSNIARGS(x) JSNIGetArgOfCallback(env, info, x)"
  command2 = sed_command(pattern2, src_file)
  execute_in_shell(command2)

def replace_args_length(src_file):
  # sed -i 's/args.Length()/JSNIGetArgsLengthOfCallback(env, info)/g'
  pattern = "s/args.Length()/JSNIGetArgsLengthOfCallback(env, info)/g"
  command = sed_command(pattern, src_file)
  execute_in_shell(command)

def replace_new(src_file):
  patterns = \
  ["s/Boolean::New\((.*, \)/JSNINewBoolean(env, /g",\
   "s/Array::New\((.*, \)/JSNINewArray(env, /g",\
   "s/Local<.*>::New(.*, /JSNIGetGlobalValue(env, /g",\
  ]
  for pattern in patterns:
    command = sed_command(pattern, src_file)
    execute_in_shell(command)

def handle_specific_addon_things(src_file):
  patterns = \
  ["s/NODE_SET_METHOD(exports/JSNIRegisterMethod(env, exports/g",\
  ]
  execute_patterns(patterns, src_file)

def remove_v8node_things(src_file):
  patterns = \
  ["/=.*Isolate::GetCurrent/d",\
   "s/->ToObject()//g",\
   "s/Local<.*Cast//g",\
   "s/Handle<.*Cast//g", \
   "s/v8:://g",\
   "s/node:://g",\
   "/using namespace.*node;/d",\
   "/using namespace.*v8;/d",\
   "/#include.*v8.*.h/d",\
   "s/#include.*node\.h>/#include <jsni.h>/g",\
   "/#include.*node.*.h/d",\
   "/NODE_MODULE(/d",\
   "/ = args.GetIsolate()/d",\
  ]
  execute_patterns(patterns, src_file)

# To relpace v8 EscapableHandleScope/HandleScope, we need to recognize the code block.
def replace_handle_scope_helper(lines, line_number_begin, scopes_pair, is_escapable):
  file_size = len(lines)
  if line_number_begin >= file_size:
    return
  line_number = line_number_begin
  while line_number < file_size + 1:
    is_end_find = False
    # search handlescope block
    if is_escapable:
      handle_scope_string = " EscapableHandleScope"
    else:
      handle_scope_string = " HandleScope"
    if handle_scope_string in lines[line_number - 1]:

      whitespace_count = lines[line_number - 1].index(handle_scope_string)
      whitespace_string = ""
      for x in xrange(0, whitespace_count + 1):
        whitespace_string += " "

      for blockend_line_number in xrange(line_number + 1, file_size + 1):
        match_line = lines[blockend_line_number - 1]
        is_whitespace = re.search(r"^" + whitespace_string, match_line)
        is_blankline = match_line.strip() == ''
        if not is_whitespace and not is_blankline:
          is_end_find = True
          scopes_pair.append([line_number, blockend_line_number, whitespace_string])
          break
    if is_end_find:
      line_number = blockend_line_number + 1
    else:
      line_number = line_number + 1

def replace_handle_scope(src_file, is_escapable):
  with open(src_file) as f:
    lines = f.readlines()
    scopes_pair = []
    replace_handle_scope_helper(lines, 1, scopes_pair, is_escapable)
    add_pop_count = 0
    for pair in scopes_pair:
      begin = str(pair[0] + add_pop_count)
      end = str(pair[1] + add_pop_count)
      whitespace_string = pair[2]
      delete_pattern = begin + "d"
      if is_escapable:
        push = "JSNIPushEscapableLocalScope(env);"
        pop = "JSNIPopEscapableLocalScope(env);"
      else:
        push = "JSNIPushLocalScope(env);"
        pop = "JSNIPopLocalScope(env);"
      add_push_pattern = begin + "i\\" + whitespace_string + push
      add_pop_pattern = end + "i\\" + whitespace_string + pop
      execute_in_shell(sed_command(delete_pattern, src_file))
      execute_in_shell(sed_command(add_push_pattern, src_file))
      execute_in_shell(sed_command(add_pop_pattern, src_file))
      add_pop_count += 1

def handle_try_catch(src_file):
  remove = "/TryCatch/d"
  execute_in_shell(sed_command(remove, src_file))
  has_caught = "s/\([a-zA-Z0-9_]*\).HasCaught()/JSNIHasException(env)/g"
  execute_in_shell(sed_command(has_caught, src_file))

def handle_node_FatalException(src_file):
  pattern = "s/FatalException(.*)/printf(\\\"Fatal error: %s\\\", __func__);exit(1)/g"
  execute_in_shell(sed_command(pattern, src_file))

# For v8 String::Utf8Value like: String::Utf8Value  xxx(yyy).
def handle_utf8value(src_file):
  previous = "String::Utf8Value \([a-zA-Z0-9_]*\)(\([a-zA-Z0-9_]*\))"
  new = "char \\1[JSNIGetStringUtf8Length(env, \\2) + 1]; JSNIGetStringUtf8Chars(env, \\2, \\1, -1)"
  pattern = "s/" + previous + "/" + new + "/g"
  execute_in_shell(sed_command(pattern, src_file))
  previous2 = "String::Utf8Value \([a-zA-Z0-9_]*\)(\([a-zA-Z0-9_(]*)\))"
  pattern2 = "s/" + previous2 + "/" + new + "/g"
  execute_in_shell(sed_command(pattern2, src_file))

# For node::ObjectWrap
def handle_node_object_wrap(src_file):
  if INSERT_OBJECT_WRAP:
    pattern = "/public.*ObjectWrap/i " + jsni_wrap_src_code + ""
    execute_in_shell(sed_command(pattern, src_file))
  patterns = [
  "s/ ObjectWrap/ jsniObjectWrap/g",\
  "s/ObjectWrap::Unwrap/jsniObjectWrap::Unwrap/g",\
  "s/\(->handle\)([a-zA-Z0-9_(]*)/\\1(env)/g",\
  "s/\(->Wrap(\)/\\1env, /g",\
  "s/\(::Unwrap\)\(<[a-zA-Z0-9_]*>\)(/\\1\\2(env, /g",\
  ]
  for p in patterns:
    execute_in_shell(sed_command(p, src_file))

# For v8: args.This/args.Holder.
def handle_get_this(src_file):
  patterns = [\
  "s/\([a-zA-Z0-9_]*\)\.This()/JSNIGetThisOfCallback(env, \\1)/g",\
  "s/\([a-zA-Z0-9_]*\)\.Holder()/JSNIGetThisOfCallback(env, \\1)/g",\
  ]
  for p in patterns:
    execute_in_shell(sed_command(p, src_file))

def handle_args(src_file):
  replace_args(src_file)
  replace_args_length(src_file)

# Place handle_args_in_the_end to the end of the program because we need it in the beginning to replace others.
def handle_args_in_the_end(src_file):
  patterns = [\
  "s/args/info/g",\
  "s/(args,/(info,/g"\
  ]
  for p in patterns:
    execute_in_shell(sed_command(p, src_file))

# For v8 api: Persistent<Value>::Rest.
def handle_persistent(src_file):
  pattern = "s/\([a-zA-Z0-9_]*\).Reset(\(.*\), \(.*\))/" +\
  "\\1 = JSNINewGlobalValue(\\2, \\3)" +\
  "\/\* TODO \\1 should be released manually in case of memory leak. \*\//g"
  execute_sed(pattern, src_file)

# For v8 api: String::NewFromUtf8.
def handle_string(src_file):
  pattern = "s/String::NewFromUtf8(\(.*\), \(.*\))/JSNINewStringFromUtf8(\\1, \\2, -1)/g"
  execute_sed(pattern, src_file)

def handle_construct_call(src_file):
  pattern = "s/\([a-zA-Z0-9_]*\).IsConstructCall()/" +\
            "true" +\
            "\/\* We do not support IsConstructCall api\*\/" +\
            "/g"
  execute_sed(pattern, src_file)

# For v8 api which is used to init class:FunctionTemplate::New/SetClassName/GetFunction
def handle_init_class(src_file):
  function_pattern = "s/FunctionTemplate::New/JSNINewFunction/g"
  execute_in_shell(sed_command(function_pattern, src_file))
  ## not support SetClassName
  deleteclass_pattern = "s/\(.*\)SetClassName\(.*\)/" + \
                         "\/\/We do not support the function:\\1SetClassName\\2/g"
  execute_in_shell(sed_command(deleteclass_pattern, src_file))
  delete_GetFunction_pattern = "s/->GetFunction()//g"
  execute_sed(delete_GetFunction_pattern, src_file)
  handle_persistent(src_file)
  handle_construct_call(src_file)

# For v8 api which is used to set property: Object::Set.
def hanlde_set_object(src_file):
  pattern = "s/" +\
  "\([a-zA-Z0-9_]*\)->Set(String::NewFromUtf8(.*, " +\
  "\(\".*\"\)), " + \
  "\(.*\))" +\
  "/" +\
  "JSNISetProperty(env, \\1, \\2, \\3);" +\
  "/g"
  execute_sed(pattern, src_file)

# We replace the token isolate with env in the end because we may need isolate token in the beginning.
def handle_isolate_in_the_end(src_file):
  pattern = "s/isolate/env/g"
  execute_in_shell(sed_command(pattern, src_file))

# For v8 api: Array::Length/Array::Get.
def handle_array(src_file):
  length_pattern = "s/\("+ value_name_pattern + "Array" + value_name_pattern + "\)->Length()/JSNIGetArrayLength(env, \\1)/g"
  get_pattern = "s/\("+ value_name_pattern + "Array" + value_name_pattern + "\)->Get(\(.*\))/JSNIGetArrayElement(env, \\1, \\2)/g"
  execute_sed(length_pattern, src_file)
  execute_sed(get_pattern, src_file)

def handle_node_makeCallback(src_file):
  pattern = "s/MakeCallback(\([a-zA-Z0-9_]*\), \([a-zA-Z0-9_]*\), \([a-zA-Z0-9_]*\),/JSNICallFunction(\\1, \\3, \\2,/g"
  execute_sed(pattern, src_file)

def handle_exception(src_file):
 e1 = "isolate->ThrowException(Exception::RangeError(String::NewFromUtf8(isolate,\(.*\))))"
 e2 = "isolate->ThrowException(Exception::ReferenceError(String::NewFromUtf8(isolate,\(.*\))))"
 e3 = "isolate->ThrowException(Exception::SyntaxError(String::NewFromUtf8(isolate,\(.*\))))"
 e4 = "isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,\(.*\))))"
 e5 = "isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate,\(.*\))))"
 pattern1 = "s/" + e1 + "/JSNIThrowRangeErrorException(env,\\1)/g"
 pattern2 = "s/" + e2 + "/JSNIThrowErrorException(env,\\1)/g"
 pattern3 = "s/" + e3 + "/JSNIThrowErrorException(env,\\1)/g"
 pattern4 = "s/" + e4 + "/JSNIThrowTypeErrorException(env,\\1)/g"
 pattern5 = "s/" + e5 + "/JSNIThrowErrorException(env,\\1)/g"
 execute_sed(pattern1, src_file)
 execute_sed(pattern2, src_file)
 execute_sed(pattern3, src_file)
 execute_sed(pattern4, src_file)
 execute_sed(pattern5, src_file)

def save_file(src_file):
  if BACKUP_ORIGINAL:
    backupfile = src_file + BACKUP_ORIGINAL_POSTFIX
    execute_in_shell("cp " + src_file + " " + backupfile)

def v8tojsni():
  src_file = sys.argv[1]
  save_file(src_file)
  replace_Local(src_file)
  # NOTE: remove v8/node first becasue the next functions are depending on this removing.
  handle_args(src_file)
  remove_v8node_things(src_file)
  handle_specific_addon_things(src_file)
  replace_handle_scope(src_file, False)
  replace_handle_scope(src_file, True)
  replace_new(src_file)
  replace_keywords(src_file)
  replace_is_type(src_file)
  replace_is_type_need_convert(src_file)
  # Convert means api replace.
  convert_number(src_file)
  convert_boolean(src_file)
  convert_is_empty(src_file)
  handle_try_catch(src_file)
  handle_node_FatalException(src_file)
  handle_utf8value(src_file)
  handle_node_object_wrap(src_file)
  handle_get_this(src_file)
  handle_init_class(src_file)
  hanlde_set_object(src_file)
  handle_array(src_file)
  handle_node_makeCallback(src_file)
  handle_exception(src_file)
  # The end
  handle_isolate_in_the_end(src_file)
  handle_args_in_the_end(src_file)
  # Leave handle_string here for future use.
  handle_string(src_file)

def main():
  if len(sys.argv) != 2:
    print_usage()
    exit()
  v8tojsni()

if __name__ == '__main__':
  main()
