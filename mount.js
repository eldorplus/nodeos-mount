"use strict";

var _binding = require('./build/Release/mount')
  , Util = require('util')
;

module.exports = {
    _binding: _binding,

    mount: _mount,
    umount: _umount,
    mountSync: _mountSync,
    umountSync: _umountSync,

    unmountSync: Util.deprecate(_umount, 'mount.unmountSync: Use mount.umountSync() instead'),
    unmount: Util.deprecate(_umount, 'mount.unmount: Use mount.umount() instead'),
    MS_RDONLY: 1,
    MS_NOSUID: 2,
    MS_NODEV: 4,
    MS_NOEXEC: 8,
    MS_SYNCHRONOUS: 16,
    MS_REMOUNT: 32,
    MS_MANDLOCK: 64,
    MS_DIRSYNC: 128,
    MS_NOATIME: 1024,
    MS_NODIRATIME: 2048,
    MS_BIND: 4096,
    MS_MOVE: 8192,
    MS_REC: 16384,
    MS_VERBOSE: 32768,
    MS_SILENT: 32768,
    MS_POSIXACL: (1<<16),
    MS_UNBINDABLE: (1<<17),
    MS_PRIVATE: (1<<18),
    MS_SLAVE: (1<<19),
    MS_SHARED: (1<<20),
    MS_RELATIME: (1<<21),
    MS_KERNMOUNT: (1<<22),
    MS_I_VERSION: (1<<23),
    MS_STRICTATIME: (1<<24),
    MS_NOSEC: (1<<28),
    MS_BORN: (1<<29),
    MS_ACTIVE: (1<<30),
    MS_NOUSER: (1<<31),
}

function __makeMountFlags(array) {
    var flags = 0
    for(var i=0; i<array.length; i++) {
        var option = array[i].toLowerCase()
        if(option == "bind") {
            flags |= module.exports.MS_BIND;
        } else if(option == "readonly") {
            flags |= module.exports.MS_RDONLY;
        } else if(option == "remount") {
            flags |= module.exports.MS_REMOUNT;
        } else if(option == "noexec"){
            flags |= module.exports.MS_NOEXEC;
        } else {
            throw new Error("Invalid option: "+option);
        }
    }
    return flags
}

function _mount() {
    var argc = arguments.length
    ;

    //At least [devFile, target, fsType, cb]
    if(argc < 4 || typeof arguments[argc-1] !== 'function') {
        throw new Error('Invalid arguments')
    }

    var cb = arguments[argc-1];

    //ensure that options is an array or number
    if(argc > 4 && (typeof arguments[3] !== 'number' && (typeof arguments[3] === 'object' && arguments[3].constructor !== Array))) {
        throw new Error('Argument options must be an array or number')
    }

    if(argc > 5 && typeof arguments[4] !== 'string') {
        throw new Error('Argument dataStr must be a string')
    }

    if(argc > 6) {
        throw new Error('Too many arguments')
    }

    //Last param is always callback
    var devFile = String(arguments[0])
      , target = String(arguments[1])
      , fsType = String(arguments[2])
      , options = 0
      , dataStr = ''
    ;

    if(argc === 5) {
        options = arguments[3]
    } else if(argc === 6) {
        options = arguments[3]
        dataStr = arguments[4]
    }

    if(options.constructor !== Number) {
        options = __makeMountFlags(options)
    }

    _binding.mount(devFile, target, fsType, options, dataStr, cb)
}

function _mountSync() {
    var argc = arguments.length;

    if(argc < 3) {
        throw new Error('Invalid arguments')
    }

    //ensure that options is an array or number
    if(argc > 4 && (typeof arguments[3] !== 'number' && (typeof arguments[3] === 'object' && arguments[3].constructor !== Array))) {
        throw new Error('Argument options must be an array or number')
    }


    if(argc > 4 && typeof arguments[4] !== 'string') {
        throw new Error('Argument dataStr must be a string')
    }

    if(argc > 5) {
        throw new Error('Too many arguments')
    }

    var devFile = String(arguments[0])
      , target = String(arguments[1])
      , fsType = String(arguments[2])
      , options = []
      , dataStr = ''
    ;

    if(argc === 4) {
        options = arguments[3]
    } else if(argc === 5) {
        options = arguments[3]
        dataStr = arguments[4]
    }

    if(options.constructor !== Number) {
        options = __makeMountFlags(options)
    }


    return _binding.mountSync(devFile, target, fsType, options, dataStr);
}

function _umount(target, cb) {

    //Require exactly 2 parameters
    if(arguments.length !== 2 || typeof cb !== 'function') {
        throw new Error('Invalid arguments')
    }

    _binding.umount(target, cb)
}

function _umountSync(target) {
    //Require exactly 1 parameter
    if(typeof target !== 'string') {
        throw new Error('Invalid arguments')
    }

    return _binding.umountSync(target)
}
