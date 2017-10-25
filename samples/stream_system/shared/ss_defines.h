
#include "../../../include/ucomm.h"
#include "../../../include/membuffer.h"
#include <vector>

#ifndef ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__
#define ___SOCKETPRO_DEFINES_STREAM_SYSTEM_I_H__

//#defines for stream system
static const unsigned int sidStreamSystem = (SPA::sidReserved + 1210);

static const unsigned short idQueryMaxMinAvgs = SPA::idReservedTwo + 1;
static const unsigned short idGetMasterSlaveConnectedSessions = SPA::idReservedTwo + 2;
static const unsigned short idUploadEmployees = SPA::idReservedTwo + 3;
static const unsigned short idStartSequence = SPA::idReservedTwo + 4;
static const unsigned short idEndSequence = SPA::idReservedTwo + 5;
static const unsigned short idGetRentalDateTimes = SPA::idReservedTwo + 6;


struct CMaxMinAvg {
	CMaxMinAvg() {
		::memset(this, 0, sizeof(CMaxMinAvg));
	}
	double Max;
	double Min;
	double Avg;
};

struct CRentalDateTimes {
	CRentalDateTimes() {
		::memset(this, 0, sizeof(CRentalDateTimes));
	}
	SPA::UINT64 Rental;
	SPA::UINT64 Return;
	SPA::UINT64 LastUpdate;
};

typedef std::vector<SPA::INT64> CInt64Array;

namespace SPA {

	static CUQueue& operator<<(CUQueue &q, const CInt64Array &v) {
		q << (unsigned int)v.size();
		q.Push((const unsigned char*)v.data(), (unsigned int)(v.size() * sizeof(INT64)));
		return q;
	}

	static CUQueue& operator>>(CUQueue &q, CInt64Array &v) {
		unsigned int count;
		v.clear();
		q >> count;
		auto data = (const INT64*)q.GetBuffer();
		v.assign(data, data + count);
		q.Pop(count * sizeof(INT64));
		return q;
	}

	static CUQueue& operator<<(CUQueue &q, const CMaxMinAvg &v) {
		q << v.Max << v.Min << v.Avg;
		return q;
	}

	static CUQueue& operator>>(CUQueue &q, CMaxMinAvg &v) {
		q >> v.Max >> v.Min >> v.Avg;
		return q;
	}

	static CUQueue& operator<<(CUQueue &q, const CRentalDateTimes &v) {
		q << v.Rental << v.Return << v.LastUpdate;
		return q;
	}

	static CUQueue& operator>>(CUQueue &q, CRentalDateTimes &v) {
		q >> v.Rental >> v.Return >> v.LastUpdate;
		return q;
	}
};

#endif