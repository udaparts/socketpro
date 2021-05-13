#pragma once

#include <functional>

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART
#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/home/classic/core/primitives/primitives.hpp>
#endif

namespace UHTTP {

#ifdef USE_SPIRIT_CLSSICAL_FOR_MULTIPART

    using namespace boost::spirit::classic;

    typedef std::function<void(const char*, const char*) > CStrAction;
    typedef std::function<void(boost::uint32_t) > CUInt32Action;
    typedef std::function<void(boost::int32_t) > CInt32Action;
    typedef std::function<void(boost::uint64_t) > CUInt64Action;
    typedef std::function<void(boost::int64_t) > CInt64Action;
    typedef std::function<void(double) > CDoubleAction;

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
