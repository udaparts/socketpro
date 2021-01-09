#pragma once

#include "../../../include/udb_client.h"
#include <node_version.h>

namespace SPA {
    namespace ClientSide {
        using namespace NJA;

        Local<v8::Object> CreateDb(Isolate* isolate, CAsyncServiceHandler *ash);

        static Local<Array> ToMeta(Isolate* isolate, CUQueue &v) {
            int length;
            Local<v8::Value> vN[] = {
                ToStr(isolate, u"DBPath", 6),
                ToStr(isolate, u"TablePath", 9),
                ToStr(isolate, u"DisplayName", 11),
                ToStr(isolate, u"OriginalName", 12),
                ToStr(isolate, u"DeclaredType", 12),
                ToStr(isolate, u"Collation", 9),
                ToStr(isolate, u"ColumnSize", 10),
                ToStr(isolate, u"Flags", 5),
                ToStr(isolate, u"DataType", 8),
                ToStr(isolate, u"Precision", 9),
                ToStr(isolate, u"Scale", 5)
            };
            auto ctx = isolate->GetCurrentContext();
            v >> length;
            Local<Array> jsMeta = Array::New(isolate, length);
            unsigned int index = 0;
            while (v.GetSize()) {
                unsigned int bytes;
                Local<Object> meta = Object::New(isolate);
                v >> bytes;
                const SPA::UTF16 *str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[0], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[1], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[2], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[3], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[4], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                str = (const SPA::UTF16 *)v.GetBuffer();
                meta->Set(ctx, vN[5], ToStr(isolate, str, bytes >> 1));
                v.Pop(bytes);
                v >> bytes;
                meta->Set(ctx, vN[6], Uint32::New(isolate, bytes));
                v >> bytes;
                meta->Set(ctx, vN[7], Uint32::New(isolate, bytes));
                unsigned short DataType;
                v >> DataType;
                meta->Set(ctx, vN[8], Uint32::New(isolate, DataType));
                unsigned char my_byte;
                v >> my_byte;
                meta->Set(ctx, vN[9], Uint32::New(isolate, my_byte));
                v >> my_byte;
                meta->Set(ctx, vN[10], Uint32::New(isolate, my_byte));
                jsMeta->Set(ctx, index, meta);
                ++index;
            }
            return jsMeta;
        }

        template<unsigned int serviceId>
        void CAsyncDBHandler<serviceId>::req_cb(uv_async_t* handle) {
            CAsyncDBHandler<serviceId>* obj = (CAsyncDBHandler<serviceId>*)handle->data; //sender
            assert(obj);
            if (!obj) return;
            Isolate* isolate = Isolate::GetCurrent();
            HandleScope handleScope(isolate); //required for Node 4.x
            {
                auto ctx = isolate->GetCurrentContext();
                Local<Value> null = Null(isolate);
                CSpinLock& cs = obj->m_csDB;
                cs.lock();
                while (obj->m_deqDBCb.size()) {
                    DBCb cb(std::move(obj->m_deqDBCb.front()));
                    obj->m_deqDBCb.pop_front();
                    cs.unlock();
                    PAsyncDBHandler processor;
                    *cb.Buffer >> processor;
                    assert(processor);
                    Local<Function> func;
                    assert(cb.Func);
                    if (cb.Func)
                        func = Local<Function>::New(isolate, *cb.Func);
                    switch (cb.Type) {
                        case tagDBEvent::eBatchHeader:
                        {
                            assert(!cb.Buffer->GetSize());
                            func->Call(ctx, null, 0, nullptr);
                        }
                            break;
                        case tagDBEvent::eRows:
                            if (!func.IsEmpty()) {
                                bool bProc;
                                int cols;
                                *cb.Buffer >> bProc >> cols;
                                assert(!cb.Buffer->GetSize());
                                Local<Array> v = Array::New(isolate, (int) cols);
                                if (cb.VData) {
                                    unsigned int index = 0;
                                    SPA::CUQueue &buff = *cb.VData;
                                    while (buff.GetSize()) {
                                        try {
                                            v->Set(ctx, index, DbFrom(isolate, buff));
                                            ++index;
                                        } catch (SPA::CUException&) {
                                            buff.SetSize(0);
                                        }
                                    }
                                }
                                Local<Value> argv[] = {v, Boolean::New(isolate, bProc), Int32::New(isolate, cols)};
                                func->Call(ctx, null, 3, argv);
                            }
                            break;
                        case tagDBEvent::eExecuteResult:
                            if (!func.IsEmpty()) {
                                int res;
                                SPA::CDBString errMsg;
                                INT64 affected;
                                unsigned int fails, oks;
                                *cb.Buffer >> res >> errMsg >> affected >> fails >> oks;
                                auto njRes = Int32::New(isolate, res);
                                auto njMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                                auto njAffected = Number::New(isolate, (double) affected);
                                auto njFails = Number::New(isolate, fails);
                                auto njOks = Number::New(isolate, oks);
                                auto njId = DbFrom(isolate, *cb.Buffer);
                                assert(!cb.Buffer->GetSize());
                                Local<Value> argv[] = {njRes, njMsg, njAffected, njOks, njFails, njId};
                                func->Call(ctx, null, 6, argv);
                            }
                            break;
                        case tagDBEvent::eResult:
                            if (!func.IsEmpty()) {
                                int res;
                                SPA::CDBString errMsg;
                                *cb.Buffer >> res >> errMsg;
                                assert(!cb.Buffer->GetSize());
                                auto njRes = Int32::New(isolate, res);
                                auto njMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                                Local<Value> argv[] = {njRes, njMsg};
                                func->Call(ctx, null, 2, argv);
                            }
                            break;
                        case tagDBEvent::eRowsetHeader:
                            if (!func.IsEmpty()) {
                                Local<Array> jsMeta = ToMeta(isolate, *cb.Buffer);
                                assert(!cb.Buffer->GetSize());
                                Local<Value> argv[] = {jsMeta};
                                func->Call(ctx, null, 1, argv);
                            }
                            break;
                        case tagDBEvent::eDiscarded:
                            if (!func.IsEmpty()) {
                                bool canceled;
                                *cb.Buffer >> canceled;
                                assert(!cb.Buffer->GetSize());
                                auto b = Boolean::New(isolate, canceled);
                                Local<Value> argv[] = {b};
                                func->Call(ctx, null, 1, argv);
                            }
                            break;
                        case tagDBEvent::eException:
                            if (!func.IsEmpty()) {
                                unsigned short reqId;
                                SPA::CDBString errMsg;
                                std::string errWhere;
                                int errCode;
                                *cb.Buffer >> reqId >> errMsg >> errWhere >> errCode;
                                assert(!cb.Buffer->GetSize());
                                Local<String> jsMsg = ToStr(isolate, errMsg.c_str(), errMsg.size());
                                Local<String> jsWhere = ToStr(isolate, errWhere.c_str());
                                Local<Value> jsCode = Number::New(isolate, errCode);
                                Local<Value> argv[] = {jsCode, jsMsg, jsWhere, Number::New(isolate, reqId)};
                                func->Call(ctx, null, 4, argv);
                            }
                        default:
                            assert(false);
                            break;
                    }
                    CScopeUQueue::Unlock(cb.Buffer);
                    cs.lock();
                }
                cs.unlock();
            }
            isolate->RunMicrotasks(); //may speed up pumping
        }
    }
}
