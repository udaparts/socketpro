
#pragma once

#include "../../../include/definebase.h"
#define NO_MIDDLE_TIER

#ifdef WIN32_64

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>

#else


#endif

#include "phpcpp.h"

#include "../../../include/aclientw.h"
#include "../../../include/aqhandler.h"
#include "../../../include/streamingfile.h"
#include "../../../include/udb_client.h"
#include "../../../include/masterpool.h"
#include "../../../include/rdbcache.h"
