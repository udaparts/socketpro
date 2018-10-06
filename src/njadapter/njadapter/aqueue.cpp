
#include "stdafx.h"
#include "aqueue.h"
#include "njasyncqueue.h"
#include "njqueue.h"

namespace NJA {

    CAQueue::CAQueue(SPA::ClientSide::CClientSocket *cs)
    : CAsyncQueue(cs) {
        ::memset(&m_qType, 0, sizeof (m_qType));
        m_qType.data = this;
        int fail = uv_async_init(uv_default_loop(), &m_qType, queue_cb);
        assert(!fail);
    }

    CAQueue::~CAQueue() {
        SPA::CAutoLock al(m_csQ);
        uv_close((uv_handle_t*) & m_qType, nullptr);
    }

    SPA::ClientSide::CAsyncServiceHandler::DDiscarded CAQueue::Get(Isolate* isolate, Local<Value> abort, bool &bad) {
        bad = false;
        DDiscarded dd;
        if (abort->IsFunction()) {
            std::shared_ptr<CNJFunc> func(new CNJFunc);
            func->Reset(isolate, Local<Function>::Cast(abort));
            dd = [func](CAsyncServiceHandler *aq, bool canceled) {
                QueueCb qcb;
                qcb.EventType = qeDiscarded;
                qcb.Func = func;
                qcb.Buffer = CScopeUQueue::Lock();
                PAQueue ash = (PAQueue) aq;
                *qcb.Buffer << ash << canceled;
                CAutoLock al(ash->m_csQ);
                ash->m_deqQCb.push_back(qcb);
                int fail = uv_async_send(&ash->m_qType);
                assert(!fail);
            };
        } else if (!abort->IsNullOrUndefined()) {
            ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
            bad = true;
        }
        return dd;
    }

