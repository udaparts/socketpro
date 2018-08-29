#pragma once

#include "../../../include/aqhandler.h"

namespace NJA {
	class CAQueue : public SPA::ClientSide::CAsyncQueue
	{
	public:
		CAQueue(SPA::ClientSide::CClientSocket *cs);
		~CAQueue();

	private:
		enum tagQueueEvent {
			qeGetKeys = 0,
			qeEnqueue,

		};

		struct QueueCb {
			tagQueueEvent EventType;
			SPA::PUQueue Buffer;
			std::shared_ptr<CNJFunc> Func;
		};
		uv_async_t m_qType;
		std::deque<QueueCb> m_deqQCb; //Protected by m_csQ;
	};
}
