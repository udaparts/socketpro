#pragma once

namespace NJA {

	class CAsyncHandler : public CAsyncServiceHandler {
	public:
		CAsyncHandler(CClientSocket *cs = nullptr);
	};
}
