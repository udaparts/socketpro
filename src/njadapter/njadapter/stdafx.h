
#pragma once

#include "../../../include/streamingfile.h"
#include "../../../include/async_odbc.h"
#include "../../../include/async_sqlite.h"
#include "../../../include/mysql/umysql.h"
#include "../../../include/rdbcache.h"
#include "../../../include/masterpool.h"

#ifdef WIN32_64

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#else


#endif
