
#include "stdafx.h"
#include "njobjects.h"
#include "../../../include/sqlite/usqlite.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/odbc/uodbc.h"
#include "njsqlite.h"


namespace SPA {
    namespace ClientSide {

        using namespace NJA;

        SPA::UINT64 CAsyncServiceHandler::SendRequest(Isolate* isolate, int args, Local<Value>* argv, unsigned short reqId, const unsigned char* pBuffer, unsigned int size) {
            if (!argv) args = 0;
            DResultHandler rh;
            DServerException se;
            DDiscarded dd;
            UINT64 callIndex = GetCallIndex();
            if (args > 0) {
                if (argv[0]->IsFunction()) {
                    bool delay = false;
                    Local<Value> d = argv[3];
                    if (!IsNullOrUndefined(d)) {
                        if (d->IsBoolean() || d->IsUint32()) {
#ifdef BOOL_ISOLATE
                            delay = d->BooleanValue(isolate);
#else
                            delay = d->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
                        } else if (!NJA::IsNullOrUndefined(d)) {
                            NJA::ThrowException(isolate, "A boolean value expected for parameter delay");
                            return 0;
                        }
                    }
                    std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[0])));
                    rh = [this, func, delay](CAsyncResult & ar) {
                        ReqCb cb(ar, tagEvent::eResult);
                        cb.Func = func;
                        PAsyncServiceHandler h = ar.AsyncServiceHandler;
                        *cb.Buffer << h << delay;
                        cb.Buffer->Push(ar.UQueue.GetBuffer(), ar.UQueue.GetSize());
                        ar.UQueue.SetSize(0);
                        this->m_cs.lock();
                        this->m_deqReqCb.push_back(std::move(cb));
                        if (this->m_deqReqCb.size() < 2) {
                            int fail = uv_async_send(&this->m_typeReq);
                            assert(!fail);
                        }
                        this->m_cs.unlock();
                    };
                } else if (!IsNullOrUndefined(argv[0])) {
                    ThrowException(isolate, "A callback expected for tracking returned results");
                    return 0;
                }
            }
            if (args > 1) {
                if (argv[1]->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[1])));
                    dd = [this, func, reqId](CAsyncServiceHandler* ash, bool canceled) {
                        ReqCb cb(reqId, tagEvent::eDiscarded);
                        cb.Func = func;
                        PAsyncServiceHandler h = ash;
                        *cb.Buffer << h << canceled;
                        this->m_cs.lock();
                        this->m_deqReqCb.push_back(std::move(cb));
                        if (this->m_deqReqCb.size() < 2) {
                            int fail = uv_async_send(&this->m_typeReq);
                            assert(!fail);
                        }
                        this->m_cs.unlock();
                    };
                } else if (!IsNullOrUndefined(argv[1])) {
                    ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
                    return 0;
                }
            }
            if (args > 2) {
                if (argv[2]->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc(isolate, Local<Function>::Cast(argv[2])));
                    se = [this, func](CAsyncServiceHandler* ash, unsigned short reqId, const wchar_t* errMsg, const char* errWhere, unsigned int errCode) {
                        ReqCb cb(reqId, tagEvent::eException);
                        cb.Func = func;
                        PAsyncServiceHandler h = ash;
                        *cb.Buffer << h << errMsg << errWhere << errCode;
                        this->m_cs.lock();
                        this->m_deqReqCb.push_back(std::move(cb));
                        if (this->m_deqReqCb.size() < 2) {
                            int fail = uv_async_send(&this->m_typeReq);
                            assert(!fail);
                        }
                        this->m_cs.unlock();
                    };
                } else if (!IsNullOrUndefined(argv[2])) {
                    ThrowException(isolate, "A callback expected for tracking exceptions from server");
                    return 0;
                }
            }
            if (!SendRequest(reqId, pBuffer, size, rh, dd, se)) {
                return INVALID_NUMBER;
            }
            return callIndex;
        }

        void CAsyncServiceHandler::req_cb(uv_async_t* handle) {
            CAsyncServiceHandler* obj = (CAsyncServiceHandler*) handle->data; //sender
            assert(obj);
            if (!obj) return;
            Isolate* isolate = Isolate::GetCurrent();
            HandleScope handleScope(isolate); //required for Node 4.x
            {
                Local<Context> ctx = isolate->GetCurrentContext();
                Local<Value> null = Null(isolate);
                CSpinLock& cs = obj->m_cs;
                cs.lock();
                while (obj->m_deqReqCb.size()) {
                    ReqCb cb(std::move(obj->m_deqReqCb.front()));
                    obj->m_deqReqCb.pop_front();
                    cs.unlock();
                    PAsyncServiceHandler processor;
                    *cb.Buffer >> processor;
                    assert(processor);
                    Local<Value> jsReqId = Uint32::New(isolate, cb.ReqId);
                    Local<Function> func;
                    assert(cb.Func);
                    if (cb.Func)
                        func = Local<Function>::New(isolate, *cb.Func);
                    switch (cb.Type) {
                        case tagEvent::eResult:
                            if (!func.IsEmpty()) {
                                bool delay = cb.Buffer->Load<bool>();
                                if (delay) {
                                    Local<Value> argv[2] = {NJQueue::New(isolate, cb.Buffer), jsReqId};
                                    func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                                } else {
                                    Local<Object> q = Local<Object>::New(isolate, g_buff);
                                    NJQueue* obj = node::ObjectWrap::Unwrap<NJQueue>(q);
                                    obj->Move(cb.Buffer);
                                    Local<Value> argv[2] = {q, jsReqId};
                                    func->Call(ctx, null, 2, argv);
                                }
                            }
                            if (cb.Buffer) {
                                CScopeUQueue::Unlock(cb.Buffer);
                            }
                            break;
                        case tagEvent::eDiscarded:
                        {
                            bool canceled;
                            *cb.Buffer >> canceled;
                            assert(!cb.Buffer->GetSize());
                            CScopeUQueue::Unlock(cb.Buffer);
                            if (!func.IsEmpty()) {
                                auto b = Boolean::New(isolate, canceled);
                                Local<Value> argv[] = {b, jsReqId};
                                func->Call(ctx, null, 2, argv);
                            }
                        }
                            break;
                        case tagEvent::eException:
                            if (!func.IsEmpty()) {
                                SPA::CDBString errMsg;
                                std::string errWhere;
                                unsigned int errCode;
                                *cb.Buffer >> errMsg >> errWhere >> errCode;
                                assert(!cb.Buffer->GetSize());
                                CScopeUQueue::Unlock(cb.Buffer);
                                Local<String> jsMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                                Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
                                Local<Value> jsCode = Number::New(isolate, errCode);
                                Local<Value> argv[] = {jsMsg, jsCode, jsWhere, jsReqId};
                                func->Call(ctx, null, 4, argv);
                            } else {
                                CScopeUQueue::Unlock(cb.Buffer);
                            }
                            break;
                        default:
                            assert(false); //shouldn't come here
                            break;
                    }
                    cs.lock();
                }
                cs.unlock();
            }
            isolate->RunMicrotasks(); //may speed up pumping
        }

        Local<Object> CreateDb(Isolate* isolate, CAsyncServiceHandler* ash) {
            Local<Object> njDB;
            unsigned int sid = ash->GetSvsID();
            switch (sid) {
                case SPA::Odbc::sidOdbc:
                case SPA::Mysql::sidMysql:
                case SPA::Sqlite::sidSqlite:
                    njDB = NJSqlite::New(isolate, (CNjDb*) ash, true);
                    break;
                default:
                    assert(false);
                    break;
            }
            return njDB;
        }
    }
}

