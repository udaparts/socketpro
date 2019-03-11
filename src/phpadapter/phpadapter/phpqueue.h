#ifndef SPA_PHP_ASYNC_QUEUE_HANDER_H
#define SPA_PHP_ASYNC_QUEUE_HANDER_H

#include "basehandler.h"
#include "phpbuffer.h"

namespace PA {

    typedef SPA::ClientSide::CAsyncQueue CAsyncQueue;
    typedef SPA::ClientSide::CSocketPool<CAsyncQueue> CPhpQueuePool;

    class CPhpQueue : public CPhpBaseHandler {
    public:
        CPhpQueue(unsigned int poolId, CAsyncQueue *aq, bool locked);
        CPhpQueue(const CPhpQueue &q) = delete;
#ifdef USE_DEQUEUE
        ~CPhpQueue();
#endif
    public:
        CPhpQueue& operator=(const CPhpQueue &q) = delete;
        static void RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs);
        Php::Value __get(const Php::Value &name);
#ifdef USE_DEQUEUE
        void __set(const Php::Value &name, const Php::Value &value);
#endif
    protected:
        void PopTopCallbacks(PACallback &cb);

    private:
        Php::Value CloseQueue(Php::Parameters &params);
        Php::Value GetKeys(Php::Parameters &params);
        Php::Value StartQueueTrans(Php::Parameters &params);
        Php::Value EndQueueTrans(Php::Parameters &params);
        Php::Value FlushQueue(Php::Parameters &params);
#ifdef USE_DEQUEUE
        Php::Value Dequeue(Php::Parameters &params);
#endif
        Php::Value Enqueue(Php::Parameters &params);
        void BatchMessage(Php::Parameters &params);
        Php::Value EnqueueBatch(Php::Parameters &params);

        CAsyncQueue::DEnqueue SetEnqueueResCallback(unsigned short reqId, const Php::Value& phpDl, std::shared_ptr<SPA::INT64> &pV, unsigned int &timeout);
        std::string GetKey(const Php::Value &v);
        CAsyncQueue::DQueueTrans SetQueueTransCallback(unsigned short reqId, const Php::Value& phpTrans, std::shared_ptr<int> &pV, unsigned int &timeout);

        static Php::Value ToDeqValue(SPA::CUQueue *q);
        static Php::Value ToFlushValue(SPA::CUQueue *q);

    private:
        CAsyncQueue *m_aq;
        std::shared_ptr<CPhpBuffer> m_pBuff;
#ifdef USE_DEQUEUE
        Php::Value m_rr;
        SPA::CUQueue *m_qRR;
#endif
        static const char *PHP_QUEUE_KEY;
        static const char *PHP_QUEUE_MESSAGES;
        static const char *PHP_QUEUE_FILESIZE;
        static const char *PHP_QUEUE_MESSAGES_DEQUEUED;
        static const char *PHP_QUEUE_BYTES_DEQUEUED;
    };

} //namespace PA

#endif