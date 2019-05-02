// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#include "../../include/definebase.h"

#ifndef __UMB_COMM_CLIENT_CORE_HEADER_H_
#define __UMB_COMM_CLIENT_CORE_HEADER_H_

#if defined(WIN32_64)

#ifndef WINCE
#include "targetver.h"
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#ifndef WINCE
#include <windows.h>
#endif

#include <iostream>

#endif


// TODO: reference additional headers your program requires here

#endif