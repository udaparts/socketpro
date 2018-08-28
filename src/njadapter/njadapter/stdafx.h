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

#include "../../../include/udb_client.h"

#include <node.h>
#include <node_object_wrap.h>
#include <node_buffer.h>

namespace NJA {
	using v8::FunctionTemplate;
	using v8::Context;
	using v8::Int32;
	using v8::Date;
	using v8::Array;

	using node::AtExit;

	using namespace SPA;
	using namespace SPA::ClientSide;
	using namespace SPA::UDB;
};


#endif
