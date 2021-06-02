#ifndef _UDAPARTS_TDS_CLIENT_HANDLER_H_
#define _UDAPARTS_TDS_CLIENT_HANDLER_H_

#include "reqbase.h"
#include <deque>

namespace tds {
	class CTdsChannel : public SPA::CBaseHandler
	{
	private:
		SPA::CScopeUQueue m_sb;

	public:
		CTdsChannel(SPA::SessionHandle sh);
		int Send(tds::CReqBase *sender, const unsigned char* buffer, unsigned int bytes);

	protected:
		void OnAvailable(const unsigned char* data, unsigned int bytes);
		void OnClosed();
		void OnConnected();

	private:
		void Reset();

	private:
		SPA::CSpinLock m_cs;
		std::deque<tds::CReqBase*> m_deq; //protected by m_cs
		SPA::CUQueue& m_buff;
		friend class CReqBase;
	};
}

#endif
