// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <iostream>

#include "../../../include/definebase.h"

#ifdef WIN32_64
#include "targetver.h"
#include <stdio.h>
#include <tchar.h>
#endif

#include "../../../pinc/hpdefines.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace nsAsio = boost::asio;
namespace nsSSL = nsAsio::ssl;
namespace nsIP = nsAsio::ip;
namespace nsPlaceHolders = nsAsio::placeholders;
typedef nsSSL::stream<nsIP::tcp::socket> CSslSocket;
typedef nsAsio::io_service CIoService;
typedef nsSSL::context CSslContext;
typedef boost::system::error_code CErrorCode;
typedef nsIP::tcp::acceptor CAcceptor;
typedef nsIP::tcp::socket CSocket;
typedef nsIP::tcp::resolver CResolver;
typedef boost::system::error_code CErrorCode;
typedef nsIP::tcp::resolver CIpResolver;



