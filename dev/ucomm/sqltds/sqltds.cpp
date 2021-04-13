#include "../include/membuffer.h"
#include "../include/channelpool.h"
#include "prelogin.h"
#include <iostream>

using namespace SPA;

bool CALLBACK CVCallback(bool preverified, int depth, int errorCode, const char *errMessage, CertInfo * ci) {
	std::cout << "depth: " << depth << ", errCode: " << errMessage << "\n";
	return true;
}

class CTdsClient : public CBaseHandler {
public:
	CTdsClient(SessionHandle sh) : CBaseHandler(sh), m_ph(tds::tagPacketType::ptInitial, 0) {}
	CUQueue m_buff;

	tds::PacketHeader m_ph;

	bool IsCompleted() {
		return (m_ph.Type != tds::tagPacketType::ptInitial && m_ph.Length == (sizeof(m_ph) + m_buff.GetSize()));
	}

protected:
	void OnAvailable(const unsigned char *data, unsigned int bytes) {
		m_buff.Push(data, bytes);
		if (m_buff.GetSize() >= sizeof(m_ph)) {
			m_buff >> m_ph;
			m_ph.Length = tds::ChangeEndian(m_ph.Length);
		}
	}
};

int main()
{
	tds::Prelogin pl(0, true);
	SPA::CScopeUQueue sb;
	bool ok = pl.GetPreloginMessage(*sb);
	CSessionPool<CTdsClient> pool(1);
	auto handler = pool.FindAClosedHandler();
	ok = handler->Connect("windesk", 1433, tagEncryptionMethod::NoEncryption, false, true);
	int res = handler->Send(sb->GetBuffer(), sb->GetSize());
	::getchar();
	CUQueue &buffer = handler->m_buff;
	ok = handler->IsCompleted();
	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}