namespace NJA {

    int time_offset(time_t utcTime) {
#ifndef WIN32_64
        struct tm gbuf;
        struct tm* ptm = gmtime_r(&utcTime, &gbuf);
#else
        struct tm* ptm = gmtime(&utcTime);
#endif
        //Request that mktime() looksup dst in timezone database
        ptm->tm_isdst = -1;
        return (int) difftime(utcTime, std::mktime(ptm));
    }

    bool IsNullOrUndefined(const Local<Value>& v) {
#ifdef HAS_NULLORUNDEFINED_FUNC
        return v->IsNullOrUndefined();
#else
        return (v->IsNull() || v->IsUndefined());
#endif
    }

    void ThrowException(Isolate* isolate, const char* str) {
        isolate->ThrowException(Exception::TypeError(ToStr(isolate, str)));
    }

    bool ToGroups(Isolate* isolate, const Local<Value>& p, std::vector<unsigned int>& v) {
        v.clear();
        if (p->IsArray()) {
            auto ctx = isolate->GetCurrentContext();
            Local<Array> jsArr = Local<Array>::Cast(p);
            unsigned int count = jsArr->Length();
            for (unsigned int n = 0; n < count; ++n) {
                auto d = jsArr->Get(ctx, n).ToLocalChecked();
                if (d->IsUint32()) {
                    v.push_back(d->Uint32Value(isolate->GetCurrentContext()).ToChecked());
                } else {
                    ThrowException(isolate, "An unsigned int value expected for a group chat id");
                    return false;
                }
            }
        } else if (p->IsUint32Array()) {
            Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(p);
            const unsigned int* groups = (const unsigned int*) vInt->Buffer()->GetContents().Data();
            unsigned int count = (unsigned int) vInt->Length();
            v.assign(groups, groups + count);
        } else {
            ThrowException(isolate, "An array of unsigned int values expected for group chat identification numbers");
            return false;
        }
        return true;
    }

