
#ifndef PHP_SPA_CLIENT_BASE_HANDLER_H
#define PHP_SPA_CLIENT_BASE_HANDLER_H

#include <atomic>

namespace PA {

    enum enumCallbackType {
        ctSendRequest = 0,
        ctDiscarded,
        ctServerException,
        ctFile,
        ctQueueTrans,
        ctEnqueueRes,
        ctQueueFlush,
        ctQueueGetKeys,
        ctDequeue,
        ctDequeueResult,
        ctDbRes,
        ctDbExeRes,
        ctDbR,
        ctDbRH
    };

    struct PACallback {
        enumCallbackType CallbackType;
        CPVPointer CallBack;
        SPA::CUQueue *Res;
    };

    enum tagRequestReturnStatus {
        rrsServerException = -3,
        rrsCanceled = -2,
        rrsTimeout = -1,
        rrsClosed = 0,
        rrsOk = 1,
    };

    class CPhpBaseHandler : public Php::Base {
    protected:
        CPhpBaseHandler(bool locked, SPA::ClientSide::CAsyncServiceHandler *h);
        CPhpBaseHandler(const CPhpBaseHandler& h) = delete;
        virtual ~CPhpBaseHandler();

    public:
        CPhpBaseHandler& operator=(const CPhpBaseHandler& h) = delete;
        Php::Value __get(const Php::Value &name);
        int __compare(const CPhpBaseHandler &pbh) const;
        unsigned int GetPoolId() const;
        void __destruct();

    public:
        static void RegisterInto(Php::Class<CPhpBaseHandler> &h, Php::Namespace &cs);

    private:
        Php::Value SendRequest(Php::Parameters &params);
        void __construct(Php::Parameters &params);
        Php::Value WaitAll(Php::Parameters &params);
        Php::Value StartBatching();
        Php::Value CommitBatching(Php::Parameters &params);
        Php::Value AbortBatching();
        Php::Value CleanCallbacks(Php::Parameters &params);
        Php::Value Interrupt(Php::Parameters &params);

    protected:
        virtual Php::Value Unlock();
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded SetAbortCallback(const Php::Value& phpCanceled, unsigned short reqId, bool sync);
        void ReqSyncEnd(bool ok, std::unique_lock<std::mutex> &lk, unsigned int timeout);
        virtual void PopTopCallbacks(PACallback &cb) = 0;
        void PopCallbacks();
        tagRequestReturnStatus GetRRS() const;

    protected:
        std::mutex m_mPhp;
        std::condition_variable m_cvPhp;
        std::deque<PACallback> m_vCallback;

    private:
        bool m_locked;
        SPA::ClientSide::CAsyncServiceHandler *m_h;
        std::atomic<tagRequestReturnStatus> m_rrs;
    };
}

#endif
