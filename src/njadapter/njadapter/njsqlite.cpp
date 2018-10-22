
#include "stdafx.h"
#include "njsqlite.h"
#include "njqueue.h"

namespace NJA {

    Persistent<Function> NJSqlite::constructor;

    NJSqlite::NJSqlite(CNjDb *db) : NJHandlerRoot(db), m_db(db) {

    }

    NJSqlite::~NJSqlite() {
        Release();
    }

    bool NJSqlite::IsValid(Isolate* isolate) {
        if (!m_db) {
            ThrowException(isolate, "Async DB handler disposed");
            return false;
        }
        return NJHandlerRoot::IsValid(isolate);
    }

    void NJSqlite::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, "CDb"));
        tpl->InstanceTemplate()->SetInternalFieldCount(3);

        NJHandlerRoot::Init(exports, tpl);

        //methods
        NODE_SET_PROTOTYPE_METHOD(tpl, "BeginTrans", BeginTrans);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Close", Close);
        NODE_SET_PROTOTYPE_METHOD(tpl, "EndTrans", EndTrans);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Execute", Execute);
        NODE_SET_PROTOTYPE_METHOD(tpl, "ExecuteBatch", ExecuteBatch);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Open", Open);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Prepare", Prepare);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDbMS", getMS);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isOpened", IsOpened);

        constructor.Reset(isolate, tpl->GetFunction());
        exports->Set(ToStr(isolate, "CDb"), tpl->GetFunction());
    }

    Local<Object> NJSqlite::New(Isolate* isolate, CNjDb *ash, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ash;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJSqlite::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue() == SECRECT_NUM && args[2]->IsNumber()) {
                bool setCb = args[0]->BooleanValue();
                SPA::INT64 ptr = args[2]->IntegerValue();
                NJSqlite *obj = new NJSqlite((CNjDb*) ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CSqlite()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJSqlite::Release() {
        {
            SPA::CAutoLock al(m_cs);
            if (m_db) {
                m_db = nullptr;
            }
        }
        NJHandlerRoot::Release();
    }

    void NJSqlite::BeginTrans(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            tagTransactionIsolation isolation = tiReadCommited;
            auto p0 = args[0];
            if (p0->IsInt32()) {
                int n = p0->Int32Value();
                if (n < 0 || n > tiIsolated) {
                    ThrowException(isolate, "Bad transaction isolation value");
                    return;
                }
                isolation = (tagTransactionIsolation) n;
            } else if (!p0->IsNullOrUndefined()) {
                ThrowException(isolate, "An integer value expected for transaction isolation");
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            SPA::UINT64 index = obj->m_db->BeginTrans(isolate, 2, argv, isolation);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::EndTrans(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            tagRollbackPlan plan = rpDefault;
            auto p0 = args[0];
            if (p0->IsInt32()) {
                int n = p0->Int32Value();
                if (n < 0 || n > rpRollbackAlways) {
                    ThrowException(isolate, "Bad rollback plan value");
                    return;
                }
                plan = (tagRollbackPlan) n;
            } else if (!p0->IsNullOrUndefined()) {
                ThrowException(isolate, "An integer value expected for rollback plan");
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            SPA::UINT64 index = obj->m_db->EndTrans(isolate, 2, argv, plan);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::IsOpened(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_db->IsOpened()));
        }
    }

    void NJSqlite::Close(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            Local<Value> argv[] = {args[0], args[1]};
            SPA::UINT64 index = obj->m_db->Close(isolate, 2, argv);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::Open(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::wstring strConnection;
            auto p0 = args[0];
            if (p0->IsString()) {
                strConnection = ToStr(p0);
            } else if (!p0->IsNullOrUndefined()) {
                ThrowException(isolate, "A string expected for DB connection");
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            unsigned int flags = 0;
            p0 = args[3];
            if (p0->IsUint32()) {
                flags = p0->Uint32Value();
            } else if (!p0->IsNullOrUndefined()) {
                ThrowException(isolate, "An unsigned int value expected for DB open flags");
                return;
            }
            SPA::UINT64 index = obj->m_db->Open(isolate, 2, argv, strConnection.c_str(), flags);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::ExecuteBatch(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto p = args[0];
            if (!p->IsInt32()) {
                ThrowException(isolate, "An integer expected for transaction isolation value");
                return;
            }
            int n = p->Int32Value();
            if (n < tiUnspecified || n > tiIsolated) {
                ThrowException(isolate, "A bad transaction isolation value");
                return;
            }
            tagTransactionIsolation ti = (tagTransactionIsolation) n;

            std::wstring sql;
            p = args[1];
            if (p->IsString()) {
                sql = ToStr(p);
            }
            if (!sql.size()) {
                ThrowException(isolate, "A valid SQL statement expected");
            }

            CDBVariantArray vParam;
            p = args[2];
            if (p->IsArray()) {
                Local<Array> jsArr = Local<Array>::Cast(p);
                unsigned int count = jsArr->Length();
                for (unsigned int n = 0; n < count; ++n) {
                    auto d = jsArr->Get(n);
                    CDBVariant vt;
                    if (!From(d, "", vt)) {
                        ThrowException(isolate, UNSUPPORTED_TYPE);
                        return;
                    }
                    vParam.push_back(std::move(vt));
                }
            } else if (p->IsObject()) {
                Local<Object> qObj = p->ToObject();
                if (NJQueue::IsUQueue(qObj)) {
                    NJQueue *njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                    if (!NJQueue::ToParamArray(njq, vParam)) {
                        njq->Release();
                        ThrowException(isolate, "Data must be serialized by calling the method SaveObject");
                        return;
                    }
                    njq->Release();
                } else {
                    ThrowException(isolate, "An array of parameter data expected");
                    return;
                }
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "An array of parameter data expected");
                return;
            }
            Local<Value> argv[] = {args[3], args[4], args[5], args[6], args[7]};
            tagRollbackPlan rp = rpDefault;
            p = args[8];
            if (p->IsInt32()) {
                n = p->Int32Value();
                if (n < rpDefault || n > rpRollbackAlways) {
                    ThrowException(isolate, "A bad rollback plan found");
                    return;
                }
                rp = (tagRollbackPlan) n;
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "An integer expected for rollback plan");
                return;
            }
            std::wstring delimiter(L";");
            p = args[9];
            if (p->IsString()) {
                delimiter = ToStr(p);
            } else if (!p->IsNullOrUndefined()) {
                ThrowException(isolate, "A string expected for sql statement delimiter");
                return;
            }

            CParameterInfoArray vPInfo;
            p = args[10];
            if (!ToPInfoArray(isolate, p, vPInfo))
                return;
            SPA::UINT64 index = obj->m_db->ExecuteBatch(isolate, 5, argv, ti, sql.c_str(), vParam, rp, delimiter.c_str(), vPInfo);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::Execute(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::UINT64 index;
            auto p = args[0];
            Local<Value> argv[] = {args[1], args[2], args[3], args[4]};
            if (p->IsArray()) {
                CDBVariantArray vParam;
                Local<Array> jsArr = Local<Array>::Cast(p);
                unsigned int count = jsArr->Length();
                for (unsigned int n = 0; n < count; ++n) {
                    auto d = jsArr->Get(n);
                    CDBVariant vt;
                    if (!From(d, "", vt)) {
                        ThrowException(isolate, UNSUPPORTED_TYPE);
                        return;
                    }
                    vParam.push_back(std::move(vt));
                }
                index = obj->m_db->Execute(isolate, 4, argv, vParam);
            } else if (p->IsObject() && !p->IsString()) {
                Local<Object> qObj = p->ToObject();
                if (NJQueue::IsUQueue(qObj)) {
                    NJQueue *njq = ObjectWrap::Unwrap<NJQueue>(qObj);
                    CDBVariantArray vParam;
                    if (!NJQueue::ToParamArray(njq, vParam)) {
                        njq->Release();
                        ThrowException(isolate, "Data must be serialized by calling the method SaveObject");
                        return;
                    }
                    njq->Release();
                    index = obj->m_db->Execute(isolate, 4, argv, vParam);
                } else {
                    ThrowException(isolate, "A SQL statement string or an array of parameter data expected");
                    return;
                }
            } else {
                std::wstring sql;
                if (p->IsString())
                    sql = ToStr(p);
                else if (!p->IsNullOrUndefined()) {
                    ThrowException(isolate, "A SQL statement string or an array of parameter data expected");
                    return;
                }
                index = obj->m_db->Execute(isolate, 4, argv, sql.c_str());
            }
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }

    void NJSqlite::getMS(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, obj->m_db->GetDBManagementSystem()));
        }
    }

    void NJSqlite::Prepare(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJSqlite* obj = ObjectWrap::Unwrap<NJSqlite>(args.Holder());
        if (obj->IsValid(isolate)) {
            std::wstring sql;
            auto p0 = args[0];
            if (p0->IsString()) {
                sql = ToStr(p0);
            }
            if (!sql.size()) {
                ThrowException(isolate, "A non-empty sql statement string expected");
                return;
            }
            Local<Value> argv[] = {args[1], args[2]};
            CParameterInfoArray vPInfo;
            p0 = args[3];
            if (!ToPInfoArray(isolate, p0, vPInfo))
                return;
            SPA::UINT64 index = obj->m_db->Prepare(isolate, 2, argv, sql.c_str(), vPInfo);
            if (index) {
                args.GetReturnValue().Set(Boolean::New(isolate, index != INVALID_NUMBER));
            }
        }
    }
}
