# jsni
jsni is the interface for JavaScript and C/C++, which is vm neutral and keeps abi/api compatible.

It is acronym for JavaScript Native Interface.

## Usage
### Required
It is required that the node version is v7.0.0 or higher.

First, prepare your addon written by jsni. Here are [examples](https://github.com/alibaba/jsni/tree/example).

Second, install jsni using npm:

    npm install jsni --save

Then, require the jsni package and use it like this:

    var jsni = require("jsni");
    var addon = nativeLoad("addon");
    console.log(addon.hello());

## License
BSD-3-Clause
