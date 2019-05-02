

#include "../../../include/membuffer.h"

#ifndef _ICOS_DEFINES_H_
#define _ICOS_DEFINES_H_


#define DEFAULT_PINNED_GENERIC_MEMORY true

// MAXIMUM number of request a block can hold
//#define DATASIZE (64 * 1024)
#define DATASIZE (128 * 1024)
// this is a add-on space in the request block for additional status info.
#define EXTRABYTES 32

namespace iCOS
{
	#pragma pack(push,1)

	struct CIndexIpCode
	{
		//it is used by HTTP client for lookup
		SPA::UINT64	Index;

		//it is ip address when it comes from HTTP server; and country and region codes when sending results from the server back to client HTTP server
		unsigned int IpCode;
	};

	//I may implement it when you require me to do so
	struct CIndexCity : public CIndexIpCode
	{
		//the length of City in BYTES (NOT IN CHARS !!!). If City is not available, it should be -1, (~0) or 0xFFFFFFFF
		unsigned int Len;

		wchar_t City[48];
	};

	#pragma pack(pop)
}

// a record for each unique location stored in the database (distinct combo of country code, region code, and city name).
typedef struct tagGEOLocation {
	char CountryCode[2];
	char RegionCode[2];
	wchar_t City[48];
	
	//two roles:
	//1. padding for 8-byte alignment of GEOLocation,
	//2. the length of City in BYTES (NOT IN CHARS !!!). If City is not available, it should be -1, (~0) or 0xFFFFFFFF
	unsigned int Len;
} GEOLocation;


typedef union tagIPBlock {
	unsigned int TargetIP;
	//unsigned long long LocationPtr;  // 64 bit
	GEOLocation *LocationPtr;
} IPBlock;

// the database record on disk
typedef struct tagGEOIPRecord {
	unsigned int startIpNum;
	unsigned int endIpNum;
	union 
	{
		unsigned int LocationID;
		GEOLocation *LocationPtr;
	};
} GEOIPRecord;


// a record for each unique location stored in the database (distinct combo of country code, region code, and city name).
typedef struct tagGEOLocationRecord {
	unsigned int LocationID;
	GEOLocation Location;
	unsigned int padloc;  // padding for 8-byte alignment of Location.
} GEOLocationRecord;

extern GEOIPRecord *GPUGEOIPDatabase;


/* Valid settings for device_sync method:
    0 cudaDeviceScheduleAuto  (Automatic Blocking)
    1 cudaDeviceScheduleSpin  (Spin Blocking)
    2 cudaDeviceScheduleYield  (Yield Blocking)
    3 (Undefined Blocking Method  DO NOT USE)
    4 cudaDeviceBlockingSync  (Blocking Sync Event) = low CPU utilization
*/
bool InitializeGPU(bool bPinGenericMemory = true, int device_sync_method = 4); //4 == cudaDeviceBlockingSync
bool DeinitializeGPU();

#define IPBLOCK_BUFFER_SIZE (DATASIZE * sizeof(IPBlock) + EXTRABYTES)

//we use the CScopeUQueueForIPBlock to avoid allocating and de-allocating memory repeatedly
typedef SPA::CScopeUQueueEx<IPBLOCK_BUFFER_SIZE, SPA::DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CScopeUQueueForIPBlock;

#endif
