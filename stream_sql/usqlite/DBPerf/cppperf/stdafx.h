

#pragma once

#include "../../../../include/udb_client.h"

#ifdef WIN32_64

#include "targetver.h"
#include <stdio.h>
#include <tchar.h>

#else
#include <chrono>
using std::chrono::system_clock;
typedef std::chrono::milliseconds ms;
#endif
