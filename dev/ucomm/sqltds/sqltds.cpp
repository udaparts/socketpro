#include <time.h>
#include "../include/membuffer.h"
#include "../include/channelpool.h"
#include "prelogin.h"
#include "login7.h"
#include <deque>
#include <iostream>
#include <ws2tcpip.h>

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

void ShowBuffer(const SPA::CUQueue &buffer);
std::vector<unsigned char> GetSSPI();

int main()
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	struct addrinfo* resAddr;
	int ret = getaddrinfo("acer", nullptr, &hints, &resAddr);

	tds::CPrelogin pl(true);
	tds::CLogin7 login;

	SPA::CScopeUQueue sb;
	CSessionPool<CTdsClient> pool(1);
	auto handler = pool.FindAClosedHandler();
	bool ok = handler->Connect("acer", 1433, tagEncryptionMethod::NoEncryption, false, true);

	char serverName[128];
	handler->GetServerName(serverName, sizeof(serverName));

	handler->m_deq.push_back(&pl);
	handler->m_deq.push_back(&login);
	ok = pl.GetClientMessage(1, *sb);
	int res = handler->Send(sb->GetBuffer(), sb->GetSize());
	//ShowBuffer(*sb);
	sb->SetSize(0);
	tds::SqlLogin rec;
	rec.database = u"sakila";
	rec.timeout = 11;
	rec.userName = u"CharlieYe";
	rec.password = u"Smash123";
	rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));
	tds::CLogin7::FeatureExtension fe;
	ok = login.GetClientMessage(2, rec, fe, *sb);
	//ShowBuffer(*sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());
	/*
	sb->SetSize(0);
	tds::CPrelogin pl2(true);
	tds::CLogin7 login2;
	handler->m_deq.push_back(&pl2);
	handler->m_deq.push_back(&login2);
	ok = pl2.GetClientMessage(3, *sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());
	sb->SetSize(0);
	rec.userName = u"sa";
	rec.database = u"sqltestdb";
	ok = login2.GetClientMessage(4, rec, fe, *sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());
	*/
	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}

void ShowBuffer(const SPA::CUQueue &buffer) {
	int n = 0, len = (int)buffer.GetSize();
	const unsigned char *data = buffer.GetBuffer();
	for (n = 0; n < 8; ++n)
	{
		char str[8] = { 0 };
		sprintf_s(str, "%02X", data[n]);
		if (n == 7)
		{
			std::cout << str << "\n";
		}
		else
		{
			std::cout << str << " ";
		}
	}
	for (; n < len; ++n)
	{
		char str[8] = { 0 };
		sprintf_s(str, "%02X", data[n]);
		if (((n - 7) % 16) == 0)
		{
			std::cout << str << "\n";
		}
		else
		{
			std::cout << str << " ";
		}
	}
	std::cout << "\n";
}

std::vector<unsigned char> GetSSPI() {
	std::vector<unsigned char> vSSPI;

	return vSSPI;
}