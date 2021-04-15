#include <time.h>
#include "../include/membuffer.h"
#include "../include/channelpool.h"
#include "prelogin.h"
#include "login7.h"
#include <deque>
#include <iostream>

using namespace SPA;

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, CertInfo * ci) {
	std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
	return true;
}

class CTdsClient : public CBaseHandler {
public:
	CTdsClient(SessionHandle sh) : CBaseHandler(sh) {}
	std::deque<tds::CReqBase *> m_deq;
	CUQueue m_buff;

protected:
	void OnAvailable(const unsigned char *data, unsigned int bytes) {
		m_buff.Push(data, bytes);
		do {
			if (m_buff.GetSize() < sizeof(tds::PacketHeader))
				break;
			tds::PacketHeader *ph = (tds::PacketHeader*)m_buff.GetBuffer();
			unsigned int len = tds::ChangeEndian(ph->Length);
			if (m_buff.GetSize() < len)
				break;
			if (!m_deq.size())
				break;
			tds::CReqBase *rb = m_deq.front();
			m_deq.pop_front();
			rb->OnResponse(m_buff.GetBuffer(), len);
			m_buff.Pop(len);
		} while (false);
	}
};

int main()
{
	tds::CPrelogin pl(true);
	tds::CLogin7 login;

	SPA::CScopeUQueue sb;
	bool ok = pl.GetClientMessage(1, *sb);
	CSessionPool<CTdsClient> pool(1);
	auto handler = pool.FindAClosedHandler();
	ok = handler->Connect("windesk", 1433, tagEncryptionMethod::NoEncryption, false, true);
	handler->m_deq.push_back(&pl);
	int res = handler->Send(sb->GetBuffer(), sb->GetSize());
	::getchar();
	ok = handler->IsConnected();
	CUQueue &buffer = handler->m_buff;
	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}
