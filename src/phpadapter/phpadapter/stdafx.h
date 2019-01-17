
#pragma once

#include "../../../include/definebase.h"
#define NO_MIDDLE_TIER

#ifdef WIN32_64

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

#define SPA_PHP_EXPORT __declspec(dllexport)
#else
#define SPA_PHP_EXPORT __attribute__ ((visibility ("default")))
#endif

#include "../../../include/aqhandler.h"
#include "../../../include/streamingfile.h"
#include "../../../include/udb_client.h"
#include "../../../include/masterpool.h"
#include "../../../include/rdbcache.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/sqlite/usqlite.h"
#include <algorithm>

namespace PA {
	void Trim(std::string &str);

	typedef SPA::ClientSide::CCachedBaseHandler<0> CAsyncHandler;
}
