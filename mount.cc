#include <sys/mount.h>
#include <v8.h>
#include <node.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nan.h>

using namespace v8;


struct Mounty {
  // All values except target are only used by mount
  std::string devFile;
  std::string fsType;
  std::string target; // used by umount
  std::string data;
  long flags;
  int error;
};


class MountWorker : public Nan::AsyncWorker {
public:
  MountWorker(Nan::Callback *callback, Mounty *mounty)
    : Nan::AsyncWorker(callback), mounty(mounty) {}
  ~MountWorker() {}

  // This function is executed in another thread at some point after it has been
  // scheduled. IT MUST NOT USE ANY V8 FUNCTIONALITY.
  void Execute() {
    #ifdef __linux__
      int ret = mount(mounty->devFile.c_str(),
              mounty->target.c_str(),
              mounty->fsType.c_str(),
              mounty->flags,
              mounty->data.c_str());
    #elif __APPLE__
      int ret = mount(
              mounty->fsType.c_str(),
              mounty->target.c_str(),
              mounty->flags,
              &mounty->data);
    #endif

    if (ret == -1) {
      mounty->error = errno;
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;

    Local<Value> argv[] = {
      Nan::Null()
    };

    if (mounty->error > 0) {
      argv[0] = Nan::ErrnoException(mounty->error, "mount", "",
                                    mounty->devFile.c_str());
    }

    callback->Call(1, argv);
  }

private:
  Mounty *mounty;
};

class UmountWorker : public Nan::AsyncWorker {
public:
  UmountWorker(Nan::Callback *callback, Mounty *mounty)
    : Nan::AsyncWorker(callback), mounty(mounty) {}
  ~UmountWorker() {}

  void Execute() {
    #ifdef __linux__
      int ret = umount(mounty->target.c_str());
    #elif __APPLE__
      int ret = unmount(
              mounty->target.c_str(),
              mounty->flags);
    #endif

    if (ret == -1) {
      mounty->error = errno;
    }
  }

  void HandleOKCallback() {
    Nan::HandleScope scope;
    Local<Value> argv[] = {
      Nan::Null()
    };

    if (mounty->error > 0) {
      argv[0] = Nan::ErrnoException(mounty->error, "umount", "",
                                    mounty->target.c_str());
    }

    callback->Call(1, argv);

    delete mounty;
  }

private:
  Mounty *mounty;
};


//       0        1       2       3        4     5
// mount(devFile, target, fsType, options, data, cb)
NAN_METHOD(Mount) {
  Nan::HandleScope scope;

  if (info.Length() != 6) {
    return Nan::ThrowError("Invalid number of arguments (must be 6)");
  }

  String::Utf8Value devFile(info[0]->ToString());
  String::Utf8Value target(info[1]->ToString());
  String::Utf8Value fsType(info[2]->ToString());
  Local<Integer> options = info[3]->ToInteger();
  String::Utf8Value dataStr(info[4]->ToString());

  // Prepare data for the async work
  Mounty* mounty = new Mounty();
  mounty->devFile = std::string(*devFile);
  mounty->target = std::string(*target);
  mounty->fsType = std::string(*fsType);
  mounty->flags = options->Value();
  mounty->data = std::string(*dataStr);

  Nan::Callback *callback = new Nan::Callback(info[5].As<Function>());

  Nan::AsyncQueueWorker(new MountWorker(callback, mounty));
  return;
}

NAN_METHOD(Umount) {
  Nan::HandleScope scope;

  if (info.Length() != 2) {
    return Nan::ThrowError("Invalid number of arguments (must be 2)");
  }

  String::Utf8Value target(info[0]->ToString());

  // Prepare data for the async work
  Mounty* mounty = new Mounty();
  mounty->target = std::string(*target);

  Nan::Callback *callback = new Nan::Callback(info[1].As<Function>());

  Nan::AsyncQueueWorker(new UmountWorker(callback, mounty));
  return;
}


NAN_METHOD(MountSync) {
  Nan::HandleScope scope;

  if (info.Length() != 5) {
    return Nan::ThrowError("Invalid number of arguments (must be 5)");
  }

  String::Utf8Value devFile(info[0]->ToString());
  String::Utf8Value target(info[1]->ToString());
  String::Utf8Value fsType(info[2]->ToString());
  Handle<Integer> options = info[3]->ToInteger();
  String::Utf8Value dataStr(info[4]->ToString());

  std::string s_devFile(*devFile);
  std::string s_target(*target);
  std::string s_fsType(*fsType);
  std::string s_dataStr(*dataStr);

  #ifdef __linux__
    int ret = mount(s_devFile.c_str(),
            s_target.c_str(),
            s_fsType.c_str(),
            options->Value(),
            s_dataStr.c_str());
  #elif __APPLE__
    int ret = mount(
            s_fsType.c_str(),
            s_target.c_str(),
            options->Value(),
            &s_dataStr);
  #endif

  if (ret) {
    return Nan::ThrowError(Nan::ErrnoException(errno, "mount", "",
                                               s_devFile.c_str()));
  }

  info.GetReturnValue().Set(Nan::True());
}

NAN_METHOD(UmountSync) {
  Nan::HandleScope scope;

  if (info.Length() != 1) {
    return Nan::ThrowError("Invalid number of arguments (must be 1)");
  }

  String::Utf8Value target(info[0]->ToString());

  std::string s_target(*target);

  #ifdef __linux__
    int ret = umount(s_target.c_str());
  #elif __APPLE__
    int ret = unmount(
            s_target.c_str(),
            0);
  #endif

  if (ret) {
    return Nan::ThrowError(Nan::ErrnoException(errno, "umount", "",
                                               s_target.c_str()));
  }

  info.GetReturnValue().Set(Nan::True());
}


void init (Handle<Object> exports) {
  exports->Set(Nan::New<String>("mount").ToLocalChecked(),
    Nan::New<FunctionTemplate>(Mount)->GetFunction());
  exports->Set(Nan::New<String>("umount").ToLocalChecked(),
    Nan::New<FunctionTemplate>(Umount)->GetFunction());
  exports->Set(Nan::New<String>("mountSync").ToLocalChecked(),
    Nan::New<FunctionTemplate>(MountSync)->GetFunction());
  exports->Set(Nan::New<String>("umountSync").ToLocalChecked(),
    Nan::New<FunctionTemplate>(UmountSync)->GetFunction());
}

NODE_MODULE(mount, init)
