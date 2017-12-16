#ifndef ___SOCKETPRO_CLIENT_HANDLER_PI_H__
#define ___SOCKETPRO_CLIENT_HANDLER_PI_H__

#include "../../../../include/aclientw.h"

using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../pi_i.h"

//client handler for service Pi

class Pi : public CAsyncServiceHandler {
public:
	Pi(CClientSocket *pClientSocket)
		: CAsyncServiceHandler(sidPiWorker, pClientSocket) {
	}

protected:
	virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
		if (IsRouteeRequest()) {
			switch (reqId) {
			case idComputePi:
				{
					double dStart;
					double dStep;
					int nNum;
					mc >> dStart >> dStep >> nNum;
					double dX = dStart + dStep / 2;
					double dd = dStep * 4.0;
					double ComputeRtn = 0.0;
					for (int n = 0; n < nNum; n++) {
						dX += dStep;
						ComputeRtn += dd / (1 + dX * dX);
					}
					SendRouteeResult(ComputeRtn, dStart);
				}
				break;
			default:
				break;
			}
		}
	}
};
#endif
