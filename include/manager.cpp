#include <stdexcept>
#include "manager.h"
#include "3rdparty/rapidjson/include/rapidjson/filereadstream.h"
#include "3rdparty/rapidjson/include/rapidjson/document.h"
#include "3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "3rdparty/rapidjson/include/rapidjson/writer.h"

#ifdef WIN32_64
#pragma warning(disable: 4996) //warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS.
#endif

namespace SPA
{
    namespace ClientSide{
        using namespace rapidjson;
        using Utilities::Trim;

        std::vector<std::string> CPoolConfig::KeysAllowed;
        DSpManagerPoolEvent CPoolConfig::PoolEvent;

        CSpConfig CSpConfig::SpConfig;

        CSpConfig::CSpConfig() : m_bQP(0), m_bMidTier(false) {}

        const std::string & CSpConfig::GetErrMsg() {
            SPA::CAutoLock al(m_cs);
            return m_errMsg;
        }

        CAsyncServiceHandler * CSpConfig::SeekHandler(const std::string & poolKey) {
            unsigned int svsId;
            void *pool = GetPool(poolKey, &svsId);
            switch (svsId) {
                case SPA::SFile::sidFile:
                    return ((CStreamingFilePool*) pool)->Seek().get();
                case SPA::Queue::sidQueue:
                    return ((CAsyncQueuePool*) pool)->Seek().get();
                case SPA::Mysql::sidMysql:
                    return ((CMysqlPool*) pool)->Seek().get();
                case SPA::Sqlite::sidSqlite:
                    return ((CSqlitePool*) pool)->Seek().get();
                case SPA::Odbc::sidOdbc:
                    return ((COdbcPool*) pool)->Seek().get();
                default:
                    return ((CPoolConfig::CMyPool*)pool)->Seek().get();
            }
        }

        CAsyncServiceHandler * CSpConfig::SeekHandlerByQueue(const std::string & poolKey) {
            unsigned int svsId;
            void *pool = GetPool(poolKey, &svsId);
            if (!m_pp.at(poolKey)->Queue.size()) {
                throw std::runtime_error(("Client queue is not availeble for pool " + poolKey).c_str());
            }
            switch (svsId) {
                case SPA::SFile::sidFile:
                    return ((CStreamingFilePool*) pool)->SeekByQueue().get();
                case SPA::Queue::sidQueue:
                    return ((CAsyncQueuePool*) pool)->SeekByQueue().get();
                case SPA::Mysql::sidMysql:
                    return ((CMysqlPool*) pool)->SeekByQueue().get();
                case SPA::Sqlite::sidSqlite:
                    return ((CSqlitePool*) pool)->SeekByQueue().get();
                case SPA::Odbc::sidOdbc:
                    return ((COdbcPool*) pool)->SeekByQueue().get();
                default:
                    return ((CPoolConfig::CMyPool*)pool)->SeekByQueue().get();
            }
        }

        void* CSpConfig::GetPool(const std::string &poolKey, unsigned int *pSvsId) {
            std::string em;
            SPA::CAutoLock al(m_cs);
            auto it = m_pp.find(poolKey);
            if (it == m_pp.end()) {
                em = "Pool key " + poolKey + " cannot be found from configuaration";
                throw std::runtime_error(em.c_str());
            }
            CPoolConfig *pc = it->second;
            em = pc->StartPool(m_bMidTier, Hosts);
#if 0
            if (em.size()) {
                throw std::runtime_error(em.c_str());
            }
#endif
            if (pSvsId) {
                *pSvsId = pc->SvsId;
            }
            return pc->Pool.get();
        }

        CConnectionContext GetCC(const GenericValue<UTF8<>> &cc) {
            CConnectionContext ctx;
            if (cc.HasMember("Host") && cc["Host"].IsString()) {
                ctx.Host = cc["Host"].GetString();
                Trim(ctx.Host);
                std::transform(ctx.Host.begin(), ctx.Host.end(), ctx.Host.begin(), ::tolower);
            }
            if (cc.HasMember("Port") && cc["Port"].IsUint()) {
                ctx.Port = cc["Port"].GetUint();
            }
            if (cc.HasMember("UserId") && cc["UserId"].IsString()) {
                std::string s = cc["UserId"].GetString();
                Trim(s);
                ctx.UserId = SPA::Utilities::ToWide(s);
            }
            if (cc.HasMember("Password") && cc["Password"].IsString()) {
                std::string s = cc["Password"].GetString();
                ctx.Password = SPA::Utilities::ToWide(s);
            }
            if (cc.HasMember("EncrytionMethod") && cc["EncrytionMethod"].IsUint()) {
                ctx.EncrytionMethod = cc["EncrytionMethod"].GetUint() ? SPA::TLSv1 : SPA::NoEncryption;
            }
            if (cc.HasMember("Zip")) {
                ctx.Zip = cc["Zip"].GetBool();
            }
            if (cc.HasMember("V6")) {
                ctx.Zip = cc["V6"].GetBool();
            }
            return ctx;
        }

