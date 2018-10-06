
#pragma once

namespace NJA {

    class NJCache : public node::ObjectWrap {
    public:
        NJCache(SPA::CDataSet *ds);
        NJCache(const NJCache &c) = delete;
        ~NJCache();

    public:
        NJCache& operator=(const NJCache &c) = delete;

        inline SPA::CDataSet* get() const {
            return m_ds;
        }

        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::CDataSet *ds, bool setCb);

    private:
        static const SPA::INT64 SECRECT_NUM = 0x7f2bb02ce455;
        bool IsValid(Isolate* isolate);
        void Release();
        static void New(const FunctionCallbackInfo<Value>& args);

        static void GetDBServerIp(const FunctionCallbackInfo<Value>& args);
        static void GetDBServerName(const FunctionCallbackInfo<Value>& args);
        static void GetUpdater(const FunctionCallbackInfo<Value>& args);
        static void getMS(const FunctionCallbackInfo<Value>& args);
        static void isEmpty(const FunctionCallbackInfo<Value>& args);
        static void getDBNameCase(const FunctionCallbackInfo<Value>& args);
        static void getTableNameCase(const FunctionCallbackInfo<Value>& args);
        static void getFieldNameCase(const FunctionCallbackInfo<Value>& args);
        static void getDataCase(const FunctionCallbackInfo<Value>& args);
        static void getDbTable(const FunctionCallbackInfo<Value>& args);
        static void Empty(const FunctionCallbackInfo<Value>& args);
        static void GetMeta(const FunctionCallbackInfo<Value>& args);
        static void GetRowCount(const FunctionCallbackInfo<Value>& args);
        static void GetColumnCount(const FunctionCallbackInfo<Value>& args);
        static void FindKeys(const FunctionCallbackInfo<Value>& args);
        static void FindOrdinal(const FunctionCallbackInfo<Value>& args);
        static void Find(const FunctionCallbackInfo<Value>& args);
        static void FindNull(const FunctionCallbackInfo<Value>& args);
        static void In(const FunctionCallbackInfo<Value>& args);
        static void Between(const FunctionCallbackInfo<Value>& args);
        static void NotIn(const FunctionCallbackInfo<Value>& args);
    private:
        static bool GetDTPair(const FunctionCallbackInfo<Value>& args, SPA::CPDbTable &p);

    private:
        static Persistent<Function> constructor;
        SPA::CDataSet *m_ds;
    };

}