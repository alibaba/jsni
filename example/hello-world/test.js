var jsni = require("jsni");

var addon = nativeLoad("test");

console.log(addon.hello()); // hello