    void CAQueue::queue_cb(uv_async_t* handle) {
        CAQueue* obj = (CAQueue*) handle->data; //sender
        assert(obj);
        if (!obj) return;
        Isolate* isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate); //required for Node 4.x
        {
            SPA::CAutoLock al(obj->m_csQ);
            while (obj->m_deqQCb.size()) {
                QueueCb &cb = obj->m_deqQCb.front();
                PAQueue processor = nullptr;
                *cb.Buffer >> processor;
                assert(processor);
                Local<Function> func = Local<Function>::New(isolate, *cb.Func);
                Local<Object> njQ = NJAsyncQueue::New(isolate, processor, true);
                switch (cb.EventType) {
                    case qeDiscarded:
                    {
                        bool canceled;
                        *cb.Buffer >> canceled;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> argv[] = {v8::Boolean::New(isolate, canceled), njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                    }
                        break;
                    case qeGetKeys:
                    {
                        unsigned int size;
                        *cb.Buffer >> size;
                        unsigned int index = 0;
                        Local<Array> jsKeys = Array::New(isolate);
                        while (cb.Buffer->GetSize()) {
                            std::string s;
                            *cb.Buffer >> s;
                            auto str = ToStr(isolate, s.c_str());
                            bool ok = jsKeys->Set(index, str);
                            assert(ok);
                            ++index;
                        }
                        assert(index == size);
                        Local<Value> argv[] = {jsKeys, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                    }
                        break;
                    case qeEnqueueBatch:
                    case qeEnqueue:
                    {
                        SPA::UINT64 indexMessage;
                        *cb.Buffer >> indexMessage;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> im = Number::New(isolate, (double) indexMessage);
                        Local<Value> argv[] = {im, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                    }
                        break;
                    case qeCloseQueue:
                    case qeEndQueueTrans:
                    case qeStartQueueTrans:
                    {
                        int errCode;
                        *cb.Buffer >> errCode;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> jsCode = Int32::New(isolate, errCode);
                        Local<Value> argv[] = {jsCode, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 2, argv);
                    }
                        break;
                    case qeFlushQueue:
                    {
                        SPA::UINT64 messageCount, fileSize;
                        *cb.Buffer >> messageCount >> fileSize;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> mc = Number::New(isolate, (double) messageCount);
                        Local<Value> fs = Number::New(isolate, (double) fileSize);
                        Local<Value> argv[] = {mc, fs, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                    }
                        break;
                    case qeDequeue:
                    {
                        SPA::UINT64 messageCount, fileSize;
                        unsigned int messagesDequeuedInBatch, bytesDequeuedInBatch;
                        *cb.Buffer >> messageCount >> fileSize >> messagesDequeuedInBatch >> bytesDequeuedInBatch;
                        assert(!cb.Buffer->GetSize());
                        Local<Value> mc = Number::New(isolate, (double) messageCount);
                        Local<Value> fs = Number::New(isolate, (double) fileSize);
                        Local<Value> mdib = Uint32::New(isolate, messagesDequeuedInBatch);
                        Local<Value> bdib = Uint32::New(isolate, bytesDequeuedInBatch);
                        Local<Value> argv[] = {mc, fs, mdib, bdib, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 5, argv);
                    }
                        break;
                    case qeResultReturned:
                    {
                        unsigned short reqId;
                        *cb.Buffer >> reqId;
                        Local<Value> jsReqid = Uint32::New(isolate, reqId);
                        auto q = NJQueue::New(isolate, cb.Buffer);
                        Local<Value> argv[] = {jsReqid, q, njQ};
                        func->Call(isolate->GetCurrentContext(), Null(isolate), 3, argv);
                        auto obj = node::ObjectWrap::Unwrap<NJQueue>(q);
                        obj->Release();
                    }
                        break;
                    default:
                        assert(false); //shouldn't come here
                        break;
                }
                CScopeUQueue::Unlock(cb.Buffer);
                obj->m_deqQCb.pop_front();
            }
        }
        isolate->RunMicrotasks();
    }

    void CAQueue::SetRR(Isolate* isolate, Local<Value> rr) {
        if (rr->IsFunction()) {
            CAutoLock al(m_csQ);
            m_rr.reset(new CNJFunc);
            m_rr->Reset(isolate, Local<Function>::Cast(rr));
        } else if (rr->IsNullOrUndefined()) {
            CAutoLock al(m_csQ);
            m_rr.reset();
        }
    }

    void CAQueue::OnResultReturned(unsigned short reqId, CUQueue &mc) {
        if (reqId > Queue::idEnqueueBatch && reqId != Queue::idBatchSizeNotified) {
            CAutoLock al(m_csQ);
            if (m_rr) {
                QueueCb qcb;
                qcb.EventType = qeResultReturned;
                qcb.Buffer = CScopeUQueue::Lock();
                PAQueue ash = this;
                *qcb.Buffer << ash << reqId;
                qcb.Buffer->Push(mc.GetBuffer(), mc.GetSize());
                qcb.Func = m_rr;
                mc.SetSize(0);
                ash->m_deqQCb.push_back(qcb);
                int fail = uv_async_send(&ash->m_qType);
                assert(!fail);
            }
        } else {
            CAsyncQueue::OnResultReturned(reqId, mc);
        }
    }

    SPA::UINT64 CAQueue::GetKeys(Isolate* isolate, int args, Local<Value> *argv) {
        SPA::UINT64 index = GetCallIndex();
        DGetKeys gk;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                gk = [func](CAsyncQueue *aq, std::vector<std::string>& v) {
                    QueueCb qcb;
                    qcb.EventType = qeGetKeys;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << (unsigned int) v.size();
                    for (auto it = v.begin(), end = v.end(); it != end; ++it) {
                        *qcb.Buffer << *it;
                    }
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for GetKeys end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::GetKeys(gk, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::StartQueueTrans(Isolate* isolate, int args, Local<Value> *argv, const char *key) {
        SPA::UINT64 index = GetCallIndex();
        DQueueTrans qt;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                qt = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb;
                    qcb.EventType = qeStartQueueTrans;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for StartTrans end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::StartQueueTrans(key, qt, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::EndQueueTrans(Isolate* isolate, int args, Local<Value> *argv, bool rollback) {
        SPA::UINT64 index = GetCallIndex();
        DQueueTrans qt;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                qt = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb;
                    qcb.EventType = qeEndQueueTrans;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for EndTrans end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::EndQueueTrans(rollback, qt, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::CloseQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, bool permanent) {
        SPA::UINT64 index = GetCallIndex();
        DClose c;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                c = [func](CAsyncQueue *aq, int errCode) {
                    QueueCb qcb;
                    qcb.EventType = qeCloseQueue;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << errCode;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for Close end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::CloseQueue(key, c, permanent, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::FlushQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, tagOptimistic option) {
        SPA::UINT64 index = GetCallIndex();
        DFlush f;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                f = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize) {
                    QueueCb qcb;
                    qcb.EventType = qeFlushQueue;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << messageCount << fileSize;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for Flush end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::FlushQueue(key, f, option, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::Dequeue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned int timeout) {
        SPA::UINT64 index = GetCallIndex();
        DDequeue d;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                d = [func](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) {
                    QueueCb qcb;
                    qcb.EventType = qeDequeue;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << messageCount << fileSize << messagesDequeuedInBatch << bytesDequeuedInBatch;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for Dequeue end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::Dequeue(key, d, timeout, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::Enqueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned short idMessage, const unsigned char *pBuffer, unsigned int size) {
        if (idMessage <= Queue::idEnqueueBatch || idMessage == Queue::idBatchSizeNotified) {
            ThrowException(isolate, "Bad message request id");
            return 0;
        }
        SPA::UINT64 index = GetCallIndex();
        DEnqueue e;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
                    QueueCb qcb;
                    qcb.EventType = qeEnqueue;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << indexMessage;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for Enqueue end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::Enqueue(key, idMessage, pBuffer, size, e, dd) ? index : INVALID_NUMBER;
    }

    SPA::UINT64 CAQueue::EnqueueBatch(Isolate* isolate, int args, Local<Value> *argv, const char *key, const unsigned char *pBuffer, unsigned int size) {
        SPA::UINT64 index = GetCallIndex();
        DEnqueue e;
        DDiscarded dd;
        if (args > 0) {
            if (argv[0]->IsFunction()) {
                std::shared_ptr<CNJFunc> func(new CNJFunc);
                func->Reset(isolate, Local<Function>::Cast(argv[0]));
                e = [func](CAsyncQueue *aq, SPA::UINT64 indexMessage) {
                    QueueCb qcb;
                    qcb.EventType = qeEnqueueBatch;
                    qcb.Func = func;
                    qcb.Buffer = CScopeUQueue::Lock();
                    PAQueue ash = (PAQueue) aq;
                    *qcb.Buffer << ash << indexMessage;
                    CAutoLock al(ash->m_csQ);
                    ash->m_deqQCb.push_back(qcb);
                    int fail = uv_async_send(&ash->m_qType);
                    assert(!fail);
                };
            } else if (!argv[0]->IsNullOrUndefined()) {
                ThrowException(isolate, "A callback expected for EnqueueBatch end result");
                return 0;
            }
        }
        if (args > 1) {
            bool bad;
            dd = Get(isolate, argv[1], bad);
            if (bad)
                return 0;
        }
        return CAsyncQueue::EnqueueBatch(key, pBuffer, size, e, dd) ? index : INVALID_NUMBER;
    }
}
