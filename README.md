# node-mount

Mount/unmount devices from node.js

Really works on linux, may work on OS X, and will never work on windows.

## Installation

```
npm install mnt
```

## Usage

```javascript
var mount = require('mount');
mount.mount('tmpfs', 'tmpDir', 'tmpfs', function(success) {
	// Do hard job.
	mount.unmount('tmpDir', function(success) {
		// Finish hard job! YAY.
	});
});
```

This module is forked from Maciej Małecki`s [repository]("https://github.com/mmalecki/node-mount"), but completely rewritten.