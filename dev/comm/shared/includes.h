#ifndef __UMB_INCLUDES_FOR_INTERNAL_H_
#define __UMB_INCLUDES_FOR_INTERNAL_H_

#include "../../include/membuffer.h"
#include <vector>
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#ifndef NOT_USE_OPENSSL
#include <boost/asio/ssl.hpp>
#endif
//#include <boost/thread/recursive_mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <time.h>

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

#ifndef NOT_USE_OPENSSL
namespace nsSSL = boost::asio::ssl;
typedef nsSSL::stream<nsIP::tcp::socket> CSslSocket;
typedef nsSSL::context CSslContext;
#endif

typedef boost::mutex::scoped_lock CAutoLock;
//typedef boost::recursive_mutex::scoped_lock CAutoLockR;
//typedef boost::recursive_mutex::scoped_lock CAutoLockRecursive;
typedef boost::condition_variable CConditionVariable;

SPA::UINT64 GetTimeTick();
unsigned int GetNumberOfCores();
void ChangeUInt32Endian(unsigned int *pGroup, unsigned int count);

class CRAutoLock {
public:

    CRAutoLock(boost::mutex &mutex) : m_mutex(mutex) {
        m_mutex.unlock();
    }

    ~CRAutoLock() {
        m_mutex.lock();
    }

private:
    boost::mutex &m_mutex;
};

/*
class CRAutoLockR {
public:

    CRAutoLockR(boost::recursive_mutex &mutex) : m_mutex(mutex) {
        m_mutex.unlock();
    }

    ~CRAutoLockR() {
        m_mutex.lock();
    }

private:
    boost::recursive_mutex &m_mutex;
};
*/

SPA::CUQueue& operator <<(SPA::CUQueue &mc, const SPA::CSwitchInfo &si);
SPA::CUQueue& operator >>(SPA::CUQueue &mc, SPA::CSwitchInfo &si);



#endif
