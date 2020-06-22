
#include "stdafx.h"
#include "njobjects.h"
#include "njclientqueue.h"
#include "njpush.h"
#include "njcert.h"
#include "njcache.h"
#include "njtable.h"

namespace NJA {

    void GetVersion(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto v = ToStr(isolate, ClientCoreLoader.GetUClientSocketVersion());
        args.GetReturnValue().Set(v);
    }

    void GetPools(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto v = Uint32::New(isolate, ClientCoreLoader.GetNumberOfSocketPools());
        args.GetReturnValue().Set(v);
    }

    void SetVerifyLocation(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto p0 = args[0];
        if (!p0->IsString()) {
            ThrowException(isolate, "A CA store location string required");
            return;
        }
#if NODE_MODULE_VERSION < 57
        String::Utf8Value str(p0);
#else
        String::Utf8Value str(isolate, p0);
#endif
        unsigned int len = (unsigned int) str.length();
        if (!len) {
            ThrowException(isolate, "A CA store location string cannot be empty");
            return;
        }
        bool ok = ClientCoreLoader.SetVerifyLocation(*str);
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void GetWorkingDir(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        std::string dir(ClientCoreLoader.GetClientWorkDirectory());
        auto v = ToStr(isolate, dir.c_str(), dir.size());
        args.GetReturnValue().Set(v);
    }

    void SetWorkingDir(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto p0 = args[0];
        if (!p0->IsString()) {
            ThrowException(isolate, "A working directory string required");
            return;
        }
#if NODE_MODULE_VERSION < 57
        String::Utf8Value str(p0);
#else
        String::Utf8Value str(isolate, p0);
#endif
        unsigned int len = (unsigned int) str.length();
        if (!len) {
            ThrowException(isolate, "A working directory string cannot be empty");
            return;
        }
        ClientCoreLoader.SetClientWorkDirectory(*str);
    }

    void SetMessageQueuePassword(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto p0 = args[0];
        if (!p0->IsString()) {
            ThrowException(isolate, "A password string required");
            return;
        }
#if NODE_MODULE_VERSION < 57
        String::Utf8Value str(p0);
#else
        String::Utf8Value str(isolate, p0);
#endif
        ClientCoreLoader.SetMessageQueuePassword(*str);
    }

    void KeyAllowed(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        auto p0 = args[0];
        if (p0->IsArray()) {
            Local<Array> jsArr = Local<Array>::Cast(p0);
            unsigned int count = jsArr->Length();
            SPA::CAutoLock al(g_cs);
            g_KeyAllowed.clear();
            for (unsigned int n = 0; n < count; ++n) {
                auto v = jsArr->Get(n);
                if (v->IsString()) {
#if NODE_MODULE_VERSION < 57
                    String::Utf8Value str(v);
#else
                    String::Utf8Value str(isolate, v);
#endif
                    std::string s = *str;
                    Trim(s);
                    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                    g_KeyAllowed.push_back(std::move(s));
                }
            }
            args.GetReturnValue().SetNull();
        } else if (IsNullOrUndefined(p0)) {
            SPA::CAutoLock al(g_cs);
            g_KeyAllowed.clear();
            args.GetReturnValue().SetNull();
        } else {
            ThrowException(isolate, "A buffer containing an array of allowed public key hex strings");
        }
    }

    void InitAll(Local<Object> exports) {
        {
            //make sure static critical sections initialized and pre-allocate an array of SPA::CUQueue
            SPA::CScopeUQueue sb;
            const unsigned int PRE_COUNT = 2;
            for (unsigned int n = 0; n < PRE_COUNT; ++n) {
                SPA::CUQueue *q = new SPA::CUQueue;
                SPA::CScopeUQueue::Unlock(q); //put into memory pool
            }
            SPA::UINT64 index = CAsyncServiceHandler::GetCallIndex();
        }
        NODE_SET_METHOD(exports, "getVersion", GetVersion);
        NODE_SET_METHOD(exports, "getPools", GetPools);
        NODE_SET_METHOD(exports, "setCA", SetVerifyLocation);
        NODE_SET_METHOD(exports, "getWorkingDir", GetWorkingDir);
        NODE_SET_METHOD(exports, "setWorkingDir", SetWorkingDir);
        NODE_SET_METHOD(exports, "setPassword", SetMessageQueuePassword);
        NODE_SET_METHOD(exports, "setKey", KeyAllowed);
        NODE_SET_METHOD(exports, "setPublicKeys", KeyAllowed);
        NJQueue::Init(exports);
        NJSocketPool::Init(exports);

        //Cannot create following objects from JavaScript code
        NJHandler::Init(exports);
        NJFile::Init(exports);
        NJAsyncQueue::Init(exports);
        NJSqlite::Init(exports);
        NJSocket::Init(exports);
        NJClientQueue::Init(exports);
        NJPush::Init(exports);
        NJCert::Init(exports);
        NJCache::Init(exports);
        NJTable::Init(exports);
    }
    NODE_MODULE(NODE_GYP_MODULE_NAME, InitAll)
}
