#pragma once

namespace NJA {

	class CAsyncHandler : public SPA::ClientSide::CCachedBaseHandler<0> {
	public:
		CAsyncHandler(SPA::ClientSide::CClientSocket *cs);
	};
}
