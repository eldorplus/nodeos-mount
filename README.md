# node-mount

Mount/unmount devices from node.js

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

## Installation

    npm install http://github.com/stackdot/node-mount.git

This module is forked from Maciej Małecki`s [repository]("https://github.com/mmalecki/node-mount"), but completely rewritten.