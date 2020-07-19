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
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, "CCert"), tpl->GetFunction(ctx).ToLocalChecked());
    }

    void NJCert::Verify(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCert* obj = ObjectWrap::Unwrap<NJCert>(args.Holder());
        int errCode;
        Local<Object> res = Object::New(isolate);
        std::string errMsg = obj->m_c->Verify(&errCode);
        auto ctx = isolate->GetCurrentContext();
        res->Set(ctx, ToStr(isolate, u"ec"), Int32::New(isolate, errCode));
        res->Set(ctx, ToStr(isolate, u"em"), ToStr(isolate, errMsg.c_str(), errMsg.size()));
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
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        Local<Context> ctx = isolate->GetCurrentContext();
        auto jsCert = cons->NewInstance(ctx, 3, argv).ToLocalChecked();

        //properties
        jsCert->Set(ctx, ToStr(isolate, u"Issuer"), ToStr(isolate, c->Issuer));
        jsCert->Set(ctx, ToStr(isolate, u"Subject"), ToStr(isolate, c->Subject));
        jsCert->Set(ctx, ToStr(isolate, u"NotBefore"), ToStr(isolate, c->NotBefore));
        jsCert->Set(ctx, ToStr(isolate, u"NotAfter"), ToStr(isolate, c->NotAfter));
        jsCert->Set(ctx, ToStr(isolate, u"Validity"), Boolean::New(isolate, c->Validity));
        jsCert->Set(ctx, ToStr(isolate, u"SigAlg"), ToStr(isolate, c->SigAlg));
        jsCert->Set(ctx, ToStr(isolate, u"CertPem"), ToStr(isolate, c->CertPem));
        jsCert->Set(ctx, ToStr(isolate, u"SessionInfo"), ToStr(isolate, c->SessionInfo));
        jsCert->Set(ctx, ToStr(isolate, u"PublicKey"), ToStr(isolate, ToString(c->PublicKey, c->PKSize).c_str()));
        jsCert->Set(ctx, ToStr(isolate, u"Algorithm"), ToStr(isolate, ToString(c->Algorithm, c->AlgSize).c_str()));
        jsCert->Set(ctx, ToStr(isolate, u"SerialNumber"), ToStr(isolate, ToString(c->SerialNumber, c->SNSize).c_str()));

        return jsCert;
    }

    void NJCert::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
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
