#include "stdafx.h"
#include "phpmanager.h"
#include "phpsocketpool.h"
#include "phpcert.h"

namespace PA
{

    CPoolStartContext::CPoolStartContext()
            : SvsId(0), Threads(1), AutoConn(true), AutoMerge(false),
            RecvTimeout(SPA::ClientSide::DEFAULT_RECV_TIMEOUT),
            ConnTimeout(SPA::ClientSide::DEFAULT_CONN_TIMEOUT),
            PhpHandler(nullptr), PoolType(tagPoolType::Regular), m_errCode(0) {
    }

    void CPoolStartContext::Clean() {
        if (PhpHandler) {
            switch (SvsId) {
                case (unsigned int) SPA::tagServiceID::sidODBC:
                case SPA::Sqlite::sidSqlite:
                case SPA::Mysql::sidMysql:
                    delete PhpDb;
                    break;
                case (unsigned int) SPA::tagServiceID::sidChat:
                    delete PhpQueue;
                    break;
                case (unsigned int) SPA::tagServiceID::sidFile:
                    delete PhpFile;
                    break;
                default:
                    delete PhpHandler;
                    break;
            }
            PhpHandler = nullptr;
        }
    }

    bool CPoolStartContext::DoSSLAuth(CClientSocket * cs) {
        SPA::IUcert *cert = cs->GetUCert();
        if (!cert) {
            assert(false); //shouldn't come here
            //permission not given
            return false;
        }
        if (!cert->Validity) {
            m_errCode = -1;
            m_errMsg = "Invalid certificate found";
            //permission not given
            return false;
        }
        m_errMsg = cert->Verify(&m_errCode);
        if (!m_errCode) {
            //server certificate is authenticated against certificate store and no issue is found
            //permission given
            return true;
        }
        std::string pk = CPhpCert::ToString(cert->PublicKey, cert->PKSize);
        const auto &vKA = CPhpManager::Manager.m_vKeyAllowed;
        //check server certificate public key against an array of allowed public keys
        if (std::find(vKA.cbegin(), vKA.cend(), pk) == vKA.cend()) {
            //permission not given
            return false;
        }
        m_errCode = 0;
        m_errMsg.clear();
        //permission given
        return true;
    }

#ifdef REQUIRE_POOL_EVENT

    void CPoolStartContext::DealWithPoolEvents(tagSocketPoolEvent spe) {
        switch (spe) {
            case SPA::ClientSide::speThreadCreated:
                break;
            case SPA::ClientSide::speKillingThread:
                break;
            default:
                break;
        }
    }
#endif

