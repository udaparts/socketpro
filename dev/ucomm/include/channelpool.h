#ifndef _U_SPA_CHANNEL_POOL_H_
#define _U_SPA_CHANNEL_POOL_H_

#include "../include/rawclient.h"
#include <vector>
#include <functional>

namespace SPA {

    class CBaseHandler {
    protected:

        CBaseHandler(SessionHandle sh) : m_session(sh) {

        }

        virtual ~CBaseHandler() {

        }

    public:

        virtual bool Connect(const char *host, unsigned int port, tagEncryptionMethod secure, bool v6, bool sync, unsigned int msTimeout = 30000) {
            return SH_Connect(m_session, host, port, secure, v6, sync, msTimeout);
        }

        virtual int Send(const unsigned char *data, unsigned int bytes) {
            return SH_Send(m_session, data, bytes);
        }

        virtual bool Shutdown(tagShutdownType st = tagShutdownType::stBoth) {
            return SH_Shutdown(m_session, st);
        }

        virtual void Close() {
            SH_Close(m_session);
        }

        int GetErrorCode(char *em, unsigned int len) const {
            return SH_GetErrorCode(m_session, em, len);
        }

        bool IsConnected() const {
            return SH_IsConnected(m_session);
        }

        unsigned int GetOutBufferSize() const {
            return SH_GetOutBufferSize(m_session);
        }

        IUcert* GetUCert() const {
            return SH_GetUCert(m_session);
        }

        bool GetPeerName(unsigned int *port, char *addr, unsigned int chars) const {
            return SH_GetPeerName(m_session, port, addr, chars);
        }

        SessionHandle GetBaseHandle() const {
            return m_session;
        }

    protected:
        virtual void OnAvailable(const unsigned char *data, unsigned int bytes) = 0;

    private:
        SessionHandle m_session;
        template<typename THandler>
        friend class CSessionPool;
    };

    template<typename THandler>
    class CSessionPool {
    protected:
        typedef CSpinAutoLock CAutoLock;

    public:

        CSessionPool(unsigned int handlers, tagThreadApartment ta = tagThreadApartment::taNone) : m_sph(CreateASessionPool(DA, SCE, 0, ta)) {
            {
                CAutoLock al(m_cs);
                m_vP.push_back(this);
            }
            for (unsigned int n = 0; n < handlers; ++n) {
                SPH_AddSession(m_sph);
            }
        }

        virtual ~CSessionPool() {
            SPH_Kill(m_sph);
            {
                CAutoLock al(m_cs);
                for (auto it = m_vP.begin(), end = m_vP.end(); it != end; ++it) {
                    if (*it == this) {
                        m_vP.erase(it);
                        break;
                    }
                }
            }
            DestroyASessionPool(m_sph);
        }

        CSessionPool(const CSessionPool&) = delete;

        typedef std::function<void(tagSessionPoolEvent spe, THandler *h) > DSessionPoolEvent;

    public:
        CSessionPool& operator=(const CSessionPool&) = delete;

        virtual bool Kill() const {
            return SPH_Kill(m_sph);
        }

        virtual THandler* Lock(unsigned int msTimeout = (unsigned int) (~0)) {
            return FindHandler(SPH_Lock(m_sph, msTimeout));
        }

        virtual bool Unlock(THandler *handler) const {
            if (!handler) {
                return false;
            }
            return SPH_Unlock(m_sph, handler->m_session);
        }

        virtual void CloseAll() const {
            SPH_CloseAll(m_sph);
        }

        virtual unsigned int ConnectAll(const char *host, unsigned int port, tagEncryptionMethod secure = tagEncryptionMethod::NoEncryption, bool v6 = false) const {
            return SPH_ConnectAll(m_sph, host, port, secure, v6);
        }

        unsigned int GetHandlers() const {
            return SPH_GetSessions(m_sph);
        }

        bool AddHandler() const {
            return SPH_AddSession(m_sph);
        }

