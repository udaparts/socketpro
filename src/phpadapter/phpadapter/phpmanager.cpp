#include "stdafx.h"
#include "phpmanager.h"

namespace PA
{
    CPhpManager CPhpManager::Manager(nullptr);

    CPhpManager::CPhpManager(CPhpManager * manager) : m_pManager(manager), m_bQP(0) {
    }

    CPhpManager::~CPhpManager() {
    }

    void CPhpManager::Clean() {
        SPA::CAutoLock al(m_cs);
        for (auto &p : Pools) {
            p.second.Clean();
            auto &slaves = p.second.Slaves;
            for (auto &s : slaves) {
                s.second.Clean();
            }
        }
    }

    CConnectionContext CPhpManager::FindByKey(const std::string & key) {
        for (auto &h : Hosts) {
            if (h.first == key) {
                return h.second;
            }
        }
        assert(false);
        CConnectionContext cc;
        return cc;
    }

    void CPhpManager::CheckHostsError() {
        for (auto it = Hosts.cbegin(), end = Hosts.cend(); it != end; ++it) {
            if (!it->first.size()) {
                m_errMsg = "Host key cannot be empty";
                break;
            }
            if (!it->second.Host.size()) {
                m_errMsg = "Remote server address cannot be empty";
                break;
            }
            if (!it->second.Port) {
                m_errMsg = "Remote server port number cannot be zero";
                break;
            }
            CMapHost::const_iterator start = it;
            ++start;
            for (; start != end; ++start) {
                if (start->second == it->second) {
                    m_errMsg = "Host connection context for key (" + it->first + ") duplicacted";
                    break;
                }
            }
        }
    }

    bool CPhpManager::FindHostKey(const std::string & key) {
        for (CMapHost::const_iterator it = Hosts.cbegin(), end = Hosts.cend(); it != end; ++it) {
            if (it->first == key) {
                return true;
            }
        }
        return false;
    }

