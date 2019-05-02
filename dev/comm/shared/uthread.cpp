
#include "uthread.h"
#ifdef WIN32_64
#include <objbase.h>
#endif

#ifdef WINCE

#elif defined(WIN32_64)
#include "../../pinc/WinCrashHandler.h"
#else

#endif

namespace SPA {

    //CIoService CUCommThread::m_io;

    CUCommThread::CUCommThread(tagThreadApartment ta)
    : m_ta(ta), m_pThread(nullptr) {

    }

    CUCommThread::~CUCommThread() {
        Kill();
        CAutoLock sl(m_mutex);
        while (m_qThreadMessage.size()) {
            CUThreadMessage message = m_qThreadMessage.front();
            SPA::CScopeUQueue::Unlock(message.m_pMessageBuffer);
            m_qThreadMessage.pop();
        }
    }

    tagThreadApartment CUCommThread::GetThreadApartment() {
        return m_ta;
    }

    const boost::thread* CUCommThread::GetBoostThread() const {
        return m_pThread;
    }

    CErrorCode CUCommThread::GetErrorCode() {
        CAutoLock sl(m_mutex);
        return m_ec;
    }

    CIoService& CUCommThread::GetIoService() {
        return m_io;
    }

    void CUCommThread::Call(const CErrorCode &ec) {
#ifdef WINCE
        bool ok = (CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK);
#elif defined(WIN32_64)
        bool ok = false;
        CCrashHandler::SetThreadExceptionHandlers();
        if (m_ta == taFree)
            ok = (CoInitializeEx(nullptr, COINIT_MULTITHREADED) == S_OK);
        else if (m_ta == taApartment)
            ok = (::CoInitialize(nullptr) == S_OK);
#else

#endif
        OnThreadStarted();
        m_cv.notify_all();
        try {
            m_io.run(m_ec);
        } catch (boost::system::system_error &err) {
#ifndef NDEBUG
            std::cout << "boost::system_error " << err.what() << ", " << __FUNCTION__ << std::endl;
#endif
        } catch (SPA::CUException &err) {
#ifndef NDEBUG
            std::cout << "SPA::CUException " << err.what() << ", " << __FUNCTION__ << std::endl;
#endif
        } catch (std::exception &err) {
#ifndef NDEBUG
            std::cout << "std::exception " << err.what() << ", " << __FUNCTION__ << std::endl;
#endif
        } catch (...) {
#ifndef NDEBUG
            std::cout << "Unknown exception " << ", " << __FUNCTION__ << std::endl;
#endif
        }
#ifdef WIN32_64
        if (ok)
            ::CoUninitialize();
#endif
        OnThreadEnded();
    }

    bool CUCommThread::Start() {
        CAutoLock sl(m_mutex);
        if (m_pThread != nullptr)
            return true;
        m_pThread = new boost::thread(boost::bind(&CIoService::run, &m_io, m_ec));
        m_io.post(boost::bind(&CUCommThread::Call, this, m_ec));
        boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(500);
        m_cv.timed_wait(sl, td);
        if (m_ec) {
            delete m_pThread;
            m_pThread = nullptr;
            return false;
        }
        return true;
    }

    bool CUCommThread::IsStarted() {
        CAutoLock sl(m_mutex);
        return (m_pThread != nullptr);
    }

    bool CUCommThread::Kill() {
        m_mutex.lock();
        do {
            if (m_pThread == nullptr) {
                m_mutex.unlock();
                break;
            }
            {
                boost::thread *p = m_pThread;
                m_pThread = nullptr;
                m_mutex.unlock();
                m_io.stop();
                while (!m_io.stopped()) {
                    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
                }
                if (p->joinable()) {
#ifndef _WIN32_WCE
                    if (!p->try_join_for(boost::chrono::seconds(1))) {
                        p->detach();
                    }
#else
                    p->join();
#endif
                }
                delete p;
            }
        } while (false);
        return true;
    }

} //namespace SPA