        CPoolConfig GetPool(bool main, const GenericValue<UTF8<>> &pool) {
            CPoolConfig pc;
            if (pool.HasMember("Queue") && pool["Queue"].IsString()) {
                pc.Queue = pool["Queue"].GetString();
                Trim(pc.Queue);
#ifdef WIN32_64
                std::transform(pc.Queue.begin(), pc.Queue.end(), pc.Queue.begin(), ::tolower);
#endif
            }
            if (pool.HasMember("DefaultDb") && pool["DefaultDb"].IsString()) {
                pc.DefaultDb = pool["DefaultDb"].GetString();
                Trim(pc.DefaultDb);
            }
            if (main) {
                if (pc.DefaultDb.size()) {
                    pc.PoolType = Master;
                } else {
                    pc.PoolType = Regular;
                }
            } else {
                pc.PoolType = Slave;
            }
            if (pool.HasMember("SvsId") && pool["SvsId"].IsUint()) {
                pc.SvsId = pool["SvsId"].GetUint();
            }
            if (pool.HasMember("Threads") && pool["Threads"].IsUint()) {
                pc.Threads = pool["Threads"].GetUint();
            }
            if (pool.HasMember("RecvTimeout") && pool["RecvTimeout"].IsUint()) {
                pc.RecvTimeout = pool["RecvTimeout"].GetUint();
            }
            if (pool.HasMember("ConnTimeout") && pool["ConnTimeout"].IsUint()) {
                pc.ConnTimeout = pool["ConnTimeout"].GetUint();
            }
            if (pool.HasMember("AutoConn") && pool["AutoConn"].IsBool()) {
                pc.AutoConn = pool["AutoConn"].GetBool();
            }
            if (pool.HasMember("AutoMerge") && pool["AutoMerge"].IsBool()) {
                pc.AutoMerge = pool["AutoMerge"].GetBool();
            }
            if (pool.HasMember("Hosts") && pool["Hosts"].IsArray()) {
                auto vH = pool["Hosts"].GetArray();
                for (auto it = vH.Begin(), end = vH.End(); it != end; ++it) {
                    if (!it->IsString()) {
                        continue;
                    }
                    std::string s = it->GetString();
                    if (s.size()) {
                        pc.Hosts.push_back(s);
                    }
                }
            }
            if (pc.PoolType == Master) {
                if (pool.HasMember("Slaves") && pool["Slaves"].IsObject()) {
                    auto vSlave = pool["Slaves"].GetObject();
                    for (auto it = vSlave.MemberBegin(), end = vSlave.MemberEnd(); it != end; ++it) {
                        std::string skey = it->name.GetString();
                        const auto& cc = it->value;
                        if (!cc.IsObject()) {
                            continue;
                        }
                        CPoolConfig slave = GetPool(false, cc);
                        slave.SvsId = pc.SvsId;
                        if (!slave.DefaultDb.size()) {
                            slave.DefaultDb = pc.DefaultDb;
                        }
                        pc.Slaves[skey] = slave;
                    }
                }
            }
            return pc;
        }

        void CSpConfig::CheckHostErrors() {
            if (!Hosts.size()) {
                m_errMsg = "No connection context given in Hosts";
                return;
            }
            std::vector<CConnectionContext *> vCC;
            for (auto it = Hosts.begin(), end = Hosts.end(); it != end; ++it) {
                if (!it->first.size()) {
                    m_errMsg = "Host key cannot be an empty string";
                    break;
                }
                CConnectionContext &cc = it->second;
                if (!cc.Host.size()) {
                    m_errMsg = "No remote host address given for host key " + it->first;
                    break;
                }
                if (!cc.Port) {
                    m_errMsg = "No port number given in connection context for host key " + it->first;
                    break;
                }
                auto pos = std::find_if(vCC.cbegin(), vCC.cend(), [&cc](const CConnectionContext * s) {
                    return (s->Host == cc.Host && s->Port == cc.Port);
                });
                if (pos != vCC.cend()) {
                    m_errMsg = "No remote host duplicated with the same address and port number for host key " + it->first;
                    break;
                }
                vCC.push_back(&cc);
            }
        }

