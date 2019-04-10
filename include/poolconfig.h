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

            std::string StartPool(bool midTier, const CMapHost &mapHost) {
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
                CClientSocket *cs;
                switch (SvsId) {
                    case SPA::Sqlite::sidSqlite:
                    {
                        CSqlitePool *db;
                        switch (PoolType) {
                            case Master:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, CSqlitePool::Handler>(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, CSqlitePool::Handler>(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, CSqlitePool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, CSqlitePool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CSqlitePool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CSqlitePool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CSqlitePool *pool, tagSocketPoolEvent spe, CSqlitePool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CSqlitePool*) p;
                        });
                    }
                        break;
                    case SPA::Mysql::sidMysql:
                    {
                        CMysqlPool *db;
                        switch (PoolType) {
                            case Master:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, CMysqlPool::Handler>(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, CMysqlPool::Handler>(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, CMysqlPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, CMysqlPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new CMysqlPool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](CMysqlPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](CMysqlPool *pool, tagSocketPoolEvent spe, CMysqlPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (CMysqlPool*) p;
                        });
                    }
                        break;
                    case SPA::Odbc::sidOdbc:
                    {
                        COdbcPool *db;
                        switch (PoolType) {
                            case Master:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, COdbcPool::Handler>(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, COdbcPool::Handler>(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                if (midTier) {
                                    db = new CSQLMasterPool<true, COdbcPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                } else {
                                    db = new CSQLMasterPool<false, COdbcPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            default:
                                db = new COdbcPool(AutoConn, RecvTimeout, ConnTimeout);
                                break;
                        }
                        if (Queue.size()) {
                            db->SetQueueName(Queue.c_str());
                        }
                        db->DoSslServerAuthentication = [this](COdbcPool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        db->SocketPoolEvent = [this](COdbcPool *pool, tagSocketPoolEvent spe, COdbcPool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = db->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            db->SetQueueAutoMerge(true);
                        }
                        cs = db->GetSockets()[0].get();
                        Pool.reset(db, [](void *p) {
                            delete (COdbcPool*) p;
                        });
                    }
                        break;
                    case SPA::Queue::sidQueue:
                    {
                        CAsyncQueuePool *pool = new CAsyncQueuePool(AutoConn, RecvTimeout, ConnTimeout);
                        if (Queue.size()) {
                            pool->SetQueueName(Queue.c_str());
                        }
                        pool->DoSslServerAuthentication = [this](CAsyncQueuePool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        pool->SocketPoolEvent = [this](CAsyncQueuePool *p, tagSocketPoolEvent spe, CAsyncQueuePool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = pool->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            pool->SetQueueAutoMerge(true);
                        }
                        cs = pool->GetSockets()[0].get();
                        Pool.reset(pool, [](void *p) {
                            delete (CAsyncQueuePool*) p;
                        });
                    }
                        break;
                    case SPA::SFile::sidFile:
                    {
                        CStreamingFilePool *pool = new CStreamingFilePool(AutoConn, RecvTimeout, ConnTimeout);
                        if (Queue.size()) {
                            pool->SetQueueName(Queue.c_str());
                        }
                        pool->DoSslServerAuthentication = [this](CStreamingFilePool *pool, CClientSocket * cs)->bool {
                            return this->DoSSLAuth(cs);
                        };
                        pool->SocketPoolEvent = [this](CStreamingFilePool *p, tagSocketPoolEvent spe, CStreamingFilePool::Handler * handler) {
                            if (PoolEvent)
                                PoolEvent(this, spe);
                        };
                        ok = pool->StartSocketPool(ppCCs, threads, socketsPerThread);
                        if (AutoMerge) {
                            pool->SetQueueAutoMerge(true);
                        }
                        cs = pool->GetSockets()[0].get();
                        Pool.reset(pool, [](void *p) {
                            delete (CStreamingFilePool*) p;
                        });
                    }
                        break;
                    default:
                    {
                        CMyPool *db;
                        switch (PoolType) {
                            case Master:
                                if (midTier) {
                                    db = new CMasterPool<true, CMyPool::Handler>(dfltDb.c_str(), RecvTimeout, SvsId);
                                } else {
                                    db = new CMasterPool<false, CMyPool::Handler>(dfltDb.c_str(), RecvTimeout, SvsId);
                                }
                                db->SetAutoConn(AutoConn);
                                db->SetConnTimeout(ConnTimeout);
                                break;
                            case Slave:
                                if (midTier) {
                                    db = new CMasterPool<true, CMyPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout, SvsId);
                                } else {
                                    db = new CMasterPool<false, CMyPool::Handler>::CSlavePool(dfltDb.c_str(), RecvTimeout, SvsId);
                                }
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
