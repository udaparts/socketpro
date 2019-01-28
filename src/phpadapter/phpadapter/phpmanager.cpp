#include "stdafx.h"
#include "phpmanager.h"
#include "phpconncontext.h"

namespace PA {

	CPhpManager CPhpManager::Manager(nullptr);

	CPhpManager::CPhpManager(CPhpManager *manager) : m_pManager(manager), m_bQP() {
	}

	CPhpManager::~CPhpManager() {
		if (!m_pManager) {
			Clean();
		}
	}

	void CPhpManager::Clean() {
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
				m_errMsg = "Remote server port number is zero";
				break;
			}
			CMapHost::const_iterator start = it;
			++start;
			for (; start != end; ++start) {
				if (start->first == it->first) {
					m_errMsg = "Host key (" + it->first + ") duplicacted";
					break;
				}
			}
		}
	}

	void CPhpManager::CheckPoolsError() {
		for (auto it = Pools.cbegin(), end = Pools.cend(); it != end; ++it) {
			if (!it->first.size()) {
				m_errMsg = "Pool key cannot be empty";
				break;
			}
			if (!it->second.DefaultDb.size() && it->second.Slaves.size()) {
				m_errMsg = "Slave array is not empty but DefaultDb string is empty";
				break;
			}
			auto &slaves = it->second.Slaves;
			for (auto sit = slaves.cbegin(), send = slaves.end(); sit != send; ++sit) {
				if (!sit->first.size()) {
					m_errMsg = "Slave pool key cannot be empty";
					break;
				}
				if (sit->first == it->first) {
					m_errMsg = "Pool key (" + it->first + ") duplicacted";
					break;
				}
				if (it->second.Queue.size() && it->second.Queue == sit->second.Queue) {
					m_errMsg = "Queue name (" + it->second.Queue + ") duplicacted";
					break;
				}
			}
			if (m_errMsg.size()) {
				break;
			}
			CPoolStartContext::CMapPool::const_iterator start = it;
			++start;
			for (; start != end; ++start) {
				if (start->first == it->first) {
					m_errMsg = "Pool key (" + it->first + ") duplicacted";
					break;
				}
				if (start->second.Queue.size() && start->second.Queue == it->second.Queue) {
					m_errMsg = "Pool queue (" + it->first + ") duplicacted";
					break;
				}
				auto &slaves = start->second.Slaves;
				for (auto sit = slaves.cbegin(), send = slaves.end(); sit != send; ++sit) {
					if (!sit->first.size()) {
						m_errMsg = "Slave pool key cannot be empty";
						break;
					}
					if (sit->first == it->first) {
						m_errMsg = "Pool key (" + it->first + ") duplicacted";
						break;
					}
					if (it->second.Queue.size() && it->second.Queue == sit->second.Queue) {
						m_errMsg = "Queue name (" + it->second.Queue + ") duplicacted";
						break;
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
			if (!WorkingDir.size()) {
				m_errMsg = "Bad working directory value";
				break;
			}
			if (m_bQP < 0) {
				m_errMsg = "Message queue password empty";
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

	void CPhpManager::__construct(Php::Parameters &params) {
	}

	Php::Value CPhpManager::GetConfig() {
		Php::Value v;
		v.set("CertStore", CertStore);
		v.set("WorkingDir", WorkingDir);
		v.set("QueuePassword", m_bQP);
		Php::Value vH;
		for (auto &h : Hosts) {
			CPhpConnContext *pcc = new CPhpConnContext;
			pcc->AnyData = h.second.AnyData;
			pcc->EncrytionMethod = h.second.EncrytionMethod;
			pcc->Host = h.second.Host;
			pcc->Port = h.second.Port;
			pcc->UserId = h.second.UserId;
			pcc->V6 = h.second.V6;
			pcc->Zip = h.second.Zip;
			//no password set
			Php::Object ctx((SPA_CS_NS + PHP_CONN_CONTEXT).c_str(), pcc);
			vH.set(h.first, ctx);
		}
		v.set("Hosts", vH);
		Php::Value vP;
		for (auto &p : Pools) {
			Php::Value obj;
			obj.set("SvsId", (int64_t)p.second.SvsId);
			
			Php::Array vHosts(p.second.Hosts);
			obj.set("Hosts", vHosts);
			
			obj.set("Threads", (int64_t)p.second.Threads);
			obj.set("Queue", p.second.Queue);
			obj.set("AutoConn", p.second.AutoConn);
			obj.set("AutoMerge", p.second.AutoMerge);
			obj.set("RecvTimeout", (int64_t)p.second.RecvTimeout);
			obj.set("ConnTimeout", (int64_t)p.second.ConnTimeout);
			obj.set("DefaultDb", p.second.DefaultDb);
			obj.set("PoolType", (int)p.second.PoolType);

			Php::Array slaves;
			int index = 0;
			for (auto &s : p.second.Slaves) {
				Php::Value v;
				Php::Array vHosts(s.second.Hosts);
				obj.set("SvsId", (int64_t)p.second.SvsId);
				v.set("Hosts", vHosts);
				v.set("Threads", (int64_t)s.second.Threads);
				v.set("Queue", s.second.Queue);
				v.set("AutoConn", s.second.AutoConn);
				v.set("AutoMerge", s.second.AutoMerge);
				v.set("RecvTimeout", (int64_t)s.second.RecvTimeout);
				v.set("ConnTimeout", (int64_t)s.second.ConnTimeout);
				v.set("DefaultDb", s.second.DefaultDb);
				v.set("PoolType", (int)s.second.PoolType);
				v.set(s.first, v);
				slaves.set(index, v);
				++index;
			}
			obj.set("Slaves", slaves);
			
			vP.set(p.first, obj);
		}
		v.set("Pools", vP);
		return v;
	}

	Php::Value CPhpManager::__get(const Php::Value &name) {
		if (m_pManager) {
			if (name == "Error") {
				return m_pManager->m_errMsg;
			}
			else if (name == "Config") {
				return m_pManager->GetConfig();
			}
		}
		return Php::Base::__get(name);
	}

	void CPhpManager::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpManager> manager(PHP_MANAGER);
		manager.method(PHP_CONSTRUCT, &CPhpManager::__construct, Php::Private);

		cs.add(manager);
	}

	Php::Value CPhpManager::Parse() {
		{
			SPA::CAutoLock al(Manager.m_cs);
			if (!Manager.m_ConfigPath.size()) {
				std::string jsFile = Php::SERVER["PHPRC"];
				Trim(jsFile);
				if (!jsFile.size()) {
					Manager.m_errMsg = "No PHPRC path available";
					throw Php::Exception(Manager.m_errMsg.c_str());
				}
				if (jsFile.back() != SYS_DIR) {
					jsFile.push_back(SYS_DIR);
				}
				Manager.WorkingDir = jsFile;
				jsFile += SP_CONFIG; //assuming sp_config.json inside PHP server directory
				Manager.m_ConfigPath = jsFile;
#ifdef WIN32_64
				Manager.CertStore = "root";
#else
				Manager.CertStore = "./";
#endif
			}
			else {
				return Php::Object((SPA_CS_NS + PHP_MANAGER).c_str(), new CPhpManager(&Manager));
			}
		}
		rapidjson::Document doc;
		doc.SetObject();
		do {
			try {
				std::shared_ptr<FILE> fp(fopen(Manager.m_ConfigPath.c_str(), "rb"), [](FILE *f) {
					::fclose(f);
				});
				if (!fp || ferror(fp.get())) {
					throw Php::Exception("Cannot open " + SP_CONFIG);
				}
				SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), 1024 * 64);
				sb->CleanTrack();
				rapidjson::FileReadStream is(fp.get(), (char*)sb->GetBuffer(), sb->GetMaxSize());
				const char *json = (const char*)sb->GetBuffer();
				rapidjson::ParseResult ok = doc.Parse(json, ::strlen(json));
				if (!ok) {
					throw Php::Exception("Bad JSON configuration object");
				}
				if (doc.HasMember("WorkingDir") && doc["WorkingDir"].IsString()) {
					Manager.WorkingDir = doc["WorkingDir"].GetString();
					Trim(Manager.WorkingDir);
				}
				if (doc.HasMember("CertStore") && doc["CertStore"].IsString()) {
					Manager.CertStore = doc["CertStore"].GetString();
					Trim(Manager.CertStore);
				}
				if (doc.HasMember("QueuePassword") && doc["QueuePassword"].IsString()) {
					std::string qp = doc["QueuePassword"].GetString();
					Trim(qp);
					if (qp.size()) {
						SPA::ClientSide::CClientSocket::QueueConfigure::SetMessageQueuePassword(qp.c_str());
						Manager.m_bQP = 1;
					}
					else {
						Manager.m_bQP = -1;
					}
				}
				SPA::ClientSide::CClientSocket::QueueConfigure::SetWorkDirectory(Manager.WorkingDir.c_str());
				SPA::ClientSide::CClientSocket::SSL::SetVerifyLocation(Manager.CertStore.c_str());

				if (doc.HasMember("Hosts") && doc["Hosts"].IsArray()) {
					auto arr = doc["Hosts"].GetArray();
					for (auto &v : arr) {
						if (!v.IsObject() || !v[0].IsString()) {
							continue;
						}
						std::string key = v[0].GetString();
						if (!v[key.c_str()].IsObject()) {
							continue;
						}
						auto cc = v[key.c_str()].GetObjectW();
						CConnectionContext ctx;
						if (cc.HasMember("Host") && cc["Host"].IsString()) {
							ctx.Host = cc["Host"].GetString();
							Trim(ctx.Host);
						}
						if (cc.HasMember("Port") && cc["Port"].IsUint()) {
							ctx.Port = cc["Port"].GetUint();
						}
						if (cc.HasMember("UserId") && cc["UserId"].IsString()) {
							std::string s = cc["UserId"].GetString();
							Trim(s);
							ctx.UserId = SPA::Utilities::ToWide(s.c_str(), s.size());
						}
						if (cc.HasMember("Password") && cc["Password"].IsString()) {
							std::string s = cc["Password"].GetString();
							Trim(s);
							ctx.Password = SPA::Utilities::ToWide(s.c_str(), s.size());
						}
						if (cc.HasMember("EncrytionMethod") && cc["EncrytionMethod"].IsInt()) {
							ctx.EncrytionMethod = cc["Port"].GetInt() ? SPA::TLSv1 : SPA::NoEncryption;
						}
						if (cc.HasMember("Zip") && cc["Zip"].IsBool()) {
							ctx.Zip = cc["Zip"].GetBool();
						}
						if (cc.HasMember("V6") && cc["V6"].IsBool()) {
							ctx.Zip = cc["V6"].GetBool();
						}
						Manager.Hosts[key] = ctx;
					}
				}
				if (doc.HasMember("Pools") && doc["Pools"].IsArray()) {
					auto arr = doc["Pools"].GetArray();
					for (auto &v : arr) {
						if (!v.Size() || !v.IsObject() || !v[0].IsString()) {
							continue;
						}
						std::string key = v[0].GetString();
						Trim(key);
						if (!v[key.c_str()].IsObject()) {
							continue;
						}
						auto cc = v[key.c_str()].GetObjectW();
						CPoolStartContext psc;
						if (cc.HasMember("Queue") && cc["Queue"].IsString()) {
							psc.Queue = cc["Queue"].GetString();
							Trim(psc.Queue);
							std::transform(psc.Queue.begin(), psc.Queue.end(), psc.Queue.begin(), ::tolower);
						}
						if (cc.HasMember("DefaultDb") && cc["DefaultDb"].IsString()) {
							psc.DefaultDb = cc["DefaultDb"].GetString();
							Trim(psc.DefaultDb);
						}
						if (cc.HasMember("SvsId") && cc["SvsId"].IsUint()) {
							psc.SvsId = cc["SvsId"].GetUint();
						}
						if (cc.HasMember("Threads") && cc["Threads"].IsUint()) {
							psc.Threads = cc["Threads"].GetUint();
						}
						if (cc.HasMember("RecvTimeout") && cc["RecvTimeout"].IsUint()) {
							psc.RecvTimeout = cc["RecvTimeout"].GetUint();
						}
						if (cc.HasMember("ConnTimeout") && cc["ConnTimeout"].IsUint()) {
							psc.ConnTimeout = cc["ConnTimeout"].GetUint();
						}
						if (cc.HasMember("AutoConn") && cc["AutoConn"].IsBool()) {
							psc.AutoConn = cc["AutoConn"].GetBool();
						}
						if (cc.HasMember("AutoMerge") && cc["AutoMerge"].IsBool()) {
							psc.AutoMerge = cc["AutoMerge"].GetBool();
						}
						if (psc.DefaultDb.size() && cc.HasMember("Slaves") && cc["Slaves"].IsArray()) {
							auto vSlave = cc["Slaves"].GetArray();
							for (auto &one : vSlave) {
								if (!one.Size() || !one.IsObject() || !one[0].IsString()) {
									continue;
								}
								std::string skey = one[0].GetString();
								Trim(skey);
								if (!one[skey.c_str()].IsObject()) {
									continue;
								}
								auto cc = one[skey.c_str()].GetObjectW();
								CPoolStartContext ps(psc);
								ps.Queue.clear();
								ps.Hosts.clear();
								ps.Threads = 1;
								ps.AutoConn = true;
								ps.AutoMerge = true;
								if (cc.HasMember("Queue") && cc["Queue"].IsString()) {
									ps.Queue = cc["Queue"].GetString();
									Trim(ps.Queue);
								}
								if (cc.HasMember("DefaultDb") && cc["DefaultDb"].IsString()) {
									ps.DefaultDb = cc["DefaultDb"].GetString();
									Trim(ps.DefaultDb);
								}
								if (cc.HasMember("Threads") && cc["Threads"].IsUint()) {
									ps.Threads = cc["Threads"].GetUint();
								}
								if (cc.HasMember("RecvTimeout") && cc["RecvTimeout"].IsUint()) {
									ps.RecvTimeout = cc["RecvTimeout"].GetUint();
								}
								if (cc.HasMember("ConnTimeout") && cc["ConnTimeout"].IsUint()) {
									ps.ConnTimeout = cc["ConnTimeout"].GetUint();
								}
								if (cc.HasMember("AutoConn") && cc["AutoConn"].IsBool()) {
									ps.AutoConn = cc["AutoConn"].GetBool();
								}
								if (cc.HasMember("AutoMerge") && cc["AutoMerge"].IsBool()) {
									ps.AutoMerge = cc["AutoMerge"].GetBool();
								}
								if (cc.HasMember("Hosts") && cc["Hosts"].IsArray()) {
									auto vH = doc["Hosts"].GetArray();
									for (auto &h : vH) {
										ps.Hosts.push_back(h.GetString());
										Trim(ps.Hosts.back());
									}
								}
								psc.Slaves[skey] = ps;
							}
						}
						if (cc.HasMember("Hosts") && cc["Hosts"].IsArray()) {
							auto vH = doc["Hosts"].GetArray();
							for (auto &h : vH) {
								psc.Hosts.push_back(h.GetString());
								Trim(psc.Hosts.back());
							}
						}
						Manager.Pools[key] = psc;
					}
				}
			}
			catch (std::exception &ex) {
				throw Php::Exception(ex.what());
			}
			catch (...) {
				throw Php::Exception("Unknown error found when parsing " + SP_CONFIG);
			}
		} while (false);
		Manager.CheckError();
		if (Manager.m_errMsg.size()) {
			throw Php::Exception(Manager.m_errMsg.c_str());
		}
		return Php::Object((SPA_CS_NS + PHP_MANAGER).c_str(), new CPhpManager(&Manager));
	}

}