    Local<String> ToStr(Isolate* isolate, const char* str, size_t len) {
        if (!str) {
            str = "";
            len = 0;
        } else if (len == (size_t) INVALID_NUMBER) {
            len = strlen(str);
        }
        return String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal, (int) len).ToLocalChecked();
    }

    Local<String> ToStr(Isolate* isolate, const SPA::UTF16* str, size_t len) {
        if (!str) {
            str = (const SPA::UTF16*)L"";
            len = 0;
        } else if (len == (size_t) INVALID_NUMBER) {
            len = SPA::Utilities::GetLen(str);
        }
#ifdef LINUX_STRING_BUG
        return String::NewFromTwoByte(isolate, (const uint16_t*) str, v8::NewStringType::kInternalized, (int) len).ToLocalChecked();
#else
        return String::NewFromTwoByte(isolate, (const uint16_t*) str, v8::NewStringType::kNormal, (int) len).ToLocalChecked();
#endif
    }

    SPA::CDBString ToStr(Isolate* isolate, const Local<Value>& s) {
        assert(s->IsString());
#if NODE_MODULE_VERSION < 57
        String::Value str(s);
#else
        String::Value str(isolate, s);
#endif
        return (const UTF16*) *str;
    }

    std::string ToAStr(Isolate* isolate, const Local<Value>& s) {
        assert(s->IsString());
#if NODE_MODULE_VERSION < 57
        String::Utf8Value str(s);
#else
        String::Utf8Value str(isolate, s);
#endif
        return *str;
    }

    Local<Value> ToDate(Isolate* isolate, SPA::UINT64 datetime) {
        unsigned int us;
        bool time_only;
        SPA::UDateTime dt(datetime);
        double time = dt.GetTime(time_only, &us) * 1000.0 + us / 1000.0;
        if (time_only) {
            //time only, convert it to js string
            char s[64] = {0};
            dt.ToDBString(s, sizeof (s));
            return ToStr(isolate, (const char*) s, strlen((const char*) s));
        }
        return Date::New(isolate->GetCurrentContext(), time).ToLocalChecked();
    }

    SPA::UINT64 ToDate(Isolate* isolate, const Local<Value>& d) {
        SPA::UINT64 millisSinceEpoch;
        if (d->IsDate()) {
            Date* dt = Date::Cast(*d);
            millisSinceEpoch = (SPA::UINT64)dt->ValueOf();
        } else if (d->IsNumber()) {
            millisSinceEpoch = (SPA::UINT64)(d->IntegerValue(isolate->GetCurrentContext()).ToChecked());
        } else {
            return INVALID_NUMBER;
        }
        std::time_t t = millisSinceEpoch / 1000;
        unsigned int ms = (unsigned int) (millisSinceEpoch % 1000);
        std::tm* ltime = std::gmtime(&t);
        SPA::UDateTime dt(*ltime, ms * 1000);
        return dt.time;
    }

    bool From(Isolate* isolate, const Local<Value>& v, const std::string& id, CComVariant& vt) {
        vt.Clear();
        if (IsNullOrUndefined(v))
            vt.vt = VT_NULL;
        else if (v->IsDate()) {
            vt.vt = VT_DATE;
            vt.ullVal = ToDate(isolate, v);
        } else if (v->IsBoolean()) {
            vt.vt = VT_BOOL;
#ifdef BOOL_ISOLATE
            vt.boolVal = v->BooleanValue(isolate) ? VARIANT_TRUE : VARIANT_FALSE;
#else
            vt.boolVal = v->BooleanValue(isolate->GetCurrentContext()).ToChecked() ? VARIANT_TRUE : VARIANT_FALSE;
#endif
        } else if (v->IsString()) {
            if (id == "a" || id == "ascii") {
                char* p;
                vt.vt = (VT_ARRAY | VT_I1);
#if NODE_MODULE_VERSION <57
                String::Utf8Value str(v);
#else
                String::Utf8Value str(isolate, v);
#endif
                unsigned int len = (unsigned int) str.length();
                SAFEARRAYBOUND sab[] = {len, 0};
                vt.parray = SafeArrayCreate(VT_I1, 1, sab);
                SafeArrayAccessData(vt.parray, (void**) &p);
                memcpy(p, *str, len);
                SafeArrayUnaccessData(vt.parray);
            } else if (id == "dec" || id == "decimal") {
#if NODE_MODULE_VERSION <57
                String::Utf8Value str(v);
#else
                String::Utf8Value str(isolate, v);
#endif
                vt.vt = VT_DECIMAL;
                SPA::ParseDec(*str, vt.decVal);
            } else {
                vt.vt = VT_BSTR;
#if NODE_MODULE_VERSION <57
                String::Value str(v);
#else
                String::Value str(isolate, v);
#endif
#ifdef WIN32_64
                vt.bstrVal = SysAllocString((const wchar_t*) * str);
#else
                vt.bstrVal = SysAllocStringLen((const UTF16*) *str, (unsigned int) str.length());
#endif
            }
        } else if (v->IsInt32() && id == "") {
            vt.vt = VT_I4;
            vt.lVal = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
        }
#ifdef HAS_BIGINT
        else if (v->IsBigInt() && id == "") {
            vt.vt = VT_I8;
            vt.llVal = v->IntegerValue(isolate->GetCurrentContext()).ToChecked();
        }
#endif
        else if (v->IsNumber()) {
            if (id == "f" || id == "float") {
                vt.vt = VT_R4;
                vt.fltVal = (float) v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "d" || id == "double") {
                vt.vt = VT_R8;
                vt.dblVal = v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "i" || id == "int") {
                vt.vt = VT_I4;
                vt.lVal = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "ui" || id == "uint") {
                vt.vt = VT_UI4;
                vt.ulVal = v->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "l" || id == "long") {
                vt.vt = VT_I8;
                vt.llVal = (SPA::INT64)v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "ul" || id == "ulong") {
                vt.vt = VT_UI8;
                vt.ullVal = (SPA::UINT64)v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "s" || id == "short") {
                vt.vt = VT_I2;
                vt.iVal = (short) v->Int32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "us" || id == "ushort") {
                vt.vt = VT_UI2;
                vt.iVal = (unsigned short) v->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "dec" || id == "decimal") {
                vt.vt = VT_DECIMAL;
#if NODE_MODULE_VERSION < 57
                String::Utf8Value str(v);
#else
                String::Utf8Value str(isolate, v);
#endif
                ParseDec(*str, vt.decVal);
            } else if (id == "c" || id == "char") {
                vt.vt = VT_I1;
                vt.cVal = (char) v->Int32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "b" || id == "byte") {
                vt.vt = VT_UI1;
                vt.bVal = (unsigned char) v->Uint32Value(isolate->GetCurrentContext()).ToChecked();
            } else if (id == "date") {
                vt.vt = VT_DATE;
                vt.ullVal = (SPA::UINT64)v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            } else {
                vt.vt = VT_R8;
                vt.dblVal = v->NumberValue(isolate->GetCurrentContext()).ToChecked();
            }
        } else if (v->IsInt8Array()) {
            char* p;
            vt.vt = (VT_ARRAY | VT_I1);
            Local<v8::Int8Array> vInt = Local<v8::Int8Array>::Cast(v);
            const char* bytes = (const char*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_I1, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len);
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsInt16Array()) {
            short* p;
            vt.vt = (VT_ARRAY | VT_I2);
            Local<v8::Int16Array> vInt = Local<v8::Int16Array>::Cast(v);
            const short* bytes = (const short*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_I2, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (short));
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsUint16Array()) {
            unsigned short* p;
            vt.vt = (VT_ARRAY | VT_UI2);
            Local<v8::Uint16Array> vInt = Local<v8::Uint16Array>::Cast(v);
            const unsigned short* bytes = (const unsigned short*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_UI2, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (unsigned short));
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsInt32Array()) {
            int* p;
            vt.vt = (VT_ARRAY | VT_I4);
            Local<v8::Int32Array> vInt = Local<v8::Int32Array>::Cast(v);
            const int* bytes = (const int*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_I4, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (int));
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsUint32Array()) {
            unsigned int* p;
            vt = (VT_ARRAY | VT_UI4);
            Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(v);
            const unsigned int* bytes = (const unsigned int*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_UI4, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (unsigned int));
            SafeArrayUnaccessData(vt.parray);
#ifdef HAS_BIGINT
        } else if (v->IsBigInt64Array()) {
            int64_t* p;
            vt.vt = (VT_ARRAY | VT_I8);
            char* bytes = node::Buffer::Data(v);
            Local<v8::BigInt64Array> vInt = Local<v8::BigInt64Array>::Cast(v);
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_I8, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (int64_t));
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsBigUint64Array()) {
            uint64_t* p;
            vt = (VT_ARRAY | VT_UI8);
            char* bytes = node::Buffer::Data(v);
            Local<v8::BigUint64Array> vInt = Local<v8::BigUint64Array>::Cast(v);
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_UI8, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (uint64_t));
            SafeArrayUnaccessData(vt.parray);
#endif
        } else if (v->IsFloat32Array()) {
            float* p;
            vt.vt = (VT_ARRAY | VT_R4);
            Local<v8::Float32Array> vInt = Local<v8::Float32Array>::Cast(v);
            const float* bytes = (const float*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_R4, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (float));
            SafeArrayUnaccessData(vt.parray);
        } else if (v->IsFloat64Array()) {
            double* p;
            vt.vt = (VT_ARRAY | VT_R8);
            Local<v8::Float64Array> vInt = Local<v8::Float64Array>::Cast(v);
            const double* bytes = (const double*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_R8, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len * sizeof (double));
            SafeArrayUnaccessData(vt.parray);
        } else if (node::Buffer::HasInstance(v)) {
            char* p;
            char* bytes = node::Buffer::Data(v);
            unsigned int len = (unsigned int) node::Buffer::Length(v);
            vt.vt = (VT_ARRAY | VT_UI1);
            SAFEARRAYBOUND sab[] = {len, 0};
            vt.parray = SafeArrayCreate(VT_UI1, 1, sab);
            SafeArrayAccessData(vt.parray, (void**) &p);
            memcpy(p, bytes, len);
            SafeArrayUnaccessData(vt.parray);
            if (len == sizeof (GUID) && (id == "u" || id == "uuid")) {
                vt.vt = VT_CLSID;
            }
        } else if (v->IsArray()) {
            tagDataType dt = tagDataType::dtUnknown;
            auto ctx = isolate->GetCurrentContext();
            Local<Array> jsArr = Local<Array>::Cast(v);
            unsigned int count = jsArr->Length();
            for (unsigned int n = 0; n < count; ++n) {
                auto d = jsArr->Get(ctx, n).ToLocalChecked();
                if (d->IsBoolean()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtBool) {
                        return false;
                    } else
                        dt = tagDataType::dtBool;
                } else if (d->IsDate()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtDate) {
                        return false;
                    } else
                        dt = tagDataType::dtDate;
                } else if (d->IsString()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtString) {
                        return false;
                    } else
                        dt = tagDataType::dtString;
