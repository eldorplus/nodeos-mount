#include <sys/mount.h>
#include <v8.h>
#include <node.h>
#include <errno.h>
#include <string.h>
#include <iostream>

//Uses an example of kkaefer's node addon tutorials for an async worker
//
//Link for the used template:
//https://github.com/kkaefer/node-cpp-modules/blob/master/05_threadpool/modulename.cpp#L88

using namespace v8;

Handle<Value> Mount(const Arguments &args);
void AsyncMount(uv_work_t* req);
void AsyncUmount(uv_work_t *req);
void AsyncAfter(uv_work_t* req);


struct Mounty {
    Persistent<Function> callback;

    //All values excpect target are only used by mount
    std::string devFile;
    std::string fsType;
    std::string target; //used by umount
    std::string data;
    long flags;
    int error;
};

void AsyncMount(uv_work_t* req){
    Mounty* mounty = static_cast<Mounty*>(req->data);

    int ret = mount(mounty->devFile.c_str(), 
                    mounty->target.c_str(), 
                    mounty->fsType.c_str(), 
                    mounty->flags, 
                    mounty->data.c_str());

    //Save error-code
    if(ret == -1){
        mounty->error = errno;
    }
}

void AsyncUmount(uv_work_t *req){
    Mounty* mounty = static_cast<Mounty*>(req->data);

    int ret = umount(mounty->target.c_str());

    //Save error-code
    if(ret == -1){
        mounty->error = errno;
    }
}

//Used for both, mount and umount since they have the same callback interface
void AsyncAfter(uv_work_t* req){
    HandleScope scope;
    Mounty* mounty = static_cast<Mounty*>(req->data);

    const unsigned argc = 1;
    Local<Value> argv[argc];

    //Call error-callback, if error... otherwise send result
    if(mounty->error > 0){
        std::string syscall;
        std::string path;
        if(mounty->devFile.empty()) {
            syscall = "umount";
            path = mounty->target;
        } else {
            syscall = "mount";
            path = mounty->devFile;
        }
        argv[0] = node::ErrnoException(mounty->error, syscall.c_str(), "", path.c_str());
    } else {
        argv[0] = Local<Value>::New(Null());
    }

    TryCatch tc;
    mounty->callback->Call(Context::GetCurrent()->Global(), argc, argv);

    if(tc.HasCaught()){
        node::FatalException(tc);
    }

    mounty->callback.Dispose();

    delete mounty;
    delete req;
}


//        0         1       2       3       4   5
//mount(devFile, target, fsType, options, data, cb) 
Handle<Value> Mount(const Arguments &args) {
    HandleScope scope;

    if(args.Length() != 6) {
        return ThrowException(Exception::Error(String::New("Invalid number of arguments (must be 6)")));
    }

    String::Utf8Value devFile(args[0]->ToString());
    String::Utf8Value target(args[1]->ToString());
    String::Utf8Value fsType(args[2]->ToString());
    Local<Integer> options = args[3]->ToInteger();
    String::Utf8Value dataStr(args[4]->ToString());
    Local<Function> cb = Local<Function>::Cast(args[5]);

    //Prepare data for the async work
    Mounty* mounty = new Mounty();
    mounty->devFile = std::string(*devFile);
    mounty->target = std::string(*target);
    mounty->fsType = std::string(*fsType);
    mounty->flags = options->Value();
    mounty->data = std::string(*dataStr);
    mounty->callback = Persistent<Function>::New(cb);

    //Create the Async work and set the prepared data
    uv_work_t *req = new uv_work_t();
    req->data = mounty;

    int status = uv_queue_work(uv_default_loop(), req, AsyncMount, (uv_after_work_cb)AsyncAfter);
    assert(status == 0);
    return scope.Close(Undefined()); 
}

Handle<Value> Umount(const Arguments &args) {
    HandleScope scope;

    if(args.Length() != 2) {
        return ThrowException(Exception::Error(String::New("Invalid number of arguments (must be 2)")));
    }

    String::Utf8Value target(args[0]->ToString());
    Local<Function> cb = Local<Function>::Cast(args[1]);

    //Prepare data for the async work
    Mounty* mounty = new Mounty();
    mounty->target = std::string(*target);
    mounty->callback = Persistent<Function>::New(cb);

    //Create the Async work and set the prepared data
    uv_work_t *req = new uv_work_t();
    req->data = mounty;

    int status = uv_queue_work(uv_default_loop(), req, AsyncUmount, (uv_after_work_cb)AsyncAfter);
    assert(status == 0);
    return scope.Close(Undefined());
}

Handle<Value> MountSync(const Arguments &args) {
    HandleScope scope;

    if(args.Length() != 5) {
        return ThrowException(Exception::Error(String::New("Invalid number of arguments (must be 5)")));
    }

    String::Utf8Value devFile(args[0]->ToString());
    String::Utf8Value target(args[1]->ToString());
    String::Utf8Value fsType(args[2]->ToString());
    Handle<Integer> options = args[3]->ToInteger();
    String::Utf8Value dataStr(args[4]->ToString());

    std::string s_devFile(*devFile);
    std::string s_target(*target);
    std::string s_fsType(*fsType);
    std::string s_dataStr(*dataStr);

    int ret = mount(s_devFile.c_str(), 
                    s_target.c_str(), 
                    s_fsType.c_str(), 
                    options->Value(), 
                    s_dataStr.c_str());

    if(ret != 0){
        return ThrowException(node::ErrnoException(errno, "mount", "", s_devFile.c_str()));
    }

    return scope.Close(True());
}

Handle<Value> UmountSync(const Arguments &args) {
    HandleScope scope;

    if(args.Length() != 1) {
        return ThrowException(Exception::Error(String::New("Invalid number of arguments (must be 1)")));
    }

    String::Utf8Value target(args[0]->ToString());

    std::string s_target(*target);

    int ret = umount(s_target.c_str());
    if(ret != 0){
        return ThrowException(node::ErrnoException(errno, "umount", "", s_target.c_str()));
    }

    return scope.Close(True());
}

void init (Handle<Object> exports, Handle<Object> module) {
    exports->Set(String::NewSymbol("mount"), FunctionTemplate::New(Mount)->GetFunction());
    exports->Set(String::NewSymbol("umount"), FunctionTemplate::New(Umount)->GetFunction());
    exports->Set(String::NewSymbol("mountSync"), FunctionTemplate::New(MountSync)->GetFunction());
    exports->Set(String::NewSymbol("umountSync"), FunctionTemplate::New(UmountSync)->GetFunction());
}

NODE_MODULE(mount, init)
