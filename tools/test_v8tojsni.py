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

def run():
  os.system("cp v8.api v8.api.tmp")
  os.system("./v8tojsni.py v8.api.tmp")

def check():
  with open("v8.api.tmp") as v8:
    with open("jsni.api") as jsni:
      v8lines = v8.readlines()
      jsnilines = jsni.readlines()
      v8size = len(v8lines)
      jsnisize = len(jsnilines)
      assert (v8size == jsnisize), str(v8size) + " not equal to " + str(jsnisize)
      for x in xrange(0, v8size):
        assert (v8lines[x] == jsnilines[x]), v8lines[x] + " not equal to " + jsnilines[x]

def main():
  try:
    run()
  except Exception as e:
    raise
  check()
  print("success.")

if __name__ == '__main__':
  main()
