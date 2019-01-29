#include "stdafx.h"
#include "poolstartcontext.h"

namespace PA {
	CPoolStartContext::CPoolStartContext()
		: SvsId(0), Threads(1), AutoConn(true), AutoMerge(true),
		RecvTimeout(SPA::ClientSide::DEFAULT_RECV_TIMEOUT),
		ConnTimeout(SPA::ClientSide::DEFAULT_CONN_TIMEOUT),
		PhpHandler(nullptr), PoolType(NotMS) {
	}

	std::string CPoolStartContext::StartPool() {
		return "";
	}
}