        THandler* FindAClosedHandler() {
            SessionHandle sh = SPH_FindAClosedSession(m_sph);
            if (!sh) {
                return nullptr;
            }
            return FindHandler(sh);
        }

        unsigned int GetConnectedHandlers() const {
            return SPH_GetConnectedSessions(m_sph);
        }

        SessionPoolHandle GetBaseHandle() const {
            return m_sph;
        }


    protected:

        THandler* FindHandler(SessionHandle sh) {
            CAutoLock al(m_sl);
            for (auto it = m_vHandlers.cbegin(), end = m_vHandlers.cend(); it != end; ++it) {
                if ((*it)->m_session == sh) {
                    return *it;
                }
            }
            return nullptr;
        }

    private:

        static void CALLBACK SCE(SessionPoolHandle sph, tagSessionPoolEvent spe, SessionHandle sh) {
            CSessionPool *sp;
            {
                CAutoLock al(m_cs);
                sp = FindPool(sph);
                if (!sp) {
                    return;
                }
            }
            switch (spe) {
                case tagSessionPoolEvent::seCreatingThread:
                case tagSessionPoolEvent::seThreadCreated:
                case tagSessionPoolEvent::seConnected:
                case tagSessionPoolEvent::seKillingThread:
                case tagSessionPoolEvent::seShutdown:
                case tagSessionPoolEvent::seSslShaking:
                case tagSessionPoolEvent::seLocked:
                case tagSessionPoolEvent::seUnlocked:
                case tagSessionPoolEvent::seThreadDestroyed:
                case tagSessionPoolEvent::seSessionClosed:
                    if (sp->PoolEvent) {
                        sp->PoolEvent(spe, sp->FindHandler(sh));
                    }
                    break;
                case tagSessionPoolEvent::seSessionDestroyed:
                    if (sp->PoolEvent) {
                        sp->PoolEvent(spe, sp->FindHandler(sh));
                    }
                {
                    CAutoLock al(sp->m_sl);
                    for (auto it = sp->m_vHandlers.begin(), end = sp->m_vHandlers.end(); it != end; ++it) {
                        THandler *h = *it;
                        if (h->m_session == sh) {
                            sp->m_vHandlers.erase(it);
                            delete h;
                            break;
                        }
                    }
                }
                    break;
                case tagSessionPoolEvent::seSessionCreated:
                {
                    THandler *h = new THandler(sh);
                    CAutoLock al(sp->m_sl);
                    sp->m_vHandlers.push_back(h);
                }
                    if (sp->PoolEvent) {
                        sp->PoolEvent(spe, sp->FindHandler(sh));
                    }
                    break;
                default:
                    break;
            }
        }

        static void CALLBACK DA(SessionPoolHandle sph, SessionHandle sh, const unsigned char* data, unsigned int bytes) {
            CSessionPool *sp;
            {
                CAutoLock al(m_cs);
                sp = FindPool(sph);
                if (!sp) {
                    return;
                }
            }
            CBaseHandler *h = sp->FindHandler(sh);
            if (h) {
                h->OnAvailable(data, bytes);
            }
        }

        static CSessionPool *FindPool(SessionPoolHandle sph) {
            for (auto it = m_vP.begin(), end = m_vP.end(); it != end; ++it) {
                if ((*it)->m_sph == sph) {
                    return *it;
                }
            }
            return nullptr;
        }



    public:
        DSessionPoolEvent PoolEvent;

    protected:
        CSpinLock m_sl;
        std::vector<THandler*> m_vHandlers; //protected by m_sl

    private:
        SessionPoolHandle m_sph;
        static CSpinLock m_cs;
        static std::vector<CSessionPool<THandler>*> m_vP; //protected by m_cs
    };

    template<typename THandler>
    CSpinLock CSessionPool<THandler>::m_cs;

    template<typename THandler>
    std::vector<CSessionPool<THandler>*> CSessionPool<THandler>::m_vP;
}

#endif
