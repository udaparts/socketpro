#include "stdafx.h"
#include "phpsocketpool.h"
#include "phpsocket.h"
#include "phpdataset.h"

namespace PA
{

    CPhpSocketPool::CPhpSocketPool(const CPoolStartContext & psc)
            : m_nSvsId(psc.SvsId), Handler(psc.PhpHandler), m_pt(psc.PoolType), m_qName(psc.Queue), m_recvTimeout(psc.RecvTimeout) {
    }

    CPhpSocketPool::~CPhpSocketPool() {
    }

    void CPhpSocketPool::__construct(Php::Parameters & params) {
    }

    void CPhpSocketPool::__destruct() {
    }

    Php::Value CPhpSocketPool::Seek() {
        switch (m_nSvsId) {
            case SPA::Queue::sidQueue:
                throw Php::Exception("Persistent message queue handler doesn't support Seek method within PHP adapter at this time. Use the method Lock instead");
            case SPA::SFile::sidFile:
            {
                auto handler = m_qName.size() ? File->SeekByQueue() : File->Seek();
                if (!handler)
                    throw Php::Exception("File handler not found");
                Php::Object obj((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(handler.get(), false));
                return obj;
            }
                break;
            default:
                if (SPA::IsDBService(m_nSvsId)) {
                    auto handler = m_qName.size() ? Db->SeekByQueue() : Db->Seek();
                    if (!handler)
                        throw Php::Exception("Database handler not found");
                    Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(handler.get(), false));
                    return obj;
                }
                break;
        }
        auto handler = m_qName.size() ? Handler->SeekByQueue() : Handler->Seek();
        if (!handler)
            throw Php::Exception("Async handler not found");
        return Php::Object((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(handler.get(), false));
    }

    Php::Value CPhpSocketPool::Lock(Php::Parameters & params) {
        unsigned int timeout = (~0);
        if (params.size()) {
            timeout = (unsigned int) params[0].numericValue();
        }
        switch (m_nSvsId) {
            case SPA::Queue::sidQueue:
            {
                auto handler = Queue->Lock(timeout);
                if (!handler)
                    throw Php::Exception("No persistent message queue handler locked");
                Php::Object obj((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(handler.get(), true));
                return obj;
            }
                break;
            case SPA::SFile::sidFile:
            {
                auto handler = File->Lock(timeout);
                if (!handler)
                    throw Php::Exception("No file handler locked");
                Php::Object obj((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(handler.get(), true));
                return obj;
            }
                break;
            default:
                if (SPA::IsDBService(m_nSvsId)) {
                    auto handler = Db->Lock(timeout);
                    if (!handler)
                        throw Php::Exception("No database handler locked");
                    Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(handler.get(), true));
                    return obj;
                }
                break;
        }
        auto handler = Handler->Lock(timeout);
        if (!handler)
            throw Php::Exception("No async handler locked");
        return Php::Object((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(handler.get(), true));
    }

    int CPhpSocketPool::__compare(const CPhpSocketPool & pool) const {
        if (!Handler || !pool.Handler) {
            return 1;
        }
        return (Handler == pool.Handler) ? 0 : 1;
    }

    Php::Value CPhpSocketPool::__get(const Php::Value & name) {
        int key = 0;
        if (name == "Handlers" || name == "AsyncHandlers") {
            Php::Array harray;
            switch (m_nSvsId) {
                case SPA::Queue::sidQueue:
                {
                    auto vH = Queue->GetAsyncHandlers();
                    for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
                        CAsyncQueue *aq = (*it).get();
                        Php::Object objQueue((SPA_CS_NS + PHP_QUEUE_HANDLER).c_str(), new CPhpQueue(aq, false));
                        harray.set(key, objQueue);
                    }
                }
                    break;
                case SPA::SFile::sidFile:
                {
                    auto vH = File->GetAsyncHandlers();
                    for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
                        CAsyncFile *af = (*it).get();
                        Php::Object objFile((SPA_CS_NS + PHP_FILE_HANDLER).c_str(), new CPhpFile(af, false));
                        harray.set(key, objFile);
                    }
                }
                    break;
                default:
                    if (SPA::IsDBService(m_nSvsId)) {
                        auto vH = Db->GetAsyncHandlers();
                        for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
                            CDBHandler* db = (*it).get();
                            Php::Object objDb((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(db, false));
                            harray.set(key, objDb);
                        }
                    } else {
                        auto vH = Handler->GetAsyncHandlers();
                        for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it, ++key) {
                            CAsyncHandler *ah = (*it).get();
                            Php::Object objHandler((SPA_CS_NS + PHP_ASYNC_HANDLER).c_str(), new CPhpHandler(ah, false));
                            harray.set(key, objHandler);
                        }
                    }
                    break;
            }
            return harray;
        } else if (name == "Sockets") {
            Php::Array harray;
            std::vector<std::shared_ptr < CClientSocket>> ss;
            switch (m_nSvsId) {
                case (unsigned int) SPA::tagServiceID::sidChat:
                    ss = Queue->GetSockets();
                    break;
                case (unsigned int) SPA::tagServiceID::sidFile:
                    ss = File->GetSockets();
                    break;
                default:
                    if (SPA::IsDBService(m_nSvsId)) {
                        ss = Db->GetSockets();
                    } else {
                        ss = Handler->GetSockets();
                    }
                    break;
            }
            int key = 0;
            for (auto it = ss.cbegin(), end = ss.cend(); it != end; ++it, ++key) {
                CClientSocket *cs = it->get();
                Php::Object objSocket((SPA_CS_NS + PHP_SOCKET).c_str(), new CPhpSocket(cs));
                harray.set(key, objSocket);
            }
            return harray;
        } else if (name == "Conns" || name == "ConnectedSockets") {
            return (int64_t) Handler->GetConnectedSockets();
        } else if (name == "Disconns" || name == "DisconnectedSockets") {
            return (int64_t) Handler->GetDisconnectedSockets();
        } else if (name == "Idles" || name == "IdleSockets") {
            return (int64_t) Handler->GetIdleSockets();
        } else if (name == "Locks" || name == "LockedSockets") {
            return (int64_t) Handler->GetLockedSockets();
        } else if (name == "spt" || name == "SocketsPerThread") {
            return (int64_t) Handler->GetSocketsPerThread();
        } else if (name == KEY_THREADS || name == "ThreadsCreated") {
            return (int64_t) Handler->GetThreadsCreated();
        } else if (name == KEY_SVS_ID || name == "ServiceId") {
            return (int64_t) m_nSvsId;
        } else if (name == KEY_AUTO_CONN) {
            return Handler->GetAutoConn();
        } else if (name == KEY_RECV_TIMEOUT) {
            return (int64_t) Handler->GetRecvTimeout();
        } else if (name == KEY_CONN_TIMEOUT) {
            return (int64_t) Handler->GetConnTimeout();
        } else if (name == "Queues") {
            return (int64_t) Handler->GetQueues();
        } else if (name == "id" || name == "PoolId") {
            return (int64_t) Handler->GetPoolId();
        } else if (name == "Avg") {
            return Handler->IsAvg();
        } else if (name == "QueueName") {
            std::string qn = Handler->GetQueueName();
            return qn;
        } else if (name == KEY_AUTO_MERGE || name == "QueueAutoMerge") {
            return Handler->GetQueueAutoMerge();
        } else if (name == "Started") {
            return Handler->IsStarted();
        } else if (name == "Cache") {
            if (m_pt != tagPoolType::Master) {
                throw Php::Exception("Non-master pool doesn't have cache");
            }
            switch (m_nSvsId) {
                case (unsigned int) SPA::tagServiceID::sidFile:
                case (unsigned int) SPA::tagServiceID::sidChat:
                    assert(false); //shouldn't come here
                    break;
                default:
                    if (SPA::IsDBService(m_nSvsId)) {
                        CSQLMaster* master = (CSQLMaster*) Db;
                        Php::Object cache((SPA_NS + PHP_DATASET).c_str(), new CPhpDataSet(master->Cache));
                        return cache;
                    } else {
                        CMasterPool *master = (CMasterPool *) Handler;
                        Php::Object cache((SPA_NS + PHP_DATASET).c_str(), new CPhpDataSet(master->Cache));
                        return cache;
                    }
                    break;
            }
        }
        return Php::Base::__get(name);
    }

    void CPhpSocketPool::RegisterInto(Php::Namespace & cs) {
        Php::Class<CPhpSocketPool> pool(PHP_SOCKET_POOL);
        pool.property("DEFAULT_RECV_TIMEOUT", (int64_t) SPA::ClientSide::DEFAULT_RECV_TIMEOUT, Php::Const);
        pool.property("DEFAULT_CONN_TIMEOUT", (int64_t) SPA::ClientSide::DEFAULT_CONN_TIMEOUT, Php::Const);

        //tagPoolType
        pool.property("Regular", (int) tagPoolType::Regular, Php::Const);
        pool.property("Slave", (int) tagPoolType::Slave, Php::Const);
        pool.property("Master", (int) tagPoolType::Master, Php::Const);

        pool.method<&CPhpSocketPool::__construct>(PHP_CONSTRUCT, Php::Private);
        pool.method<&CPhpSocketPool::Lock>("Lock",{
            Php::ByVal(PHP_TIMEOUT, Php::Type::Numeric, false)
        });
        pool.method<&CPhpSocketPool::Seek>("Seek");
        cs.add(std::move(pool));
    }
}
