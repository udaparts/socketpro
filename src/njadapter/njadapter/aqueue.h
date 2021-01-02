#pragma once

#include "../../../include/aqhandler.h"

namespace NJA {

    class CAQueue : public SPA::ClientSide::CAsyncQueue {
    public:
        CAQueue(SPA::ClientSide::CClientSocket *cs);
        CAQueue(const CAQueue &aq) = delete;
        ~CAQueue();

        typedef CAQueue* PAQueue;

    public:
        CAQueue& operator=(const CAQueue &aq) = delete;
        SPA::UINT64 GetKeys(Isolate* isolate, int args, Local<Value> *argv);
        SPA::UINT64 StartQueueTrans(Isolate* isolate, int args, Local<Value> *argv, const char *key);
        SPA::UINT64 EndQueueTrans(Isolate* isolate, int args, Local<Value> *argv, bool rollback);
        SPA::UINT64 CloseQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, bool permanent);
        SPA::UINT64 FlushQueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, SPA::tagOptimistic option);
        SPA::UINT64 Dequeue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned int timeout);
        SPA::UINT64 Enqueue(Isolate* isolate, int args, Local<Value> *argv, const char *key, unsigned short idMessage, const unsigned char *pBuffer, unsigned int size);
        SPA::UINT64 EnqueueBatch(Isolate* isolate, int args, Local<Value> *argv, const char *key, const unsigned char *pBuffer, unsigned int size);
        void SetRR(Isolate* isolate, Local<Value> rr);

    protected:
        virtual void OnResultReturned(unsigned short reqId, CUQueue &mc);

    private:
        static void queue_cb(uv_async_t* handle);
        DDiscarded Get(Isolate* isolate, Local<Value> abort, bool &bad);
        DServerException GetSE(Isolate* isolate, Local<Value> se, bool& bad);

    private:

        enum class tagQueueEvent {
            qeGetKeys = 0,
            qeEnqueue,
            qeStartQueueTrans,
            qeEndQueueTrans,
            qeCloseQueue,
            qeFlushQueue,
            qeDequeue,
            qeEnqueueBatch,
            qeResultReturned,
            qeDiscarded,
            qeException
        };

        struct QueueCb {
            tagQueueEvent EventType;
            SPA::PUQueue Buffer;
            std::shared_ptr<CNJFunc> Func;

            QueueCb(tagQueueEvent et) : EventType(et), Buffer(SPA::CScopeUQueue::Lock()) {
            }

            QueueCb(QueueCb&& qcb) noexcept : EventType(qcb.EventType), Buffer(qcb.Buffer), Func(std::move(qcb.Func)) {
                qcb.Buffer = nullptr;
            }

            QueueCb(const QueueCb& qcb) = delete;
            QueueCb& operator=(const QueueCb& qcb) = delete;
            QueueCb& operator=(QueueCb&& qcb) = delete;
        };

        typedef SPA::CSpinAutoLock CAutoLock;
        SPA::CSpinLock m_csJQ;
        uv_async_t m_qType;
        std::deque<QueueCb> m_deqQCb; //Protected by m_csQ;
        std::shared_ptr<CNJFunc> m_rr; //OnResultReturned protected by m_csQ
    };
}
