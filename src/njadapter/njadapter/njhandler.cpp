
#include "stdafx.h"
#include "njhandler.h"

namespace NJA {

    Persistent<Function> NJHandler::constructor;

    NJHandler::NJHandler(CAsyncServiceHandler *ash) : NJHandlerRoot(ash) {

    }

    void NJHandler::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, u"CHandler"));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        NJHandlerRoot::Init(exports, tpl);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CHandler"), tpl->GetFunction(ctx).ToLocalChecked());
    }

    Local<Object> NJHandler::New(Isolate* isolate, CAsyncServiceHandler *ash, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ash;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJHandler::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                NJHandler *obj = new NJHandler((CAsyncServiceHandler*) ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CHandler()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }
}
