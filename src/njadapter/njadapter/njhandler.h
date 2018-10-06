#pragma once

#include "njhandlerroot.h"

namespace NJA {

    class NJHandler : public NJHandlerRoot {
    public:
        NJHandler(SPA::ClientSide::CAsyncServiceHandler *ash);
        NJHandler(const NJHandler &h) = delete;

    public:
        NJHandler& operator=(const NJHandler &h) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::ClientSide::CAsyncServiceHandler *ash, bool setCb);

    private:
        static const SPA::INT64 SECRECT_NUM = 0x7fa12ff345fb;
        static void New(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
    };
}
