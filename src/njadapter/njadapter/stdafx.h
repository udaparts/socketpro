#ifndef __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__
#define __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__


#if defined(WIN32) || defined(_WIN64)

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#else

#endif

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <node_buffer.h>

#include "../../../include/async_odbc.h"
#include "../../../include/aqhandler.h"
#include "../../../include/streamingfile.h"
#include "../../../include/async_sqlite.h"
#include "../../../include/async_mysql.h"
#include "../../../include/rdbcache.h"
#include "../../../include/masterpool.h"


namespace NJA {
	using v8::Function;
	using v8::FunctionCallbackInfo;
	using v8::Isolate;
	using v8::Local;
	using v8::Global;
	using v8::Null;
	using v8::Object;
	using v8::String;
	using v8::Value;
	using v8::Number;
	using v8::Boolean;
	using v8::Undefined;
	using v8::Symbol;
	using v8::Exception;
	using v8::FunctionTemplate;
	using v8::Persistent;
	using v8::Context;
	using v8::Exception;
	using v8::Int32;
	using v8::Uint32;
	using v8::Integer;
	using v8::Date;
	using v8::Array;
	using v8::FunctionCallback;
	using v8::HandleScope;

	using node::AtExit;

	using namespace SPA;
	using namespace SPA::ClientSide;
	using namespace SPA::UDB;
};


#endif
