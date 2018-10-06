#pragma once

namespace NJA {
    using SPA::CUQueue;

    class NJQueue : public node::ObjectWrap {
    public:
        NJQueue(CUQueue *buffer, unsigned int initialSize, unsigned int blockSize);
        NJQueue(const NJQueue &jq) = delete;
        ~NJQueue();

    public:

        inline CUQueue* get() const {
            return m_Buffer;
        }
        void Release();
        NJQueue& operator=(const NJQueue &jq) = delete;
        static bool IsUQueue(Local<Object> obj);
        static bool ToParamArray(NJQueue *obj, SPA::UDB::CDBVariantArray &vParam);
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::PUQueue &q);
        unsigned int Load(Isolate* isolate, SPA::UDB::CDBVariant &vt);
        static const char *NO_BUFFER_AVAILABLE;

    private:
        void Ensure();
        static const SPA::INT64 SECRECT_NUM = 0x7ff12ff345ff;
        static void New(const FunctionCallbackInfo<Value>& args);

        template <class ctype>
        unsigned int Load(Isolate* isolate, ctype &buffer) {
            if (!m_Buffer || m_Buffer->GetSize() < sizeof (ctype)) {
                ThrowException(isolate, NO_BUFFER_AVAILABLE);
                Release();
                return 0;
            }
            unsigned int size = m_Buffer->Pop((unsigned char*) &buffer, sizeof (ctype), 0);
            if (!m_Buffer->GetSize())
                Release();
            return size;
        }

        template <class ctype>
        void Save(const FunctionCallbackInfo<Value>& args, const ctype &buffer) {
            Ensure();
            m_Buffer->Push((const unsigned char *) &buffer, sizeof (ctype));
            args.GetReturnValue().Set(args.Holder());
        }

        static void Discard(const FunctionCallbackInfo<Value>& args);
        static void Empty(const FunctionCallbackInfo<Value>& args);
        static void getSize(const FunctionCallbackInfo<Value>& args);
        static void setSize(const FunctionCallbackInfo<Value>& args);
        static void getOS(const FunctionCallbackInfo<Value>& args);
        static void getMaxBufferSize(const FunctionCallbackInfo<Value>& args);
        static void Realloc(const FunctionCallbackInfo<Value>& args);
        static void UseStrForDec(const FunctionCallbackInfo<Value>& args);

        static void LoadBoolean(const FunctionCallbackInfo<Value>& args);
        static void LoadByte(const FunctionCallbackInfo<Value>& args);
        static void LoadAChar(const FunctionCallbackInfo<Value>& args);
        static void LoadShort(const FunctionCallbackInfo<Value>& args);
        static void LoadInt(const FunctionCallbackInfo<Value>& args);
        static void LoadFloat(const FunctionCallbackInfo<Value>& args);
        static void LoadDouble(const FunctionCallbackInfo<Value>& args);
        static void LoadLong(const FunctionCallbackInfo<Value>& args);
        static void LoadULong(const FunctionCallbackInfo<Value>& args);
        static void LoadUInt(const FunctionCallbackInfo<Value>& args);
        static void LoadUShort(const FunctionCallbackInfo<Value>& args);
        static void LoadBytes(const FunctionCallbackInfo<Value>& args);
        static void LoadAString(const FunctionCallbackInfo<Value>& args);
        static void LoadString(const FunctionCallbackInfo<Value>& args);
        static void LoadDecimal(const FunctionCallbackInfo<Value>& args);
        static void LoadDate(const FunctionCallbackInfo<Value>& args);
        static void LoadUUID(const FunctionCallbackInfo<Value>& args);
        static void LoadByClass(const FunctionCallbackInfo<Value>& args);
        static void LoadObject(const FunctionCallbackInfo<Value>& args);
        static void PopBytes(const FunctionCallbackInfo<Value>& args);

        static void SaveBoolean(const FunctionCallbackInfo<Value>& args);
        static void SaveByte(const FunctionCallbackInfo<Value>& args);
        static void SaveAChar(const FunctionCallbackInfo<Value>& args);
        static void SaveShort(const FunctionCallbackInfo<Value>& args);
        static void SaveInt(const FunctionCallbackInfo<Value>& args);
        static void SaveFloat(const FunctionCallbackInfo<Value>& args);
        static void SaveDouble(const FunctionCallbackInfo<Value>& args);
        static void SaveLong(const FunctionCallbackInfo<Value>& args);
        static void SaveULong(const FunctionCallbackInfo<Value>& args);
        static void SaveUInt(const FunctionCallbackInfo<Value>& args);
        static void SaveUShort(const FunctionCallbackInfo<Value>& args);
        static void SaveBytes(const FunctionCallbackInfo<Value>& args);
        static void SaveAString(const FunctionCallbackInfo<Value>& args);
        static void SaveString(const FunctionCallbackInfo<Value>& args);
        static void SaveDecimal(const FunctionCallbackInfo<Value>& args);
        static void SaveDate(const FunctionCallbackInfo<Value>& args);
        static void SaveUUID(const FunctionCallbackInfo<Value>& args);
        static void SaveByClass(const FunctionCallbackInfo<Value>& args);
        static void SaveObject(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        static Persistent<v8::FunctionTemplate> m_tpl;

        CUQueue *m_Buffer;
        unsigned int m_initSize;
        unsigned int m_blockSize;
        bool m_StrForDec;
    };
}
