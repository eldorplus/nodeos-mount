"use strict";

var _binding = require('./build/Release/mount')

var detectSeries = require('async').detectSeries

module.exports = {
    _binding: _binding,

    mount: _mount,
    umount: _umount,

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

function checkArguments(devFile, target, fsType, options, dataStr)
{
  if(devFile === undefined) throw new Error('devFile is mandatory')
  if(target  === undefined) throw new Error('target is mandatory')

  if(typeof fsType === 'number' || fsType instanceof Array)
  {
    dataStr = options
    options = fsType
    fsType  = undefined
  }

  // default values
  fsType  = fsType  || 'auto'
  options = options || 0
  dataStr = dataStr || ''

  //ensure that options is an array or number
  if(typeof options !== 'number' && options.constructor !== Array)
    throw new Error('Argument options must be an array or number')

  //ensure that dataStr is a string
  if(typeof dataStr !== 'string')
    throw new Error('Argument dataStr must be a string')

  if(options.constructor !== Number)
    options = __makeMountFlags(options)

  return [devFile, target, fsType, options, dataStr]
}

function filterNoDev(value)
{
  return value.indexOf('nodev') > -1
}

function _mount(devFile, target, fsType, options, dataStr, cb) {
  var argc = arguments.length

  //Last param is always callback
  if(typeof arguments[argc-1] !== 'function')
    throw new Error('Last argument must be a callback function')

  cb = arguments[argc-1]

  switch(argc)
  {
    case 3: fsType  = undefined; break
    case 4: options = undefined; break
    case 5: dataStr = undefined; break
  }

  var argv = checkArguments(devFile, target, fsType, options, dataStr)

  if(argv[2] == 'auto')
    fs.readFile('/proc/filesystems', 'utf8', function(error, data)
    {
      if(error) return cb(error)

      var filesystems = data.split('/n').filter(filterNoDev)

      detectSeries(filesystems, function(item, callback)
      {
        argv[2] = item
        argv[6] = function(error)
        {
          callback(!error)
        }

        _binding.mount.apply(_binding, argv)
      },
      function(result)
      {
        cb(result ? null : new Error('Unknown filesystem for ' + devFile ? devFile : target))
      })
    })
  else
  {
    argv.push(cb)

    _binding.mount.apply(_binding, argv)
  }
}

function _umount(target, cb) {
    //Require exactly 2 parameters
    if(arguments.length !== 2 || typeof cb !== 'function') {
        throw new Error('Invalid arguments')
    }

    _binding.umount(target, cb)
}