    void CPhpManager::CheckPoolsError() {
        for (auto it = Pools.cbegin(), end = Pools.cend(); it != end; ++it) {
            if (!it->first.size()) {
                m_errMsg = "Pool key cannot be empty";
                return;
            }
            const CPoolStartContext &psc = it->second;
            if (!psc.DefaultDb.size() && psc.Slaves.size()) {
                m_errMsg = "Slave array is not empty but DefaultDb string is empty";
                return;
            }
            switch (psc.SvsId) {
                case (unsigned int) SPA::tagServiceID::sidChat:
                    if (psc.DefaultDb.size() || psc.Slaves.size()) {
                        m_errMsg = "Server queue service does not support master or slave pool";
                        return;
                    }
                    break;
                case (unsigned int) SPA::tagServiceID::sidFile:
                    if (psc.DefaultDb.size() || psc.Slaves.size()) {
                        m_errMsg = "Remote file service does not support master or slave pool";
                        return;
                    }
                    break;
                case (unsigned int) SPA::tagServiceID::sidODBC:
                    break;
                default:
                    if (psc.SvsId <= (unsigned int) SPA::tagServiceID::sidReserved) {
                        m_errMsg = "Bad Service identification number found";
                        return;
                    }
                    break;
            }
            const std::vector<std::string> &hosts = psc.Hosts;
            for (auto &h : hosts) {
                if (!FindHostKey(h)) {
                    m_errMsg = "Host key not found in root host array";
                    return;
                }
            }
            for (CPoolStartContext::CMapPool::const_iterator start = it; start != end; ++start) {
                if (it != start) {
                    if (start->first == it->first) {
                        m_errMsg = "Pool key (" + it->first + ") duplicacted";
                        return;
                    }
                    if (start->second.Queue.size() && start->second.Queue == psc.Queue) {
                        m_errMsg = "Pool queue (" + psc.Queue + ") duplicacted";
                        return;
                    }
                }
                auto &slaves = start->second.Slaves;
                for (auto sit = slaves.cbegin(), send = slaves.end(); sit != send; ++sit) {
                    if (sit->second.Slaves.size()) {
                        m_errMsg = "A slave pool cannot have its own slave";
                        return;
                    }
                    if (!sit->first.size()) {
                        m_errMsg = "Slave pool key cannot be empty";
                        return;
                    }
                    if (sit->first == it->first) {
                        m_errMsg = "Pool key (" + it->first + ") duplicacted";
                        return;
                    }
                    if (psc.Queue.size() && psc.Queue == sit->second.Queue) {
                        m_errMsg = "Queue name (" + psc.Queue + ") duplicacted";
                        return;
                    }
                    const std::vector<std::string> &hosts = sit->second.Hosts;
                    if (!hosts.size()) {
                        m_errMsg = "Slave host array cannot be empty";
                        return;
                    } else {
                        for (auto &h : hosts) {
                            if (!FindHostKey(h)) {
                                m_errMsg = "Slave host key not found in root host array";
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    void CPhpManager::CheckError() {
        do {
            if (!CertStore.size()) {
                m_errMsg = "Bad certificate store value";
                break;
            }
            if (!Hosts.size()) {
                m_errMsg = "No remote host specified";
                break;
            }
            if (!Pools.size()) {
                m_errMsg = "No pool specified";
                break;
            }
            CheckHostsError();
            if (m_errMsg.size()) {
                break;
            }
            CheckPoolsError();
        } while (false);
    }

    void CPhpManager::__construct(Php::Parameters & params) {
    }

    void CPhpManager::__destruct() {
    }

    Php::Value CPhpManager::GetConfig() {
        SPA::CAutoLock al(m_cs);
        if (m_jsonConfig.size()) {
            return m_jsonConfig;
        }
        SPA::JSON::JObject<char> objRoot;
        objRoot[KEY_CERT_STORE] = CertStore;
        std::string dir = SPA::ClientSide::CClientSocket::QueueConfigure::GetWorkDirectory();
        Trim(dir);
        objRoot[KEY_WORKING_DIR] = dir;
        objRoot[KEY_QUEUE_PASSWORD] = m_bQP;
        SPA::JSON::JObject<char> vH;
        for (auto &h : Hosts) {
            SPA::JSON::JObject<char> obj;
            auto &ctx = h.second;
            obj[KEY_HOST] = ctx.Host;
            obj[KEY_PORT] = ctx.Port;
            obj[KEY_USER_ID] = SPA::Utilities::ToUTF8(ctx.UserId);
            obj[KEY_PASSWORD] = SPA::Utilities::ToUTF8(ctx.Password);
            obj[KEY_ENCRYPTION_METHOD] = (int) ctx.EncrytionMethod;
            obj[KEY_ZIP] = ctx.Zip;
            obj[KEY_V6] = ctx.V6;
            vH[h.first] = std::move(obj);
        }
        objRoot[KEY_HOSTS] = std::move(vH);
        SPA::JSON::JArray<char> vKA;
        for (auto &ka : m_vKeyAllowed) {
            vKA.push_back(ka);
        }
        objRoot[KEY_KEYS_ALLOWED] = std::move(vKA);
        SPA::JSON::JObject<char> vP;
        for (auto &p : Pools) {
            const CPoolStartContext &pscMain = p.second;
            SPA::JSON::JObject<char> objMain;
            objMain[KEY_SVS_ID] = pscMain.SvsId;
            SPA::JSON::JArray<char> vH;
            for (auto &h : pscMain.Hosts) {
                vH.push_back(h);
            }
            objMain[KEY_HOSTS] = std::move(vH);
            objMain[KEY_THREADS] = pscMain.Threads;
            objMain[KEY_QUEUE_NAME] = pscMain.Queue;
            objMain[KEY_AUTO_CONN] = pscMain.AutoConn;
            objMain[KEY_AUTO_MERGE] = pscMain.AutoMerge;
            objMain[KEY_RECV_TIMEOUT] = pscMain.RecvTimeout;
            objMain[KEY_CONN_TIMEOUT] = pscMain.ConnTimeout;
            objMain[KEY_DEFAULT_DB] = pscMain.DefaultDb;

            //Slaves
            if (pscMain.Slaves.size()) {
                SPA::JSON::JObject<char> vS;
                for (auto &one : pscMain.Slaves) {
                    const CPoolStartContext &psc = one.second;
                    SPA::JSON::JObject<char> obj;
                    obj[KEY_SVS_ID] = psc.SvsId;
                    SPA::JSON::JArray<char> vH;
                    for (auto &h : psc.Hosts) {
                        vH.push_back(h);
                    }
                    obj[KEY_HOSTS] = std::move(vH);
                    obj[KEY_THREADS] = psc.Threads;
                    obj[KEY_QUEUE_NAME] = psc.Queue;

                    obj[KEY_AUTO_CONN] = psc.AutoConn;
                    obj[KEY_AUTO_MERGE] = psc.AutoMerge;
                    obj[KEY_RECV_TIMEOUT] = psc.RecvTimeout;
                    obj[KEY_CONN_TIMEOUT] = psc.ConnTimeout;
                    obj[KEY_DEFAULT_DB] = psc.DefaultDb;
                    obj[KEY_POOL_TYPE] = (int) psc.PoolType;
                    vS[one.first] = std::move(obj);
                }
                objMain[KEY_SLAVES] = std::move(vS);
            }
            objMain[KEY_POOL_TYPE] = (int) pscMain.PoolType;
            vP[p.first] = std::move(objMain);
        }
        objRoot[KEY_POOLS] = std::move(vP);
        SPA::JSON::JValue<char> jv(std::move(objRoot));
        m_jsonConfig = jv.Stringify();
        return m_jsonConfig;
    }

    Php::Value CPhpManager::__get(const Php::Value & name) {
        if (m_pManager) {
            if (name == "Error") {
                return m_pManager->m_errMsg;
            } else if (name == "Config") {
                return m_pManager->GetConfig();
            } else if (name == "Pools") {
                return (int64_t) SPA::ClientSide::ClientCoreLoader.GetNumberOfSocketPools();
            } else if (name == "Version") {
                return SPA::ClientSide::ClientCoreLoader.GetUClientSocketVersion();
            }
        }
        return Php::Base::__get(name);
    }

    Php::Value CPhpManager::GetPool(const Php::Value &vKey, int64_t secret) {
        std::string key = vKey.stringValue();
        Php::Value pool;
        SPA::CAutoLock al(m_cs);
        for (auto &p : Pools) {
            auto &slaves = p.second.Slaves;
            if (p.first == key) {
                pool = p.second.GetPool();
                break;
            } else if (slaves.size()) {
                for (auto &s : slaves) {
                    if (s.first == key) {
                        pool = s.second.GetPool();
                    }
                }
                if (!pool.isNull()) {
                    break;
                }
            }
        }
        if (pool.isObject()) {
            return pool;
        } else if (pool.isString()) {
            m_errMsg = pool.stringValue();
        } else if (!m_errMsg.size()) {
            m_errMsg = "Pool not found by a key " + key;
        }
        if (secret == PA::PHP_ADAPTER_SECRET) {
            Manager.m_errMsg = m_errMsg;
            return nullptr;
        }
        throw Php::Exception(m_errMsg);
    }

    Php::Value CPhpManager::GetPool(Php::Parameters & params) {
        return GetPool(params[0], 0);
    }

    void CPhpManager::RegisterInto(Php::Namespace & cs) {
        Php::Class<CPhpManager> manager(PHP_MANAGER);
        manager.method<&CPhpManager::__construct>(PHP_CONSTRUCT, Php::Private);
        manager.method<&CPhpManager::GetPool>("GetPool",{
            Php::ByVal("key", Php::Type::String),
            Php::ByVal("secret", Php::Type::Numeric, false)
        });
        cs.add(manager);
    }

    CConnectionContext CPhpManager::GetCC(const SPA::JSON::JValue<char> &cc) {
        CConnectionContext ctx;
        auto jv = cc.Child(KEY_HOST);
        if (jv && jv->GetType() == SPA::JSON::enumType::String) {
            ctx.Host = jv->AsString();
            Trim(SPA::JSON::Unescape(ctx.Host));
        }
        jv = cc.Child(KEY_PORT);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            ctx.Port = (unsigned short) jv->AsUint64();
        }
        jv = cc.Child(KEY_USER_ID);
        if (jv && jv->GetType() == SPA::JSON::enumType::String) {
            std::string s = jv->AsString();
            ctx.UserId = SPA::Utilities::ToWide(Trim(SPA::JSON::Unescape(s)));
        }
        jv = cc.Child(KEY_PASSWORD);
        if (jv && jv->GetType() == SPA::JSON::enumType::String) {
            std::string s = jv->AsString();
            ctx.Password = SPA::Utilities::ToWide(Trim(SPA::JSON::Unescape(s)));
        }
        jv = cc.Child(KEY_ENCRYPTION_METHOD);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            ctx.EncrytionMethod = jv->AsUint64() ? SPA::tagEncryptionMethod::TLSv1 : SPA::tagEncryptionMethod::NoEncryption;
        }
        jv = cc.Child(KEY_ZIP);
        if (jv && jv->GetType() == SPA::JSON::enumType::Bool) {
            ctx.Zip = jv->AsBool();
        }
        jv = cc.Child(KEY_V6);
        if (jv && jv->GetType() == SPA::JSON::enumType::Bool) {
            ctx.V6 = jv->AsBool();
        }
        return ctx;
    }

    CPoolStartContext CPhpManager::GetPool(const SPA::JSON::JValue<char>& cc) {
        CPoolStartContext psc;
        auto jv = cc.Child(KEY_QUEUE_NAME);
        if (jv && jv->GetType() == SPA::JSON::enumType::String) {
            psc.Queue = jv->AsString();
            Trim(SPA::JSON::Unescape(psc.Queue));
#ifdef WIN32_64
            SPA::ToLower(psc.Queue);
#endif
        }
        jv = cc.Child(KEY_DEFAULT_DB);
        if (jv && jv->GetType() == SPA::JSON::enumType::String) {
            psc.DefaultDb = jv->AsString();
            Trim(SPA::JSON::Unescape(psc.DefaultDb));
        }

        jv = cc.Child(KEY_SVS_ID);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            psc.SvsId = (unsigned int) jv->AsUint64();
        }
        jv = cc.Child(KEY_THREADS);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            psc.Threads = (unsigned int) jv->AsUint64();
        }
        jv = cc.Child(KEY_RECV_TIMEOUT);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            psc.RecvTimeout = (unsigned int) jv->AsUint64();
        }
        jv = cc.Child(KEY_CONN_TIMEOUT);
        if (jv && jv->GetType() == SPA::JSON::enumType::Uint64) {
            psc.ConnTimeout = (unsigned int) jv->AsUint64();
        }
        jv = cc.Child(KEY_AUTO_CONN);
        if (jv && jv->GetType() == SPA::JSON::enumType::Bool) {
            psc.AutoConn = jv->AsBool();
        }
        jv = cc.Child(KEY_AUTO_MERGE);
        if (jv && jv->GetType() == SPA::JSON::enumType::Bool) {
            psc.AutoMerge = jv->AsBool();
        }
        jv = cc.Child(KEY_HOSTS);
        if (jv && jv->GetType() == SPA::JSON::enumType::Array) {
            auto& vH = jv->AsArray();
            for (const auto& h : vH) {
                if (h.GetType() == SPA::JSON::enumType::String) {
                    std::string host = h.AsString();
                    psc.Hosts.push_back(std::move(Trim(SPA::JSON::Unescape(host))));
                }
            }
        }
        return psc;
    }

    Php::Value CPhpManager::Parse() {
        SPA::CAutoLock al(Manager.m_cs);
        if (!Manager.m_ConfigPath.size()) {
            std::string jsFile = Php::ini_get(SP_CONFIG_DIR.c_str());
            if (!jsFile.size()) {
                jsFile = Php::call("php_ini_loaded_file").stringValue();
                size_t pos = jsFile.rfind(SYS_DIR);
                jsFile = jsFile.substr(0, pos);
            }
            Trim(jsFile);
            if (!jsFile.size()) {
                Manager.m_errMsg = "No php.ini path found";
                throw Php::Exception(Manager.m_errMsg.c_str());
            }
            if (jsFile.back() != SYS_DIR) {
                jsFile.push_back(SYS_DIR);
            }
            SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(jsFile.c_str());
            jsFile += SP_CONFIG; //assuming sp_config.json inside PHP server directory
            Manager.m_ConfigPath = jsFile;
#ifdef WIN32_64
            Manager.CertStore = "root";
#else
            Manager.CertStore = Php::call("openssl_get_cert_locations").get("default_cert_dir").stringValue();
#endif
        } else {
            return Php::Object((SPA_CS_NS + PHP_MANAGER).c_str(), new CPhpManager(&Manager));
        }
        do {
            try{
                int errCode = 0;
                std::shared_ptr<SPA::JSON::JValue<char>> config(SPA::JSON::ParseFromFile(Manager.m_ConfigPath.c_str(), errCode));
                if (errCode) {
                    throw Php::Exception("Cannot open " + SP_CONFIG);
                } else if (!config) {
                    throw Php::Exception("Bad JSON configuration object");
                }
                auto jv = config->Child(KEY_WORKING_DIR);
                if (jv && jv->GetType() == SPA::JSON::enumType::String) {
                    std::string dir = jv->AsString();
                    Trim(SPA::JSON::Unescape(dir));
                    SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(dir.c_str());
                }
                jv = config->Child(KEY_CERT_STORE);
                if (jv && jv->GetType() == SPA::JSON::enumType::String) {
                    Manager.CertStore = jv->AsString();
                    Trim(SPA::JSON::Unescape(Manager.CertStore));
                }
                jv = config->Child(KEY_QUEUE_PASSWORD);
                if (jv && jv->GetType() == SPA::JSON::enumType::String) {
                    std::string qp = jv->AsString();
                    Trim(SPA::JSON::Unescape(qp));
                    if (qp.size()) {
                        SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(qp.c_str());
                        Manager.m_bQP = 1;
                    }
                }
                jv = config->Child(KEY_KEYS_ALLOWED);
                if (jv && jv->GetType() == SPA::JSON::enumType::Array) {
                    auto& arr = jv->AsArray();
                    for (auto& v : arr) {
                        if (v.GetType() != SPA::JSON::enumType::String) {
                            continue;
                        }
                        std::string s = v.AsString();
                        Trim(s);
                        if (s.size()) {
                            SPA::ToLower(s);
                            Manager.m_vKeyAllowed.push_back(std::move(s));
                        }
                    }
                }
                SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation(Manager.CertStore.c_str());
                jv = config->Child(KEY_HOSTS);
                if (jv && jv->GetType() == SPA::JSON::enumType::Object) {
                    auto& arr = jv->AsObject();
                    for (auto it = arr.begin(), end = arr.end(); it != end; ++it) {
                        auto& cc = it->second;
                        if (cc.GetType() != SPA::JSON::enumType::Object) {
                            continue;
                        }
                        Manager.Hosts[it->first] = GetCC(cc);
                    }
                }
                jv = config->Child(KEY_POOLS);
                if (jv && jv->GetType() == SPA::JSON::enumType::Object) {
                    auto& arr = jv->AsObject();
                    for (auto it = arr.begin(), end = arr.end(); it != end; ++it) {
                        auto& ccMain = it->second;
                        if (ccMain.GetType() != SPA::JSON::enumType::Object) {
                            continue;
                        }
                        CPoolStartContext psc = GetPool(ccMain);
                        if (psc.DefaultDb.size()) {
                            psc.PoolType = tagPoolType::Master;
                        } else {
                            psc.PoolType = tagPoolType::Regular;
                        }
                        if (psc.DefaultDb.size() && ccMain.Child(KEY_SLAVES) && ccMain.Child(KEY_SLAVES)->GetType() == SPA::JSON::enumType::Object) {
                            auto& vSlave = ccMain[KEY_SLAVES].AsObject();
                            for (auto it = vSlave.begin(), end = vSlave.end(); it != end; ++it) {
                                auto& cc = it->second;
                                if (cc.GetType() != SPA::JSON::enumType::Object) {
                                    continue;
                                }
                                CPoolStartContext ps = GetPool(cc);
                                ps.SvsId = psc.SvsId;
                                if (!ps.DefaultDb.size()) {
                                    ps.DefaultDb = psc.DefaultDb;
                                }
                                ps.PoolType = tagPoolType::Slave;
                                psc.Slaves[it->first] = ps;
                            }
                        }
                        Manager.Pools[it->first] = psc;
                    }
                }
            }

            catch(std::exception & ex) {
                Manager.m_errMsg = ex.what();
            }

            catch(...) {
                Manager.m_errMsg = "Unknown error found when parsing " + SP_CONFIG;
            }
        } while (false);
        if (!Manager.m_errMsg.size()) {
            Manager.CheckError();
        }
        if (Manager.m_errMsg.size()) {
            Manager.m_vKeyAllowed.clear();
            Manager.CertStore.clear();
            Manager.Pools.clear();
            Manager.m_ConfigPath.clear();
            Manager.Hosts.clear();
            throw Php::Exception(Manager.m_errMsg.c_str());
        } else {
            Manager.SetSettings();
        }
        return Php::Object((SPA_CS_NS + PHP_MANAGER).c_str(), new CPhpManager(&Manager));
    }

    void CPhpManager::SetErrorMsg(const std::string & em) {
        m_errMsg = em;
    }

    std::string CPhpManager::GetErrorMsg() {
        SPA::CAutoLock al(m_cs);
        return m_errMsg;
    }

    void CPhpManager::SetSettings() {
        for (auto &p : Pools) {
            CPoolStartContext &psc = p.second;
            psc.AutoConn = true; //set autoconn to true for now
            if (psc.SvsId == (unsigned int) SPA::tagServiceID::sidChat || psc.SvsId == (unsigned int) SPA::tagServiceID::sidFile) {
                //don't support master/slave at all
                psc.Slaves.clear();
                psc.DefaultDb.clear();
                psc.PoolType = tagPoolType::Regular; //regular socket pool
            }
            if (psc.Queue.size() && psc.AutoMerge && ComputeDiff(psc.Hosts) <= 1) {
                psc.AutoMerge = false;
            }
            if (!psc.Slaves.size()) {
                continue;
            }
            for (auto &s : psc.Slaves) {
                CPoolStartContext &ps = s.second;

                ps.AutoConn = true; //set autoconn to true for now

                if (ps.Queue.size() && ps.AutoMerge && ComputeDiff(ps.Hosts) <= 1) {
                    ps.AutoMerge = false;
                }
            }
        }
    }

    size_t CPhpManager::ComputeDiff(const std::vector<std::string> &v) {
        std::vector<std::string> vHost;
        for (auto it = v.cbegin(), end = v.cend(); it != end; ++it) {
            auto found = std::find(vHost.cbegin(), vHost.cend(), *it);
            if (found == vHost.cend()) {
                vHost.push_back(*it);
            }
        }
        return vHost.size();
    }

}
