#ifndef __UMB_COMM_CLIENT_CORE_HEADER_H_
#define __UMB_COMM_CLIENT_CORE_HEADER_H_

#include "../include/definebase.h"

#if defined(WIN32_64)
#include <atlbase.h>
#ifndef WINCE
#include "targetver.h"
#endif

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#ifndef WINCE
#include <windows.h>
#endif

#endif

#endif