#ifdef HAS_BIGINT
                } else if (d->IsBigInt() || id == "l" || id == "long") {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtInt64) {
                        return false;
                    } else
                        dt = tagDataType::dtInt64;
#endif
                } else if (d->IsInt32() || id == "i" || id == "int") {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtInt32)
                        return false;
                    else {
                        dt = tagDataType::dtInt32;
                    }
                } else if (d->IsNumber()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtDouble)
                        return false;
                    else {
                        dt = tagDataType::dtDouble;
                    }
                } else {
                    return false;
                }
            }
            VARTYPE vtType = VT_EMPTY;
            switch (dt) {
                case tagDataType::dtString:
                    vtType = VT_BSTR;
                    break;
                case tagDataType::dtBool:
                    vtType = VT_BOOL;
                    break;
                case tagDataType::dtDate:
                    vtType = VT_DATE;
                    break;
#ifdef HAS_BIGINT
                case tagDataType::dtInt64:
                    vtType = VT_I8;
                    break;
#endif
                case tagDataType::dtInt32:
                    vtType = VT_I4;
                    break;
                case tagDataType::dtDouble:
                    vtType = VT_R8;
                    break;
                default:
                    assert(false); //shouldn't come here
                    break;
            }
            void* p;
            vt.vt = (VT_ARRAY | vtType);
            SAFEARRAYBOUND sab[] = {count, 0};
            vt.parray = SafeArrayCreate(vtType, 1, sab);
            SafeArrayAccessData(vt.parray, &p);
            for (unsigned int n = 0; n < count; ++n) {
                auto d = jsArr->Get(ctx, n).ToLocalChecked();
                switch (vtType) {
                    case VT_BSTR:
                    {
                        BSTR* pbstr = (BSTR*) p;
#if NODE_MODULE_VERSION < 57
                        String::Value str(d);
#else
                        String::Value str(isolate, d);
#endif
#ifdef WIN32_64
                        pbstr[n] = ::SysAllocString((const wchar_t*) * str);
#else
                        pbstr[n] = ::SysAllocStringLen((const UTF16*) *str, (unsigned int) str.length());
#endif
                    }
                        break;
                    case VT_BOOL:
                    {
                        VARIANT_BOOL* pb = (VARIANT_BOOL*) p;
#ifdef BOOL_ISOLATE
                        pb[n] = d->BooleanValue(isolate) ? VARIANT_TRUE : VARIANT_FALSE;
#else
                        pb[n] = d->BooleanValue(isolate->GetCurrentContext()).ToChecked() ? VARIANT_TRUE : VARIANT_FALSE;
#endif
                    }
                        break;
                    case VT_DATE:
                    {
                        SPA::UINT64* pd = (SPA::UINT64*)p;
                        pd[n] = ToDate(isolate, d);
                    }
                        break;
                    case VT_I4:
                    {
                        int* pi = (int*) p;
                        pi[n] = d->Int32Value(isolate->GetCurrentContext()).ToChecked();
                    }
                        break;
                    case VT_I8:
                    {
                        int64_t* pi = (int64_t*) p;
                        pi[n] = d->IntegerValue(isolate->GetCurrentContext()).ToChecked();
                    }
                        break;
                    case VT_R8:
                    {
                        double* pd = (double*) p;
                        pd[n] = d->NumberValue(isolate->GetCurrentContext()).ToChecked();
                    }
                        break;
                    default:
                        assert(false); //shouldn't come here
                        break;
                }
            }
            SafeArrayUnaccessData(vt.parray);
        } else {
            return false; //not supported
        }
        return true;
    }

    Local<Value> DbFrom(Isolate* isolate, SPA::CUQueue& buff) {
        VARTYPE type;
        buff >> type;
        switch (type) {
            case VT_NULL:
            case VT_EMPTY:
                return Null(isolate);
            case VT_BOOL:
            {
                VARIANT_BOOL boolVal;
                buff >> boolVal;
                return Boolean::New(isolate, boolVal ? true : false);
            }
            case VT_I1:
            {
                char cVal;
                buff >> cVal;
                return Int32::New(isolate, cVal);
            }
            case VT_I2:
            {
                short iVal;
                buff >> iVal;
                return Int32::New(isolate, iVal);
            }
            case VT_I4:
            case VT_INT:
            {
                int intVal;
                buff >> intVal;
                return Int32::New(isolate, intVal);
            }
            case VT_I8:
            {
                SPA::INT64 llVal;
                buff >> llVal;
                return Number::New(isolate, (double) llVal);
            }
            case VT_UI1:
            {
                unsigned char bVal;
                buff >> bVal;
                return Uint32::New(isolate, bVal);
            }
            case VT_UI2:
            {
                unsigned short uiVal;
                buff >> uiVal;
                return Uint32::New(isolate, uiVal);
            }
            case VT_UI4:
            case VT_UINT:
            {
                unsigned int uintVal;
                buff >> uintVal;
                return Uint32::New(isolate, uintVal);
            }
            case VT_UI8:
            {
                SPA::UINT64 ullVal;
                buff >> ullVal;
                return Number::New(isolate, (double) ullVal);
            }
            case VT_R4:
            {
                float fltVal;
                buff >> fltVal;
                return Number::New(isolate, fltVal);
            }
            case VT_R8:
            {
                double dblVal;
                buff >> dblVal;
                return Number::New(isolate, dblVal);
            }
            case VT_CY:
            {
                SPA::INT64 llVal;
                buff >> llVal;
                double d = (double) llVal;
                d /= 10000;
                return Number::New(isolate, d);
            }
            case VT_DECIMAL:
            {
                DECIMAL decVal;
                buff >> decVal;
                if (decVal.Hi32 || decVal.Lo64 > SAFE_DOUBLE)
                    return ToStr(isolate, SPA::ToString_long(decVal).c_str());
                return Number::New(isolate, ToDouble(decVal));
            }
            case VT_DATE:
            {
                SPA::UINT64 ullVal;
                buff >> ullVal;
                return ToDate(isolate, ullVal);
            }
            case (VT_I1 | VT_ARRAY):
            {
                unsigned int len;
                buff >> len;
                if (len == SPA::UQUEUE_NULL_LENGTH) {
                    return v8::Null(isolate);
                } else if (len > buff.GetSize()) {
                    throw SPA::CUException("Bad string data type");
                }
                const char* str = (const char*) buff.GetBuffer();
                auto s = ToStr(isolate, str, len);
                buff.Pop(len);
                return s;
            }
            case VT_CLSID:
            case (VT_UI1 | VT_ARRAY):
            {
                unsigned int len;
                buff >> len;
                if (len > buff.GetSize()) {
                    throw SPA::CUException("Bad data type");
                }
                const char* str = (const char*) buff.GetBuffer();
                auto bytes = node::Buffer::Copy(isolate, (const char*) str, len).ToLocalChecked();
                buff.Pop(len);
                return bytes;
            }
            case VT_BSTR:
            {
                unsigned int len;
                buff >> len;
                if (len == SPA::UQUEUE_NULL_LENGTH) {
                    return Null(isolate);
                } else if (len > buff.GetSize()) {
                    throw SPA::CUException("Bad string data type");
                }
                const UTF16* str = (const UTF16*) buff.GetBuffer();
                auto s = ToStr(isolate, str, len >> 1);
                buff.Pop(len);
                return s;
            }
            default:
                bool is_array = ((type & VT_ARRAY) == VT_ARRAY);
                if (is_array) {
                    type = (type & (~VT_ARRAY));
                    unsigned int count = buff.Load<unsigned int>();
                    auto ctx = isolate->GetCurrentContext();
                    switch (type) {
                        case VT_BOOL:
                        {
                            Local<Array> v = Array::New(isolate, (int) count);
                            VARIANT_BOOL* p = (VARIANT_BOOL*) buff.GetBuffer();
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, Boolean::New(isolate, (p[n] == VARIANT_FALSE) ? false : true));
                            }
                            buff.Pop((unsigned int) count * sizeof (VARIANT_BOOL));
                            return v;
                        }
                        case VT_I8:
                        {
                            SPA::INT64* p = (SPA::INT64*)buff.GetBuffer();
#ifndef HAS_BIGINT
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, Number::New(isolate, (double) (p[n])));
                            }
#else
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (SPA::INT64));
                            Local<v8::BigInt64Array> v = v8::BigInt64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (SPA::INT64));
