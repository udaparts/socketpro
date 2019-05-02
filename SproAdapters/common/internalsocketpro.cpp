// stdafx.cpp : source file that includes just the standard includes
// SProAdapter.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

namespace SocketProAdapter
{
#ifndef _WIN32_WCE
	namespace ServerSide
	{
		CSocketProServerLoader	g_SocketProLoader;
	}
#endif
}