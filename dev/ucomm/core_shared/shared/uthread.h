#ifndef _UMB_THREAD_BASE_H__
#define _UMB_THREAD_BASE_H__

#include "includes.h"
#include "../../include/definebase.h"
#ifdef WIN32_64
#include<atlbase.h>
#endif
#include "../../include/spvariant.h"
#include "../../include/membuffer.h"
#include "../../include/ucomm.h"

namespace SPA {

#ifndef WINCE
    using mutex = std::mutex;
#else
    using mutex = boost::mutex;
#endif

    class U_MODULE_HIDDEN CUCommThread {
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
        void Call();

    protected:
#ifndef WINCE
        typedef std::unique_lock<std::mutex> CAutoLock;
#else
        typedef boost::mutex::scoped_lock CAutoLock;
#endif
        CIoService m_io;
        CErrorCode m_ec;
        mutex m_mutex;
        tagThreadApartment m_ta;
        thread *m_pThread;

    private:
        CConditionVariable m_cv;
    };

};

#endif

