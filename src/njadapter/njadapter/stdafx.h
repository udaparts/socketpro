#ifndef __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__
#define __SOCKETPRO_NODEJS_ADAPTER_STDAFX_H__

#include "../../../include/definebase.h"

#ifdef WIN32_64

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#else


#endif

#include "../include/node.h"
#include "../include/node_object_wrap.h"
#include "../include/uv.h"

#include "../../../include/streamingfile.h"
#include "../../../include/async_odbc.h"
#include "../../../include/async_sqlite.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/rdbcache.h"
#include "../../../include/masterpool.h"

namespace NJA {
	using v8::Function;
	using v8::FunctionCallbackInfo;
	using v8::Isolate;
	using v8::Local;
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
	using node::AtExit;
};

#endif
