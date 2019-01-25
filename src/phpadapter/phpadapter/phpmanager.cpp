#include "stdafx.h"
#include "phpmanager.h"

namespace PA {

	CPhpManager::CPhpManager(const char *config_path) : m_ConfigPath(config_path ? config_path : "") {
		Trim(m_ConfigPath);
	}

	CPhpManager::~CPhpManager() {
	}

	std::string CPhpManager::Parse() {
		CertStore.clear();
		Hosts.clear();
		Pools.clear();
		WorkingDir.clear();
		rapidjson::Document doc;
		doc.SetObject();
		do {
			if (!m_ConfigPath.size()) {
				doc.AddMember("ec", -1, doc.GetAllocator());
				doc.AddMember("em", "No configuration file name available", doc.GetAllocator());
				break;
			}
			try {
				std::shared_ptr<FILE> fp(fopen(m_ConfigPath.c_str(), "rb"), [](FILE *f) {
					::fclose(f);
				});
				if (!fp || ferror(fp.get())) {
					doc.AddMember("ec", -2, doc.GetAllocator());
					doc.AddMember("em", "Cannot open sp_config.json", doc.GetAllocator());
					break;
				}
				SPA::CScopeUQueue sb(SPA::GetOS(), SPA::IsBigEndian(), 1024 * 64);
				sb->CleanTrack();
				rapidjson::FileReadStream is(fp.get(), (char*)sb->GetBuffer(), sb->GetMaxSize());
				rapidjson::Document doc;
				const char *json = (const char*)sb->GetBuffer();
				rapidjson::ParseResult ok = doc.Parse(json, ::strlen(json));
				if (!ok) {
					doc.Clear();
					doc.SetObject();
					doc.AddMember("ec", -3, doc.GetAllocator());
					doc.AddMember("em", "Bad JSON object", doc.GetAllocator());
					break;
				}
				if (doc.HasMember("WorkingDir") && doc["WorkingDir"].IsString()) {
					WorkingDir = doc["WorkingDir"].GetString();
				}
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
						Hosts[key] = ctx;
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
						Pools[key] = psc;
					}
				}
			}
			catch (std::exception &ex) {
				doc.Clear();
				doc.SetObject();
				doc.AddMember("ec", -4, doc.GetAllocator());
				rapidjson::Value em;
				em.SetString(ex.what(), doc.GetAllocator());
				doc.AddMember("em", em, doc.GetAllocator());
			}
		} while (false);
		rapidjson::StringBuffer buffer;
		buffer.Clear();
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		return buffer.GetString();
	}

}