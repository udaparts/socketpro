
#include "stdafx.h"
#include "njcache.h"
#include "njtable.h"

namespace NJA {

    Persistent<Function> NJCache::constructor;

    NJCache::NJCache(SPA::CDataSet *ds) : m_ds(ds) {
    }

    NJCache::~NJCache() {
    }

    bool NJCache::IsValid(Isolate* isolate) {
        if (!m_ds) {
            ThrowException(isolate, "Cache handler disposed");
            return false;
        }
        return true;
    }

    void NJCache::Release() {
        m_ds = nullptr;
    }

    void NJCache::Init(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();

        // Prepare constructor template
        Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
        tpl->SetClassName(ToStr(isolate, u"CCache", 6));
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        //methods
        //NODE_SET_PROTOTYPE_METHOD(tpl, "Empty", Empty);
        NODE_SET_PROTOTYPE_METHOD(tpl, "GetMeta", GetMeta);
        NODE_SET_PROTOTYPE_METHOD(tpl, "GetRowCount", GetRowCount);
        NODE_SET_PROTOTYPE_METHOD(tpl, "GetFields", GetColumnCount);
        NODE_SET_PROTOTYPE_METHOD(tpl, "FindOrdinal", FindOrdinal);
        NODE_SET_PROTOTYPE_METHOD(tpl, "FindKeys", FindKeys);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Find", Find);
        NODE_SET_PROTOTYPE_METHOD(tpl, "FindNull", FindNull);
        NODE_SET_PROTOTYPE_METHOD(tpl, "In", In);
        NODE_SET_PROTOTYPE_METHOD(tpl, "Between", Between);
        NODE_SET_PROTOTYPE_METHOD(tpl, "NotIn", NotIn);

        //properties
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDbIp", GetDBServerIp);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDbName", GetDBServerName);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getUpdater", GetUpdater);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getMS", getMS);
        NODE_SET_PROTOTYPE_METHOD(tpl, "isEmpty", isEmpty);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDBNameCase", getDBNameCase);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getTableNameCase", getTableNameCase);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getFieldNameCase", getFieldNameCase);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDataCase", getDataCase);
        NODE_SET_PROTOTYPE_METHOD(tpl, "getDbTable", getDbTable);
        auto ctx = isolate->GetCurrentContext();
        constructor.Reset(isolate, tpl->GetFunction(ctx).ToLocalChecked());
        exports->Set(ctx, ToStr(isolate, u"CCache", 6), tpl->GetFunction(ctx).ToLocalChecked());
    }

    Local<Object> NJCache::New(Isolate* isolate, SPA::CDataSet *ds, bool setCb) {
        SPA::UINT64 ptr = (SPA::UINT64)ds;
        Local<Value> argv[] = {Boolean::New(isolate, setCb), Number::New(isolate, (double) SECRECT_NUM), Number::New(isolate, (double) ptr)};
        Local<Context> context = isolate->GetCurrentContext();
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        return cons->NewInstance(context, 3, argv).ToLocalChecked();
    }

    void NJCache::New(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        if (args.IsConstructCall()) {
            if (args[0]->IsBoolean() && args[1]->IsNumber() && args[1]->IntegerValue(isolate->GetCurrentContext()).ToChecked() == SECRECT_NUM && args[2]->IsNumber()) {
#ifdef BOOL_ISOLATE
                //bool setCb = args[0]->BooleanValue(isolate);
#else
                //bool setCb = args[0]->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
                SPA::INT64 ptr = args[2]->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                NJCache *obj = new NJCache((SPA::CDataSet*)ptr);
                obj->Wrap(args.This());
                args.GetReturnValue().Set(args.This());
            } else {
                args.GetReturnValue().SetNull();
            }
        } else {
            // Invoked as plain function `CCache()`, turn into construct call.
            Local<Context> context = isolate->GetCurrentContext();
            Local<Function> cons = Local<Function>::New(isolate, constructor);
            Local<Object> result = cons->NewInstance(context, 0, nullptr).ToLocalChecked();
            args.GetReturnValue().Set(result);
        }
    }

    void NJCache::GetDBServerIp(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(ToStr(isolate, obj->m_ds->GetDBServerIp().c_str()));
        }
    }

    void NJCache::GetDBServerName(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
#ifdef WIN32_64
            args.GetReturnValue().Set(ToStr(isolate, (const UTF16*) obj->m_ds->GetDBServerName().c_str()));
#else
            auto s = Utilities::ToUTF16(obj->m_ds->GetDBServerName());
            args.GetReturnValue().Set(ToStr(isolate, s.c_str(), s.size()));
#endif
        }
    }

    void NJCache::GetUpdater(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
#ifdef WIN32_64
            args.GetReturnValue().Set(ToStr(isolate, (const UTF16*) obj->m_ds->GetUpdater().c_str()));
#else
            auto s = Utilities::ToUTF16(obj->m_ds->GetUpdater());
            args.GetReturnValue().Set(ToStr(isolate, s.c_str(), s.size()));
#endif
        }
    }

    void NJCache::getMS(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Int32::New(isolate, (int) obj->m_ds->GetDBManagementSystem()));
        }
    }

    void NJCache::isEmpty(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_ds->IsEmpty()));
        }
    }

    void NJCache::getDBNameCase(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_ds->GetDBNameCaseSensitive()));
        }
    }

    void NJCache::getTableNameCase(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_ds->GetTableNameCaseSensitive()));
        }
    }

    void NJCache::getFieldNameCase(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_ds->GetFieldNameCaseSensitive()));
        }
    }

    void NJCache::getDataCase(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            args.GetReturnValue().Set(Boolean::New(isolate, obj->m_ds->GetDataCaseSensitive()));
        }
    }

    void NJCache::getDbTable(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            auto ctx = isolate->GetCurrentContext();
            auto db_table = obj->m_ds->GetDBTablePair();
            Local<Array> v = Array::New(isolate, (int) db_table.size());
            unsigned int index = 0;
            for (auto it = db_table.begin(), end = db_table.end(); it != end; ++it, ++index) {
                Local<Object> p = Object::New(isolate);
                p->Set(ctx, ToStr(isolate, u"db", 2), ToStr(isolate, it->first.c_str()));
                p->Set(ctx, ToStr(isolate, u"table", 5), ToStr(isolate, it->second.c_str()));
                v->Set(ctx, index, p);
            }
            args.GetReturnValue().Set(v);
        }
    }

    void NJCache::Empty(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            obj->m_ds->Empty();
        }
    }

    bool NJCache::GetDTPair(const FunctionCallbackInfo<Value>& args, SPA::CPDbTable &p) {
        auto p0 = args[0];
        auto p1 = args[1];
        if (!p0->IsString() || !p1->IsString()) {
            ThrowException(args.GetIsolate(), "DB and table names expected");
            return false;
        }
        p.first = ToStr(args.GetIsolate(), p0);
        p.second = ToStr(args.GetIsolate(), p1);
        return true;
    }

    void NJCache::GetMeta(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                Local<Array> jsMeta = ToMeta(isolate, obj->m_ds->GetColumMeta(p.first.c_str(), p.second.c_str()));
                args.GetReturnValue().Set(jsMeta);
            }
        }
    }

    void NJCache::GetRowCount(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                Local<Value> jsRows = Number::New(isolate, (double) obj->m_ds->GetRowCount(p.first.c_str(), p.second.c_str()));
                args.GetReturnValue().Set(jsRows);
            }
        }
    }

    void NJCache::GetColumnCount(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                Local<Value> jsCols = Number::New(isolate, (double) obj->m_ds->GetColumnCount(p.first.c_str(), p.second.c_str()));
                args.GetReturnValue().Set(jsCols);
            }
        }
    }

    void NJCache::FindKeys(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto mapKey = obj->m_ds->FindKeys(p.first.c_str(), p.second.c_str());
                Local<Array> jsMeta = ToMeta(isolate, mapKey);
                args.GetReturnValue().Set(jsMeta);
            }
        }
    }

    void NJCache::FindOrdinal(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsString()) {
                    ThrowException(args.GetIsolate(), "Column name expected");
                    return;
                }
                SPA::CDBString colName = ToStr(isolate, p2);
                int ordinal = (int) obj->m_ds->FindOrdinal(p.first.c_str(), p.second.c_str(), colName.c_str());
                Local<Value> jsOrdinal = Int32::New(isolate, ordinal);
                args.GetReturnValue().Set(jsOrdinal);
            }
        }
    }

    void NJCache::FindNull(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsUint32()) {
                    ThrowException(isolate, NJTable::COLUMN_ORDINAL_EXPECTED);
                    return;
                }
                unsigned int ordinal = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                SPA::CTable *pTable = new SPA::CTable;
                int res = obj->m_ds->FindNull(p.first.c_str(), p.second.c_str(), ordinal, *pTable);
                if (res <= 0) {
                    delete pTable;
                    NJTable::ThrowException(res, isolate);
                } else {
                    Local<Object> jsTable = NJTable::New(isolate, pTable, true);
                    args.GetReturnValue().Set(jsTable);
                }
            }
        }
    }

    void NJCache::Between(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsUint32()) {
                    ThrowException(isolate, NJTable::COLUMN_ORDINAL_EXPECTED);
                    return;
                }
                unsigned int ordinal = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                CComVariant vt0, vt1;
                if (!From(isolate, args[3], "", vt0) || !From(isolate, args[4], "", vt1) || vt0.vt <= VT_NULL || vt1.vt <= VT_NULL) {
                    ThrowException(isolate, "Fourth and fifth argurments expected");
                }
                SPA::CTable *pTable = new SPA::CTable;
                int res = obj->m_ds->Between(p.first.c_str(), p.second.c_str(), ordinal, vt0, vt1, *pTable);
                if (res <= 0) {
                    delete pTable;
                    NJTable::ThrowException(res, isolate);
                } else {
                    Local<Object> jsTable = NJTable::New(isolate, pTable, true);
                    args.GetReturnValue().Set(jsTable);
                }
            }
        }
    }

    void NJCache::Find(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsUint32()) {
                    ThrowException(isolate, NJTable::COLUMN_ORDINAL_EXPECTED);
                    return;
                }
                unsigned int ordinal = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                auto p3 = args[3];
                if (!p3->IsUint32()) {
                    ThrowException(isolate, NJTable::OPERATION_EXPECTED);
                    return;
                }
                unsigned int data = p3->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                if (data > (unsigned int) CTable::Operator::is_null) {
                    ThrowException(isolate, NJTable::BAD_OPERATION);
                    return;
                }
                std::string hint;
                SPA::CTable::Operator op = (SPA::CTable::Operator)data;
                auto p4 = args[4];
                auto p5 = args[5];
                if (p5->IsString()) {
#if NODE_MODULE_VERSION < 57	
                    String::Utf8Value str(p5);
#else
                    String::Utf8Value str(isolate, p5);
#endif
                    hint = *str;
                    ToLower(hint);
                }
                CComVariant vt;
                if (!From(isolate, p4, hint, vt)) {
                    ThrowException(isolate, UNSUPPORTED_TYPE);
                    return;
                }
                SPA::CTable *pTable = new SPA::CTable;
                int res = obj->m_ds->Find(p.first.c_str(), p.second.c_str(), ordinal, op, vt, *pTable);
                if (res <= 0) {
                    delete pTable;
                    NJTable::ThrowException(res, isolate);
                } else {
                    Local<Object> jsTable = NJTable::New(isolate, pTable, true);
                    args.GetReturnValue().Set(jsTable);
                }
            }
        }
    }

    void NJCache::NotIn(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsUint32()) {
                    ThrowException(isolate, NJTable::COLUMN_ORDINAL_EXPECTED);
                    return;
                }
                unsigned int ordinal = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                SPA::UDB::CDBVariantArray v;
                auto p3 = args[3];
                if (!ToArray(isolate, p3, v)) {
                    return;
                }
                SPA::CTable *pTable = new SPA::CTable;
                int res = obj->m_ds->NotIn(p.first.c_str(), p.second.c_str(), ordinal, v, *pTable);
                if (res <= 0) {
                    delete pTable;
                    NJTable::ThrowException(res, isolate);
                } else {
                    Local<Object> jsTable = NJTable::New(isolate, pTable, true);
                    args.GetReturnValue().Set(jsTable);
                }
            }
        }
    }

    void NJCache::In(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
        NJCache* obj = ObjectWrap::Unwrap<NJCache>(args.Holder());
        if (obj->IsValid(isolate)) {
            SPA::CPDbTable p;
            if (GetDTPair(args, p)) {
                auto p2 = args[2];
                if (!p2->IsUint32()) {
                    ThrowException(isolate, NJTable::COLUMN_ORDINAL_EXPECTED);
                    return;
                }
                unsigned int ordinal = p2->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                SPA::UDB::CDBVariantArray v;
                auto p3 = args[3];
                if (!ToArray(isolate, p3, v)) {
                    return;
                }
                SPA::CTable *pTable = new SPA::CTable;
                int res = obj->m_ds->In(p.first.c_str(), p.second.c_str(), ordinal, v, *pTable);
                if (res <= 0) {
                    delete pTable;
                    NJTable::ThrowException(res, isolate);
                } else {
                    Local<Object> jsTable = NJTable::New(isolate, pTable, true);
                    args.GetReturnValue().Set(jsTable);
                }
            }
        }
    }
}
