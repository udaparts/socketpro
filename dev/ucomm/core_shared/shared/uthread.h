#ifndef _UMB_THREAD_BASE_H__
#define _UMB_THREAD_BASE_H__

#include "includes.h"
#include "../../include/membuffer.h"
#include "../../include/ucomm.h"

namespace SPA {

    struct CUThreadMessage {
        unsigned int m_nMsgId;
        SPA::CUQueue *m_pMessageBuffer;
        unsigned short m_uRequestId;

        CUThreadMessage() : m_nMsgId(0), m_pMessageBuffer(nullptr), m_uRequestId(0) {

        }

        CUThreadMessage(unsigned int nMsgId, SPA::CUQueue *pUQueue, unsigned short reqId)
        : m_nMsgId(nMsgId), m_pMessageBuffer(pUQueue), m_uRequestId(reqId) {

        }

        CUThreadMessage(const CUThreadMessage &msg)
        : m_nMsgId(msg.m_nMsgId), m_pMessageBuffer(msg.m_pMessageBuffer), m_uRequestId(msg.m_uRequestId) {
        }

        CUThreadMessage &operator=(const CUThreadMessage &msg) {
            if (this == &msg)
                return *this;
            m_nMsgId = msg.m_nMsgId;
            m_pMessageBuffer = msg.m_pMessageBuffer;
            m_uRequestId = msg.m_uRequestId;
            return *this;
        }
    };
#ifndef WINCE
	using mutex = std::mutex;
#else
	using mutex = boost::mutex;
#endif
    class CUCommThread {
    public:
        CUCommThread(tagThreadApartment ta);
        virtual ~CUCommThread();
		using thread = boost::thread;

    public:
        CErrorCode GetErrorCode();
        bool IsStarted();
        virtual bool IsBusy() = 0;
        virtual bool Kill();
        virtual bool Start();
        const thread* GetBoostThread() const;
        tagThreadApartment GetThreadApartment();

    protected:
        CIoService& GetIoService();
        virtual void OnThreadStarted() = 0;
        virtual void OnThreadEnded() = 0;

    private:
		CUCommThread(const CUCommThread &t);
		CUCommThread& operator=(const CUCommThread &t);
        void Call(const CErrorCode &ec);

    protected:
#ifndef WINCE
		typedef std::unique_lock<std::mutex> CAutoLock;
#else
		typedef boost::mutex::scoped_lock CAutoLock;
#endif
        CIoService m_io;
        CErrorCode m_ec;
        mutex m_mutex;
        std::queue<CUThreadMessage> m_qThreadMessage;
        tagThreadApartment m_ta;
        thread *m_pThread;

    private:
        CConditionVariable m_cv;
    };

};

#endif

