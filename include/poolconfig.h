#ifndef _SPA_CLIENTSIDE_POOL_CONFIG_H_
#define _SPA_CLIENTSIDE_POOL_CONFIG_H_

#include <map>
#include "aqhandler.h"
#include "async_sqlite.h"
#include "async_odbc.h"
#include "async_mysql.h"
#include "streamingfile.h"
#include "rdbcache.h"
#include "masterpool.h"

namespace SPA {
    namespace ClientSide {

        enum tagPoolType {
            Regular = 0,
            Slave = 1,
            Master = 2
        };

        class CPoolConfig;

        typedef std::function<void(CPoolConfig *pc, tagSocketPoolEvent spe) > DSpManagerPoolEvent;

        typedef std::unordered_map<std::string, CConnectionContext> CMapHost;

        class CPoolConfig {
        public:

            CPoolConfig()
            : SvsId(0), Threads(1), AutoConn(true), AutoMerge(false),
            RecvTimeout(DEFAULT_RECV_TIMEOUT), ConnTimeout(DEFAULT_CONN_TIMEOUT),
            PoolType(Regular), m_errCode(0) {
            }
            unsigned int SvsId;
            std::vector<std::string> Hosts;
            unsigned int Threads;
            std::string Queue;
            bool AutoConn;
            bool AutoMerge;
            unsigned int RecvTimeout;
            unsigned int ConnTimeout;
            std::string DefaultDb;
            typedef std::map<std::string, CPoolConfig> CMapPool;
            CMapPool Slaves;
            tagPoolType PoolType;
            std::shared_ptr<void> Pool;

            static DSpManagerPoolEvent PoolEvent;
            static std::vector<std::string> KeysAllowed;

            typedef CCachedBaseHandler<0> CMyHandler;
            typedef CSocketPool<CMyHandler> CMyPool;

        private:
            int m_errCode;
            std::string m_errMsg;

        public:

            int GetErrCode() const {
                return m_errCode;
            }

