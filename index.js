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
var jsni = {};

buildType = process.config.target_defaults.default_configuration;

var nativeLoad = require("./build/" + buildType + "/nativeLoad").nativeLoad;

function tryComplete(filename) {
  return filename + '.node';
}

jsni.nativeLoad = function(filename) {
  const Module = require("module");
  var workPath = process.cwd();
  searchPaths = [
    workPath + "/build/" + buildType,
  ]
  var filename_resolved = Module._findPath(filename);

  if (!filename_resolved) {
    var completeFilename = tryComplete(filename);

    filename_resolved =
      Module._findPath(completeFilename, searchPaths);

    if (!filename_resolved) {
      var err = new Error(
        "Cannot find module '" + filename
        + "' under: " + searchPaths);
      err.code = "NATIVE_MODULE_NOT_FOUND";
      throw err;
    }
  }

  var native_exports = {};
  nativeLoad(native_exports, filename_resolved);
  return native_exports;
};

jsni.include = '"' + __dirname + "/src/" + '"';

global.nativeLoad = jsni.nativeLoad

module.exports = jsni;