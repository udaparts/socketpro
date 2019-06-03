#ifndef __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__
#define __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__

#include "../../../include/definebase.h"


#ifdef WIN32_64

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

#else
#define USE_BOOST_LARGE_INTEGER_FOR_DECIMAL
#endif

#include "../../../include/tablecache.h"
#include "dbreqcb.h"

#include <node.h>

#if NODE_VERSION_AT_LEAST(8,0,0)
#define HAS_NULLORUNDEFINED_FUNC
#endif

#if NODE_VERSION_AT_LEAST(10,4,0)
#define HAS_BIGINT
#endif

#include <node_object_wrap.h>
#include <node_buffer.h>

namespace NJA {
    using v8::FunctionTemplate;
    using v8::Context;
    using v8::Date;

    using namespace SPA;
    using namespace SPA::ClientSide;
    using namespace SPA::UDB;

    enum tagDataType {
        dtUnknown = 0,
        dtString = 1,
        dtBool = 2,
        dtDate,
        dtInt32,
        dtInt64,
        dtDouble
    };

    //internal utility methods
    bool From(Isolate* isolate, const Local<Value>& v, const std::string &hint, CComVariant &vt);
    SPA::UINT64 ToDate(Isolate* isolate, const Local<Value>& d);
    Local<Value> ToDate(Isolate* isolate, SPA::UINT64 dt);
    Local<String> ToStr(Isolate* isolate, const uint16_t *str, size_t len = (size_t) INVALID_NUMBER);
    std::wstring ToStr(Isolate* isolate, const Local<Value> &s);
    std::string ToAStr(Isolate* isolate, const Local<Value> &s);
    bool ToPInfoArray(Isolate* isolate, const Local<Value> &pInfo, CParameterInfoArray &vInfo);
    bool ToGroups(Isolate* isolate, const Local<Value>& p, std::vector<unsigned int> &v);
    Local<Array> ToMeta(Isolate* isolate, const CDBColumnInfoArray &v);
    Local<Array> ToMeta(Isolate* isolate, const SPA::CKeyMap &mapkey);
    bool ToArray(Isolate* isolate, const Local<Value> &data, CDBVariantArray &v);
    using SPA::Utilities::Trim;
    int time_offset(time_t rawtime);

    extern SPA::CUCriticalSection g_cs;
    extern std::vector<std::string> g_KeyAllowed;
    extern const char* UNSUPPORTED_TYPE;
    extern const char* UNSUPPORTED_ARRAY_TYPE;
    extern const char* BOOLEAN_EXPECTED;
    extern const char* BAD_DATA_TYPE;
    extern const char* INTEGER_EXPECTED;
};

#endif
