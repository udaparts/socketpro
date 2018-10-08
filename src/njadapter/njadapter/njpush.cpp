#include "stdafx.h"
#include "njpush.h"
#include "njqueue.h"

namespace NJA {

    Persistent<Function> NJPush::constructor;
    const char* NJPush::BAD_MESSAGE_SENT = "Bad message to be sent";

    NJPush::NJPush(SPA::IPushEx *p) : m_p(p) {
    }

    NJPush::~NJPush() {
    }

    void NJPush::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CPush"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Publish", Publish);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Subscribe", Subscribe);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Unsubscribe", Unsubscribe);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SendUserMessage", SendUserMessage);

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CPush"), tpl->GetFunction());
    }

    Local<Object> NJPush::New(Isolate* isolate, SPA::IPushEx *p, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)p;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJPush::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
                bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue();
                NJPush *obj = new NJPush((SPA::IPushEx*)ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CPush()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJPush::Publish(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJPush* obj = ObjectWrap::Unwrap<NJPush>(args.Holder());
        auto p0 = args[0];
        auto p1 = args[1];
        if (!p1->IsUint32Array()) {
            ThrowException(isolate, "An array of unsigned int expected for group ids");
            return;
        }
        auto groups = ToGroups(p1);
        if (node::Buffer::HasInstance(p0)) {
            char *bytes = node::Buffer::Data(p0);
            size_t len = node::Buffer::Length(p0);
            bool ok = obj->m_p->PublishEx((const unsigned char*) bytes, (unsigned int) len, groups.data(), (unsigned int) groups.size());
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        } else if (p0->IsObject()) {
            Local<Object> qObj = p0->ToObject();
            if (NJQueue::IsUQueue(qObj)) {
                NJQueue* njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                CDBVariant vtMsg;
                if (njq->Load(isolate, vtMsg)) {
                    bool ok = obj->m_p->Publish(vtMsg, groups.data(), (unsigned int) groups.size());
                    args.GetReturnValue().Set(Boolean::New(isolate, ok));
                }
                njq->Release();
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        } else {
            CDBVariant vtMsg;
            std::string hint;
            auto p2 = args[2];
            if (p2->IsString()) {
                hint = ToAStr(p2);
            }
            if (From(p0, hint, vtMsg)) {
                bool ok = obj->m_p->Publish(vtMsg, groups.data(), (unsigned int) groups.size());
                args.GetReturnValue().Set(Boolean::New(isolate, ok));
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        }
    }

    void NJPush::Subscribe(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJPush* obj = ObjectWrap::Unwrap<NJPush>(args.Holder());
        auto p = args[0];
        if (!p->IsUint32Array()) {
            ThrowException(isolate, "An array of unsigned int expected for group ids");
            return;
        }
        auto groups = ToGroups(p);
        bool ok = obj->m_p->Subscribe(groups.data(), (unsigned int) groups.size());
        args.GetReturnValue().Set(Boolean::New(isolate, ok));
    }

    void NJPush::Unsubscribe(const FunctionCallbackInfo<Value>& args) {
        NJPush* obj = ObjectWrap::Unwrap<NJPush>(args.Holder());
        obj->m_p->Unsubscribe();
    }

    void NJPush::SendUserMessage(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJPush* obj = ObjectWrap::Unwrap<NJPush>(args.Holder());
        std::wstring user;
        auto p0 = args[0];
        if (p0->IsString()) {
            user = ToStr(p0);
        }
        if (!user.size()) {
            ThrowException(isolate, "A non-empty string expected for user id");
            return;
        }
        auto p1 = args[1];
        if (node::Buffer::HasInstance(p1)) {
            char *bytes = node::Buffer::Data(p1);
            size_t len = node::Buffer::Length(p1);
            bool ok = obj->m_p->SendUserMessageEx(user.c_str(), (const unsigned char*) bytes, (unsigned int) len);
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        } else if (p1->IsObject()) {
            Local<Object> qObj = p1->ToObject();
            if (NJQueue::IsUQueue(qObj)) {
                NJQueue* njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                CDBVariant vtMsg;
                if (njq->Load(isolate, vtMsg)) {
                    bool ok = obj->m_p->SendUserMessage(vtMsg, user.c_str());
                    args.GetReturnValue().Set(Boolean::New(isolate, ok));
                }
                njq->Release();
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        } else {
            CDBVariant vtMsg;
            std::string hint;
            auto p2 = args[2];
            if (p2->IsString()) {
                hint = ToAStr(p2);
            }
            if (From(p1, hint, vtMsg)) {
                bool ok = obj->m_p->SendUserMessage(vtMsg, user.c_str());
                args.GetReturnValue().Set(Boolean::New(isolate, ok));
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        }
    }
}
