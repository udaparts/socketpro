#include "aqueue.h"
#include "stdafx.h"

namespace NJA {

	CAQueue::CAQueue(SPA::ClientSide::CClientSocket *cs) 
		: CAsyncQueue(cs)
	{
	}

	CAQueue::~CAQueue()
	{
	}

}