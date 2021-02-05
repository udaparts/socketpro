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
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value cs;
        cs.SetString(CertStore.c_str(), (rapidjson::SizeType)CertStore.size());
        doc.AddMember(KEY_CERT_STORE, cs, allocator);

        rapidjson::Value wd;
        std::string dir = SPA::ClientSide::CClientSocket::QueueConfigure::GetWorkDirectory();
        Trim(dir);
        wd.SetString(dir.c_str(), (rapidjson::SizeType)dir.size());
        doc.AddMember(KEY_WORKING_DIR, wd, allocator);

        doc.AddMember(KEY_QUEUE_PASSWORD, m_bQP, allocator);

        rapidjson::Value vH(rapidjson::kObjectType);
        for (auto &h : Hosts) {
            rapidjson::Value key(h.first.c_str(), (rapidjson::SizeType)h.first.size(), allocator);

            auto &ctx = h.second;
            rapidjson::Value obj(rapidjson::kObjectType);

            rapidjson::Value s(ctx.Host.c_str(), (rapidjson::SizeType)ctx.Host.size(), allocator);
            obj.AddMember(KEY_HOST, s, allocator);

            obj.AddMember(KEY_PORT, ctx.Port, doc.GetAllocator());

            std::string str = SPA::Utilities::ToUTF8(ctx.UserId);
            s.SetString(str.c_str(), (rapidjson::SizeType)str.size(), allocator);
            obj.AddMember(KEY_USER_ID, s, allocator);

            str = SPA::Utilities::ToUTF8(ctx.Password);
            s.SetString(str.c_str(), (rapidjson::SizeType)str.size(), allocator);
            obj.AddMember(KEY_PASSWORD, s, allocator);

            obj.AddMember(KEY_ENCRYPTION_METHOD, (int) ctx.EncrytionMethod, allocator);
            obj.AddMember(KEY_ZIP, ctx.Zip, allocator);
            obj.AddMember(KEY_V6, ctx.V6, allocator);
            vH.AddMember(key, obj, allocator);
        }
        doc.AddMember(KEY_HOSTS, vH, allocator);

        rapidjson::Value vKA(rapidjson::kArrayType);
        for (auto &ka : m_vKeyAllowed) {
            rapidjson::Value key(ka.c_str(), (rapidjson::SizeType)ka.size(), allocator);
            vKA.PushBack(key, allocator);
        }
        doc.AddMember(KEY_KEYS_ALLOWED, vKA, allocator);

        rapidjson::Value vP(rapidjson::kObjectType);
        for (auto &p : Pools) {
            rapidjson::Value key(p.first.c_str(), (rapidjson::SizeType)p.first.size(), allocator);
            const CPoolStartContext &pscMain = p.second;
            rapidjson::Value objMain(rapidjson::kObjectType);
            objMain.AddMember(KEY_SVS_ID, pscMain.SvsId, allocator);

            rapidjson::Value vH(rapidjson::kArrayType);
            for (auto &h : pscMain.Hosts) {
                rapidjson::Value s(h.c_str(), (rapidjson::SizeType)h.size(), allocator);
                vH.PushBack(s, allocator);
            }
            objMain.AddMember(KEY_HOSTS, vH, allocator);

            objMain.AddMember(KEY_THREADS, pscMain.Threads, allocator);
            rapidjson::Value s(pscMain.Queue.c_str(), (rapidjson::SizeType)pscMain.Queue.size(), allocator);
            objMain.AddMember(KEY_QUEUE_NAME, s, allocator);
            objMain.AddMember(KEY_AUTO_CONN, pscMain.AutoConn, allocator);
            objMain.AddMember(KEY_AUTO_MERGE, pscMain.AutoMerge, allocator);
            objMain.AddMember(KEY_RECV_TIMEOUT, pscMain.RecvTimeout, allocator);
            objMain.AddMember(KEY_CONN_TIMEOUT, pscMain.RecvTimeout, allocator);
            s.SetString(pscMain.DefaultDb.c_str(), (rapidjson::SizeType)pscMain.DefaultDb.size(), allocator);
            objMain.AddMember(KEY_DEFAULT_DB, s, allocator);

            //Slaves
            if (pscMain.Slaves.size()) {
                rapidjson::Value vS(rapidjson::kObjectType);
                for (auto &one : pscMain.Slaves) {
                    rapidjson::Value key(one.first.c_str(), (rapidjson::SizeType)one.first.size(), allocator);
                    const CPoolStartContext &psc = one.second;
                    rapidjson::Value obj(rapidjson::kObjectType);
                    obj.AddMember(KEY_SVS_ID, psc.SvsId, allocator);

                    rapidjson::Value vH(rapidjson::kArrayType);
                    for (auto &h : psc.Hosts) {
                        rapidjson::Value s(h.c_str(), (rapidjson::SizeType)h.size(), allocator);
                        vH.PushBack(s, allocator);
                    }
                    obj.AddMember(KEY_HOSTS, vH, allocator);

                    obj.AddMember(KEY_THREADS, psc.Threads, allocator);
                    rapidjson::Value s(psc.Queue.c_str(), (rapidjson::SizeType)psc.Queue.size(), allocator);
                    obj.AddMember(KEY_QUEUE_NAME, s, allocator);
                    obj.AddMember(KEY_AUTO_CONN, psc.AutoConn, allocator);
                    obj.AddMember(KEY_AUTO_MERGE, psc.AutoMerge, allocator);
                    obj.AddMember(KEY_RECV_TIMEOUT, psc.RecvTimeout, allocator);
                    obj.AddMember(KEY_CONN_TIMEOUT, psc.RecvTimeout, allocator);
                    s.SetString(psc.DefaultDb.c_str(), (rapidjson::SizeType)psc.DefaultDb.size(), allocator);
                    obj.AddMember(KEY_DEFAULT_DB, s, allocator);
                    obj.AddMember(KEY_POOL_TYPE, (int) psc.PoolType, allocator);
                    vS.AddMember(key, obj, allocator);
                }
                objMain.AddMember(KEY_SLAVES, vS, allocator);
            }
            objMain.AddMember(KEY_POOL_TYPE, (int) pscMain.PoolType, allocator);
            vP.AddMember(key, objMain, allocator);
        }
        doc.AddMember(KEY_POOLS, vP, allocator);

        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);
        m_jsonConfig = buffer.GetString();
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

    CConnectionContext CPhpManager::GetCC(const rapidjson::Value & cc) {
        CConnectionContext ctx;
        if (cc.HasMember(KEY_HOST) && cc[KEY_HOST].IsString()) {
            ctx.Host = cc[KEY_HOST].GetString();
            Trim(ctx.Host);
        }
        if (cc.HasMember(KEY_PORT) && cc[KEY_PORT].IsUint()) {
            ctx.Port = cc[KEY_PORT].GetUint();
        }
        if (cc.HasMember(KEY_USER_ID) && cc[KEY_USER_ID].IsString()) {
            std::string s = cc[KEY_USER_ID].GetString();
            Trim(s);
            ctx.UserId = SPA::Utilities::ToWide(s);
        }
        if (cc.HasMember(KEY_PASSWORD) && cc[KEY_PASSWORD].IsString()) {
            std::string s = cc[KEY_PASSWORD].GetString();
            Trim(s);
            ctx.Password = SPA::Utilities::ToWide(s);
        }
        if (cc.HasMember(KEY_ENCRYPTION_METHOD) && cc[KEY_ENCRYPTION_METHOD].IsUint()) {
            ctx.EncrytionMethod = cc[KEY_ENCRYPTION_METHOD].GetUint() ? SPA::tagEncryptionMethod::TLSv1 : SPA::tagEncryptionMethod::NoEncryption;
        }
        if (cc.HasMember(KEY_ZIP)) {
            ctx.Zip = cc[KEY_ZIP].GetBool();
        }
        if (cc.HasMember(KEY_V6)) {
            ctx.Zip = cc[KEY_V6].GetBool();
        }
        return ctx;
    }

    CPoolStartContext CPhpManager::GetPool(const rapidjson::Value & vPool) {
        CPoolStartContext psc;
        if (vPool.HasMember(KEY_QUEUE_NAME) && vPool[KEY_QUEUE_NAME].IsString()) {
            psc.Queue = vPool[KEY_QUEUE_NAME].GetString();
            Trim(psc.Queue);
#ifdef WIN32_64
            SPA::ToLower(psc.Queue);
#endif
        }
        if (vPool.HasMember(KEY_DEFAULT_DB) && vPool[KEY_DEFAULT_DB].IsString()) {
            psc.DefaultDb = vPool[KEY_DEFAULT_DB].GetString();
            Trim(psc.DefaultDb);
        }
        if (vPool.HasMember(KEY_SVS_ID) && vPool[KEY_SVS_ID].IsUint()) {
            psc.SvsId = vPool[KEY_SVS_ID].GetUint();
        }
        if (vPool.HasMember(KEY_THREADS) && vPool[KEY_THREADS].IsUint()) {
            psc.Threads = vPool[KEY_THREADS].GetUint();
        }
        if (vPool.HasMember(KEY_RECV_TIMEOUT) && vPool[KEY_RECV_TIMEOUT].IsUint()) {
            psc.RecvTimeout = vPool[KEY_RECV_TIMEOUT].GetUint();
        }
        if (vPool.HasMember(KEY_CONN_TIMEOUT) && vPool[KEY_CONN_TIMEOUT].IsUint()) {
            psc.ConnTimeout = vPool[KEY_CONN_TIMEOUT].GetUint();
        }
        if (vPool.HasMember(KEY_AUTO_CONN) && vPool[KEY_AUTO_CONN].IsBool()) {
            psc.AutoConn = vPool[KEY_AUTO_CONN].GetBool();
        }
        if (vPool.HasMember(KEY_AUTO_MERGE) && vPool[KEY_AUTO_MERGE].IsBool()) {
            psc.AutoMerge = vPool[KEY_AUTO_MERGE].GetBool();
        }
        if (vPool.HasMember(KEY_HOSTS) && vPool[KEY_HOSTS].IsArray()) {
            const auto& vH = vPool[KEY_HOSTS].GetArray();
            for (const auto& h : vH) {
                psc.Hosts.push_back(h.GetString());
            }
        }
        return psc;
    }

    Php::Value CPhpManager::Parse() {
        rapidjson::Document doc;
        doc.SetObject();
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
                std::shared_ptr<FILE> fp(fopen(Manager.m_ConfigPath.c_str(), "rb"), [](FILE * f) {
                    if (f) ::fclose(f);
                });
                if (!fp || ferror(fp.get())) {
                    throw Php::Exception("Cannot open " + SP_CONFIG);
                }
                fseek(fp.get(), 0, SEEK_END);
                long size = ftell(fp.get()) + sizeof (wchar_t);
                fseek(fp.get(), 0, SEEK_SET);
                SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), (unsigned int) size);
                sb->CleanTrack();
                rapidjson::FileReadStream is(fp.get(), (char*) sb->GetBuffer(), sb->GetMaxSize());
                const char *json = (const char*) sb->GetBuffer();
                rapidjson::ParseResult ok = doc.Parse(json, ::strlen(json));
                if (!ok) {
                    throw Php::Exception("Bad JSON configuration object");
                }
                if (doc.HasMember(KEY_WORKING_DIR) && doc[KEY_WORKING_DIR].IsString()) {
                    std::string dir = doc[KEY_WORKING_DIR].GetString();
                    Trim(dir);
                    SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(dir.c_str());
                }
                if (doc.HasMember(KEY_CERT_STORE) && doc[KEY_CERT_STORE].IsString()) {
                    Manager.CertStore = doc[KEY_CERT_STORE].GetString();
                    Trim(Manager.CertStore);
                }
                if (doc.HasMember(KEY_QUEUE_PASSWORD) && doc[KEY_QUEUE_PASSWORD].IsString()) {
                    std::string qp = doc[KEY_QUEUE_PASSWORD].GetString();
                    Trim(qp);
                    if (qp.size()) {
                        SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(qp.c_str());
                        Manager.m_bQP = 1;
                    }
                }
                if (doc.HasMember(KEY_KEYS_ALLOWED) && doc[KEY_KEYS_ALLOWED].IsArray()) {
                    const auto& arr = doc[KEY_KEYS_ALLOWED].GetArray();
                    for (const auto& v : arr) {
                        if (!v.IsString()) {
                            continue;
                        }
                        std::string s = v.GetString();
                        Trim(s);
                        if (s.size()) {
                            SPA::ToLower(s);
                            Manager.m_vKeyAllowed.push_back(s);
                        }
                    }
                }
                SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation(Manager.CertStore.c_str());
                if (doc.HasMember(KEY_HOSTS) && doc[KEY_HOSTS].IsObject()) {
                    const auto& arr = doc[KEY_HOSTS].GetObject();
                    for (auto it = arr.MemberBegin(), end = arr.MemberEnd(); it != end; ++it) {
                        const char* key = it->name.GetString();
                        const auto& cc = it->value;
                        if (!cc.IsObject()) {
                            continue;
                        }
                        Manager.Hosts[key] = GetCC(cc);
                    }
                }
                if (doc.HasMember(KEY_POOLS) && doc[KEY_POOLS].IsObject()) {
                    const auto& arr = doc[KEY_POOLS].GetObject();
                    for (auto it = arr.MemberBegin(), end = arr.MemberEnd(); it != end; ++it) {
                        const char* key = it->name.GetString();
                        const auto& ccMain = it->value;
                        if (!ccMain.IsObject()) {
                            continue;
                        }
                        CPoolStartContext psc = GetPool(ccMain);
                        if (psc.DefaultDb.size()) {
                            psc.PoolType = tagPoolType::Master;
                        } else {
                            psc.PoolType = tagPoolType::Regular;
                        }
                        if (psc.DefaultDb.size() && ccMain.HasMember(KEY_SLAVES) && ccMain[KEY_SLAVES].IsObject()) {
                            const auto& vSlave = ccMain[KEY_SLAVES].GetObject();
                            for (auto it = vSlave.MemberBegin(), end = vSlave.MemberEnd(); it != end; ++it) {
                                const char* skey = it->name.GetString();
                                const auto& cc = it->value;
                                if (!cc.IsObject()) {
                                    continue;
                                }
                                CPoolStartContext ps = GetPool(cc);
                                ps.SvsId = psc.SvsId;
                                if (!ps.DefaultDb.size()) {
                                    ps.DefaultDb = psc.DefaultDb;
                                }
                                ps.PoolType = tagPoolType::Slave;
                                psc.Slaves[skey] = ps;
                            }
                        }
                        Manager.Pools[key] = psc;
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