#endif
                            buff.Pop((unsigned int) count * sizeof (SPA::INT64));
                            return v;
                        }
                        case VT_UI8:
                        {
                            SPA::UINT64* p = (SPA::UINT64*)buff.GetBuffer();
#ifndef HAS_BIGINT
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, Number::New(isolate, (double) (p[n])));
                            }
#else
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (SPA::UINT64));
                            Local<v8::BigUint64Array> v = v8::BigUint64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (SPA::UINT64));
#endif
                            buff.Pop((unsigned int) count * sizeof (SPA::UINT64));
                            return v;
                        }
                        case VT_I2:
                        {
                            short* p = (short*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (short));
                            Local<v8::Int16Array> v = v8::Int16Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (short));
                            buff.Pop((unsigned int) count * sizeof (short));
                            return v;
                        }
                        case VT_UI2:
                        {
                            unsigned short* p = (unsigned short*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (unsigned short));
                            Local<v8::Uint16Array> v = v8::Uint16Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (unsigned short));
                            buff.Pop((unsigned int) count * sizeof (unsigned short));
                            return v;
                        }
                        case VT_I4:
                        case VT_INT:
                        {
                            int* p = (int*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (int));
                            Local<v8::Int32Array> v = v8::Int32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (int));
                            buff.Pop((unsigned int) count * sizeof (int));
                            return v;
                        }
                        case VT_UI4:
                        case VT_UINT:
                        {
                            unsigned int* p = (unsigned int*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (unsigned int));
                            Local<v8::Uint32Array> v = v8::Uint32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (unsigned int));
                            buff.Pop((unsigned int) count * sizeof (unsigned int));
                            return v;
                        }
                        case VT_BSTR:
                        {
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                CDBString str;
                                buff >> str;
                                v->Set(ctx, n, ToStr(isolate, str.c_str(), str.size()));
                            }
                            return v;
                        }
                        case VT_R4:
                        {
                            float* p = (float*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (float));
                            Local<v8::Float32Array> v = v8::Float32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (float));
                            buff.Pop((unsigned int) count * sizeof (float));
                            return v;
                        }
                        case VT_R8:
                        {
                            double* p = (double*) buff.GetBuffer();
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (double));
                            Local<v8::Float64Array> v = v8::Float64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, p, count * sizeof (double));
                            buff.Pop((unsigned int) count * sizeof (double));
                            return v;
                        }
                        case VT_DECIMAL:
                        {
                            DECIMAL* p = (DECIMAL*) buff.GetBuffer();
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                if (p[n].Hi32 || p[n].Lo64 > SAFE_DOUBLE)
                                    v->Set(ctx, n, ToStr(isolate, SPA::ToString_long(p[n]).c_str()));
                                else
                                    v->Set(ctx, n, Number::New(isolate, SPA::ToDouble(p[n])));
                            }
                            buff.Pop((unsigned int) count * sizeof (DECIMAL));
                            return v;
                        }
                        case VT_DATE:
                        {
                            SPA::UINT64* p = (SPA::UINT64*)buff.GetBuffer();
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, ToDate(isolate, p[n]));
                            }
                            buff.Pop((unsigned int) count * sizeof (SPA::UINT64));
                            return v;
                        }
                        case VT_CY:
                        {
                            SPA::INT64* p = (SPA::INT64*)buff.GetBuffer();
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, Number::New(isolate, ((double) p[n]) / 10000));
                            }
                            buff.Pop((unsigned int) count * sizeof (SPA::INT64));
                            return v;
                        }
                        case VT_VARIANT:
                        {
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                v->Set(ctx, n, DbFrom(isolate, buff));
                            }
                            return v;
                        }
                        default:
                            break;
                    }
                }
                break;
        }
        return Undefined(isolate);
    }

    Local<Value> From(Isolate* isolate, const VARIANT& vt) {
        VARTYPE type = vt.vt;
        switch (type) {
            case VT_NULL:
            case VT_EMPTY:
                return Null(isolate);
            case VT_BOOL:
                return Boolean::New(isolate, vt.boolVal ? true : false);
            case VT_I1:
                return Int32::New(isolate, vt.cVal);
            case VT_I2:
                return Int32::New(isolate, vt.iVal);
            case VT_INT:
                return Int32::New(isolate, vt.intVal);
            case VT_I4:
                return Int32::New(isolate, vt.lVal);
            case VT_I8:
                return Number::New(isolate, (double) vt.llVal);
            case VT_UI1:
                return Uint32::New(isolate, vt.bVal);
            case VT_UI2:
                return Uint32::New(isolate, vt.uiVal);
            case VT_UINT:
                return Uint32::New(isolate, vt.uintVal);
            case VT_UI4:
                return Uint32::New(isolate, vt.ulVal);
            case VT_UI8:
                return Number::New(isolate, (double) vt.ullVal);
            case VT_R4:
                return Number::New(isolate, vt.fltVal);
            case VT_R8:
                return Number::New(isolate, vt.dblVal);
            case VT_CY:
            {
                double d = (double) vt.llVal;
                d /= 10000;
                return Number::New(isolate, d);
            }
            case VT_DECIMAL:
                if (vt.decVal.Hi32 || vt.decVal.Lo64 > SAFE_DOUBLE)
                    return ToStr(isolate, SPA::ToString(vt.decVal).c_str());
                return Number::New(isolate, ToDouble(vt.decVal));
            case VT_DATE:
                return ToDate(isolate, vt.ullVal);
            case (VT_I1 | VT_ARRAY):
            {
                const char* str = nullptr;
                unsigned int len = vt.parray->rgsabound->cElements;
                ::SafeArrayAccessData(vt.parray, (void**) &str);
                auto s = ToStr(isolate, str, len);
                ::SafeArrayUnaccessData(vt.parray);
                return s;
            }
            case VT_CLSID:
            case (VT_UI1 | VT_ARRAY):
            {
                char* str = nullptr;
                unsigned int len = vt.parray->rgsabound->cElements;
                ::SafeArrayAccessData(vt.parray, (void**) &str);
                auto bytes = node::Buffer::Copy(isolate, (const char*) str, len).ToLocalChecked();
                ::SafeArrayUnaccessData(vt.parray);
                return bytes;
            }
            case VT_BSTR:
                return ToStr(isolate, (const UTF16*) vt.bstrVal, SysStringLen(vt.bstrVal));
            default:
            {
                bool is_array = ((type & VT_ARRAY) == VT_ARRAY);
                if (is_array) {
                    void* pvt;
                    unsigned int count = vt.parray->rgsabound->cElements;
                    ::SafeArrayAccessData(vt.parray, &pvt);
                    type = (type & (~VT_ARRAY));
                    switch (type) {
                        case VT_BOOL:
                        case VT_BSTR:
                        case VT_DATE:
#ifndef HAS_BIGINT
                        case VT_I8:
                        case VT_UI8:
#endif
                        case VT_CY:
                        case VT_DECIMAL:
                        case VT_VARIANT:
                        {
                            auto ctx = isolate->GetCurrentContext();
                            Local<Array> v = Array::New(isolate, (int) count);
                            for (unsigned int n = 0; n < count; ++n) {
                                switch (type) {
                                    case VT_BOOL:
                                    {
                                        VARIANT_BOOL* p = (VARIANT_BOOL*) pvt;
                                        v->Set(ctx, n, Boolean::New(isolate, (p[n] == VARIANT_FALSE) ? false : true));
                                    }
                                        break;
                                    case VT_UI8:
                                    {
                                        SPA::UINT64* p = (SPA::UINT64*)pvt;
                                        v->Set(ctx, n, Number::New(isolate, (double) (p[n])));
                                    }
                                        break;
                                    case VT_I8:
                                    {
                                        SPA::INT64* p = (SPA::INT64*)pvt;
                                        v->Set(ctx, n, Number::New(isolate, (double) (p[n])));
                                    }
                                        break;
                                    case VT_CY:
                                    {
                                        SPA::INT64* p = (SPA::INT64*)pvt;
                                        v->Set(ctx, n, Number::New(isolate, ((double) p[n]) / 10000));
                                    }
                                        break;
                                    case VT_DECIMAL:
                                    {
                                        DECIMAL* p = (DECIMAL*) pvt;
                                        if (p->Hi32 || p->Lo64 > SAFE_DOUBLE)
                                            v->Set(ctx, n, ToStr(isolate, SPA::ToString_long(p[n]).c_str()));
                                        else
                                            v->Set(ctx, n, Number::New(isolate, SPA::ToDouble(p[n])));
                                    }
                                        break;
                                    case VT_BSTR:
                                    {
                                        BSTR* p = (BSTR*) pvt;
                                        if (p[n]) {
                                            auto s = ToStr(isolate, (const UTF16*) p[n], SysStringLen(p[n]));
                                            v->Set(ctx, n, s);
                                        } else
                                            v->Set(ctx, n, Null(isolate));
                                    }
                                        break;
                                    case VT_DATE:
                                    {
                                        SPA::UINT64* p = (SPA::UINT64*)pvt;
                                        v->Set(ctx, n, ToDate(isolate, p[n]));
                                    }
                                        break;
                                    case VT_VARIANT:
                                    {
                                        Local<Array> v = Array::New(isolate, (int) count);
                                        for (unsigned int n = 0; n < count; ++n) {
                                            VARIANT* p = (VARIANT*) pvt;
                                            v->Set(ctx, n, From(isolate, p[n]));
                                        }
                                        ::SafeArrayUnaccessData(vt.parray);
                                        return v;
                                    }
                                    default:
                                        assert(false); //shouldn't come here
                                        break;
                                }
                            }
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_I4:
                        case VT_INT:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (int));
                            Local<v8::Int32Array> v = v8::Int32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (int));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_UI4:
                        case VT_UINT:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (unsigned int));
                            Local<v8::Uint32Array> v = v8::Uint32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (unsigned int));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_I2:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (short));
                            Local<v8::Int16Array> v = v8::Int16Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (short));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_UI2:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (unsigned short));
                            Local<v8::Uint16Array> v = v8::Uint16Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (unsigned short));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_R4:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (float));
                            Local<v8::Float32Array> v = v8::Float32Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (float));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_R8:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (double));
                            Local<v8::Float64Array> v = v8::Float64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (double));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
