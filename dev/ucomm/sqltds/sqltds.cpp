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
	SPA::CScopeUQueue m_sb;

public:
	CTdsClient(SessionHandle sh) : CBaseHandler(sh), m_buff(*m_sb) {
	}

public:
	virtual int Send(const unsigned char* data, unsigned int bytes, void* sender) {
		tds::CReqBase* rb = reinterpret_cast<tds::CReqBase*>(sender);
		assert(rb);
		std::lock_guard<std::mutex> al(m_csSender);
		m_cs.lock();
		m_deq.push_back(rb);
		m_cs.unlock();
		int fail = CBaseHandler::Send(data, bytes, sender);
		if (fail) {
			m_cs.lock();
			m_deq.pop_back();
			m_cs.unlock();
		}
		return fail;
	}

protected:
	void OnAvailable(const unsigned char *data, unsigned int bytes) {
		m_buff.Push(data, bytes);
		do {
			if (m_buff.GetSize() < sizeof(tds::PacketHeader)) {
				break;
			}
			tds::PacketHeader *ph = (tds::PacketHeader*)m_buff.GetBuffer();
			unsigned int len = tds::ChangeEndian(ph->Length);
			if (m_buff.GetSize() < len) {
				break;
			}
			{
				SPA::CSpinAutoLock al(m_cs);
#ifndef NDEBUG
				if (!m_deq.size()) {
					assert(false);
					break;
				}
#endif
				tds::CReqBase* rb = m_deq.front();
				rb->OnResponse(m_buff.GetBuffer(), len);
				if (ph->Status == tds::tagPacketStatus::psEOM) {
					m_deq.pop_front();
				}
			}
			m_buff.Pop(len);
		} while (false);
	}

private:
	std::mutex m_csSender;
	SPA::CSpinLock m_cs;
	std::deque<tds::CReqBase*> m_deq;
	CUQueue &m_buff;
};

void ShowBuffer(const SPA::CUQueue &buffer);

int main() {
	CSessionPool<CTdsClient> pool(1);
	auto handler = pool.FindAClosedHandler();
	bool ok = handler->Connect("windesk", 1433, tagEncryptionMethod::NoEncryption, false, true);

	char serverName[128];
	handler->GetServerName(serverName, sizeof(serverName));

	tds::CPrelogin pl(*handler);
	tds::CLogin7 login(*handler);
	//tds::CTransManager tmBegin(*handler);
	//tds::CTransManager tmEnd(*handler);

	int res = pl.SendMessage();

	tds::SqlLogin rec;
	rec.database = u"sakila";
	rec.timeout = 11;
	rec.userName = u"sa";
	rec.password = u"Smash123";
	rec.serverName = tds::CDBString(serverName, serverName + strlen(serverName));
	tds::CLogin7::FeatureExtension fe;
	res = login.SendMessage(rec, fe);

	tds::CSqlBatch sqlbatch(*handler);
	res = sqlbatch.SendMessage(u"select * from actor where actor_id=10");

	unsigned int parameters;
	CParameterInfoArray vPInfo;
	CDBString errMsg;
	res = sqlbatch.Prepare(u"select * from actor where actor_id=? and first_name<>?;select * from actor where actor_id=? and first_name<>?", vPInfo, parameters);
	ok = sqlbatch.Wait(1500);

	CDBVariantArray vParam;
	vParam.push_back(1);
	vParam.push_back("NICK");
	vParam.push_back(2);
	vParam.push_back("NICK");
	res = sqlbatch.SendMessage(vParam);

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
