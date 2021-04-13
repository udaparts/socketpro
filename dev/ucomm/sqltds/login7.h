#ifndef _U_TDS_LOGIN7_H_
#define _U_TDS_LOGIN7_H_

#include "reqbase.h"

namespace tds {
	class CLogin7 : public CReqBase
	{
	public:
		bool GetClientMessage(SPA::CUQueue &buffer);
		void OnResponse(const unsigned char *data, unsigned int bytes);
	};

}

#endif