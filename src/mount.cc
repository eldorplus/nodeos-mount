#include <sys/mount.h>
#include <v8.h>
#include <node.h>
#include <errno.h>
#include <string.h>
#include <iostream>

using namespace v8;

//        0         1       2       3       4
//mount(devFile, mntPath, fsType, options, data) 
Handle<Value> Mount(const Arguments &args) {
    HandleScope scope;
    
    int argsLength = args.Length();
    if (argsLength <= 2) {
        if (argsLength == 0){
            return ThrowException(String::New("Missing 'devFile'"));
        }

        if (argsLength == 1){
            return ThrowException(String::New("Missing 'mntPath'"));
        }

        if(argsLength == 2){
            return ThrowException(String::New("Missing 'fsType'"));
        }
    }
   
    Local<Function> cb;
    Handle<Array> options;
    Local<String> data;

    if(!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()){
        return ThrowException(String::New("Wrong argument types... expected strings"));
    }

    //Check and set options array (ignores all non-array values)
    if(argsLength >= 3) {
        if (args[3]->IsArray()) {
            options = Handle<Array>::Cast(args[3]);
        }
        else{
            options = Array::New(0);
        }
    } 
   
    if(argsLength == 6) {
        if (args[4]->IsString()) {
            data = Local<String>::Cast(args[4]);
        }
    }

    //Set callback, if provided as last argument
    if(args[argsLength - 1]->IsFunction()){
        cb = Local<Function>::Cast(args[argsLength - 1]);
    }

    int mask = 0;
    for (unsigned int i = 0; i < options->Length(); i++) {
        Local<String> opt = Local<String>::Cast(options->Get(i));
        if(opt == String::New("bind")) {
            mask |= MS_BIND;
        } else if(opt == String::New("readonly")) {
            mask |= MS_RDONLY;
        } else if(opt == String::New("remount")) {
            mask |= MS_REMOUNT;
        } else if(opt == String::New("noexec")){
            mask |= MS_NOEXEC;
        }
    }

    String::Utf8Value device(args[0]->ToString());
    String::Utf8Value path(args[1]->ToString());
    String::Utf8Value type(args[2]->ToString());

    int mountNum = mount(*device, *path, *type, mask, *data);

    bool mountAction = (mountNum == 0) ? true : false;

    Local<Value> mounted = Local<Value>::New(Boolean::New(mountAction));

    //Execute callback, if provided 
    if(cb->IsFunction()) {
        const unsigned argc = 1;
        Local<Value> argv[argc] = {mounted};
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }

    return scope.Close(mounted); 
}

Handle<Value> Umount(const Arguments &args) {
    HandleScope scope;

    if (args.Length() < 1) {
        return ThrowException(String::New("Missing target"));
    }

    bool callback = false;
    Local<Function> cb;

    if(args.Length() == 2) {
        callback = true;
        cb = Local<Function>::Cast(args[1]);
    }

    String::Utf8Value path(args[0]->ToString());

    bool mountAction = (umount(*path) == 0) ? true : false;

    Local<Value> mounted = Local<Value>::New(Boolean::New(mountAction));

    if(callback) {
        const unsigned argc = 1;
        Local<Value> argv[argc] = {mounted};
        cb->Call(Context::GetCurrent()->Global(), argc, argv);
    }

    return scope.Close(mounted);
}

void init (Handle<Object> exports, Handle<Object> module) {
    exports->Set(String::NewSymbol("mount"), FunctionTemplate::New(Mount)->GetFunction());
    exports->Set(String::NewSymbol("umount"), FunctionTemplate::New(Umount)->GetFunction());
}

NODE_MODULE(mount, init)
