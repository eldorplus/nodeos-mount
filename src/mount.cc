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
    int flags;

    int error;
    bool success;  
};

//        0         1       2       3       4
//mount(devFile, target, fsType, options, data) 
Handle<Value> Mount(const Arguments &args) {
    HandleScope scope;
    
    int argsLength = args.Length();
    if (argsLength <= 2) {
        if (argsLength == 0){
            return ThrowException(String::New("Missing 'devFile'"));
        }

        if (argsLength == 1){
            return ThrowException(String::New("Missing 'target'"));
        }

        if(argsLength == 2){
            return ThrowException(String::New("Missing 'fsType'"));
        }

        return ThrowException(String::New("mount needs at least 3 parameters"));
    }
   
    Local<Function> cb;
    Handle<Array> options = Array::New(0);
    Local<Value> data = String::New("");

    if(!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()){
        return ThrowException(String::New("Wrong argument types... expected strings"));
    }

    //Check and set options array (ignores all non-array values)
    if(argsLength >= 3) {
        if (args[3]->IsArray()) {
            options = Handle<Array>::Cast(args[3]);
        }
    } 
   
    if(argsLength == 6) {
        if (args[4]->IsString()) {
            data = args[4]; 
        }
    }

    //Set callback, if provided as last argument
    if(args[argsLength - 1]->IsFunction()){
        cb = Local<Function>::Cast(args[argsLength - 1]);
    }
    else{
        cb = FunctionTemplate::New()->GetFunction();
    }

    int flags = 0;
    for (unsigned int i = 0; i < options->Length(); i++) {
        String::Utf8Value str(options->Get(i)->ToString());

        if(!strcmp(*str, "bind")) {
            flags |= MS_BIND;
        } else if(!strcmp(*str, "readonly")) {
            flags |= MS_RDONLY;
        } else if(!strcmp(*str, "remount")) {
            flags |= MS_REMOUNT;
        } else if(!strcmp(*str, "noexec")){
            flags |= MS_NOEXEC;
        }
    }

    String::Utf8Value devFile(args[0]->ToString());
    String::Utf8Value target(args[1]->ToString());
    String::Utf8Value fsType(args[2]->ToString());
    String::Utf8Value dataStr(data->ToString());

    //Prepare data for the async work
    Mounty* mounty = new Mounty();

    mounty->callback = Persistent<Function>::New(cb);
    mounty->success = false;

    mounty->devFile = std::string(*devFile);
    mounty->target = std::string(*target);
    mounty->fsType = std::string(*fsType);
    mounty->data = std::string(*dataStr);
    mounty->flags = flags;

    //Create the Async work and set the prepared data
    uv_work_t *req = new uv_work_t();
    req->data = mounty;

    int status = uv_queue_work(uv_default_loop(), req, AsyncMount, (uv_after_work_cb)AsyncAfter);

    assert(status == 0);

    return scope.Close(Undefined()); 
}

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

    mounty->success = (ret == 0);
}

//Used for both, mount and umount since they have the same callback interface
void AsyncAfter(uv_work_t* req){
    HandleScope scope;
    Mounty* mounty = static_cast<Mounty*>(req->data);

    //Call error-callback, if error... otherwise send result
    if(mounty->error > 0){
        Local<String> s = Integer::New((int32_t)mounty->error)->ToString();
        Local<Value> err = Exception::Error(s);

        const unsigned argc = 1;
        Local<Value> argv[argc] = { err };

        TryCatch tc;
        mounty->callback->Call(Context::GetCurrent()->Global(), argc, argv);

        if(tc.HasCaught()){
            node::FatalException(tc);
        }
    }
    else{
        const unsigned argc = 2;
        Local<Value> argv[argc] = {
            Local<Value>::New(Null()),
            Local<Value>::New(Boolean::New(mounty->success))
        };

        TryCatch tc;
        mounty->callback->Call(Context::GetCurrent()->Global(), argc, argv);

        if(tc.HasCaught()){
            node::FatalException(tc);
        }
    }

    mounty->callback.Dispose();

    delete mounty;
    delete req;
}

Handle<Value> Umount(const Arguments &args) {
    HandleScope scope;

    if (args.Length() < 1) {
        return ThrowException(String::New("Missing argument 'target'"));
    }

    Local<Function> cb;

    if(args.Length() == 2 && args[1]->IsFunction()) {
        cb = Local<Function>::Cast(args[1]);
    }
    else{
        cb = FunctionTemplate::New()->GetFunction();
    }

    String::Utf8Value target(args[0]->ToString());

    //Prepare data for the async work
    Mounty* mounty = new Mounty();

    mounty->callback = Persistent<Function>::New(cb);
    mounty->target = std::string(*target);

    //Create the Async work and set the prepared data
    uv_work_t *req = new uv_work_t();
    req->data = mounty;

    int status = uv_queue_work(uv_default_loop(), req, AsyncUmount, (uv_after_work_cb)AsyncAfter);

    assert(status == 0);

    return scope.Close(Undefined());
}

void AsyncUmount(uv_work_t *req){
    Mounty* mounty = static_cast<Mounty*>(req->data);

    int ret = umount(mounty->target.c_str()); 

    //Save error-code
    if(ret == -1){
        mounty->error = errno;
    }

    mounty->success = (ret == 0);
}

void init (Handle<Object> exports, Handle<Object> module) {
    exports->Set(String::NewSymbol("mount"), FunctionTemplate::New(Mount)->GetFunction());
    exports->Set(String::NewSymbol("umount"), FunctionTemplate::New(Umount)->GetFunction());
}

NODE_MODULE(mount, init)
