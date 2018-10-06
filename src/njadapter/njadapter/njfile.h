
#pragma once

#include "sfile.h"
#include "njhandlerroot.h"

namespace NJA {

    class NJFile : public NJHandlerRoot {
    public:
        NJFile(CSFile *file);
        NJFile(const NJFile &h) = delete;
        ~NJFile();

    public:
        NJFile& operator=(const NJFile &h) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, CSFile *ash, bool setCb);

    private:
        void Release();
        bool IsValid(Isolate* isolate);

        static const SPA::INT64 SECRECT_NUM = 0x7fa114ff2345;
        static void New(const FunctionCallbackInfo<Value>& args);
        static void getFilesQueued(const FunctionCallbackInfo<Value>& args);
        static void Upload(const FunctionCallbackInfo<Value>& args);
        static void Download(const FunctionCallbackInfo<Value>& args);
        static void Exchange(bool download, const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        CSFile *m_file;
    };
}