        bool CSpConfig::ExistHost(const std::string & key) {
            for (auto it = Hosts.begin(), end = Hosts.end(); it != end; ++it) {
                if (it->first == key) {
                    return true;
                }
            }
            return false;
        }

        void CSpConfig::CheckPoolErrors() {
            if (!Pools.size()) {
                m_errMsg = "No pool context given in Pools";
                return;
            }
            for (auto it = Pools.begin(), end = Pools.end(); it != end; ++it) {
                if (m_pp.find(it->first) != m_pp.cend()) {
                    m_errMsg = "Pool key " + it->first + " duplicated in Pools";
                    break;
                }
                CPoolConfig &pc = it->second;
                switch (pc.SvsId) {
                    case SPA::SFile::sidFile:
                        if (pc.PoolType != Regular) {
                            m_errMsg = "Remote file service has not support to either master or slave pool";
                            return;
                        }
                        break;
                    case SPA::Queue::sidQueue:
                        if (pc.PoolType != Regular) {
                            m_errMsg = "Server persistent queue service has not support to either master or slave pool";
                            return;
                        }
                        break;
                    case SPA::Mysql::sidMysql:
                    case SPA::Sqlite::sidSqlite:
                    case SPA::Odbc::sidOdbc:
                        break;
                    default:
                        if (pc.SvsId <= sidReserved) {
                            m_errMsg = "Service id must be larger than " + std::to_string((UINT64) sidReserved);
                            return;
                        }
                        break;
                }
                for (auto h = pc.Hosts.cbegin(), he = pc.Hosts.cend(); h != he; ++h) {
                    if (!ExistHost(*h)) {
                        m_errMsg = "Host " + *h + " not found in Hosts for pool " + it->first;
                    }
                }
                m_pp[it->first] = &pc;
            }

            for (auto it = Pools.begin(), end = Pools.end(); it != end; ++it) {
                auto &slaves = it->second.Slaves;
                if (!slaves.size()) {
                    continue;
                }
                for (auto sit = slaves.begin(), send = slaves.end(); sit != send; ++sit) {
                    if (m_pp.find(sit->first) != m_pp.cend()) {
                        m_errMsg = "Pool key " + sit->first + " duplicated in Pools";
                        return;
                    }
                    CPoolConfig &pc = sit->second;
                    for (auto h = pc.Hosts.cbegin(), he = pc.Hosts.cend(); h != he; ++h) {
                        if (!ExistHost(*h)) {
                            m_errMsg = "Host " + *h + " not found in Hosts for pool " + it->first;
                        }
                    }
                    m_pp[sit->first] = &pc;
                }
            }
        }

        void CSpConfig::Normalize() {
            CheckHostErrors();
            if (m_errMsg.size()) {
                return;
            }
            CheckPoolErrors();
        }

