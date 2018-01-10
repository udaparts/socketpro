
#pragma once

#include "../../../include/async_sqlite.h"
typedef SPA::ClientSide::CSqlite CSql;


#ifdef WIN32_64

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

#else

#endif

#include <iostream>

using namespace SPA;
using namespace SPA::ClientSide;
