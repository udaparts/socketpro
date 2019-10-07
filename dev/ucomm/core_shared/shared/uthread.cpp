#include "uthread.h"
#ifdef WIN32_64
#include <objbase.h>
#endif

#ifdef WINCE

#elif defined(WIN32_64)
#include "../pinc/WinCrashHandler.h"
#else

#endif

namespace SPA
{

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

    const CUCommThread::thread * CUCommThread::GetBoostThread() const {
        return m_pThread;
    }

    CErrorCode CUCommThread::GetErrorCode() {
        CAutoLock sl(m_mutex);
        return m_ec;
    }

    CIoService & CUCommThread::GetIoService() {
        return m_io;
    }

    void CUCommThread::Call() {
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
        try{
			m_io.post([this]() {
				m_cv.notify_all();
			});
            m_io.run();
#ifndef NDEBUG
        }

        catch(boost::system::system_error & err) {
            std::cout << "boost::system_error " << err.what() << ", " << __FUNCTION__ << std::endl;
        }

        catch(SPA::CUException & err) {
            std::cout << "SPA::CUException " << err.what() << ", " << __FUNCTION__ << std::endl;
        }

        catch(std::exception & err) {
            std::cout << "std::exception " << err.what() << ", " << __FUNCTION__ << std::endl;
        }

        catch(...) {
            std::cout << "Unknown exception " << ", " << __FUNCTION__ << std::endl;
        }
#else
        }

        catch(...) {
        }
#endif

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
		m_pThread = new thread(boost::bind(&CIoService::run_one, &m_io));
        m_io.post(boost::bind(&CUCommThread::Call, this));
#ifndef WINCE
        m_cv.wait_for(sl, std::chrono::milliseconds(5000));
#else
        boost::system_time td = boost::get_system_time() + boost::posix_time::milliseconds(500);
        m_cv.timed_wait(sl, td);
#endif
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
                thread *p = m_pThread;
                m_pThread = nullptr;
                m_mutex.unlock();
                m_io.stop();
                while (!m_io.stopped()) {
                    sleep(boost::posix_time::milliseconds(1));
                }
                if (p->joinable()) {
                    p->join();
                }
                delete p;
            }
        } while (false);
        return true;
    }

} //namespace SPA