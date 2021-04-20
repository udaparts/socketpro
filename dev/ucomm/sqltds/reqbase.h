#ifndef _U_TDS_REQUEST_BASE_H_
#define _U_TDS_REQUEST_BASE_H_

#include "tdsdef.h"

namespace tds {

	class CReqBase
	{
	public:
		CReqBase();
		virtual ~CReqBase();

	public:
		virtual void OnResponse(const unsigned char *data, unsigned int bytes) = 0;
		virtual bool IsDone() const;
		const PacketHeader& GetResponseHeader() const;

	protected:
		virtual void Reset();

	protected:
		PacketHeader ResponseHeader;
	};

}

#endif