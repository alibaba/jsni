const assert = require('assert');
const jsni = require('../index');

var addon = nativeLoad('test');

assert.equal(addon.hello(), 'hello'); // hello
