#include "stdafx.h"
#include "njfile.h"

namespace NJA {

    Persistent<Function> NJFile::constructor;

    NJFile::NJFile(CSFile *file) : NJHandlerRoot(file), m_file(file) {

    }

    NJFile::~NJFile() {
        Release();
    }

    bool NJFile::IsValid(Isolate* isolate) {
        if (!m_file) {
            ThrowException(isolate, "File handler disposed");
            return false;
        }
        return NJHandlerRoot::IsValid(isolate);
    }

    void NJFile::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, u"CAsyncFile"));
        tpl->InstanceTemplate()->SetInternalFieldCount(2);

        NJHandlerRoot::Init(exports, tpl);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Upload", Upload);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Download", Download);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Cancel", Cancel);

        //property
        NODE_SET_PROTOTYPE_METHOD(tpl, "getFilesQueued", getFilesQueued);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getFilesStreamed", getFilesStreamed);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setFilesStreamed", setFilesStreamed);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CAsyncFile"), tpl->GetFunction(ctx).ToLocalChecked());
    }

    Local<Object> NJFile::New(Isolate* isolate, CSFile *ash, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ash;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJFile::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                NJFile *obj = new NJFile((CSFile*) ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CAsyncFile()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJFile::Release() {
        {
            SPA::CAutoLock al(m_cs);
            if (m_file) {
                m_file = nullptr;
            }
        }
        NJHandlerRoot::Release();
    }

    void NJFile::getFilesQueued(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
        if (obj->IsValid(isolate)) {
            size_t data = obj->m_file->GetFilesQueued();
            args.GetReturnValue().Set(Number::New(isolate, (double) data));
        }
    }

    void NJFile::getFilesStreamed(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int data = obj->m_file->GetFilesStreamed();
            args.GetReturnValue().Set(Number::New(isolate, (double) data));
        }
    }

    void NJFile::setFilesStreamed(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int max = 1;
            auto p = args[0];
            if (p->IsUint32()) {
                max = p->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                obj->m_file->SetFilesStreamed(max);
            } else if (!IsNullOrUndefined(p)) {
                ThrowException(isolate, "An unsigned int value expected");
                return;
            }
            args.GetReturnValue().Set(Number::New(isolate, obj->m_file->GetFilesStreamed()));
        }
    }

    void NJFile::Cancel(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
        if (obj->IsValid(isolate)) {
            size_t data = obj->m_file->Cancel();
            args.GetReturnValue().Set(Number::New(isolate, (double) data));
        }
    }

    void NJFile::Upload(const FunctionCallbackInfo<Value>& args) {
        Exchange(false, args);
    }

    void NJFile::Exchange(bool download, const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJFile* obj = ObjectWrap::Unwrap<NJFile>(args.Holder());
        if (obj->IsValid(isolate)) {
            unsigned int flags = SPA::SFile::FILE_OPEN_TRUNCACTED;
            auto p0 = args[0];
            if (!p0->IsString()) {
                ThrowException(isolate, "A local file path required");
                return;
            }
#if NODE_MODULE_VERSION < 57
            String::Utf8Value str0(p0);
#else
            String::Utf8Value str0(isolate, p0);
#endif
            std::wstring local = Utilities::ToWide(*str0);
            auto p1 = args[1];
            if (!p0->IsString()) {
                ThrowException(isolate, "A remote file path required");
                return;
            }
#if NODE_MODULE_VERSION < 57
            String::Utf8Value str1(p1);
#else
            String::Utf8Value str1(isolate, p1);
#endif
            std::wstring remote = Utilities::ToWide(*str1);
            Local<Value> argv[] = {args[2], args[3], args[4]};
            auto p2 = args[5];
            if (p2->IsUint32())
                flags = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            else if (!IsNullOrUndefined(p2)) {
                ThrowException(isolate, "Unsigned int required for file creating flags");
                return;
            }
            SPA::UINT64 index = obj->m_file->Exchange(isolate, 3, argv, download, local.c_str(), remote.c_str(), flags);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJFile::Download(const FunctionCallbackInfo<Value>& args) {
        Exchange(true, args);
    }
}
