// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include "../../include/definebase.h"

#ifndef __UMB_COMM_CLIENT_CORE_HEADER_H_
#define __UMB_COMM_CLIENT_CORE_HEADER_H_

#if defined(WIN32_64)

#ifndef WINCE
#include "targetver.h"
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifndef WINCE
#include <windows.h>
#endif

#include <iostream>

#endif

#include <boost/asio.hpp>
#ifndef NOT_USE_OPENSSL
#include <boost/asio/ssl.hpp>
#else
#include "sslcontext_win.h"
#endif

#ifndef NOT_USE_OPENSSL
namespace nsIP = boost::asio::ip;
namespace nsSSL = boost::asio::ssl;
typedef nsSSL::stream<nsIP::tcp::socket> CSslSocket;
typedef nsSSL::context CSslContext;
#else
typedef SPA::CSslContext CSslContext;
#endif
// TODO: reference additional headers your program requires here

#endif