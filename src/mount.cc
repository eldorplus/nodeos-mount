#include <sys/mount.h>
#include <v8.h>
#include <node.h>

v8::Handle<v8::Value> Mount(const v8::Arguments &args) {
  v8::HandleScope scope;

  if (args.Length() < 3) {
    // TODO: make it raise a proper exception
    return v8::ThrowException(v8::String::New("`mount` needs at least 3 parameters"));
  }

  bool callback = false;
  v8::Local<v8::Function> cb;
  v8::Handle<v8::Array> options;

  if(args.Length() == 4) {
    callback = true;
    cb = v8::Local<v8::Function>::Cast(args[3]);
    options = v8::Array::New(0);
  } else if(args.Length() == 5) {
    callback = true;
    cb = v8::Local<v8::Function>::Cast(args[4]);
    if (args[3]->IsArray()) {
      options = v8::Handle<v8::Array>::Cast(args[3]);
    }
  }

  int mask = 0;
  for (unsigned int i = 0; i < options->Length(); i++) {
    v8::Local<v8::String> opt = v8::Local<v8::String>::Cast(options->Get(i));
    if(opt == v8::String::New("bind")) {
      mask |= MS_BIND;
    } else if(opt == v8::String::New("readonly")) {
      mask |= MS_RDONLY;
    } else if(opt == v8::String::New("remount")) {
      mask |= MS_REMOUNT;
    }
  }

  v8::String::Utf8Value device(args[0]->ToString());
  v8::String::Utf8Value path(args[1]->ToString());
  v8::String::Utf8Value type(args[2]->ToString());

  bool mountAction = (mount(*device, *path, *type, mask, NULL) == 0) ? true : false;

  v8::Local<v8::Value> mounted = v8::Local<v8::Value>::New(v8::Boolean::New(mountAction));

  if(callback) {
    const unsigned argc = 1;
    v8::Local<v8::Value> argv[argc] = {mounted};
    cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  return mounted;
}

v8::Handle<v8::Value> Umount(const v8::Arguments &args) {
  v8::HandleScope scope;

  if (args.Length() < 1) {
    return v8::ThrowException(v8::String::New("`umount` needs at least 1 parameter"));
  }

  bool callback = false;
  v8::Local<v8::Function> cb;
  
  if(args.Length() == 2) {
    callback = true;
    cb = v8::Local<v8::Function>::Cast(args[1]);
  }

  v8::String::Utf8Value path(args[0]->ToString());

  bool mountAction = (umount(*path) == 0) ? true : false;

  v8::Local<v8::Value> mounted = v8::Local<v8::Value>::New(v8::Boolean::New(mountAction));

  if(callback) {
    const unsigned argc = 1;
    v8::Local<v8::Value> argv[argc] = {mounted};
    cb->Call(v8::Context::GetCurrent()->Global(), argc, argv);
  }
  return mounted;
}

void init (v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module) {
  exports->Set(v8::String::NewSymbol("mount"), v8::FunctionTemplate::New(Mount)->GetFunction());
  exports->Set(v8::String::NewSymbol("umount"), v8::FunctionTemplate::New(Umount)->GetFunction());
}
NODE_MODULE(mount, init)

