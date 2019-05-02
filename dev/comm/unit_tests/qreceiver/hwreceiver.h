#ifndef ___SOCKETPRO_CLIENT_HANDLER_QHW_H__
#define ___SOCKETPRO_CLIENT_HANDLER_QHW_H__

#include "aclientw.h"
using namespace SPA;
using namespace SPA::ClientSide;

/* **** including all of defines, service id(s) and request id(s) ***** */
#include "../qhw_i.h"

//client handler for service HWReceiver
class HWReceiver : public CAsyncServiceHandler
{
public:
	HWReceiver(CClientSocket *pClientSocket)
	: CAsyncServiceHandler(sidHWReceiver, pClientSocket)
	{
	}

public:
	SPA::UINT64 DoDequeue(unsigned int count, bool notified)
	{
		SPA::UINT64 DoDequeueRtn;
		bool bProcessRy = ProcessR1(idDoDequeueHWReceiver, count, notified, DoDequeueRtn);
		return DoDequeueRtn;
	}

protected:
	virtual void OnBaseRequestProcessed(unsigned short reqId) {
		switch(reqId)
		{
		case SPA::idMessageQueued:
			SendRequest(idDoDequeueHWReceiver, [](CAsyncResult & ar) {
				SPA::UINT64 res;
				ar >> res;
				res = 0;
			});
			break;
		default:
			break;
		}
	}

	virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
		switch(reqId)
		{
		case idHWMessage:
			{
				unsigned int index;
				std::string msg;
				mc >> msg >> index;
				std::cout << "Message = " << msg <<", index = " << index << std::endl;
			}
			break;
		default:
			break;
		}
	}
};

#endif
