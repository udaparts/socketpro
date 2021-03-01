#include <stdexcept>
#include "manager.h"
#include "jsonvalue.h"

namespace SPA
{
    namespace ClientSide{
        using namespace JSON;
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
            if (!pool) {
                return nullptr;
            }
            return ((CBasePool*) pool)->Seek().get();
        }

        CAsyncServiceHandler * CSpConfig::SeekHandlerByQueue(const std::string & poolKey) {
            unsigned int svsId;
            void *pool = GetPool(poolKey, &svsId);
            if (!m_pp.at(poolKey)->Queue.size()) {
                throw std::runtime_error(("Client queue is not availeble for pool " + poolKey).c_str());
            }
            if (!pool) {
                return nullptr;
            }
            return ((CBasePool*) pool)->SeekByQueue().get();
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

        CConnectionContext GetCC(const JObject<char> & cc) {
            CConnectionContext ctx;
            auto it = cc.find("Host"), end = cc.end();
            if (it != end && it->second.GetType() == enumType::String) {
                ctx.Host = it->second.AsString();
                Trim(ctx.Host);
                ToLower(ctx.Host);
            }
            it = cc.find("Port");
            if (it != end && it->second.GetType() == enumType::Uint64) {
                ctx.Port = (unsigned short) it->second.AsUint64();
            }
            it = cc.find("UserId");
            if (it != end && it->second.GetType() == enumType::String) {
                std::string s = it->second.AsString();
                Trim(s);
                ctx.UserId = SPA::Utilities::ToWide(s);
            }
            it = cc.find("Password");
            if (it != end && it->second.GetType() == enumType::String) {
                std::string s = it->second.AsString();
                ctx.Password = SPA::Utilities::ToWide(s);
            }
            it = cc.find("EncrytionMethod");
            if (it != end && it->second.GetType() == enumType::Uint64) {
                ctx.EncrytionMethod = it->second.AsUint64() ? SPA::tagEncryptionMethod::TLSv1 : SPA::tagEncryptionMethod::NoEncryption;
            }
            it = cc.find("Zip");
            if (it != end && it->second.GetType() == enumType::Bool) {
                ctx.Zip = it->second.AsBool();
            }
            it = cc.find("V6");
            if (it != end && it->second.GetType() == enumType::Bool) {
                ctx.Zip = it->second.AsBool();
            }
            return ctx;
        }

        CPoolConfig GetPool(bool main, const JValue<char> & pool) {
            CPoolConfig pc;
            JValue<char>* jv = pool.Child("Queue");
            if (jv && jv->GetType() == enumType::String) {
                pc.Queue = jv->AsString();
                Trim(pc.Queue);
#ifdef WIN32_64
                ToLower(pc.Queue);
#endif
            }
            jv = pool.Child("DefaultDb");
            if (jv && jv->GetType() == enumType::String) {
                pc.DefaultDb = jv->AsString();
                Trim(pc.DefaultDb);
            }
            if (main) {
                if (pc.DefaultDb.size()) {
                    pc.PoolType = tagPoolType::Master;
                } else {
                    pc.PoolType = tagPoolType::Regular;
                }
            } else {
                pc.PoolType = tagPoolType::Slave;
            }
            jv = pool.Child("SvsId");
            if (jv && jv->GetType() == enumType::Uint64) {
                pc.SvsId = (unsigned int) jv->AsUint64();
            }
            jv = pool.Child("Threads");
            if (jv && jv->GetType() == enumType::Uint64) {
                pc.Threads = (unsigned int) jv->AsUint64();
            }
            jv = pool.Child("RecvTimeout");
            if (jv && jv->GetType() == enumType::Uint64) {
                pc.RecvTimeout = (unsigned int) jv->AsUint64();
            }
            jv = pool.Child("ConnTimeout");
            if (jv && jv->GetType() == enumType::Uint64) {
                pc.ConnTimeout = (unsigned int) jv->AsUint64();
            }
            jv = pool.Child("AutoConn");
            if (jv && jv->GetType() == enumType::Bool) {
                pc.AutoConn = jv->AsBool();
            }
            jv = pool.Child("AutoMerge");
            if (jv && jv->GetType() == enumType::Bool) {
                pc.AutoMerge = jv->AsBool();
            }
            jv = pool.Child("Hosts");
            if (jv && jv->GetType() == enumType::Array) {
                const auto& vH = jv->AsArray();
                for (auto it = vH.cbegin(), end = vH.cend(); it != end; ++it) {
                    if (it->GetType() != enumType::String) {
                        continue;
                    }
                    pc.Hosts.push_back(it->AsString());
                }
            }
            if (pc.PoolType == tagPoolType::Master) {
                jv = pool.Child("Slaves");
                if (jv && jv->GetType() == enumType::Object) {
                    const auto& vSlave = jv->AsObject();
                    for (auto it = vSlave.cbegin(), end = vSlave.cend(); it != end; ++it) {
                        const auto& cc = it->second;
                        if (cc.GetType() != enumType::Object) {
                            continue;
                        }
                        CPoolConfig slave = GetPool(false, cc);
                        slave.SvsId = pc.SvsId;
                        if (!slave.DefaultDb.size()) {
                            slave.DefaultDb = pc.DefaultDb;
                        }
                        pc.Slaves[it->first] = slave;
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
                        if (pc.PoolType != tagPoolType::Regular) {
                            m_errMsg = "Remote file service has not support to either master or slave pool";
                            return;
                        }
                        break;
                    case SPA::Queue::sidQueue:
                        if (pc.PoolType != tagPoolType::Regular) {
                            m_errMsg = "Server persistent queue service has not support to either master or slave pool";
                            return;
                        }
                        break;
                    case SPA::Mysql::sidMysql:
                    case SPA::Sqlite::sidSqlite:
                    case SPA::Odbc::sidOdbc:
                        break;
                    default:
                        if (pc.SvsId <= (unsigned int) tagServiceID::sidReserved) {
                            m_errMsg = "Service id must be larger than " + std::to_string((unsigned int) tagServiceID::sidReserved);
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
            JObject<char> objRoot;
            {
                SPA::CAutoLock al(m_cs);
                if (!Pools.size()) {
                    return "";
                }
            }
            objRoot["CertStore"] = JValue<char>(CertStore, false);
            objRoot["WorkingDir"] = CClientSocket::QueueConfigure::GetWorkDirectory();
            objRoot["QueuePassword"] = m_bQP;
            JValue<char> jvRoot(std::move(objRoot));
            JObject<char> vH;
            for (auto it = Hosts.cbegin(), end = Hosts.cend(); it != end; ++it) {
                auto &ctx = it->second;
                JObject<char> obj;
                obj["Host"] = ctx.Host;
                obj["Port"] = ctx.Port;
                obj["UserId"] = JValue<char>(Utilities::ToUTF8(ctx.UserId), false);
                obj["Password"] = JValue<char>(Utilities::ToUTF8(ctx.Password), false);
                obj["Port"] = ctx.Port;
                obj["EncrytionMethod"] = (int) ctx.EncrytionMethod;
                obj["Zip"] = ctx.Zip;
                obj["V6"] = ctx.V6;
                vH[it->first] = std::move(obj);
            }
            jvRoot.AsObject()["Hosts"] = std::move(vH);
            JArray<char> vKA;
            for (auto it = CPoolConfig::KeysAllowed.cbegin(), end = CPoolConfig::KeysAllowed.cend(); it != end; ++it) {
                vKA.push_back(JValue<char>(*it, false));
            }
            jvRoot.AsObject()["KeysAllowed"] = std::move(vKA);
            JObject<char> vP;
            for (auto it = Pools.cbegin(), end = Pools.cend(); it != end; ++it) {
                const CPoolConfig &pscMain = it->second;
                JObject<char> objMain;
                objMain["SvsId"] = pscMain.SvsId;
                JArray<char> vH;
                for (auto h = pscMain.Hosts.cbegin(), he = pscMain.Hosts.cend(); h != he; ++h) {
                    vH.push_back(JValue<char>(*h, false));
                }
                objMain["Hosts"] = std::move(vH);
                objMain["Threads"] = pscMain.Threads;
                objMain["Queue"] = pscMain.Queue;
                objMain["AutoConn"] = pscMain.AutoConn;
                objMain["AutoMerge"] = pscMain.AutoMerge;
                objMain["RecvTimeout"] = pscMain.RecvTimeout;
                objMain["ConnTimeout"] = pscMain.RecvTimeout;
                objMain["DefaultDb"] = pscMain.DefaultDb;

                //Slaves
                if (pscMain.Slaves.size()) {
                    JObject<char> vS;
                    for (auto one = pscMain.Slaves.cbegin(), onee = pscMain.Slaves.cend(); one != onee; ++one) {
                        const CPoolConfig &psc = one->second;
                        JObject<char> obj;
                        obj["SvsId"] = psc.SvsId;
                        JArray<char> vH;
                        for (auto h = psc.Hosts.cbegin(), he = psc.Hosts.cend(); h != he; ++h) {
                            vH.push_back(*h);
                        }
                        obj["Hosts"] = std::move(vH);
                        obj["Threads"] = psc.Threads;
                        obj["Queue"] = psc.Queue;
                        obj["AutoConn"] = psc.AutoConn;
                        obj["AutoMerge"] = psc.AutoMerge;
                        obj["RecvTimeout"] = psc.RecvTimeout;
                        obj["ConnTimeout"] = psc.ConnTimeout;
                        obj["DefaultDb"] = psc.DefaultDb;
                        obj["PoolType"] = (int) psc.PoolType;
                        vS[one->first] = std::move(obj);
                    }
                    objMain["Slaves"] = std::move(vS);
                }
                objMain["PoolType"] = (int) pscMain.PoolType;
                vP[it->first] = std::move(objMain);
            }
            jvRoot.AsObject()["Pools"] = std::move(vP);
            return jvRoot.Stringify();
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
            std::string jsFile(jsonConfig ? jsonConfig : "");
            Trim(jsFile);
            if (!jsFile.size()) {
                jsFile = "sp_config.json";
            }
            int errCode = 0;
            std::unique_ptr<JValue<char>> jv(JSON::ParseFromFile(jsFile.c_str(), errCode));
            if (errCode) {
                m_errMsg = "Cannot open configuration file " + jsFile;
                return false;
            } else if (!jv) {
                m_errMsg = "Bad JSON configuration object";
                return false;
            }
            JValue<char>* v = jv->Child("WorkingDir");
            if (v && v->GetType() == enumType::String) {
                std::string dir = v->AsString();
#ifdef WIN32_64
                Trim(Unescape(dir));
#else
                Trim(dir);
#endif
                SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(dir.c_str());
            }
            v = jv->Child("CertStore");
            if (v && v->GetType() == enumType::String) {
                CertStore = v->AsString();
                Trim(CertStore);
                if (CertStore.size()) {
                    CClientSocket::SSL::SetVerifyLocation(CertStore.c_str());
                }
            }
            v = jv->Child("QueuePassword");
            if (v && v->GetType() == enumType::String) {
                std::string qp = v->AsString();
                Trim(qp);
                if (qp.size()) {
                    SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(qp.c_str());
                    m_bQP = 1;
                }
            }
            v = jv->Child("KeysAllowed");
            if (v && v->GetType() == enumType::Array) {
                const auto& arr = v->AsArray();
                for (auto it = arr.cbegin(), end = arr.cend(); it != end; ++it) {
                    if (it->GetType() != enumType::String) {
                        continue;
                    }
                    std::string s = it->AsString();
                    Trim(s);
                    if (s.size()) {
                        ToLower(s);
                        CPoolConfig::KeysAllowed.push_back(std::move(s));
                    }
                }
            }
            v = jv->Child("Hosts");
            if (v && v->GetType() == enumType::Object) {
                const auto& arr = v->AsObject();
                for (auto it = arr.cbegin(), end = arr.cend(); it != end; ++it) {
                    const auto& cc = it->second;
                    if (cc.GetType() != enumType::Object) {
                        continue;
                    }
                    Hosts[it->first] = GetCC(cc.AsObject());
                }
            }
            v = jv->Child("Pools");
            if (v && v->GetType() == enumType::Object) {
                const auto& arr = v->AsObject();
                for (auto it = arr.cbegin(), end = arr.cend(); it != end; ++it) {
                    const auto& cc = it->second;
                    if (cc.GetType() != enumType::Object) {
                        continue;
                    }
                    Pools[it->first] = ClientSide::GetPool(true, cc);
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