    std::string CPoolStartContext::StartPool() {
        assert(!PhpHandler);
        std::wstring dfltDb = SPA::Utilities::ToWide(DefaultDb);
        switch (SvsId) {
            case (unsigned int) SPA::tagServiceID::sidODBC:
            case SPA::Sqlite::sidSqlite:
            case SPA::Mysql::sidMysql:
                switch (PoolType) {
                    case PA::tagPoolType::Regular:
                        PhpDb = new CPhpDbPool(AutoConn, RecvTimeout, ConnTimeout, SvsId);
                        break;
                    case PA::tagPoolType::Slave:
                        PhpDb = new CSQLMaster::CSlavePool(dfltDb.c_str(), RecvTimeout, SvsId);
                        PhpDb->SetConnTimeout(ConnTimeout);
                        PhpDb->SetAutoConn(AutoConn);
                        break;
                    case PA::tagPoolType::Master:
                        PhpDb = new CSQLMaster(dfltDb.c_str(), RecvTimeout, SvsId);
                        PhpDb->SetConnTimeout(ConnTimeout);
                        PhpDb->SetAutoConn(AutoConn);
                        break;
                    default:
                        assert(false);
                        break;
                }
                if (Queue.size()) {
                    PhpDb->SetQueueName(Queue.c_str());
                }
                PhpDb->DoSslServerAuthentication = [this](CPhpDbPool *pool, CClientSocket * cs)->bool {
                    return this->DoSSLAuth(cs);
                };
#ifdef REQUIRE_POOL_EVENT
                PhpDb->SocketPoolEvent = [this](CPhpDbPool *pool, tagSocketPoolEvent spe, CDBHandler * handler) {
                    this->DealWithPoolEvents(spe);
                };
#endif
                break;
            case (unsigned int) SPA::tagServiceID::sidChat:
                PhpQueue = new CPhpQueuePool(AutoConn, RecvTimeout, ConnTimeout, SvsId);
                PhpQueue->DoSslServerAuthentication = [this](CPhpQueuePool *pool, CClientSocket * cs)->bool {
                    return this->DoSSLAuth(cs);
                };
#ifdef REQUIRE_POOL_EVENT
                PhpQueue->SocketPoolEvent = [this](CPhpQueuePool *pool, tagSocketPoolEvent spe, CAsyncQueue * handler) {
                    this->DealWithPoolEvents(spe);
                };
#endif
                if (Queue.size()) {
                    PhpQueue->SetQueueName(Queue.c_str());
                }
                break;
            case (unsigned int) SPA::tagServiceID::sidFile:
                PhpFile = new CPhpFilePool(AutoConn, RecvTimeout, ConnTimeout, SvsId);
                PhpFile->DoSslServerAuthentication = [this](CPhpFilePool *pool, CClientSocket * cs)->bool {
                    return this->DoSSLAuth(cs);
                };
#ifdef REQUIRE_POOL_EVENT
                PhpFile->SocketPoolEvent = [this](CPhpFilePool *pool, tagSocketPoolEvent spe, CAsyncFile * handler) {
                    this->DealWithPoolEvents(spe);
                };
#endif
                if (Queue.size()) {
                    PhpFile->SetQueueName(Queue.c_str());
                }
                break;
            default:
                switch (PoolType) {
                    case PA::tagPoolType::Regular:
                        PhpHandler = new CPhpPool(AutoConn, RecvTimeout, ConnTimeout, SvsId);
                        break;
                    case PA::tagPoolType::Slave:
                        PhpHandler = new CMasterPool::CSlavePool(dfltDb.c_str(), RecvTimeout, SvsId);
                        PhpHandler->SetConnTimeout(ConnTimeout);
                        PhpHandler->SetAutoConn(AutoConn);
                        break;
                    case PA::tagPoolType::Master:
                        PhpHandler = new CMasterPool(dfltDb.c_str(), RecvTimeout, SvsId);
                        PhpHandler->SetConnTimeout(ConnTimeout);
                        PhpHandler->SetAutoConn(AutoConn);
                        break;
                    default:
                        assert(false);
                        break;
                }
                PhpHandler->DoSslServerAuthentication = [this](CPhpPool *pool, CClientSocket * cs)->bool {
                    return this->DoSSLAuth(cs);
                };
#ifdef REQUIRE_POOL_EVENT
                PhpHandler->SocketPoolEvent = [this](CPhpPool *pool, tagSocketPoolEvent spe, CAsyncHandler * handler) {
                    this->DealWithPoolEvents(spe);
                };
#endif
                if (Queue.size()) {
                    PhpHandler->SetQueueName(Queue.c_str());
                }
                break;
        }

        std::vector<CConnectionContext> v;
        for (const auto &h : Hosts) {
            v.push_back(CPhpManager::Manager.FindByKey(h));
        }
        typedef CConnectionContext *PCConnectionContext;
        unsigned int threads = Threads;
        unsigned int socketsPerThread = (unsigned int) Hosts.size();
        std::shared_ptr<PCConnectionContext> ppCC(new PCConnectionContext[threads], [](PCConnectionContext * p) {
            if (p) {
                delete[]p;
            }
        });
        PCConnectionContext *ppCCs = ppCC.get();
        for (unsigned int n = 0; n < threads; ++n) {
            ppCCs[n] = v.data();
        }
        bool ok = false;
        switch (SvsId) {
            case SPA::Mysql::sidMysql:
            case SPA::Odbc::sidOdbc:
            case SPA::Sqlite::sidSqlite:
                ok = PhpDb->StartSocketPool(ppCCs, socketsPerThread, threads);
                if (AutoMerge) {
                    PhpDb->SetQueueAutoMerge(true);
                }
                break;
            case SPA::Queue::sidQueue:
                ok = PhpQueue->StartSocketPool(ppCCs, socketsPerThread, threads);
                if (AutoMerge) {
                    PhpQueue->SetQueueAutoMerge(true);
                }
                break;
            case SPA::SFile::sidFile:
                ok = PhpFile->StartSocketPool(ppCCs, socketsPerThread, threads);
                if (AutoMerge) {
                    PhpFile->SetQueueAutoMerge(true);
                }
                break;
            default:
                ok = PhpHandler->StartSocketPool(ppCCs, socketsPerThread, threads);
                if (AutoMerge) {
                    PhpHandler->SetQueueAutoMerge(true);
                }
                break;
        }
        if (!ok && !m_errMsg.size()) {
            auto cs = PhpHandler->GetSockets()[0];
            m_errCode = cs->GetErrorCode();
            m_errMsg = "No connection to anyone of remote servers";
        }
        return m_errMsg;
    }

    Php::Value CPoolStartContext::GetPool() {
        if (!PhpHandler) {
            std::string errMsg = StartPool();
            if (errMsg.size()) {
                CPhpManager::Manager.SetErrorMsg(errMsg);
                return errMsg;
            }
        } else if (!Queue.size() && PhpHandler->GetConnectedSockets() == 0) {
            std::string errMsg = "No connection to anyone of remote servers";
            CPhpManager::Manager.SetErrorMsg(errMsg);
            return errMsg;
        }
        return Php::Object((SPA_CS_NS + PHP_SOCKET_POOL).c_str(), new CPhpSocketPool(*this));
    }
}
