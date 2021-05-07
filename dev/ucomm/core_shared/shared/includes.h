
#ifndef __UMB_INCLUDES_FOR_INTERNAL_H_
#define __UMB_INCLUDES_FOR_INTERNAL_H_

#include "../../include/membuffer.h"

#ifdef WIN32_64
typedef DWORD UTHREAD_ID;
#else
typedef pthread_t UTHREAD_ID;
#endif

#include <vector>
#include <queue>

#ifndef WINCE
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>
using std::atomic;
#include <thread>
using namespace std::this_thread;
#include <functional>
#else
#include <boost/atomic.hpp>
#include <boost/thread/condition_variable.hpp>
using boost::atomic;
#include <boost/thread.hpp>
using namespace boost::this_thread;
#include <boost/bind.hpp>
#endif

#include <boost/asio.hpp>
#ifndef WIN32_64
#include <boost/asio/ssl.hpp>
#endif

#include "streamhead.h"
#include "../../include/ucomm.h"
typedef boost::system::error_code CErrorCode;

namespace nsIP = boost::asio::ip;
namespace nsPlaceHolders = boost::asio::placeholders;
typedef boost::asio::io_service CIoService;
typedef nsIP::tcp::acceptor CAcceptor;
typedef nsIP::tcp::socket CSocket;
typedef nsIP::tcp::resolver CResolver;
typedef boost::asio::ip::tcp::resolver CResolver;

#ifndef WIN32_64
namespace nsSSL = boost::asio::ssl;
typedef nsSSL::stream<nsIP::tcp::socket> CSslSocket;
typedef nsSSL::context CSslContext;
#endif

#ifndef WINCE
typedef std::unique_lock<std::mutex> CAutoLock;
//typedef std::unique_lock<std::recursive_mutex> CAutoLockR;
//typedef std::unique_lock<std::recursive_mutex> CAutoLockRecursive;
typedef std::condition_variable CConditionVariable;
#else
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
typedef boost::mutex::scoped_lock CAutoLock;
//#include <boost/thread/recursive_mutex.hpp>
//typedef boost::recursive_mutex::scoped_lock CAutoLockR;
//typedef boost::recursive_mutex::scoped_lock CAutoLockRecursive;
typedef boost::condition_variable CConditionVariable;
#endif

unsigned int GetNumberOfCores();
void ChangeUInt32Endian(unsigned int *pGroup, unsigned int count);

class U_MODULE_HIDDEN CRAutoLock {
public:
#ifndef WINCE

    CRAutoLock(std::mutex &mutex, bool &chatting) : m_mutex(mutex), m_Chatting(chatting) {
#else

    CRAutoLock(boost::mutex &mutex, bool &chatting) : m_mutex(mutex), m_Chatting(chatting) {
#endif
        assert(!chatting);
        m_Chatting = true;
        m_mutex.unlock();
    }

    ~CRAutoLock() {
        m_mutex.lock();
        m_Chatting = false;
    }

private:
    bool &m_Chatting;
#ifndef WINCE
    std::mutex &m_mutex;
#else
    boost::mutex &m_mutex;
#endif
};

SPA::CUQueue& operator<<(SPA::CUQueue &mc, const SPA::CSwitchInfo &si);
SPA::CUQueue& operator>>(SPA::CUQueue &mc, SPA::CSwitchInfo &si);

#endif

