#pragma once


namespace NJA {

    class NJCert : public node::ObjectWrap {
    public:
        NJCert(SPA::IUcert *c);
        NJCert(const NJCert &c) = delete;

    public:
        NJCert& operator=(const NJCert &c) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::IUcert *c, bool setCb);

    private:
        static const SPA::INT64 SECRECT_NUM = 0x7f1bd02ce4a5;
        static void New(const FunctionCallbackInfo<Value>& args);
        static void Verify(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        SPA::IUcert *m_c;
    };
}
