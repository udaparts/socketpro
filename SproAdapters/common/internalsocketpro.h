// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef __SOCKETPRO_ADAPTER_INTERNAL_H___
#define __SOCKETPRO_ADAPTER_INTERNAL_H___

#include <vcclr.h>
using namespace System;

namespace SocketProAdapter
{
#ifndef _WIN32_WCE
	namespace ServerSide
	{
/*		#include "uscktpro.h"
		//Visual C++
		#pragma comment(lib, "usktpror.lib")
*/
		#include "sploader.h"
		extern CSocketProServerLoader g_SocketProLoader;
	};
#endif
};

#endif