        std::string CSpConfig::GetConfig() {
            Document doc;
            doc.SetObject();
            Document::AllocatorType& allocator = doc.GetAllocator();
            {
                SPA::CAutoLock al(m_cs);
                if (!Pools.size()) {
                    return "";
                }
            }
            if (CertStore.size()) {
                Value cs;
                cs.SetString(CertStore.c_str(), (SizeType) CertStore.size());
                doc.AddMember("CertStore", cs, allocator);
            }
            std::string dir = CClientSocket::QueueConfigure::GetWorkDirectory();
            if (dir.size()) {
                Value wd;
                wd.SetString(dir.c_str(), (SizeType) dir.size());
                doc.AddMember("WorkingDir", wd, allocator);
            }
            doc.AddMember("QueuePassword", m_bQP, allocator);
            Value vH(kObjectType);
            for (auto it = Hosts.cbegin(), end = Hosts.cend(); it != end; ++it) {
                Value key(it->first.c_str(), (SizeType) it->first.size(), allocator);

                auto &ctx = it->second;
                Value obj(kObjectType);

                Value s(ctx.Host.c_str(), (SizeType) ctx.Host.size(), allocator);
                obj.AddMember("Host", s, allocator);

                obj.AddMember("Port", ctx.Port, doc.GetAllocator());

                std::string str = Utilities::ToUTF8(ctx.UserId);
                s.SetString(str.c_str(), (SizeType) str.size(), allocator);
                obj.AddMember("UserId", s, allocator);

                str = SPA::Utilities::ToUTF8(ctx.Password);
                s.SetString(str.c_str(), (SizeType) str.size(), allocator);
                obj.AddMember("Password", s, allocator);

                obj.AddMember("EncrytionMethod", (int) ctx.EncrytionMethod, allocator);
                obj.AddMember("Zip", ctx.Zip, allocator);
                obj.AddMember("V6", ctx.V6, allocator);
                vH.AddMember(key, obj, allocator);
            }
            doc.AddMember("Hosts", vH, allocator);
            if (CPoolConfig::KeysAllowed.size()) {
                Value vKA(kArrayType);
                for (auto it = CPoolConfig::KeysAllowed.cbegin(), end = CPoolConfig::KeysAllowed.cend(); it != end; ++it) {
                    Value key(it->c_str(), (SizeType) it->size(), allocator);
                    vKA.PushBack(key, allocator);
                }
                doc.AddMember("KeysAllowed", vKA, allocator);
            }

            Value vP(kObjectType);
            for (auto it = Pools.cbegin(), end = Pools.cend(); it != end; ++it) {
                Value key(it->first.c_str(), (SizeType) it->first.size(), allocator);
                const CPoolConfig &pscMain = it->second;
                Value objMain(kObjectType);
                objMain.AddMember("SvsId", pscMain.SvsId, allocator);

                Value vH(kArrayType);
                for (auto h = pscMain.Hosts.cbegin(), he = pscMain.Hosts.cend(); h != he; ++h) {
                    Value s(h->c_str(), (SizeType) h->size(), allocator);
                    vH.PushBack(s, allocator);
                }
                objMain.AddMember("Hosts", vH, allocator);

                objMain.AddMember("Threads", pscMain.Threads, allocator);
                if (pscMain.Queue.size()) {
                    Value s(pscMain.Queue.c_str(), (SizeType) pscMain.Queue.size(), allocator);
                    objMain.AddMember("Queue", s, allocator);
                }
                objMain.AddMember("AutoConn", pscMain.AutoConn, allocator);
                objMain.AddMember("AutoMerge", pscMain.AutoMerge, allocator);
                objMain.AddMember("RecvTimeout", pscMain.RecvTimeout, allocator);
                objMain.AddMember("ConnTimeout", pscMain.RecvTimeout, allocator);
                if (pscMain.DefaultDb.size()) {
                    Value s(pscMain.DefaultDb.c_str(), (SizeType) pscMain.DefaultDb.size(), allocator);
                    objMain.AddMember("DefaultDb", s, allocator);
                }
                //Slaves
                if (pscMain.Slaves.size()) {
                    Value vS(kObjectType);
                    for (auto one = pscMain.Slaves.cbegin(), onee = pscMain.Slaves.cend(); one != onee; ++one) {
                        Value key(one->first.c_str(), (SizeType) one->first.size(), allocator);
                        const CPoolConfig &psc = one->second;
                        Value obj(kObjectType);
                        obj.AddMember("SvsId", psc.SvsId, allocator);

                        Value vH(kArrayType);
                        for (auto h = psc.Hosts.cbegin(), he = psc.Hosts.cend(); h != he; ++h) {
                            Value s(h->c_str(), (SizeType) h->size(), allocator);
                            vH.PushBack(s, allocator);
                        }
                        obj.AddMember("Hosts", vH, allocator);

                        obj.AddMember("Threads", psc.Threads, allocator);
                        if (psc.Queue.size()) {
                            Value s(psc.Queue.c_str(), (SizeType) psc.Queue.size(), allocator);
                            obj.AddMember("Queue", s, allocator);
                        }
                        obj.AddMember("AutoConn", psc.AutoConn, allocator);
                        obj.AddMember("AutoMerge", psc.AutoMerge, allocator);
                        obj.AddMember("RecvTimeout", psc.RecvTimeout, allocator);
                        obj.AddMember("ConnTimeout", psc.RecvTimeout, allocator);
                        Value s(psc.DefaultDb.c_str(), (SizeType) psc.DefaultDb.size(), allocator);
                        obj.AddMember("DefaultDb", s, allocator);
                        obj.AddMember("PoolType", psc.PoolType, allocator);
                        vS.AddMember(key, obj, allocator);
                    }
                    objMain.AddMember("Slaves", vS, allocator);
                }
                objMain.AddMember("PoolType", pscMain.PoolType, allocator);
                vP.AddMember(key, objMain, allocator);
            }
            doc.AddMember("Pools", vP, allocator);

            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            doc.Accept(writer);
            return buffer.GetString();
        }

