# node-mount

Mount/umount devices from node.js

Really works on linux, may work on OS X, and will never work on windows.

##Difference to the original version?
- True asynchronous mount call, so it will not block the whole application if my DVD is scratched and takes ages to mount
- Corrected terms... "unmount" was renamed to "umount" (equal to the C-API)
- More flag options (noexec)
- Node-like error-callback to report the original error-code of the C-API
- More detailed documentation 
- Some small tests

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
mount(devFile, target, fsType, options, data)
```
- `devFile` - `{String}` - Device-File being mounted (located in /dev)
- `target` - `{String}` - Directory to mount the device to 
- `options` - `{String[]}` - Array containing String options (see reference list)

*options*
These (limited number of) options are equivalent to the mountflags parameter used by
the C mount function (http://man7.org/linux/man-pages/man2/mount.2.html). 
Following options are available: 
- `bind` 
- `readonly`
- `remount`
- `noexec`

### umount
```javascript
umount(target)
```
- `target` - `{String}` - Target path to umount (equivalent to `target` of `mount`)

## Usage

*Mount Tmpfs:*
```javascript
var mount = require("mount");
mount.mount('tmpfs', 'tmpDir', 'tmpfs', function(err) {
	if(err){
        return;
    }
    //Tmpfs mounted successfully
});
```

*Mount DVD:*
```javascript
var mount = require("mount");
mount.mount('/dev/sr0', 'myDir', 'iso9660', function(err) {
    if(err){
        return;
    }

    //
});
```

*Umount after successful mount:*
```javascript
var mount = requrie("mount");
mount.umount('myDir', function(err) {
    if(err){
        console.log("Umount went wrong: " + err);
        return;
    }

});
```

## Credits
- Directly forked from Stackdot`s [repository](https://github.com/stackdot/node-mount)
- Stackdot forked this from Maciej Małecki`s [repository](https://github.com/mmalecki/node-mount)
