// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once


#include "../../include/commutil.h"
#include <stdio.h>


#ifdef WIN32_64

#include "targetver.h"
#include <tchar.h>

#else

#endif

#include <functional>
#include "../../pinc/hpdefines.h"
#include <boost/asio.hpp>
#ifndef NOT_USE_OPENSSL
#include <boost/asio/ssl.hpp>
#endif
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/home/classic/core/primitives/primitives.hpp>
#endif

#include <boost/lexical_cast.hpp>

namespace nsAsio = boost::asio;

namespace nsIP = nsAsio::ip;
namespace nsPlaceHolders = nsAsio::placeholders;
typedef nsAsio::io_service CIoService;
typedef boost::system::error_code CErrorCode;
typedef nsIP::tcp::acceptor CAcceptor;
typedef nsIP::tcp::socket CSocket;
typedef nsIP::tcp::resolver CResolver;

#ifndef NOT_USE_OPENSSL
namespace nsSSL = nsAsio::ssl;
typedef nsSSL::stream<nsIP::tcp::socket> CSslSocket;
typedef nsSSL::context CSslContext;
#endif

namespace UHTTP {

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART

    using namespace boost::spirit::classic;

    typedef boost::function<void(const char*, const char*) > CStrAction;
    typedef boost::function<void(boost::uint32_t) > CUInt32Action;
    typedef boost::function<void(boost::int32_t) > CInt32Action;
    typedef boost::function<void(boost::uint64_t) > CUInt64Action;
    typedef boost::function<void(boost::int64_t) > CInt64Action;
    typedef boost::function<void(double) > CDoubleAction;

    /*typedef skipper_iteration_policy<> iter_policy_t;
    typedef scanner_policies<iter_policy_t> scanner_policies_t;
    typedef scanner<const char*, scanner_policies_t> CScanner;
     */

    typedef scanner<const char*, scanner_policies<> > CScanner;
    typedef rule<CScanner> CRule;

    extern CRule R_CRLF;
    extern CRule R_HEADER_END;
    extern CRule R_HEADER;
    extern CRule R_VALUE;
#endif
};


// TODO: reference additional headers your program requires here
