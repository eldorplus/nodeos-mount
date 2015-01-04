#include <sys/mount.h>
#include <v8.h>
#include <node.h>
#include <errno.h>
#include <string.h>
#include <iostream>
#include <nan.h>

using namespace v8;

struct Mounty {
    //All values except target are only used by mount
    std::string devFile;
    std::string fsType;
    std::string target; //used by umount
    std::string data;
    long flags;
    int error;
};

class MountWorker : public NanAsyncWorker {
public:
    MountWorker(NanCallback *callback, Mounty *mounty)
        : NanAsyncWorker(callback), mounty(mounty) {}
    ~MountWorker() {}

    // This function is executed in another thread at some point after it has been
    // scheduled. IT MUST NOT USE ANY V8 FUNCTIONALITY.
    void Execute() {
        int ret = mount(mounty->devFile.c_str(),
                        mounty->target.c_str(),
                        mounty->fsType.c_str(),
                        mounty->flags,
                        mounty->data.c_str());

        if (ret == -1) {
            mounty->error = errno;
        }
    }

    void HandleOKCallback() {
        NanScope();

        Local<Value> argv[] = {
            NanNull()
        };

        if (mounty->error > 0) {
            argv[0] = node::ErrnoException(mounty->error, "mount", "", mounty->devFile.c_str());
        }

        callback->Call(1, argv);
    }

private:
    Mounty *mounty;
};

class UmountWorker : public NanAsyncWorker {
public:
    UmountWorker(NanCallback *callback, Mounty *mounty)
        : NanAsyncWorker(callback), mounty(mounty) {}
    ~UmountWorker() {}

    void Execute() {
        int ret = umount(mounty->target.c_str());

        if (ret == -1) {
            mounty->error = errno;
        }
    }

    void HandleOKCallback() {
        NanScope();
        Local<Value> argv[] = {
            NanNull()
        };

        if (mounty->error > 0) {
            argv[0] = node::ErrnoException(mounty->error, "umount", "", mounty->target.c_str());
        }

        callback->Call(1, argv);
    }

private:
    Mounty *mounty;
};

//        0         1       2       3       4   5
//mount(devFile, target, fsType, options, data, cb)
NAN_METHOD(Mount) {
    NanScope();
    if (args.Length() != 6) {
        return NanThrowError("Invalid number of arguments (must be 6)");
    }

    String::Utf8Value devFile(args[0]->ToString());
    String::Utf8Value target(args[1]->ToString());
    String::Utf8Value fsType(args[2]->ToString());
    Local<Integer> options = args[3]->ToInteger();
    String::Utf8Value dataStr(args[4]->ToString());

    //Prepare data for the async work
    Mounty* mounty = new Mounty();
    mounty->devFile = std::string(*devFile);
    mounty->target = std::string(*target);
    mounty->fsType = std::string(*fsType);
    mounty->flags = options->Value();
    mounty->data = std::string(*dataStr);

    NanCallback *callback = new NanCallback(args[5].As<Function>());

    NanAsyncQueueWorker(new MountWorker(callback, mounty));
    NanReturnUndefined();
}

NAN_METHOD(Umount) {
    NanScope();

    if (args.Length() != 2) {
        return NanThrowError("Invalid number of arguments (must be 2)");
    }

    String::Utf8Value target(args[0]->ToString());

    //Prepare data for the async work
    Mounty* mounty = new Mounty();
    mounty->target = std::string(*target);

    NanCallback *callback = new NanCallback(args[1].As<Function>());

    NanAsyncQueueWorker(new UmountWorker(callback, mounty));
    NanReturnUndefined();
}


void init (Handle<Object> exports) {
    exports->Set(NanNew<String>("mount"),
        NanNew<FunctionTemplate>(Mount)->GetFunction());
    exports->Set(NanNew<String>("umount"),
        NanNew<FunctionTemplate>(Umount)->GetFunction());
}

NODE_MODULE(mount, init)
