#pragma once

#include "../../../include/generalcache.h"

namespace NJA {

	class CAsyncHandler : public CCachedBaseHandler<0> {
	public:
		CAsyncHandler(CClientSocket *cs = nullptr);
	};
}
