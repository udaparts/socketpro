#pragma once
namespace NJA {

    class NJTable : public node::ObjectWrap {
    public:
        NJTable(SPA::CTable *tbl);
        NJTable(const NJTable &tbl) = delete;
        ~NJTable();

    public:

        inline SPA::CTable* get() const {
            return m_table;
        }
        NJTable& operator=(const NJTable &tbl) = delete;
        static void Init(Local<Object> exports);
        static Local<Object> New(Isolate* isolate, SPA::CTable *tbl, bool setCb);

        static void ThrowException(int errCode, Isolate *isolate);

        static const char *COLUMN_ORDINAL_EXPECTED;
        static const char *OPERATION_EXPECTED;
        static const char *BAD_OPERATION;

    private:
        bool IsValid(Isolate* isolate);
        void Release();
        static bool IsTable(Local<Object> obj);

        static const SPA::INT64 SECRECT_NUM = 0x4f25b02ce415;
        static void New(const FunctionCallbackInfo<Value>& args);

        static void getMeta(const FunctionCallbackInfo<Value>& args);
        static void getData(const FunctionCallbackInfo<Value>& args);
        static void getKeys(const FunctionCallbackInfo<Value>& args);
        static void getRows(const FunctionCallbackInfo<Value>& args);
        static void getColumns(const FunctionCallbackInfo<Value>& args);

        static void Append(const FunctionCallbackInfo<Value>& args);
        static void Sort(const FunctionCallbackInfo<Value>& args);
        static void FindOrdinal(const FunctionCallbackInfo<Value>& args);
        static void Find(const FunctionCallbackInfo<Value>& args);
        static void FindNull(const FunctionCallbackInfo<Value>& args);
        static void In(const FunctionCallbackInfo<Value>& args);
        static void Between(const FunctionCallbackInfo<Value>& args);
        static void NotIn(const FunctionCallbackInfo<Value>& args);
        static void Dispose(const FunctionCallbackInfo<Value>& args);

    private:
        static Persistent<Function> constructor;
        static Persistent<v8::FunctionTemplate> m_tpl;

        SPA::CTable *m_table;
    };

}