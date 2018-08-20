
#include "stdafx.h"
#include "asynchandler.h"


namespace NJA {

	CAsyncHandler::CAsyncHandler(CClientSocket *cs) : CAsyncServiceHandler(0, cs) {
	}
}
