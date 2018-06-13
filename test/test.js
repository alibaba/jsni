const jsni = require("../index");

var addon = nativeLoad("test");

console.log(addon.hello()); // hello
