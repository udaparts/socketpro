
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
            auto ctx = isolate->GetCurrentContext();
            {
                SPA::CAutoLock al(obj->m_csDB);
                while (obj->m_deqDBCb.size()) {
                    DBCb &cb = obj->m_deqDBCb.front();
                    PAsyncDBHandler processor = nullptr;
                    *cb.Buffer >> processor;
                    assert(processor);
                    Local<Function> func;
                    assert(cb.Func);
                    if (cb.Func)
                        func = Local<Function>::New(isolate, *cb.Func);
                    switch (cb.Type) {
                        case eBatchHeader:
                        {
                            assert(!cb.Buffer->GetSize());
                            func->Call(ctx, Null(isolate), 0, nullptr);
                        }
                            break;
                        case eRows:
                            if (!func.IsEmpty()) {
                                bool bProc;
                                *cb.Buffer >> bProc;
                                assert(!cb.Buffer->GetSize());
                                Local<Array> arr;
                                if (cb.VData) {
                                    SPA::CUQueue &buff = *cb.VData;
                                    try {
                                        std::vector<Local < Value>> v;
                                        while (buff.GetSize()) {
                                            v.push_back(DbFrom(isolate, buff));
                                        }
                                        arr = Array::New(isolate, &v.front(), v.size());
                                    } catch (SPA::CUException&) {
                                        buff.SetSize(0);
                                    }
                                }
                                Local<Value> argv[] = {arr, Boolean::New(isolate, bProc)};
                                func->Call(ctx, Null(isolate), 2, argv);
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
                                Local<Value> argv[] = {njRes, njMsg, njAffected, njOks, njFails, njId};
                                func->Call(ctx, Null(isolate), 6, argv);
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
                                Local<Value> argv[] = {njRes, njMsg};
                                func->Call(ctx, Null(isolate), 2, argv);
                            }
                            break;
                        case eRowsetHeader:
                            if (!func.IsEmpty()) {
                                CDBColumnInfoArray v;
                                *cb.Buffer >> v;
                                assert(!cb.Buffer->GetSize());
                                Local<Array> jsMeta = ToMeta(isolate, v);
                                Local<Value> argv[] = {jsMeta};
                                func->Call(ctx, Null(isolate), 1, argv);
                            }
                            break;
                        case eDiscarded:
                            if (!func.IsEmpty()) {
                                bool canceled;
                                *cb.Buffer >> canceled;
                                assert(!cb.Buffer->GetSize());
                                auto b = Boolean::New(isolate, canceled);
                                Local<Value> argv[] = {b};
                                func->Call(ctx, Null(isolate), 1, argv);
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
