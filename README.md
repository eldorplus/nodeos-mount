# node-mount

Mount/unmount devices from node.js

Really works on linux, may work on OS X, and will never work on windows.

This version was created to apply some small corrections and offers a more verbose documentation.
The most opinionated change was the function name change of `unmount` to `umount` since I 
like consistency through the whole stack.

##Installation
This package is not featured in NPM, so you have to add this file in your package.json
file:

**Part of package.json:**
```
{
    "name" : "myproject",
    "version" : "1.0",
    ...
    ...
    "dependencies" : {
        "mount" : "git+ssh://git@github.com:magicpat/node-mount.git"
    }
}
```
## Syntax

### mount
```javascript
mount(devFile, mntPath, fsType, options, data)
```
- `devFile` - {String} Device-File being mounted (located in /dev)
- `mntPath` - {String} Directory to mount the device to 
- `options` - {Array.<String>} Array containing String options (see reference list)

*options*
These (limited number of) options are equivalent to the mountflags parameter used by
the C mount function (http://man7.org/linux/man-pages/man2/mount.2.html). 
Following options are available: 
- `bind` 
- `readonly`
- `remount`
- `noexec`

### umount*
```javascript
umount(target)
```
- `target` - {String} Mount-Path to umount (equivalent to `mntPath` of `mount`)

## Usage

*Mount Tmpfs:*
```javascript
var mount = require('mount');
mount.mount('tmpfs', 'tmpDir', 'tmpfs', function(success) {
	// Do hard job.
	mount.unmount('tmpDir', function(success) {
		// Finish hard job! YAY.
	});
});
```

*Mount DVD:*
```javascript
var mount = require('mount');
mount.mount('/dev/sr0', 'myDir', 'iso9660', function(success) {
	// Do something 
	mount.unmount('myDir', function(success) {
		// Do something 
	});
});
```

## Credits
- Directly forked from Stackdot`s [repository](https://github.com/stackdot/node-mount)
- Stackdot forked this from Maciej Małecki`s [repository](https://github.com/mmalecki/node-mount)
