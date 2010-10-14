/* This code is PUBLIC DOMAIN, and is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND. See the accompanying 
 * LICENSE file.
 */

#include <v8.h>
#include <node.h>

#include <unistd.h>

#include <sys/types.h>
#include <dirent.h>

using namespace node;
using namespace v8;

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a function")));  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

class DirReaderEio: ObjectWrap
{
private:
  //int m_count;
  DIR *dir;
public:

  static Persistent<FunctionTemplate> s_ct;
  static void Init(Handle<Object> target)
  {
    HandleScope scope;

    Local<FunctionTemplate> t = FunctionTemplate::New(New);

    s_ct = Persistent<FunctionTemplate>::New(t);
    s_ct->InstanceTemplate()->SetInternalFieldCount(1);
    s_ct->SetClassName(String::NewSymbol("DirReaderEio"));

    NODE_SET_PROTOTYPE_METHOD(s_ct, "open", Open);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "close", Close);
    NODE_SET_PROTOTYPE_METHOD(s_ct, "read", Read);

    target->Set(String::NewSymbol("DirReaderEio"),
                s_ct->GetFunction());
  }

  DirReaderEio()
  {

  }

  ~DirReaderEio()
  {
   if(this->dir)
    closedir(this->dir);
  }

  static Handle<Value> New(const Arguments& args)
  {
    HandleScope scope;
    DirReaderEio* hw = new DirReaderEio();
    hw->Wrap(args.This());
    return args.This();
  }

  struct hello_baton_t {
    DirReaderEio *hw;
    Persistent<String> path;
    //DIR *dir;
    Persistent<Function> cb;
    Local<Array> files;
    int num;
    bool done;
  };

 ///////////////////////////////////////////OPEN//////////////
  static Handle<Value> Open(const Arguments& args)
  {
    HandleScope scope;

    REQ_FUN_ARG(1, cb);

    DirReaderEio* hw = ObjectWrap::Unwrap<DirReaderEio>(args.This());

    hello_baton_t *baton = new hello_baton_t();
    baton->hw = hw;
    baton->cb = Persistent<Function>::New(cb);
    Local<String> path = Local<String>::Cast(args[0]);
    baton->path = Persistent<String>::New(path);
    hw->Ref();

    eio_custom(EIO_Open, EIO_PRI_DEFAULT, EIO_AfterOpen, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Open(eio_req *req)
  {
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);

    //sleep(baton->sleep_for);
    String::Utf8Value path(baton->path->ToString());
    DIR *dir = opendir(*path);
    baton->hw->dir=dir;
    //baton->hw->m_count += baton->increment_by;

    return 0;
  }

  static int EIO_AfterOpen(eio_req *req)
  {
    HandleScope scope;
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);

    Local<Value> argv[1];
    if(baton->hw->dir)
     argv[0] = Local<Value>::New(True());
    else
     argv[0] = Local<Value>::New(False());

    ev_unref(EV_DEFAULT_UC);
    baton->hw->Unref();

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    baton->path.Dispose();
    baton->cb.Dispose();

    delete baton;
    return 0;
  }




/////////////////////////////////////READ///////
  static Handle<Value> Read(const Arguments& args)
  {
    HandleScope scope;
    REQ_FUN_ARG(1, cb);

    DirReaderEio* hw = ObjectWrap::Unwrap<DirReaderEio>(args.This());

    hello_baton_t *baton = new hello_baton_t();
    baton->hw = hw;

//    if(args.Length()==2)
  //  {
     baton->num= args[0]->Int32Value();
   // }
   // else
   // {
   //  baton->num=1;
   // }
    baton->cb = Persistent<Function>::New(cb);
    hw->Ref();

    eio_custom(EIO_Read, EIO_PRI_DEFAULT, EIO_AfterRead, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Read(eio_req *req)
  {
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);
    DIR *dir = baton->hw->dir;
    int num=baton->num;
    char *name;
    int i = 0;
    struct dirent *ent;
    baton->done=false;
    //HandleScope scope;
    Local<Array> files = Array::New();
    //Local<Array> files = Array::New();
    while (true)
    {
      if(i>num) break;
       ent = readdir(dir);
      if(!ent)
      {
       baton->done=true;
       break;
      }
      name = ent->d_name;

      if ( (name[0] == '.' && !name[1]) || ( name[0]=='.' && name[1] == '.' && !name[2])) continue;
      files->Set(Integer::New(i), String::New(name));
      i++;
    }
    baton->files=files;
    // return scope.Close(files);
    return 0;
  }

  static int EIO_AfterRead(eio_req *req)
  {
    HandleScope scope;
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);

    Local<Value> argv[2];
    //if(!baton->hw->dir) // error
    argv[0]=baton->files;
    argv[1]=Local<Value>::New(baton->done?True():False());
    ev_unref(EV_DEFAULT_UC);
    baton->hw->Unref();

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 2, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    //baton->files.Dispose();
    baton->cb.Dispose();

    delete baton;
    return 0;
  }

/////////////////////////////////////////////////
  static Handle<Value> Close(const Arguments& args)
  {
    HandleScope scope;

    REQ_FUN_ARG(0, cb);

    DirReaderEio* hw = ObjectWrap::Unwrap<DirReaderEio>(args.This());

    hello_baton_t *baton = new hello_baton_t();
    baton->hw = hw;
    baton->cb = Persistent<Function>::New(cb);
    //Local<String> path = Local<String>::Cast(args[0]);
   // baton->path = Persistent<String>::New(path);
    hw->Ref();

    eio_custom(EIO_Close, EIO_PRI_DEFAULT, EIO_AfterClose, baton);
    ev_ref(EV_DEFAULT_UC);

    return Undefined();
  }


  static int EIO_Close(eio_req *req)
  {
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);

    //sleep(baton->sleep_for);
    closedir(baton->hw->dir);

    //baton->hw->m_count += baton->increment_by;

    return 0;
  }

  static int EIO_AfterClose(eio_req *req)
  {
    HandleScope scope;
    hello_baton_t *baton = static_cast<hello_baton_t *>(req->data);

    Local<Value> argv[1];
    if(baton->hw->dir)
     argv[0] = Local<Value>::New(True());
    else
     argv[0] = Local<Value>::New(False());

    ev_unref(EV_DEFAULT_UC);
    baton->hw->Unref();

    TryCatch try_catch;

    baton->cb->Call(Context::GetCurrent()->Global(), 1, argv);

    if (try_catch.HasCaught()) {
      FatalException(try_catch);
    }

    //baton->path.Dispose();
    baton->cb.Dispose();

    delete baton;
    return 0;
  }

};

Persistent<FunctionTemplate> DirReaderEio::s_ct;

extern "C" {
  static void init (Handle<Object> target)
  {
    DirReaderEio::Init(target);
  }

  NODE_MODULE(DirReader_eio, init);
}
