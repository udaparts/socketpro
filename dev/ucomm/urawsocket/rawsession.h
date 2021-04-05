#ifndef _U_RAW_SESSION_H_
#define _U_RAW_SESSION_H_

#include "rawclient.h"
#include "../core_shared/shared/includes.h"

class CRawThread;

class CRawSession : public USessionBase {

public:
	CRawSession(CIoService &IoService, CRawThread *rt);
	CRawSession(const CRawSession &rs) = delete;
	~CRawSession();

public:
	CRawSession& operator=(const CRawSession &rs) = delete;

private:
	CRawThread *m_rt;
};

typedef CRawSession *PRawSession;

#endif