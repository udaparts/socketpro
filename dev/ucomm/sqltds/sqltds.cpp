#include <time.h>
#include "../include/membuffer.h"
#include "../include/channelpool.h"
#include "prelogin.h"
#include "login7.h"
#include "sqlbatch.h"
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
			rb->OnResponse(m_buff.GetBuffer(), len);
			m_buff.Pop(len);
			if (ph->Status == tds::tagPacketStatus::psEOM) {
				m_deq.pop_front();
			}
		} while (false);
	}
};

void ShowBuffer(const SPA::CUQueue &buffer);
std::vector<unsigned char> GetSSPI();

struct MyDateTime {
	SPA::UINT64 MicroSecond : 20;
	SPA::UINT64 Second : 6;
	SPA::UINT64 Minute : 6;
	SPA::UINT64 Hour : 5;
	SPA::UINT64 Day : 5;
	SPA::UINT64 Month : 4;
	SPA::UINT64 Year : 18;
};
static_assert(sizeof(MyDateTime) == sizeof(SPA::UINT64), "Wrong MyDateTime size");


union MyDateTime2 {
	MyDateTime DateTime;
	SPA::UINT64 time = 0;
};
static_assert(sizeof(MyDateTime2) == sizeof(SPA::UINT64), "Wrong MyDateTime2 size");


int main() {
	unsigned int fraction = pow(10, 7);

	int jdn1 = tds::ToTdsJDN(1900, 1, 1);

	tds::CPrelogin pl(false);
	tds::CLogin7 login;
	//tds::CTransManager tmBegin;
	tds::CSqlBatch sqlbatch(true);
	//tds::CTransManager tmEnd;

	MyDateTime2 mdt;
	mdt.time = 8532668707717584;
/*
	MyDateTime &dt = mdt.DateTime;
	dt.Year = 121;
	dt.Month = 4;
	dt.Day = 3;
	dt.Hour = 10;
	dt.Minute = 47;
	dt.Second = 52;
	dt.MicroSecond = 999888;
*/
	SPA::UINT64 value = mdt.time;

	SPA::CScopeUQueue sb;
	CSessionPool<CTdsClient> pool(1);
	auto handler = pool.FindAClosedHandler();
	bool ok = handler->Connect("windesk", 1433, tagEncryptionMethod::NoEncryption, false, true);

	char serverName[128];
	handler->GetServerName(serverName, sizeof(serverName));

	handler->m_deq.push_back(&pl);
	handler->m_deq.push_back(&login);
	//handler->m_deq.push_back(&tmBegin);
	handler->m_deq.push_back(&sqlbatch);
	//handler->m_deq.push_back(&tmEnd);

	ok = pl.GetClientMessage(*sb);
	int res = handler->Send(sb->GetBuffer(), sb->GetSize());
	//ShowBuffer(*sb);
	sb->SetSize(0);

	tds::SqlLogin rec;
	rec.database = u"sqltestdb";
	rec.timeout = 11;
	rec.userName = u"sa";
	rec.password = u"Smash123";
	rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));
	tds::CLogin7::FeatureExtension fe;
	ok = login.GetClientMessage(rec, fe, *sb);
	//ShowBuffer(*sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());
	sb->SetSize(0);

	//ok = tmBegin.GetClientMessage(tds::CTransManager::tagRequestType::rtBeginTrans, tds::CTransManager::tagIsolationLevel::ilReadCommitted, 0, *sb);
	//res = handler->Send(sb->GetBuffer(), sb->GetSize());
	////ShowBuffer(*sb);
	//sb->SetSize(0);
	//ok = tmBegin.Wait(100);

	//sqlbatch.GetClientMessage(u"SELECT testid,myguid,mydate,myvariant,mybool,mymoney,mytinyint,mydateimeoffset,mytime,mydatetime2,mysmallmoney,mysmalldatetime,mynum,mybinary,myntext,myhid,mytimestamp,myxml FROM test_rare1", *sb);
	//ShowBuffer(*sb);
	//sqlbatch.GetClientMessage(u"SELECT * FROM SpatialTable;select * from test_rare1;select * from company;select * from employee;select * from pet;select * from vtest", *sb, tmBegin.GetTransDescriptor());
	//sqlbatch.GetClientMessage(u"exec sp_sproc_columns sp_TestProc;SELECT * FROM SpatialTable;select * from test_rare1;select * from company;select * from employee", *sb, tmBegin.GetTransDescriptor());

	sqlbatch.GetClientMessage(u"SELECT * from test_rare1", *sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());
	sb->SetSize(0);

	/*ok = tmEnd.GetClientMessage(tds::CTransManager::tagRequestType::rtRollback, tds::CTransManager::tagIsolationLevel::ilCurrent, tmBegin.GetTransDescriptor(), *sb);
	res = handler->Send(sb->GetBuffer(), sb->GetSize());*/

	std::cout << "Press a key to shut down the application ......\n";
	::getchar();
	return 0;
}

void ShowBuffer(const SPA::CUQueue &buffer) {
	int n = 0, len = (int)buffer.GetSize();
	const unsigned char *data = buffer.GetBuffer();
	for (n = 0; n < 8; ++n) {
		char str[8] = { 0 };
		sprintf_s(str, "%02X", data[n]);
		if (n == 7) {
			std::cout << str << "\n";
		}
		else {
			std::cout << str << " ";
		}
	}
	for (; n < len; ++n) {
		char str[8] = { 0 };
		sprintf_s(str, "%02X", data[n]);
		if (((n - 7) % 16) == 0) {
			std::cout << str << "\n";
		}
		else {
			std::cout << str << " ";
		}
	}
	std::cout << "\n";
}

std::vector<unsigned char> GetSSPI() {
	std::vector<unsigned char> vSSPI;

	return vSSPI;
}