#ifdef HAS_BIGINT
                        case VT_I8:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (SPA::INT64));
                            Local<v8::BigInt64Array> v = v8::BigInt64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (SPA::INT64));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
                        case VT_UI8:
                        {
                            Local<v8::ArrayBuffer> buf = v8::ArrayBuffer::New(isolate, count * sizeof (SPA::UINT64));
                            Local<v8::BigUint64Array> v = v8::BigUint64Array::New(buf, 0, count);
                            char* bytes = (char*) v->Buffer()->GetContents().Data();
                            memcpy(bytes, pvt, count * sizeof (SPA::UINT64));
                            ::SafeArrayUnaccessData(vt.parray);
                            return v;
                        }
#endif
                        default:
                            break;
                    }
                    ::SafeArrayUnaccessData(vt.parray);
                }
            }
                break;
        }
        return Undefined(isolate);
    }

    template<typename T>
    void ToArray(const T* p, unsigned int len, CDBVariantArray& v) {
        for (unsigned int n = 0; n < len; ++n) {
            v.push_back(p[n]);
        }
    }

    bool ToArray(Isolate* isolate, const Local<Value>& data, CDBVariantArray& v) {
        if (data->IsInt32Array()) {
            Local<v8::Int32Array> vInt = Local<v8::Int32Array>::Cast(data);
            const int* p = (const int*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsFloat64Array()) {
            Local<v8::Float64Array> vDouble = Local<v8::Float64Array>::Cast(data);
            const double* p = (const double*) vDouble->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vDouble->Length();
            ToArray(p, len, v);
        } else if (data->IsFloat32Array()) {
            Local<v8::Float32Array> vDouble = Local<v8::Float32Array>::Cast(data);
            const float* p = (const float*) vDouble->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vDouble->Length();
            ToArray(p, len, v);
        } else if (data->IsInt16Array()) {
            Local<v8::Int16Array> vInt = Local<v8::Int16Array>::Cast(data);
            const short* p = (const short*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsInt8Array()) {
            Local<v8::Int8Array> vInt = Local<v8::Int8Array>::Cast(data);
            const char* p = (const char*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsUint8Array()) {
            Local<v8::Uint8Array> vInt = Local<v8::Uint8Array>::Cast(data);
            const unsigned char* p = (const unsigned char*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsUint32Array()) {
            Local<v8::Uint32Array> vInt = Local<v8::Uint32Array>::Cast(data);
            const unsigned int* p = (const unsigned int*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
#ifdef HAS_BIGINT
        } else if (data->IsBigInt64Array()) {
            int64_t* p = (int64_t*) node::Buffer::Data(data);
            Local<v8::BigInt64Array> vInt = Local<v8::BigInt64Array>::Cast(data);
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsBigUint64Array()) {
            uint64_t* p = (uint64_t*) node::Buffer::Data(data);
            Local<v8::BigUint64Array> vInt = Local<v8::BigUint64Array>::Cast(data);
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
#endif
        } else if (data->IsUint16Array()) {
            Local<v8::Uint16Array> vInt = Local<v8::Uint16Array>::Cast(data);
            const unsigned short* p = (const unsigned short*) vInt->Buffer()->GetContents().Data();
            unsigned int len = (unsigned int) vInt->Length();
            ToArray(p, len, v);
        } else if (data->IsArray()) {
            bool ok = true;
            auto ctx = isolate->GetCurrentContext();
            tagDataType dt = tagDataType::dtUnknown;
            Local<Array> jsArr = Local<Array>::Cast(data);
            unsigned int count = jsArr->Length();
            for (unsigned int n = 0; n < count && ok; ++n) {
                auto d = jsArr->Get(ctx, n).ToLocalChecked();
                if (d->IsBoolean()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtBool)
                        ok = false;
                    else
                        dt = tagDataType::dtBool;
                } else if (d->IsDate()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtDate)
                        ok = false;
                    else
                        dt = tagDataType::dtDate;
                } else if (d->IsString()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtString)
                        ok = false;
                    else
                        dt = tagDataType::dtString;
#ifdef HAS_BIGINT
                } else if (d->IsBigInt()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtInt64)
                        ok = false;
                    else
                        dt = tagDataType::dtInt64;
#endif
                } else if (d->IsInt32()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtInt32)
                        ok = false;
                    else
                        dt = tagDataType::dtInt32;
                } else if (d->IsNumber()) {
                    if (dt != tagDataType::dtUnknown && dt != tagDataType::dtDouble)
                        ok = false;
                    else
                        dt = tagDataType::dtDouble;
                } else {
                    ok = false;
                }
            }
            if (!ok) {
                ThrowException(isolate, UNSUPPORTED_ARRAY_TYPE);
                return false;
            }
            for (unsigned int n = 0; n < count; ++n) {
                auto d = jsArr->Get(ctx, n).ToLocalChecked();
                switch (dt) {
                    case tagDataType::dtString:
                        v.push_back(ToStr(isolate, d).c_str());
                        break;
                    case tagDataType::dtBool:
#ifdef BOOL_ISOLATE
                        v.push_back(d->BooleanValue(isolate));
#else
                        v.push_back(d->BooleanValue(isolate->GetCurrentContext()).ToChecked());
#endif
                        break;
                    case tagDataType::dtDate:
                    {
                        CDBVariant vt;
                        vt.ullVal = ToDate(isolate, d);
                        vt.vt = VT_DATE;
                        v.push_back(vt);
                    }
                        break;
#ifdef HAS_BIGINT
                    case tagDataType::dtInt64:
                        v.push_back(d->IntegerValue(isolate->GetCurrentContext()).ToChecked());
                        break;
#endif
                    case tagDataType::dtInt32:
                        v.push_back(d->Int32Value(isolate->GetCurrentContext()).ToChecked());
                        break;
                    case tagDataType::dtDouble:
                        v.push_back(d->NumberValue(isolate->GetCurrentContext()).ToChecked());
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        } else {
            ThrowException(isolate, UNSUPPORTED_ARRAY_TYPE);
            return false;
        }
        return true;
    }

    bool ToPInfoArray(Isolate* isolate, const Local<Value>& p0, CParameterInfoArray& vInfo) {
        vInfo.clear();
        if (p0->IsArray()) {
            auto ctx = isolate->GetCurrentContext();
            Local<Array> jsArr = Local<Array>::Cast(p0);
            unsigned int count = jsArr->Length();
            for (unsigned int n = 0; n < count; ++n) {
                auto jsP = jsArr->Get(ctx, n).ToLocalChecked();
                if (!jsP->IsObject()) {
                    ThrowException(isolate, "Invalid parameter meta found");
                    return false;
                }
                auto pi = jsP->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
                CParameterInfo pInfo;
                auto v = pi->Get(ctx, ToStr(isolate, u"Direction", 9)).ToLocalChecked();
                if (v->IsInt32()) {
                    int d = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
                    if (d < (int) tagParameterDirection::pdInput || d > (int) tagParameterDirection::pdReturnValue) {
                        ThrowException(isolate, "Bad parameter direction value found");
                        return false;
                    }
                    pInfo.Direction = (tagParameterDirection) d;
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter direction");
                    return false;
                }
                v = pi->Get(ctx, ToStr(isolate, u"DataType", 8)).ToLocalChecked();
                if (v->IsUint32()) {
                    unsigned int d = v->Uint32Value(isolate->GetCurrentContext()).ToChecked();
                    pInfo.DataType = (VARTYPE) d;
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter data type");
                    return false;
                }
                v = pi->Get(ctx, ToStr(isolate, u"ColumnSize", 10)).ToLocalChecked();
                if (v->IsInt32()) {
                    int d = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
                    /*
                    if (d < -1) {
                            ThrowException(isolate, "Bad parameter column size value found");
                            return false;
                    }
                     */
                    pInfo.ColumnSize = (unsigned int) d;
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter column size");
                    return false;
                }
                v = pi->Get(ctx, ToStr(isolate, u"Precision", 9)).ToLocalChecked();
                if (v->IsInt32()) {
                    int d = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
                    if (d < 0 || d > 64) {
                        ThrowException(isolate, "Bad parameter precision value found");
                        return false;
                    }
                    pInfo.Precision = (unsigned char) d;
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter data precision");
                    return false;
                }
                v = pi->Get(ctx, ToStr(isolate, u"Scale", 5)).ToLocalChecked();
                if (v->IsInt32()) {
                    int d = v->Int32Value(isolate->GetCurrentContext()).ToChecked();
                    if (d < 0 || d > 64) {
                        ThrowException(isolate, "Bad parameter scale value found");
                        return false;
                    }
                    pInfo.Precision = (unsigned char) d;
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter data scale");
                    return false;
                }
                v = pi->Get(ctx, ToStr(isolate, u"ParameterName", 13)).ToLocalChecked();
                if (v->IsString()) {
                    pInfo.ParameterName = ToStr(isolate, v);
                } else if (!IsNullOrUndefined(v)) {
                    ThrowException(isolate, "An integer value expected for parameter data scale");
                    return false;
                }
            }
        } else if (!IsNullOrUndefined(p0)) {
            ThrowException(isolate, "An array of parameter meta data expected");
            return false;
        }
        return true;
    }

    Local<Array> ToMeta(Isolate* isolate, const CDBColumnInfoArray& v) {
        Local<Value> vN[] = {
            Local<String>::New(isolate, DBPath),
            Local<String>::New(isolate, TablePath),
            Local<String>::New(isolate, DisplayName),
            Local<String>::New(isolate, OriginalName),
            Local<String>::New(isolate, DeclaredType),
            Local<String>::New(isolate, Collation),
            Local<String>::New(isolate, ColumnSize),
            Local<String>::New(isolate, Flags),
            Local<String>::New(isolate, DataType),
            Local<String>::New(isolate, Precision),
            Local<String>::New(isolate, Scale)
        };
        auto ctx = isolate->GetCurrentContext();
        Local<Array> jsMeta = Array::New(isolate, (int) v.size());
        unsigned int index = 0;
        for (auto it = v.begin(), end = v.end(); it != end; ++it, ++index) {
            Local<Object> meta = Object::New(isolate);
            meta->Set(ctx, vN[0], ToStr(isolate, it->DBPath.c_str(), it->DBPath.size()));
            meta->Set(ctx, vN[1], ToStr(isolate, it->TablePath.c_str(), it->TablePath.size()));
            meta->Set(ctx, vN[2], ToStr(isolate, it->DisplayName.c_str(), it->DisplayName.size()));
            meta->Set(ctx, vN[3], ToStr(isolate, it->OriginalName.c_str(), it->OriginalName.size()));
            meta->Set(ctx, vN[4], ToStr(isolate, it->DeclaredType.c_str(), it->DeclaredType.size()));
            meta->Set(ctx, vN[5], ToStr(isolate, it->Collation.c_str(), it->Collation.size()));
            meta->Set(ctx, vN[6], Uint32::New(isolate, it->ColumnSize));
            meta->Set(ctx, vN[7], Uint32::New(isolate, it->Flags));
            meta->Set(ctx, vN[8], Uint32::New(isolate, it->DataType));
            meta->Set(ctx, vN[9], Uint32::New(isolate, it->Precision));
            meta->Set(ctx, vN[10], Uint32::New(isolate, it->Scale));
            jsMeta->Set(ctx, index, meta);
        }
        return jsMeta;
    }

    Local<Array> ToMeta(Isolate* isolate, const SPA::CKeyMap& mapkey) {
        Local<Value> vN[] = {
            Local<String>::New(isolate, DBPath),
            Local<String>::New(isolate, TablePath),
            Local<String>::New(isolate, DisplayName),
            Local<String>::New(isolate, OriginalName),
            Local<String>::New(isolate, DeclaredType),
            Local<String>::New(isolate, Collation),
            Local<String>::New(isolate, ColumnSize),
            Local<String>::New(isolate, Flags),
            Local<String>::New(isolate, DataType),
            Local<String>::New(isolate, Precision),
            Local<String>::New(isolate, Scale)
        };
        auto ctx = isolate->GetCurrentContext();
        Local<Array> jsMeta = Array::New(isolate, (int) mapkey.size());
        unsigned int index = 0;
        for (auto it = mapkey.begin(), end = mapkey.end(); it != end; ++it, ++index) {
            Local<Object> p = Object::New(isolate);
            p->Set(ctx, ToStr(isolate, u"Ordinal", 7), Number::New(isolate, it->first));
            Local<Object> meta = Object::New(isolate);
            meta->Set(ctx, vN[0], ToStr(isolate, it->second.DBPath.c_str(), it->second.DBPath.size()));
            meta->Set(ctx, vN[1], ToStr(isolate, it->second.TablePath.c_str(), it->second.TablePath.size()));
            meta->Set(ctx, vN[2], ToStr(isolate, it->second.DisplayName.c_str(), it->second.DisplayName.size()));
            meta->Set(ctx, vN[3], ToStr(isolate, it->second.OriginalName.c_str(), it->second.OriginalName.size()));
            meta->Set(ctx, vN[4], ToStr(isolate, it->second.DeclaredType.c_str(), it->second.DeclaredType.size()));
            meta->Set(ctx, vN[5], ToStr(isolate, it->second.Collation.c_str(), it->second.Collation.size()));
            meta->Set(ctx, vN[6], Uint32::New(isolate, it->second.ColumnSize));
            meta->Set(ctx, vN[7], Uint32::New(isolate, it->second.Flags));
            meta->Set(ctx, vN[8], Uint32::New(isolate, it->second.DataType));
            meta->Set(ctx, vN[9], Uint32::New(isolate, it->second.Precision));
            meta->Set(ctx, vN[10], Uint32::New(isolate, it->second.Scale));
            p->Set(ctx, ToStr(isolate, u"Field", 5), meta);
            jsMeta->Set(ctx, index, p);
        }
        return jsMeta;
    }

    SPA::CUCriticalSection g_cs;
    std::vector<std::string> g_KeyAllowed;
    const char* UNSUPPORTED_TYPE = "Unsupported data type";
    const char* UNSUPPORTED_ARRAY_TYPE = "Unsupported data array type";
    const char* BOOLEAN_EXPECTED = "A boolean value expected";
    const char* BAD_DATA_TYPE = "Bad data type";
    const char* INTEGER_EXPECTED = "An integer value expected";
}
