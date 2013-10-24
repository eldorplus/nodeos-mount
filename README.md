# node-mount
Copyright (C) 2011 by Maciej Małecki  
MIT License (see LICENSE file)

Mount your device from node.js like what.

## Usage

```javascript
var mount = require('mount');
mount.mount('tmpfs', 'tmpDir', 'tmpfs', function(success) {
	// ... do some stuff in tmpDir
});

mount.unmount('tmpDir', function(success) {
	// BAH! everything from tmpDir is gone!
});
```

## Installation

    npm install mount

