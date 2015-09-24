"use strict";


var fs = require('fs')

var detectSeries = require('async').detectSeries

var _binding = require('./build/Release/mount')


module.exports =
{
  _binding: _binding,

  mount:      _mount,
  umount:     _umount,
  mountSync:  _mountSync,
  umountSync: _umountSync,

  overrideOrder: ['ext4', 'ext3', 'ext2'],

  MS_RDONLY:       1,
  MS_NOSUID:       2,
  MS_NODEV:        4,
  MS_NOEXEC:       8,
  MS_SYNCHRONOUS: 16,
  MS_REMOUNT:     32,
  MS_MANDLOCK:    64,
  MS_DIRSYNC:    128,

  MS_NOATIME:    1024,
  MS_NODIRATIME: 2048,
  MS_BIND:       4096,
  MS_MOVE:       8192,
  MS_REC:       16384,
  MS_VERBOSE:   32768,
  MS_SILENT:    32768,

  MS_POSIXACL:    (1<<16),
  MS_UNBINDABLE:  (1<<17),
  MS_PRIVATE:     (1<<18),
  MS_SLAVE:       (1<<19),
  MS_SHARED:      (1<<20),
  MS_RELATIME:    (1<<21),
  MS_KERNMOUNT:   (1<<22),
  MS_I_VERSION:   (1<<23),
  MS_STRICTATIME: (1<<24),
  MS_NOSEC:       (1<<28),
  MS_BORN:        (1<<29),
  MS_ACTIVE:      (1<<30),
  MS_NOUSER:      (1<<31),
}


function makeMountFlags_reduce(flags, option)
{
  option = option.toLowerCase()

  switch(option)
  {
    case "bind":
      flags |= module.exports.MS_BIND;
    break

    case "readonly":
      flags |= module.exports.MS_RDONLY;
    break

    case "remount":
      flags |= module.exports.MS_REMOUNT;
    break

    case "noexec":
      flags |= module.exports.MS_NOEXEC;
    break

    default:
      throw new Error("Invalid option: "+option);
  }

  return flags
}

function makeMountFlags(array)
{
  return array.reduce(makeMountFlags_reduce, 0)
}


function makeMountDataStr(object)
{
  var result = []

  for(var key in object)
    result.push(key+'='+object[key])

  return result.join(',')
}


function checkArguments(devFile, target, fsType, options, dataStr, callback)
{
  if(devFile === undefined) throw new Error('devFile is mandatory')
  if(target  === undefined) throw new Error('target is mandatory')

  if(typeof fsType === 'number' || fsType instanceof Array)
  {
    callback = dataStr
    dataStr  = options
    options  = fsType
    fsType   = undefined
  }

  if(options != undefined && options.constructor.name === 'Object'
  || options instanceof Function)
  {
    callback = dataStr
    dataStr  = options
    options  = undefined
  }

  if(dataStr instanceof Function)
  {
    callback = dataStr
    dataStr  = undefined
  }

  // default values
  fsType  = fsType  || 'auto'
  options = options || 0
  dataStr = dataStr || ''

  //ensure that options is an array or number
  if(typeof options !== 'number' && options.constructor !== Array)
    throw new Error('options must be an array or a number, not '+typeof options)

  //ensure that dataStr is a string or a literal object
  if(typeof dataStr !== 'string' && dataStr.constructor !== Object)
    throw new Error('dataStr must be a string or an object, not '+typeof dataStr)

  if(options instanceof Array)
    options = makeMountFlags(options)

  if(dataStr.constructor === Object)
    dataStr = makeMountDataStr(dataStr)

  return [devFile, target, fsType, options, dataStr, callback]
}


function removeNoDev(value)
{
  return value && value.indexOf('nodev') < 0
}

function trim(value)
{
  return value.trim()
}


function removeDuplicates(item, pos, self)
{
  return self.indexOf(item) === pos;
}

function fsList(data)
{
  var supported = data.split('\n').filter(removeNoDev).map(trim)

  return module.exports.overrideOrder.concat(supported).filter(removeDuplicates)
}


function _mount(devFile, target, fsType, options, dataStr, cb)
{
  var argv = checkArguments(devFile, target, fsType, options, dataStr, cb)

  cb = argv[5]

  //Last param is always callback
  if(typeof cb !== 'function')
    throw new Error('Last argument must be a callback function')

  if(argv[2] === 'auto' && !(argv[3] & module.exports.MS_MOVE))
    fs.readFile('/proc/filesystems', 'utf8', function(error, filesystems)
    {
      if(error) return cb(error)

      filesystems = fsList(filesystems)

      detectSeries(filesystems, function(item, callback)
      {
        argv[2] = item
        argv[5] = function(error)
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
    _binding.mount.apply(_binding, argv)
}

function _mountSync(devFile, target, fsType, options, dataStr)
{
  var argv = checkArguments(devFile, target, fsType, options, dataStr)

  argv.length = 5

  if(argv[2] === 'auto' && !(argv[3] & module.exports.MS_MOVE))
  {
    var filesystems = fs.readFileSync('/proc/filesystems', 'utf8')

    filesystems = fsList(filesystems)

    for(argv[2] in filesystems)
      if(_binding.mountSync.apply(_binding, argv))
        return true

    throw new Error('Unknown filesystem for ' + devFile ? devFile : target)
  }

  return _binding.mountSync.apply(_binding, argv)
}

function _umount(target, cb)
{
  //Require exactly 2 parameters
  if(arguments.length !== 2 || typeof cb !== 'function')
    throw new Error('Invalid arguments')

  _binding.umount(target, cb)
}

function _umountSync(target)
{
  //Require exactly 1 parameter
  if(typeof target !== 'string')
    throw new Error('Invalid arguments')

  return _binding.umountSync(target)
}
