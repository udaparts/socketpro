#include "pch.h"
#include "rawsession.h"
#include "rawthread.h"

CRawSession::CRawSession(CIoService &IoService, CRawThread *rt) : m_rt(rt) {
	auto sc = rt->GetSessionCallback();
	if (sc) {
		sc(rt, tagSessionEvent::seSessionCreated, this);
	}
}

CRawSession::~CRawSession() {
	auto sc = m_rt->GetSessionCallback();
	if (sc) {
		//bool chatting = false;
		//CRAutoLock sl(m_mutex, chatting);
		sc(m_rt, tagSessionEvent::seSessionKilled, this);
	}
}
