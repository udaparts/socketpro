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
        tpl->SetClassName(ToStr(isolate, u"CPush", 5));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "Publish", Publish);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Subscribe", Subscribe);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Unsubscribe", Unsubscribe);
        NODE_SET_PROTOTYPE_METHOD(tpl, "SendUserMessage", SendUserMessage);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CPush", 5), tpl->GetFunction(ctx).ToLocalChecked());
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
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
#ifdef BOOL_ISOLATE
                //bool setCb = args[0]->BooleanValue(isolate);
#else
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
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
        std::vector<unsigned int> groups;
        if (!ToGroups(isolate, p1, groups)) {
            return;
        }
        if (node::Buffer::HasInstance(p0)) {
            char *bytes = node::Buffer::Data(p0);
            size_t len = node::Buffer::Length(p0);
            bool ok = obj->m_p->PublishEx((const unsigned char*) bytes, (unsigned int) len, groups.data(), (unsigned int) groups.size());
            args.GetReturnValue().Set(Boolean::New(isolate, ok));
        } else if (p0->IsObject()) {
            Local<Object> qObj = p0->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
            if (NJQueue::IsUQueue(isolate, qObj)) {
                NJQueue* njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                CDBVariant vtMsg;
                if (njq->Load(isolate, vtMsg)) {
                    bool ok = obj->m_p->Publish(vtMsg, groups.data(), (unsigned int) groups.size());
                    args.GetReturnValue().Set(Boolean::New(isolate, ok));
                }
                CUQueue* q = njq->get();
                if (q)
                    q->SetSize(0);
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        } else {
            CDBVariant vtMsg;
            std::string hint;
            auto p2 = args[2];
            if (p2->IsString()) {
                hint = ToAStr(isolate, p2);
            }
            if (From(isolate, p0, hint, vtMsg)) {
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
        std::vector<unsigned int> groups;
        if (!ToGroups(isolate, p, groups)) {
            return;
        }
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
#ifdef WIN32_64
            user = (const wchar_t*)ToStr(isolate, p0).c_str();
#else
            auto s = ToAStr(isolate, p0);
            user = Utilities::ToWide(s);
#endif
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
            Local<Object> qObj = p1->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
            if (NJQueue::IsUQueue(isolate, qObj)) {
                NJQueue* njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                CDBVariant vtMsg;
                if (njq->Load(isolate, vtMsg)) {
                    bool ok = obj->m_p->SendUserMessage(vtMsg, user.c_str());
                    args.GetReturnValue().Set(Boolean::New(isolate, ok));
                }
                CUQueue* q = njq->get();
                if (q)
                    q->SetSize(0);
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        } else {
            CDBVariant vtMsg;
            std::string hint;
            auto p2 = args[2];
            if (p2->IsString()) {
                hint = ToAStr(isolate, p2);
            }
            if (From(isolate, p1, hint, vtMsg)) {
                bool ok = obj->m_p->SendUserMessage(vtMsg, user.c_str());
                args.GetReturnValue().Set(Boolean::New(isolate, ok));
            } else {
                ThrowException(isolate, BAD_MESSAGE_SENT);
            }
        }
    }
}
