#include "stdafx.h"
#include "njcert.h"

namespace NJA {

    Persistent<Function> NJCert::constructor;

    NJCert::NJCert(SPA::IUcert *c) : m_c(c) {
    }

    void NJCert::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CCert"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //method
        NODE_SET_PROTOTYPE_METHOD(tpl, "Verify", Verify);

        constructor.Reset(isolate, tpl->GetFunction(isolate->GetCurrentContext()).ToLocalChecked());
        exports->Set(ToStr(isolate, "CCert"), tpl->GetFunction(isolate->GetCurrentContext()).ToLocalChecked());
    }

    void NJCert::Verify(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCert* obj = ObjectWrap::Unwrap<NJCert>(args.Holder());
        int errCode;
        Local<Object> res = Object::New(isolate);
        std::string errMsg = obj->m_c->Verify(&errCode);
        res->Set(ToStr(isolate, "ec"), Int32::New(isolate, errCode));
        res->Set(ToStr(isolate, "em"), ToStr(isolate, errMsg.c_str(), errMsg.size()));
        args.GetReturnValue().Set(res);
    }

    std::string NJCert::ToString(const unsigned char *buffer, unsigned int bytes) {
        std::string s;
        char str[8] = {0};
        if (!buffer) bytes = 0;
        for (unsigned int n = 0; n < bytes; ++n) {
#ifdef WIN32_64
            sprintf_s(str, "%02x", buffer[n]);
#else
            sprintf(str, "%02x", buffer[n]);
#endif
            s += str;
        }
        return s;
    }

    Local<Object> NJCert::New(Isolate* isolate, SPA::IUcert *c, bool setCb) {
        assert(c);
        SPA::UINT64 ptr = (SPA::UINT64)c;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        auto jsCert = cons->NewInstance(context, 3, argv).ToLocalChecked();

        //properties
        jsCert->Set(ToStr(isolate, "Issuer"), ToStr(isolate, c->Issuer));
        jsCert->Set(ToStr(isolate, "Subject"), ToStr(isolate, c->Subject));
        jsCert->Set(ToStr(isolate, "NotBefore"), ToStr(isolate, c->NotBefore));
        jsCert->Set(ToStr(isolate, "NotAfter"), ToStr(isolate, c->NotAfter));
        jsCert->Set(ToStr(isolate, "Validity"), Boolean::New(isolate, c->Validity));
        jsCert->Set(ToStr(isolate, "SigAlg"), ToStr(isolate, c->SigAlg));
        jsCert->Set(ToStr(isolate, "CertPem"), ToStr(isolate, c->CertPem));
        jsCert->Set(ToStr(isolate, "SessionInfo"), ToStr(isolate, c->SessionInfo));
        jsCert->Set(ToStr(isolate, "PublicKey"), ToStr(isolate, ToString(c->PublicKey, c->PKSize).c_str()));
        jsCert->Set(ToStr(isolate, "Algorithm"), ToStr(isolate, ToString(c->Algorithm, c->AlgSize).c_str()));
        jsCert->Set(ToStr(isolate, "SerialNumber"), ToStr(isolate, ToString(c->SerialNumber, c->SNSize).c_str()));

        return jsCert;
    }

    void NJCert::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
                //bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                NJCert *obj = new NJCert((SPA::IUcert*)ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CCert()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }
}
