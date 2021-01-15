#include "stdafx.h"
#include "njqueue.h"

namespace NJA {
    using SPA::CScopeUQueue;
    Persistent<Function> NJQueue::constructor;
    Persistent<FunctionTemplate> NJQueue::m_tpl;

    const char* NJQueue::NO_BUFFER_AVAILABLE = "No buffer available";

    NJQueue::NJQueue(CUQueue *buffer, unsigned int initialSize, unsigned int blockSize) : m_Buffer(buffer), m_initSize(initialSize), m_blockSize(blockSize), m_StrForDec(false) {
        if (m_Buffer) {
            m_Buffer->ToUtf8(true);
#ifdef WIN32_64
            m_Buffer->TimeEx(true);
#endif
        }
    }

    NJQueue::~NJQueue() {
        CScopeUQueue::Unlock(m_Buffer);
    }

    void NJQueue::Release() {
        CScopeUQueue::Unlock(m_Buffer);
    }

    void NJQueue::Move(SPA::PUQueue& q) {
        CScopeUQueue::Unlock(m_Buffer);
        m_Buffer = q;
        q = nullptr;
    }

    void NJQueue::Ensure() {
        if (!m_Buffer) {
            m_Buffer = CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), m_initSize, m_blockSize);
            m_Buffer->ToUtf8(true);
#ifdef WIN32_64
            m_Buffer->TimeEx(true);
#endif
        }
    }

    bool NJQueue::ToParamArray(NJQueue *obj, CDBVariantArray &vParam) {
        CDBVariant vt;
        vParam.clear();
        if (obj && obj->m_Buffer) {
            try {
                SPA::CUQueue &q = *obj->m_Buffer;
                while (q.GetSize()) {
                    vParam.push_back(vt);
                    q >> vParam.back();
                }
            } catch (...) {
                return false;
            }
        }
        return true;
    }

    bool NJQueue::IsUQueue(Isolate* isolate, Local<Object> obj) {
        HandleScope handleScope(isolate); //required for Node 4.x or later
        Local<FunctionTemplate> cb = Local<FunctionTemplate>::New(isolate, m_tpl);
        return cb->HasInstance(obj);
    }

    void NJQueue::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, u"CUQueue", 7));
        tpl->InstanceTemplate()->SetInternalFieldCount(4);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadBool", LoadBoolean);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadByte", LoadByte);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadAChar", LoadAChar);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadShort", LoadShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadChar", LoadUShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadInt", LoadInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadLong", LoadLong);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadFloat", LoadFloat);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDouble", LoadDouble);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadBytes", LoadBytes);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadAString", LoadAString);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadString", LoadString);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUShort", LoadUShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUInt", LoadUInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadULong", LoadULong);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDecimal", LoadDecimal);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadDate", LoadDate);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadUUID", LoadUUID);
        NODE_SET_PROTOTYPE_METHOD(tpl, "LoadObject", LoadObject);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Load", LoadByClass);
        NODE_SET_PROTOTYPE_METHOD(tpl, "PopBytes", PopBytes);

        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveBool", SaveBoolean);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveByte", SaveByte);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveAChar", SaveAChar);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveShort", SaveShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveChar", SaveUShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveInt", SaveInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveLong", SaveLong);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveFloat", SaveFloat);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDouble", SaveDouble);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveBytes", SaveBytes);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveAString", SaveAString);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveString", SaveString);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUShort", SaveUShort);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUInt", SaveUInt);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveULong", SaveULong);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDecimal", SaveDecimal);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveDate", SaveDate);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveUUID", SaveUUID);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SaveObject", SaveObject);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Save", SaveByClass);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Discard", Discard);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Reset", Empty);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Empty", Empty);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Dispose", Empty);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Realloc", Realloc);
        NODE_SET_PROTOTYPE_METHOD(tpl, "UseStrForDec", UseStrForDec);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getBufferSize", getMaxBufferSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getSize", getSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "setSize", setSize);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getOS", getOS);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CUQueue", 7), tpl->GetFunction(ctx).ToLocalChecked());
        m_tpl.Reset(isolate, tpl);
    }

    void NJQueue::getOS(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (buff)
            args.GetReturnValue().Set(Int32::New(args.GetIsolate(), (int) buff->GetOS()));
        else
            args.GetReturnValue().Set(Int32::New(args.GetIsolate(), (int) SPA::GetOS()));
    }

    void NJQueue::Discard(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        unsigned int ret = 0;
        Isolate* isolate = args.GetIsolate();
        CUQueue* buff = obj->m_Buffer;
        if (buff && args[0]->IsUint32()) {
            ret = buff->Pop((unsigned int) args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());
        }
        args.GetReturnValue().Set(Uint32::New(isolate, ret));
    }

    void NJQueue::setSize(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        unsigned int size = 0;
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsUint32())
            size = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
        else {
            ThrowException(isolate, "Unsigned int expected");
            return;
        }
        unsigned int ret = 0;
        CUQueue* buff = obj->m_Buffer;
        if (buff) {
            buff->SetSize(size);
            ret = buff->GetSize();
        }
        args.GetReturnValue().Set(Uint32::New(isolate, ret));
    }

    void NJQueue::getMaxBufferSize(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        unsigned int ret = 0;
        CUQueue* buff = obj->m_Buffer;
        if (buff)
            ret = buff->GetMaxSize();
        args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
    }

    void NJQueue::Realloc(const FunctionCallbackInfo<Value>& args) {
        unsigned int size = 0;
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsUint32())
            size = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
        else {
            ThrowException(isolate, "Unsigned int expected");
            return;
        }
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (buff)
            buff->ReallocBuffer(size);
        else
            buff = CScopeUQueue::Lock(SPA::GetOS(), SPA::IsBigEndian(), size, obj->m_blockSize);
    }

    void NJQueue::UseStrForDec(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        Isolate* isolate = args.GetIsolate();
        if (args[0]->IsBoolean() || args[0]->IsUint32()) {
#ifdef BOOL_ISOLATE
            obj->m_StrForDec = args[0]->BooleanValue(isolate);
#else
            obj->m_StrForDec = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
        } else if (IsNullOrUndefined(args[0]))
            obj->m_StrForDec = false;
        else {
            ThrowException(isolate, BOOLEAN_EXPECTED);
            return;
        }
        args.GetReturnValue().Set(Boolean::New(isolate, obj->m_StrForDec));
    }

    void NJQueue::getSize(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        unsigned int ret = 0;
        CUQueue* buff = obj->m_Buffer;
        if (buff)
            ret = buff->GetSize();
        args.GetReturnValue().Set(Uint32::New(args.GetIsolate(), ret));
    }

    void NJQueue::LoadBoolean(const FunctionCallbackInfo<Value>& args) {
        bool b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Boolean::New(isolate, b));
        }
    }

    void NJQueue::LoadByte(const FunctionCallbackInfo<Value>& args) {
        unsigned char b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Uint32::New(isolate, b));
        }
    }

    void NJQueue::LoadAChar(const FunctionCallbackInfo<Value>& args) {
        char b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Int32::New(isolate, b));
        }
    }

    void NJQueue::LoadShort(const FunctionCallbackInfo<Value>& args) {
        short b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Int32::New(isolate, b));
        }
    }

    void NJQueue::LoadInt(const FunctionCallbackInfo<Value>& args) {
        int b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Int32::New(isolate, b));
        }
    }

    void NJQueue::LoadFloat(const FunctionCallbackInfo<Value>& args) {
        float b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Number::New(isolate, b));
        }
    }

    void NJQueue::LoadLong(const FunctionCallbackInfo<Value>& args) {
        SPA::INT64 b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Number::New(isolate, (double) b));
        }
    }

    void NJQueue::LoadULong(const FunctionCallbackInfo<Value>& args) {
        SPA::UINT64 b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Number::New(isolate, (double) b));
        }
    }

    void NJQueue::LoadUInt(const FunctionCallbackInfo<Value>& args) {
        unsigned int b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Uint32::New(isolate, b));
        }
    }

    void NJQueue::LoadUShort(const FunctionCallbackInfo<Value>& args) {
        unsigned short b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Uint32::New(isolate, b));
        }
    }

    void NJQueue::LoadDate(const FunctionCallbackInfo<Value>& args) {
        SPA::UINT64 date;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, date)) {
            args.GetReturnValue().Set(ToDate(isolate, date));
        }
    }

    void NJQueue::LoadDecimal(const FunctionCallbackInfo<Value>& args) {
        DECIMAL b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            if (b.Hi32 || b.Lo64 > SAFE_DOUBLE)
                args.GetReturnValue().Set(ToStr(isolate, SPA::ToString_long(b).c_str()));
            else
                args.GetReturnValue().Set(Number::New(isolate, ToDouble(b)));
        }
    }

    void NJQueue::LoadDouble(const FunctionCallbackInfo<Value>& args) {
        double b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(Number::New(isolate, b));
        }
    }

    void NJQueue::LoadUUID(const FunctionCallbackInfo<Value>& args) {
        GUID b;
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        if (obj->Load(isolate, b)) {
            args.GetReturnValue().Set(node::Buffer::Copy(isolate, (const char*) &b, sizeof (b)).ToLocalChecked());
        }
    }

    void NJQueue::PopBytes(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        unsigned int size = 0;
        unsigned int all = (~0);
        CUQueue* buff = obj->m_Buffer;
        if (buff) {
            size = buff->GetSize();
        }
        if (args.Length() && args[0]->IsUint32())
            all = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
        if (size > all)
            size = all;
        if (size) {
            args.GetReturnValue().Set(node::Buffer::Copy(isolate, (const char*) buff->GetBuffer(), size).ToLocalChecked());
            buff->Pop(size);
        } else {
            args.GetReturnValue().Set(node::Buffer::Copy(isolate, (const char*) "", 0).ToLocalChecked());
        }
    }

    void NJQueue::LoadBytes(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (!buff) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
            return;
        }
        try {
            unsigned int len;
            *buff >> len;
            if (len == (unsigned int) (~0)) {
                args.GetReturnValue().SetNull();
            } else {
                if (len <= buff->GetSize()) {
                    args.GetReturnValue().Set(node::Buffer::Copy(isolate, (const char*) buff->GetBuffer(), len).ToLocalChecked());
                    buff->Pop(len);
                } else {
                    buff->Pop(len);
                    ThrowException(isolate, "Bad data for loading byte array");
                }
            }
        } catch (std::exception &err) {
            buff->SetSize(0);
            ThrowException(isolate, err.what());
        }
    }

    void NJQueue::LoadAString(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (!buff) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
            return;
        }
        try {
            unsigned int len;
            *buff >> len;
            if (len == (unsigned int) (~0)) {
                args.GetReturnValue().SetNull();
            } else {
                if (len <= buff->GetSize()) {
                    args.GetReturnValue().Set(ToStr(isolate, (const char*) buff->GetBuffer(), len));
                    buff->Pop(len);
                } else {
                    buff->Pop(len);
                    ThrowException(isolate, "Bad data for loading ASCII string");
                }
            }
        } catch (std::exception &err) {
            buff->SetSize(0);
            ThrowException(isolate, err.what());
        }
    }

    void NJQueue::LoadString(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (!buff) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
            return;
        }
        try {
            unsigned int len;
            *buff >> len;
            if (len == (unsigned int) (~0)) {
                args.GetReturnValue().SetNull();
            } else if (len <= buff->GetSize()) {
                const UTF16 *str = (const UTF16 *) buff->GetBuffer();
                args.GetReturnValue().Set(ToStr(isolate, str, len / sizeof (SPA::UTF16)));
                buff->Pop(len);
            } else {
                ThrowException(isolate, "Bad unicode string found");
                buff->SetSize(0);
            }
        } catch (std::exception &err) {
            buff->SetSize(0);
            ThrowException(isolate, err.what());
        }
    }

    void NJQueue::SaveBoolean(const FunctionCallbackInfo<Value>& args) {
        if (args.Length()) {
            bool b = args[0]->IsTrue();
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Save(args, b);
        } else {
            Isolate* isolate = args.GetIsolate();
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveByte(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsUint32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            unsigned char b = (unsigned char) (args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveAChar(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsInt32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            char b = (char) (args[0]->Int32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {

            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveShort(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsInt32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            short b = (short) (args[0]->Int32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveInt(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsInt32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            int b = (int) (args[0]->Int32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveFloat(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsNumber()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            float b = (float) (args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveDouble(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsNumber()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            double b = (args[0]->NumberValue(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveLong(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsNumber()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            SPA::INT64 b = args[0]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveULong(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsNumber()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            SPA::UINT64 b = (SPA::UINT64)(args[0]->IntegerValue(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveUInt(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsUint32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            unsigned int b = (args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveUShort(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length() && args[0]->IsUint32()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            unsigned short b = (unsigned short) (args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked());
            obj->Save(args, b);
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveBytes(const FunctionCallbackInfo<Value>& args) {
        if (args.Length()) {
            auto p = args[0];
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            CUQueue* buff = obj->m_Buffer;
            if (IsNullOrUndefined(p)) {
                *buff << (const char *) nullptr;
                args.GetReturnValue().Set(args.Holder());
                return;
            } else if (node::Buffer::HasInstance(p)) {
                char *bytes = node::Buffer::Data(p);
                unsigned int len = (unsigned int) node::Buffer::Length(p);
                *buff << len;
                buff->Push((const unsigned char*) bytes, len);
                args.GetReturnValue().Set(args.Holder());
                return;
            }
        }
        Isolate* isolate = args.GetIsolate();
        ThrowException(isolate, "Invalid Node.js Buffer object");
    }

    void NJQueue::SaveUUID(const FunctionCallbackInfo<Value>& args) {
        if (args.Length()) {
            auto p = args[0];
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            if (node::Buffer::HasInstance(p)) {
                char *bytes = node::Buffer::Data(p);
                unsigned int len = (unsigned int) node::Buffer::Length(p);
                if (len == sizeof (GUID)) {
                    obj->m_Buffer->Push((const unsigned char*) bytes, len);
                    args.GetReturnValue().Set(args.Holder());
                    return;
                }
            }
        }
        Isolate* isolate = args.GetIsolate();
        ThrowException(isolate, "Invalid UUID Node.js Buffer object");
    }

    void NJQueue::SaveAString(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            auto p = args[0];
            CUQueue* buff = obj->m_Buffer;
            if (IsNullOrUndefined(p)) {
                *buff << (const char*) nullptr;
            } else {
                if (!p->IsString())
                    p = p->ToString(isolate->GetCurrentContext()).ToLocalChecked();
#if NODE_MODULE_VERSION < 57
                String::Utf8Value str(p);
#else
                String::Utf8Value str(isolate, p);
#endif
                unsigned int len = (unsigned int) str.length();
                *buff << len;
                buff->Push((const unsigned char*) (*str), len);
            }
            args.GetReturnValue().Set(args.Holder());
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveString(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            auto p = args[0];
            CUQueue* buff = obj->m_Buffer;
            if (IsNullOrUndefined(p)) {
                *buff << (const wchar_t *)nullptr;
            } else {
                if (!p->IsString())
                    p = p->ToString(isolate->GetCurrentContext()).ToLocalChecked();
#if NODE_MODULE_VERSION < 57
                String::Value str(p);
#else
                String::Value str(isolate, p);
#endif
                unsigned int len = (unsigned int) str.length();
                len *= sizeof (uint16_t);
                *buff << len;
                buff->Push((const unsigned char*) (*str), len);
            }
            args.GetReturnValue().Set(args.Holder());
        } else {
            ThrowException(isolate, BAD_DATA_TYPE);
        }
    }

    void NJQueue::SaveDecimal(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            auto p = args[0];
            if (p->IsNumber() || p->IsString()) {
                if (!p->IsString())
                    p = p->ToString(isolate->GetCurrentContext()).ToLocalChecked();
#if NODE_MODULE_VERSION < 57
                String::Utf8Value str(p);
#else
                String::Utf8Value str(isolate, p);
#endif
                const char *s = *str;
                DECIMAL dec;
                SPA::ParseDec_long(s, dec);
                *obj->m_Buffer << dec;
                args.GetReturnValue().Set(args.Holder());
                return;
            }
        }
        ThrowException(isolate, BAD_DATA_TYPE);
    }

    void NJQueue::SaveDate(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.Length()) {
            NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
            obj->Ensure();
            auto p = args[0];
            SPA::UINT64 d = ToDate(isolate, p);
            if (d == INVALID_NUMBER) {
                ThrowException(isolate, BAD_DATA_TYPE);
                return;
            }
            *obj->m_Buffer << d;
            args.GetReturnValue().Set(args.Holder());
            return;
        }
        ThrowException(isolate, "No date provided");
    }

    unsigned int NJQueue::Load(Isolate* isolate, SPA::UDB::CDBVariant &vt) {
        if (!m_Buffer) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
            return 0;
        }
        try {
            unsigned int start = m_Buffer->GetSize();
            *m_Buffer >> vt;
            return (start - m_Buffer->GetSize());
        } catch (std::exception &err) {
            m_Buffer->SetSize(0);
            ThrowException(isolate, err.what());
        }
        return 0;
    }

    void NJQueue::LoadByClass(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (!buff) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
        } else if (args[0]->IsFunction()) {
            Local<Function> cb = Local<Function>::Cast(args[0]);
            Local<Value> argv[] = {args.Holder()};
            args.GetReturnValue().Set(cb->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv).ToLocalChecked());
        } else {
            buff->SetSize(0);
            ThrowException(isolate, "An function expected for class or struct de-serialization");
        }
    }

    void NJQueue::LoadObject(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        CUQueue* buff = obj->m_Buffer;
        if (!buff || buff->GetSize() < sizeof (VARTYPE)) {
            ThrowException(isolate, NO_BUFFER_AVAILABLE);
        }
        try {
            VARTYPE* pvt = (VARTYPE*) buff->GetBuffer();
            if (*pvt == SPA::VT_USERIALIZER_OBJECT) {
                buff->Pop((unsigned int) sizeof (VARTYPE));
                LoadByClass(args);
                return;
            }
            auto v = DbFrom(isolate, *buff);
            if (v->IsUndefined())
                ThrowException(isolate, UNSUPPORTED_TYPE);
            else
                args.GetReturnValue().Set(v);
        } catch (std::exception& ex) {
            ThrowException(isolate, ex.what());
        }
    }

    void NJQueue::SaveByClass(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        obj->Ensure();
        auto p0 = args[0];
        if (p0->IsFunction()) {
            Local<Function> cb = Local<Function>::Cast(p0);
            Local<Value> argv[] = {args.Holder()};
            cb->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv);
            args.GetReturnValue().Set(args.Holder());
        } else {
            ThrowException(isolate, "A callback function expected");
        }
    }

    bool NJQueue::SaveObject(Isolate* isolate, Local<Value> p0, Local<Value> holder, const std::string& id, CUQueue& buff) {
        VARTYPE vt;
        if (IsNullOrUndefined(p0)) {
            vt = VT_NULL;
            buff << vt;
        } else if (p0->IsFunction()) {
            vt = SPA::VT_USERIALIZER_OBJECT;
            buff << vt;
            Local<Function> cb = Local<Function>::Cast(p0);
            Local<Value> argv[] = {holder};
            cb->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv);
        } else if (p0->IsDate()) {
            vt = VT_DATE;
            SPA::UINT64 d = ToDate(isolate, p0);
            buff << vt << d;
        } else if (p0->IsBoolean()) {
            vt = VT_BOOL;
            short v = p0->IsTrue() ? -1 : 0;
            buff << vt << v;
        } else if (p0->IsString()) {
            if (id.size() && (id == "a" || id == "ascii" || id == "dec" || id == "decimal")) {
#if NODE_MODULE_VERSION < 57
                String::Utf8Value str(p0);
#else
                String::Utf8Value str(isolate, p0);
#endif
                if (id == "a" || id == "ascii") {
                    vt = (VT_ARRAY | VT_I1);
                    unsigned int len = (unsigned int) str.length();
                    buff << vt << len;
                    buff.Push((const unsigned char*) (*str), len);
                } else {
                    vt = VT_DECIMAL;
                    const char* s = *str;
                    DECIMAL dec;
                    SPA::ParseDec_long(s, dec);
                    buff << vt << dec;
                }
            } else {
                vt = VT_BSTR;
#if NODE_MODULE_VERSION < 57
                String::Value str(p0);
#else
                String::Value str(isolate, p0);
#endif
                unsigned int len = (unsigned int) str.length();
                len <<= 1;
                buff << vt << len;
                buff.Push((const unsigned char*) (*str), len);
            }
        } else if (p0->IsInt32() && !id.size()) {
            vt = VT_I4;
            buff << vt << p0->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }
#ifdef HAS_BIGINT
        else if (p0->IsBigInt() && !id.size()) {
            vt = VT_I8;
            buff << vt << p0->IntegerValue(isolate->GetCurrentContext()).ToChecked();
        }
#endif
        else if (p0->IsNumber()) {
            do {
                if (id.size()) {
                    if (id == "f" || id == "float") {
                        vt = VT_R4;
                        buff << vt << (float) p0->NumberValue(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "d" || id == "double") {
                        vt = VT_R8;
                        buff << vt << p0->NumberValue(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "i" || id == "int") {
                        vt = VT_I4;
                        buff << vt << p0->Int32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "ui" || id == "uint") {
                        vt = VT_UI4;
                        buff << vt << p0->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "l" || id == "long") {
                        vt = VT_I8;
                        buff << vt << p0->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "ul" || id == "ulong") {
                        vt = VT_UI8;
                        buff << vt << (SPA::UINT64)p0->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "s" || id == "short") {
                        vt = VT_I2;
                        buff << vt << (short) p0->Int32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "us" || id == "ushort") {
                        vt = VT_UI2;
                        buff << vt << (unsigned short) p0->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "dec" || id == "decimal") {
                        vt = VT_DECIMAL;
                        double d = p0->NumberValue(isolate->GetCurrentContext()).ToChecked();
                        DECIMAL dec;
                        SPA::ToDecimal(d, dec);
                        buff << vt << dec;
                        break;
                    } else if (id == "c" || id == "char") {
                        vt = VT_I1;
                        buff << vt << (char) p0->Int32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "b" || id == "byte") {
                        vt = VT_UI1;
                        buff << vt << (unsigned char) p0->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                        break;
                    } else if (id == "date") {
                        SPA::UINT64 d = ToDate(isolate, p0);
                        if (d == INVALID_NUMBER) {
                            ThrowException(isolate, BAD_DATA_TYPE);
                            return false;
                        }
                        vt = VT_DATE;
                        buff << vt << d;
                        break;
                    }
                }
                if (p0->IsInt32()) {
                    vt = VT_I4;
                    buff << vt << p0->Int32Value(isolate->GetCurrentContext()).ToChecked();
                }
#ifdef HAS_BIGINT
                else if (p0->IsBigInt()) {
                    vt = VT_I8;
                    buff << vt << p0->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                }
#endif
                else {
                    vt = VT_R8;
                    buff << vt << p0->NumberValue(isolate->GetCurrentContext()).ToChecked();
                }
            } while (false);
        } else if (node::Buffer::HasInstance(p0)) {
            char* bytes = node::Buffer::Data(p0);
            unsigned int len = (unsigned int) node::Buffer::Length(p0);
            if (len == sizeof (GUID) && (id == "u" || id == "uuid")) {
                vt = VT_CLSID;
                buff << vt;
                buff.Push((const unsigned char*) bytes, len);
            } else {
                vt = (VT_ARRAY | VT_UI1);
                buff << vt << len;
                buff.Push((const unsigned char*) bytes, len);
            }
        } else if (p0->IsTypedArray()) {
            if (p0->IsInt32Array()) {
                vt = (VT_ARRAY | VT_I4);
                Local<v8::Int32Array> vInt = Local<v8::Int32Array>::Cast(p0);
                const int* p = (const int*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (int));
            } else if (p0->IsFloat64Array()) {
                vt = (VT_ARRAY | VT_R8);
                Local<v8::Float64Array> vInt = Local<v8::Float64Array>::Cast(p0);
                const double* p = (const double*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (double));
            } else if (p0->IsUint8Array()) {
                vt = (VT_ARRAY | VT_I1);
                Local<v8::Uint8Array> vInt = Local<v8::Uint8Array>::Cast(p0);
                const unsigned char* p = (const unsigned char*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (char));
            } else if (p0->IsInt8Array()) {
                vt = (VT_ARRAY | VT_I1);
                Local<v8::Int8Array> vInt = Local<v8::Int8Array>::Cast(p0);
                const char* p = (const char*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (char));
            } else if (p0->IsInt16Array()) {
                vt = (VT_ARRAY | VT_I2);
                Local<v8::Int16Array> vInt = Local<v8::Int16Array>::Cast(p0);
                const short* p = (const short*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (short));
            } else if (p0->IsUint16Array()) {
                vt = (VT_ARRAY | VT_UI2);
                Local<v8::Uint16Array> vInt = Local<v8::Uint16Array>::Cast(p0);
                const unsigned short* p = (const unsigned short*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (unsigned short));
            } else if (p0->IsUint32Array()) {
                vt = (VT_ARRAY | VT_UI4);
                Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(p0);
                const unsigned int* p = (const unsigned int*) vInt->Buffer()->GetContents().Data();
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (unsigned int));
            }
#ifdef HAS_BIGINT
            else if (p0->IsBigUint64Array()) {
                vt = (VT_ARRAY | VT_UI8);
                char* bytes = node::Buffer::Data(p0);
                Local<v8::BigUint64Array> vInt = Local<v8::BigUint64Array>::Cast(p0);
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) bytes, count * sizeof (uint64_t));
            } else if (p0->IsBigInt64Array()) {
                vt = (VT_ARRAY | VT_UI4);
                char* bytes = node::Buffer::Data(p0);
                Local<v8::BigInt64Array> vInt = Local<v8::BigInt64Array>::Cast(p0);
                unsigned int count = (unsigned int) vInt->Length();
                buff << vt << count;
                buff.Push((const unsigned char*) bytes, count * sizeof (int64_t));
            }
#endif
            else if (p0->IsFloat32Array()) {
                vt = (VT_ARRAY | VT_R4);
                Local<v8::Float32Array> vInt = Local<v8::Float32Array>::Cast(p0);
                unsigned int count = (unsigned int) vInt->Length();
                const float* p = (const float*) vInt->Buffer()->GetContents().Data();
                buff << vt << count;
                buff.Push((const unsigned char*) p, count * sizeof (float));
            } else {
                ThrowException(isolate, UNSUPPORTED_ARRAY_TYPE);
                return false;
            }
        } else if (p0->IsArray()) {
            vt = (VT_ARRAY | VT_VARIANT);
            auto ctx = isolate->GetCurrentContext();
            Local<Array> jsArr = Local<Array>::Cast(p0);
            unsigned int count = jsArr->Length();
            buff << vt << count;
            for (unsigned int n = 0; n < count; ++n) {
                if (!SaveObject(isolate, jsArr->Get(ctx, n).ToLocalChecked(), holder, id, buff)) {
                    return false;
                }
            }
        } else {
            ThrowException(isolate, UNSUPPORTED_TYPE);
            return false;
        }
        return true;
    }

    void NJQueue::SaveObject(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        obj->Ensure();
        std::string id;
        auto p1 = args[1];
        if (p1->IsString()) {
#if NODE_MODULE_VERSION < 57
            String::Utf8Value str(p1);
#else
            String::Utf8Value str(isolate, p1);
#endif
            const char *s = *str;
            id.assign(s, str.length());
            std::transform(id.begin(), id.end(), id.begin(), ::tolower);
        }
        CUQueue &buff = *obj->m_Buffer;
        if (SaveObject(isolate, args[0], args.Holder(), id, buff)) {
            args.GetReturnValue().Set(args.Holder());
        }
    }

    void NJQueue::Empty(const FunctionCallbackInfo<Value>& args) {
        NJQueue* obj = ObjectWrap::Unwrap<NJQueue>(args.Holder());
        obj->Release();
    }

    Local<Object> NJQueue::New(Isolate* isolate, PUQueue &q) {
        SPA::UINT64 ptr = (SPA::UINT64)q;
        Local<Value> argv[] = {Boolean::New(isolate, true), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        q = nullptr;
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJQueue::New(const FunctionCallbackInfo<Value>& args) {
        auto isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            NJQueue* obj;
            unsigned int initSize = SPA::DEFAULT_INITIAL_MEMORY_BUFFER_SIZE;
            unsigned int blockSize = SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
#ifdef BOOL_ISOLATE
            if (args[0]->IsBoolean() && args[0]->BooleanValue(isolate) && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
#else
            if (args[0]->IsBoolean() && args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
#endif
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                // Invoked as constructor: `new CUQueue(...)`
                obj = new NJQueue((CUQueue*) ptr, initSize, blockSize);
            } else {
                if (args.Length() > 0 && args[0]->IsUint32()) {
                    initSize = args[0]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                }
                if (args.Length() > 1 && args[1]->IsUint32()) {
                    blockSize = args[1]->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                }
                // Invoked as constructor: `new CUQueue(...)`
                obj = new NJQueue(nullptr, initSize, blockSize);
            }
            obj->Wrap(args.This());
            args.GetReturnValue().Set(args.This());
        } else {
            // Invoked as plain function `CUQueue(...)`, turn into construct call.
            Local<Value> argv[] = {args[0], args[1]};
            Isolate* isolate = args.GetIsolate();
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 2, argv).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }
}
