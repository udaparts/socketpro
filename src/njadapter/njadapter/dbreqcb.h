
#pragma once

#include "../../../include/udb_client.h"

namespace SPA {
    namespace ClientSide {
        using namespace NJA;

        Local<v8::Object> CreateDb(Isolate* isolate, CAsyncServiceHandler *ash);

        template<unsigned int serviceId>
        void CAsyncDBHandler<serviceId>::req_cb(uv_async_t* handle) {
            CAsyncDBHandler<serviceId>* obj = (CAsyncDBHandler<serviceId>*)handle->data; //sender
            assert(obj);
            if (!obj) return;
            Isolate* isolate = Isolate::GetCurrent();
            HandleScope handleScope(isolate); //required for Node 4.x
            {
                SPA::CAutoLock al(obj->m_csDB);
                while (obj->m_deqDBCb.size()) {
                    DBCb &cb = obj->m_deqDBCb.front();
                    PAsyncDBHandler processor = nullptr;
                    *cb.Buffer >> processor;
                    assert(processor);
                    Local<v8::Object> njDB = CreateDb(isolate, processor);
                    Local<Function> func;
                    assert(cb.Func);
                    if (cb.Func)
                        func = Local<Function>::New(isolate, *cb.Func);
                    switch (cb.Type) {
                        case eBatchHeader:
                        {
                            assert(!cb.Buffer->GetSize());
                            Local<Value> argv[] = {njDB};
                            func->Call(isolate->GetCurrentContext(), Null(isolate), 1, argv);
                        }
                            break;
                        case eRows:
                            if (!func.IsEmpty()) {
                                bool bProc;
                                *cb.Buffer >> bProc;
                                assert(!cb.Buffer->GetSize());
                                Local<Array> v = Array::New(isolate);
                                if (cb.VData) {
                                    unsigned int index = 0;
                                    const CDBVariantArray &vData = *cb.VData;
                                    for (auto it = vData.begin(), end = vData.end(); it != end; ++it, ++index) {
                                        auto d = From(isolate, *it);
                                        v->Set(index, d);
                                    }
                                }
                                Local<Value> argv[] = {v, Boolean::New(isolate, bProc), njDB};
                                func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                            }
                            break;
                        case eExecuteResult:
                            if (!func.IsEmpty()) {
                                int res;
                                std::wstring errMsg;
                                INT64 affected;
                                unsigned int fails, oks;
                                CDBVariant vtId;
                                *cb.Buffer >> res >> errMsg >> affected >> fails >> oks >> vtId;
                                assert(!cb.Buffer->GetSize());
                                auto njRes = Int32::New(isolate, res);
                                auto njMsg = ToStr(isolate, errMsg.c_str());
                                auto njAffected = Number::New(isolate, (double) affected);
                                auto njFails = Number::New(isolate, fails);
                                auto njOks = Number::New(isolate, oks);
                                auto njId = From(isolate, vtId);
                                Local<Value> argv[] = {njRes, njMsg, njAffected, njOks, njFails, njId, njDB};
                                func->Call(isolate->GetCurrentContext(), Null(isolate), 7, argv);
                            }
                            break;
                        case eResult:
                            if (!func.IsEmpty()) {
                                int res;
                                std::wstring errMsg;
                                *cb.Buffer >> res >> errMsg;
                                assert(!cb.Buffer->GetSize());
                                auto njRes = Int32::New(isolate, res);
                                auto njMsg = ToStr(isolate, errMsg.c_str());
                                Local<Value> argv[] = {njRes, njMsg, njDB};
                                func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                            }
                            break;
                        case eRowsetHeader:
                            if (!func.IsEmpty()) {
                                CDBColumnInfoArray v;
                                *cb.Buffer >> v;
                                assert(!cb.Buffer->GetSize());
                                Local<Array> jsMeta = ToMeta(isolate, v);
                                Local<Value> argv[] = {jsMeta, njDB};
                                func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                            }
                            break;
                        case eDiscarded:
                            if (!func.IsEmpty()) {
                                bool canceled;
                                *cb.Buffer >> canceled;
                                assert(!cb.Buffer->GetSize());
                                auto b = Boolean::New(isolate, canceled);
                                Local<Value> argv[] = {b, njDB};
                                func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                            }
                            break;
                        default:
                            assert(false);
                            break;
                    }
                    CScopeUQueue::Unlock(cb.Buffer);
                    obj->m_deqDBCb.pop_front();
                }
            }
            //isolate->RunMicrotasks();
        }
    }
}
