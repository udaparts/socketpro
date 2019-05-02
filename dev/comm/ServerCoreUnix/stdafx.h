// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __UMB_COMM_SERVER_CORE_HEADER_H_
#define __UMB_COMM_SERVER_CORE_HEADER_H_

#include "../../include/membuffer.h"

#include <stdio.h>

#ifdef WIN32_64
#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#endif

#include <functional>
#include "../../pinc/hpdefines.h"
#include <boost/asio.hpp>
#ifndef NOT_USE_OPENSSL
#include <boost/asio/ssl.hpp>
#endif
#include <boost/algorithm/string.hpp>
#include <boost/noncopyable.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/home/classic/core/primitives/primitives.hpp>

// TODO: reference additional headers your program requires here

namespace UHTTP {

    using namespace boost::spirit::classic;

    typedef boost::function<void(const char*, const char*) > CStrAction;
    typedef boost::function<void(boost::uint32_t) > CUInt32Action;
    typedef boost::function<void(boost::int32_t) > CInt32Action;
    typedef boost::function<void(boost::uint64_t) > CUInt64Action;
    typedef boost::function<void(boost::int64_t) > CInt64Action;
    typedef boost::function<void(double) > CDoubleAction;

    typedef scanner<const char*, scanner_policies<> > CScanner;
    typedef rule<CScanner> CRule;

    extern CRule R_CRLF;
    extern CRule R_HEADER_END;
    extern CRule R_HEADER;
    extern CRule R_VALUE;
};

#endif

#endif