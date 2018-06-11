# jsni
jsni is the interface for JavaScript and C/C++, which is vm neutral and keeps abi/api compatible.

It is acronym for JavaScript Native Interface.

## Usage
Prerequisites:
  * node 7.x or above
  * npm

We can try jsni beginning with the [hello-world](https://github.com/alibaba/jsni/tree/example) example.

First, clone it to your local machine.

    git clone https://github.com/alibaba/jsni.git -b example

Second, install jsni using npm:

    cd jsni/hello-world/
    npm install jsni

Finnally we can build and run it:

    node-gyp rebuild
    node test.js

And it will show "hello".

The essential JavaScript code is like this:

    var jsni = require("jsni");
    var addon = nativeLoad("addon");
    console.log(addon.hello());

## Documentation
[API Reference](https://alibaba.github.io/jsni/2.2/html/jsni_8h.html)

## Collaborators
* **Jin Yue** &lt;jinyue.derek@gmail.com&gt;
* **hyj1991** &lt;66cfat66@gmail.com&gt;

## License
BSD-3-Clause