            template<bool midTier>
            std::string StartPool(const CMapHost &mapHost) {
                if (Pool)
                    return "";
                bool ok = false;
                typedef CConnectionContext *PCConnectionContext;
                unsigned int threads = Threads;
                unsigned int socketsPerThread = (unsigned int) Hosts.size();
                std::shared_ptr<PCConnectionContext> ppCC(new PCConnectionContext[threads], [](PCConnectionContext * p) {
                    if (p) {
                        delete[]p;
                    }
                });
                std::vector<CConnectionContext> vHost;
                for (auto it = Hosts.begin(), end = Hosts.end(); it != end; ++it) {
                    const auto &value = mapHost.at(*it);
                    vHost.push_back(value);
                }
                PCConnectionContext *ppCCs = ppCC.get();
                for (unsigned int n = 0; n < threads; ++n) {
                    ppCCs[n] = vHost.data();
                }
                std::wstring dfltDb = SPA::Utilities::ToWide(DefaultDb);
                CClientSocket *cs = nullptr;
                switch (SvsId) {
                    case SPA::Sqlite::sidSqlite:
                    {
                        typedef CSocketPool<CSqlite> CDbPool;
                        typedef CSQLMasterPool<midTier, CSqlite> CSQLMaster;
                        CDbPool *db = nullptr;
                        switch (PoolType) {
                            case Master:
                                db = new CSQLMaster(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                db = new CSQLMaster::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CDbPool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CDbPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CDbPool *pool, tagSocketPoolEvent spe, CDbPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CDbPool*) p;
                        });
                    }
                        break;
                    case SPA::Mysql::sidMysql:
                    {
                        typedef CSocketPool<CMysql> CDbPool;
                        typedef CSQLMasterPool<midTier, CMysql> CSQLMaster;
                        CDbPool *db = nullptr;
                        switch (PoolType) {
                            case Master:
                                db = new CSQLMaster(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                db = new CSQLMaster::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CDbPool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CDbPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CDbPool *pool, tagSocketPoolEvent spe, CDbPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CDbPool*) p;
                        });
                    }
                        break;
                    case SPA::Odbc::sidOdbc:
                    {
                        typedef CSocketPool<COdbc> CDbPool;
                        typedef CSQLMasterPool<midTier, COdbc> CSQLMaster;
                        CDbPool *db = nullptr;
                        switch (PoolType) {
                            case Master:
                                db = new CSQLMaster(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                db = new CSQLMaster::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CDbPool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CDbPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CDbPool *pool, tagSocketPoolEvent spe, CDbPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CDbPool*) p;
                        });
                    }
                        break;
                    case SPA::Queue::sidQueue:
                    {
                        typedef CSocketPool<CAsyncQueue> CPool;
                        CPool *pool = new CPool(AutoConn, RecvTimeout, ConnTimeout);
                        if (Queue.size()) {
                            pool->SetQueueName(Queue.c_str());
                        }
                        pool->DoSslServerAuthentication = [this](CPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        pool->SocketPoolEvent = [this](CPool *p, tagSocketPoolEvent spe, CPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = pool->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            pool->SetQueueAutoMerge(true);
                        }
                        cs = pool->GetSockets()[0].get();
                        Pool.reset(pool, [](void *p) {
                            delete (CPool*) p;
                        });
                    }
                        break;
                    case SPA::SFile::sidFile:
                    {
                        typedef CSocketPool<CStreamingFile> CPool;
                        CPool *pool = new CPool(AutoConn, RecvTimeout, ConnTimeout);
                        if (Queue.size()) {
                            pool->SetQueueName(Queue.c_str());
                        }
                        pool->DoSslServerAuthentication = [this](CPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        pool->SocketPoolEvent = [this](CPool *p, tagSocketPoolEvent spe, CPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = pool->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            pool->SetQueueAutoMerge(true);
                        }
                        cs = pool->GetSockets()[0].get();
                        Pool.reset(pool, [](void *p) {
                            delete (CPool*) p;
                        });
                    }
                        break;
                    default:
                    {
                        typedef CMasterPool<midTier, CMyPool::Handler> CMaster;
                        CMyPool *db = nullptr;
                        switch (PoolType) {
                            case Master:
                                db = new CMaster(dfltDb.c_str(), RecvTimeout, SvsId);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                db = new CMaster::CSlavePool(dfltDb.c_str(), RecvTimeout, SvsId);
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CMyPool(AutoConn, RecvTimeout, ConnTimeout, SvsId);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CMyPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CMyPool *pool, tagSocketPoolEvent spe, CMyPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CMyPool*) p;
                        });
                    }
                        break;
                }
                if (!ok && !m_errMsg.size()) {
                    m_errCode = cs->GetErrorCode();
                    m_errMsg = "No connection to anyone of remote servers";
                }
                return m_errMsg;
            }

        private:

            static std::string ToString(const unsigned char *buffer, unsigned int bytes) {
                std::string s;
                char str[8] = {0};
                if (!buffer) bytes = 0;
                for (unsigned int n = 0; n < bytes; ++n) {
#ifdef WIN32_64
                    sprintf_s(str, "%02x", buffer[n]);
#else
                    sprintf(str, "%02x", buffer[n]);
#endif
                    s += str;
                }
                return s;
            }

            bool DoSSLAuth(CClientSocket *cs) {
                IUcert *cert = cs->GetUCert();
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
                std::string pk = ToString(cert->PublicKey, cert->PKSize);
                //check server certificate public key against an array of allowed public keys
                if (std::find(KeysAllowed.cbegin(), KeysAllowed.cend(), pk) == KeysAllowed.cend()) {
                    //permission not given
                    return false;
                }
                m_errCode = 0;
                m_errMsg.clear();
                //permission given
                return true;
            }
        };
        typedef CPoolConfig* PPoolConfig;

    } //namespace ClientSide
} //namespace PA

#endif
