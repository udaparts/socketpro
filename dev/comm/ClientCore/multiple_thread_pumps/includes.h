#ifndef __UMB_INCLUDES_FOR_INTERNAL_H_
#define __UMB_INCLUDES_FOR_INTERNAL_H_

#include "../../include/membuffer.h"
#include <vector>
#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "startup.h"

#include <boost/thread/recursive_mutex.hpp>
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

typedef boost::mutex::scoped_lock CAutoLock;
//typedef boost::recursive_mutex::scoped_lock CAutoLockR;
//typedef boost::recursive_mutex::scoped_lock CAutoLockRecursive;
typedef boost::condition_variable CConditionVariable;

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

class ssl_category : public boost::system::error_category
{
public:
  const char* name() const BOOST_ASIO_ERROR_CATEGORY_NOEXCEPT
  {
    return "asio.ssl";
  }

  std::string message(int value) const
  {
	  char str[256] = {0};
	  const char* s = g_pStartup->ERR_error_string(value, str);
      return s ? s : "asio.ssl error";
  }
};

static const boost::system::error_category& get_ssl_category()
{
  static ssl_category instance;
  return instance;
}

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
