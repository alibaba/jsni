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

const jsni = require('../index');
var native = nativeLoad('test');

var assert = require('assert');
var Module = require('module');

function testVersion() {
  native.testVersion();
}

function testInNativeOnly() {
  testVersion();
}

function testArray() {
  var arr = [];
  var obj0 = {p0: 100};
  var obj1 = {p1: 200};
  arr[0] = obj0;
  arr[1] = obj1;
  var new_array = native.testArray(arr);
  assert(new_array[0] === obj0);
  assert(new_array[1] === obj1);
}

function testBoolean() {
  assert(native.testBoolean());
}

function testErrorInfo() {
  var notPrimitive = {a : 1};
  var notObject = 123;
  var notFunc = 123;

  native.testBoolCheck(notPrimitive);
  native.testNumberCheck(notPrimitive);
  native.testStringCheck(notPrimitive);
  try {
    native.testFunctionCheck(notFunc);
  } catch(e) {
    var indexString = 'is not a function';
    assert.notEqual(e.toString().indexOf(indexString), -1);
  }

  native.testArrayCheck(notObject);
  native.testObjectCheck(notObject);
}

function testException() {
  var str = 'string';

  try {
    native.testThrowTypeError(str);
  } catch(error) {
    assert(error.toString() === 'TypeError: Wrong parameter type.');
  }

  try {
    native.testThrowRangeError(str);
  } catch(error) {
    assert(error.toString() === 'RangeError: Range is out of limit.');
  }

  try {
    native.testThrowError(str);
  } catch(error) {
    assert(error.toString() === 'Error: Error.');
  }

  // Check pending exception and clear pending exception.
  var err = new Error('error.');
  function throwError() {
    throw err;
  }

  var flag = false;

  try {
    native.testHasPendingException(throwError);
  } catch(error) {
    flag = true;
  }
  assert(flag);

  try {
    native.testClearPendingException(throwError);
  } catch(error) {
    console.log('No error should be catched.');
    assert(false);
  }

  // Test Error creation and throw.
  try {
    native.testNewError();
  } catch(error) {
    assert(error.toString() === 'Error: error!!!');
  }

  try {
    native.testNewRangeError();
  } catch(error) {
    assert(error.toString() === 'RangeError: range error!!!');
  }

  try {
    native.testNewTypeError();
  } catch(error) {
    assert(error.toString() === 'TypeError: type error!!!');
  }
}

function testFunction() {
  var func = native.testNewNativeFunction();
  var ret = func(200);
  assert(ret === 200);

  assert(typeof func === 'function');
  native.testIsFunction(func);

  var ret1 = native.testCallFunction(func);
  assert(ret1 === 200);

  var this_value = native.testGetThis();
  assert(this_value === native);
}

function testGlobalRef() {
  assert(native.testGlobal() === 100);
  native.testGlobalGC();
  native.testGCCallback();
  native.testAcquireRelease();
}

function testInternalField() {
  native.testHidden();
}

function testLocalScope() {
  assert(native.testLocalScope() === 200);
}

function testNull() {
  assert(native.testNull() === null);
}

function testNumber() {
  assert(native.testNewNumber() === 1);
  native.testIsNumber(100);
  native.testNumber2Native(100.1);
  native.testInt32(100);
  native.testUint32(100);
  native.testInt64(100);
}

function testObject() {
  native.testObject();

  var obj = {};
  var protoObj = {proto: 'proto'};
  Object.setPrototypeOf(obj, protoObj);
  assert(protoObj === native.testGetProto(obj));
}

function testProperty() {
  // Test getter and setter.
  var obj = {};
  native.testDefineProperty(obj);

  assert(obj.abc === 101);
  obj.abc = 1000;
  assert(obj.abc === 1001);
  assert(obj.getter === 'getter is set.');
  assert(obj.setter === 'setter is set.');

  // Test define property.
  var obj_2 = {};
  native.testDefineProperty2(obj_2);
  assert(obj_2.abc_2 === 'Hello world!');
}

function testString() {
  assert(native.testUtf8('string') === 'hello, world!');
  native.testString();
}

function testSymbol() {
  var s = native.testSymbol('symbol_s');
  var empty_s = native.testSymbol();

  assert(typeof s === 'symbol');
  assert(s.toString(), 'Symbol(symbol_s)');

  assert(typeof empty_s === 'symbol');
  assert(empty_s.toString(), 'Symbol()');
}

function testTypedArray() {
  var typedArray = native.testCreateTypedArray();

  console.assert(typeof typedArray === 'object',
    'should be an object(typedArray)');

  assert(typedArray.constructor === Uint8Array);
  assert(typedArray[0] === 1);
  assert(typedArray[1] === 2);
  assert(typedArray[2] === 3);

  var arr = [];
  native.testIsArray(arr);

  native.testIsExternailized();
}

function testUndefined() {
  assert(native.testUndefined() === undefined);
}

var test_cases = [
  testInNativeOnly,
  testArray,
  testBoolean,
  testErrorInfo,
  testException,
  testFunction,
  testGlobalRef,
  testInternalField,
  testLocalScope,
  testNull,
  testNumber,
  testObject,
  testProperty,
  testString,
  testSymbol,
  testTypedArray,
  testUndefined,
];

var report = {
  code: 0,
  fail_count: 0,
  pass_count: 0,
  message: ''
};

function getReportPath() {
  return process.argv[2].split('=')[1];
}

function dumpReportFile() {
  var fs = require('fs');
  if (report.message === '') {
    report.message = 'Success.';
  } else {
    report.message = 'Fail cases:' + report.message;
  }

  var report_string = JSON.stringify(report);
  var report_path = getReportPath();

  fs.writeFileSync(report_path, report_string);
  console.log('Report result is saved in ' + report_path);
}

function needDumpReport() {
  if (process.argv[2] != undefined) {
    return process.argv[2].includes('dump');
  }
  return false;
}

function runTest(test) {
  try {
    test();
    report.pass_count += 1;
  } catch (e) {
    report.fail_count += 1;
    report.message += ' ' + test.name;
    console.log(e);
  }
}

function run() {
  test_cases.forEach(function(test) {
    runTest(test);
  });

  if (needDumpReport()) {
    dumpReportFile();
  }
}

run();

process.exit();