        bool CSpConfig::Parse(bool midTier, const char *jsonConfig) {
            SPA::CAutoLock al(m_cs);
            if (Pools.size()) {
                return true;
            }
            Pools.clear();
            m_errMsg.clear();
            CertStore.clear();
            m_bQP = 0;
            m_pp.clear();
            Document doc;
            doc.SetObject();
            std::string jsFile(jsonConfig ? jsonConfig : "");
            Trim(jsFile);
            if (!jsFile.size()) {
                jsFile = "sp_config.json";
            }
            std::shared_ptr<FILE> fp(fopen(jsFile.c_str(), "rb"), [](FILE * f) {
                if (f) {
                    ::fclose(f);
                }
            });
            if (!fp || ferror(fp.get())) {
                m_errMsg = "Cannot open configuration file " + jsFile;
                return false;
            }
            fseek(fp.get(), 0, SEEK_END);
            long size = ftell(fp.get()) + sizeof (wchar_t);
            fseek(fp.get(), 0, SEEK_SET);
            SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int) size);
            sb->CleanTrack();
            FileReadStream is(fp.get(), (char*) sb->GetBuffer(), sb->GetMaxSize());
            const char *json = (const char*) sb->GetBuffer();
            ParseResult ok = doc.Parse(json, ::strlen(json));
            if (!ok) {
                m_errMsg = "Bad JSON configuration object";
                return false;
            }
            if (doc.HasMember("WorkingDir") && doc["WorkingDir"].IsString()) {
                std::string dir = doc["WorkingDir"].GetString();
                Trim(dir);
                SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(dir.c_str());
            }
            if (doc.HasMember("CertStore") && doc["CertStore"].IsString()) {
                CertStore = doc["CertStore"].GetString();
                Trim(CertStore);
                if (CertStore.size()) {
                    CClientSocket::SSL::SetVerifyLocation(CertStore.c_str());
                }
            }
            if (doc.HasMember("QueuePassword") && doc["QueuePassword"].IsString()) {
                std::string qp = doc["QueuePassword"].GetString();
                Trim(qp);
                if (qp.size()) {
                    SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(qp.c_str());
                    m_bQP = 1;
                }
            }
            if (doc.HasMember("KeysAllowed") && doc["KeysAllowed"].IsArray()) {
                auto arr = doc["KeysAllowed"].GetArray();
                for (auto it = arr.Begin(), end = arr.End(); it != end; ++it) {
                    if (!it->IsString()) {
                        continue;
                    }
                    std::string s = it->GetString();
                    Trim(s);
                    if (s.size()) {
                        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                        CPoolConfig::KeysAllowed.push_back(s);
                    }
                }
            } else {
                CPoolConfig::KeysAllowed.clear();
            }
            if (doc.HasMember("Hosts") && doc["Hosts"].IsObject()) {
                auto arr = doc["Hosts"].GetObject();
                for (auto it = arr.MemberBegin(), end = arr.MemberEnd(); it != end; ++it) {
                    std::string key = it->name.GetString();
                    const auto &cc = it->value;
                    if (!cc.IsObject()) {
                        continue;
                    }
                    Hosts[key] = GetCC(cc);
                }
            }
            if (doc.HasMember("Pools") && doc["Pools"].IsObject()) {
                auto arr = doc["Pools"].GetObject();
                for (auto it = arr.MemberBegin(), end = arr.MemberEnd(); it != end; ++it) {
                    std::string key = it->name.GetString();
                    const auto &cc = it->value;
                    if (!cc.IsObject()) {
                        continue;
                    }
                    Pools[key] = ClientSide::GetPool(true, cc);
                }
            }
            Normalize();
            if (m_errMsg.size()) {
                Pools.clear();
                return false;
            }
            m_bMidTier = midTier;
            return true;
        }

        CSpConfig & SpManager::SetConfig(bool midTier, const char *jsonConfig) {
            CSpConfig::SpConfig.Parse(midTier, jsonConfig);
            if (CSpConfig::SpConfig.GetErrMsg().size()) {
                throw std::runtime_error(CSpConfig::SpConfig.GetErrMsg().c_str());
            }
            return CSpConfig::SpConfig;
        }

        void* SpManager::GetPool(const std::string & poolKey, unsigned int *pSvsId) {
            SetConfig();
            if (!poolKey.size()) {
                throw std::runtime_error("Pool key cannot be empty");
            }
            return CSpConfig::SpConfig.GetPool(poolKey, pSvsId);
        }

        CAsyncServiceHandler * SpManager::SeekHandler(const std::string & poolKey) {
            SetConfig();
            if (!poolKey.size()) {
                throw std::runtime_error("Pool key cannot be empty");
            }
            return CSpConfig::SpConfig.SeekHandler(poolKey);
        }

        CAsyncServiceHandler * SpManager::SeekHandlerByQueue(const std::string & poolKey) {
            SetConfig();
            if (!poolKey.size()) {
                throw std::runtime_error("Pool key cannot be empty");
            }
            return CSpConfig::SpConfig.SeekHandlerByQueue(poolKey);
        }
    } //namespace ClientSide
} //namespace PA
