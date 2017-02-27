
#pragma once

#include "../../include/aclientw.h"

#ifdef WIN32_64

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#else

//linux defines
#if defined __LP64__ || defined __PPC64__ || defined __x86_64__ || defined __s390x__
#define db2Is64bit
#endif

#if !defined DB2LINUX
#define DB2LINUX 1
#endif

#if !defined SQL_API_RC
#define SQL_API_RC  int
#define SQL_STRUCTURE   struct
#define PSQL_API_FN  *
#define SQL_API_FN
#define SQL_POINTER
#define SQL_API_INTR

#endif

#endif
