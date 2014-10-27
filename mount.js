"use strict";

var _binding = require('./build/Release/mount')
  , Util = require('util')
;

function _mount() {
    var argc = arguments.length
    ;

    //At least [devFile, target, fsType, cb]
    if(argc < 4 || typeof arguments[argc-1] !== 'function') {
        throw new Error('Invalid arguments')
    }

    var cb = arguments[argc-1];

    //ensure that options is an array 
    if(argc > 4 && (!arguments[3] || arguments[3].constructor !== Array)) {
        throw new Error('Argument options must be an array')
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
      , options = []
      , dataStr = ''
    ;

    if(argc === 5) {
        options = arguments[3]
    } else if(argc === 6) {
        options = arguments[3]
        dataStr = arguments[4]
    }

    _binding.mount(devFile, target, fsType, options, dataStr, cb)
}

function _mountSync() {
    var argc = arguments.length;

    if(argc < 3) {
        throw new Error('Invalid arguments')
    }
    
    //ensure that options is an array 
    if(argc > 3 && (!arguments[3] || arguments[3].constructor !== Array)) {
        throw new Error('Argument options must be an array')
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

module.exports = {
    _binding: _binding,

    mount: _mount,
    umount: _umount,
    mountSync: _mountSync,
    umountSync: _umountSync,

    unmountSync: Util.deprecate(_umount, 'mount.unmountSync: Use mount.umountSync() instead'),
    unmount: Util.deprecate(_umount, 'mount.unmount: Use mount.umount() instead')
}

