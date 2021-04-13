#include "pch.h"
#include "rawthread.h"

SPA::SessionPoolHandle WINAPI CreateASessionPool(SPA::PDataArrive da, SPA::PSessionCallback sc, unsigned int sessions, SPA::tagThreadApartment ta) {
	if (!da) {
        return nullptr;
    }
    return new SPA::CRawThread(da, sc, sessions, ta);
}

void WINAPI DestroyASessionPool(SPA::SessionPoolHandle sph) {
    delete sph;
}

//SessionPoolHandle

unsigned int WINAPI SPH_GetSessions(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return 0;
    }
    return rt->GetSessions();
}

bool WINAPI SPH_AddSession(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return false;
    }
    return rt->AddSession();
}

SPA::SessionHandle WINAPI SPH_FindAClosedSession(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return nullptr;
    }
    return rt->FindAClosedSession();
}

unsigned int WINAPI SPH_GetConnectedSessions(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return 0;
    }
    return rt->GetConnectedSessions();
}

SPA::SessionHandle WINAPI SPH_Lock(SPA::SessionPoolHandle sph, unsigned int msTimeout) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return nullptr;
    }
    return rt->Lock(msTimeout);
}

bool WINAPI SPH_Unlock(SPA::SessionPoolHandle sph, SPA::SessionHandle session) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return false;
    }
    return rt->Unlock(session);
}

void WINAPI SPH_CloseAll(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return;
    }
    rt->CloseAll();
}

unsigned int WINAPI SPH_ConnectAll(SPA::SessionPoolHandle sph, const char *host, unsigned int port, SPA::tagEncryptionMethod secure, bool v6) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return 0;
    }
    return rt->ConnectAll(host, port, secure, v6);
}

bool WINAPI SPH_Kill(SPA::SessionPoolHandle sph) {
    SPA::CRawThread *rt = (SPA::CRawThread*)sph;
    if (!rt) {
        return false;
    }
    return rt->Kill();
}


//Session

bool WINAPI SH_Connect(SPA::SessionHandle session, const char *host, unsigned int port, SPA::tagEncryptionMethod secure, bool v6, bool sync, unsigned int timeout) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return false;
    }
    return rs->Connect(host, port, secure, v6, sync, timeout);
}

bool WINAPI SH_Shutdown(SPA::SessionHandle session, SPA::tagShutdownType st) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return false;
    }
    return rs->Shutdown(st);
}

int WINAPI SH_GetErrorCode(SPA::SessionHandle session, char *em, unsigned int len) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return 0;
    }
    return rs->GetErrorCode(em, len);
}

bool WINAPI SH_IsConnected(SPA::SessionHandle session) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return false;
    }
    return rs->IsConnected();
}

void WINAPI SH_Close(SPA::SessionHandle session) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return;
    }
    rs->Close();
}

int WINAPI SH_Send(SPA::SessionHandle session, const unsigned char *data, unsigned int bytes) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return -1;
    }
    return rs->Send(data, bytes);
}

SPA::IUcert* WINAPI SH_GetUCert(SPA::SessionHandle session) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return nullptr;
    }
    return rs->GetUCert();
}

unsigned int WINAPI SH_GetOutBufferSize(SPA::SessionHandle session) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return 0;
    }
    return rs->GetOutBufferSize();
}

bool WINAPI SH_GetPeerName(SPA::SessionHandle session, unsigned int *port, char *addr, unsigned int chars) {
    SPA::CRawSession *rs = (SPA::CRawSession*)session;
    if (!rs) {
        return false;
    }
    return rs->GetPeerName(port, addr, chars);
}
