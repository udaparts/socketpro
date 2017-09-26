
#include "../../../include/ucomm.h"
#include "../../../include/membuffer.h"

#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//defines for service HelloWorld
static const unsigned int sidStreamSystem = (SPA::sidReserved + 121);

static const unsigned short idSetDefaultDatabaseName = ((unsigned short)SPA::idReservedTwo + 20);
static const unsigned short idSubscribeAndGetInitialCachedTablesData = idSetDefaultDatabaseName + 1;
static const unsigned short idBeginBatchProcessing = idSetDefaultDatabaseName + 2;
static const unsigned short idEndBatchProcessing = idSetDefaultDatabaseName + 3;

static const unsigned short idQueryMaxMinAvgs = idSetDefaultDatabaseName + 4;

struct CMaxMinAvg {
	SPA::INT64 Max;
	SPA::INT64 Min;
	double Avg;
};

#endif