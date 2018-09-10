#ifndef __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__
#define __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__

#include "../../../include/definebase.h"

#if defined(WIN32) || defined(_WIN64)

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#else

#endif

#include <atomic>

#include "dbreqcb.h"

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

namespace NJA {
	using v8::FunctionTemplate;
	using v8::Context;
	using v8::Int32;
	using v8::Date;
	using v8::Array;
	using v8::Promise;

	using node::AtExit;

	using namespace SPA;
	using namespace SPA::ClientSide;
	using namespace SPA::UDB;

	enum tagDataType {
		dtUnknown = 0,
		dtString,
		dtBool,
		dtDate,
	};

	//internal utility methods
	Local<Value> From(Isolate* isolate, const VARIANT &vt, bool strForDec = false);
	bool From(const Local<Value>& v, const std::string &hint, CComVariant &vt);
	SPA::UINT64 ToDate(const Local<Value>& d);
	Local<Value> ToDate(Isolate* isolate, SPA::UINT64 dt);
	Local<String> ToStr(Isolate* isolate, const char *str, size_t len = (size_t)INVALID_NUMBER);
	Local<String> ToStr(Isolate* isolate, const wchar_t *str, size_t len = (size_t)INVALID_NUMBER);
	Local<String> ToStr(Isolate* isolate, const uint16_t *str, size_t len = (size_t)INVALID_NUMBER);
	std::wstring ToStr(const Local<Value> &s);
	std::string ToAStr(const Local<Value> &s);
	void ThrowException(Isolate* isolate, const char *str);
	bool ToPInfoArray(Isolate* isolate, const Local<Value> &pInfo, CParameterInfoArray &vInfo);
	std::vector<unsigned int>ToGroups(const Local<Value>& p);

	extern std::atomic_bool g_bSelfSigned;
};


#endif
