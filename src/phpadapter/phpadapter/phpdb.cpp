#include "stdafx.h"
#include "phpdb.h"
#include "phpdbcolumninfo.h"

namespace PA {

	CPhpDb::CPhpDb(unsigned int poolId, CDBHandler *db, bool locked)
		: CPhpBaseHandler(locked, db, poolId), m_db(db) {
	}

	void CPhpDb::RegisterInto(Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		Register(handler);

		handler.method("Open", &CPhpDb::Open, {
			Php::ByVal("conn", Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Close", &CPhpDb::Close, {
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Prepare", &CPhpDb::Prepare, {
			Php::ByVal("sql", Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("Execute", &CPhpDb::Execute, {
			Php::ByVal("sql", Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("BeginTrans", &CPhpDb::BeginTrans, {
			Php::ByVal("isolation", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("EndTrans", &CPhpDb::EndTrans, {
			Php::ByVal("plan", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method("ExecuteBatch", &CPhpDb::ExecuteBatch, {
			Php::ByVal("isolation", Php::Type::Numeric),
			Php::ByVal("sql", Php::Type::String),
			Php::ByVal("vParam", Php::Type::Array),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		cs.add(handler);
	}

	Php::Value CPhpDb::Open(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::Execute(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::ExecuteBatch(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::Prepare(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::Close(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::BeginTrans(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::EndTrans(Php::Parameters &params) {
		return nullptr;
	}

	Php::Value CPhpDb::__get(const Php::Value &name) {
		if (name == "Opened") {
			return m_db->IsOpened();
		}
		else if (name == "Outputs") {
			return (int64_t)m_db->GetOutputs();
		}
		else if (name == "Parameters") {
			return (int64_t)m_db->GetParameters();
		}
		else if (name == "LastAffected") {
			return m_db->GetLastAffected();
		}
		else if (name == "DBMS" || name == "DBManagementSystem") {
			return m_db->GetDBManagementSystem();
		}
		else if (name == "LastDBError") {
			Php::Value dbErr;
			dbErr.set(PHP_ERR_CODE, m_db->GetLastDBErrorCode());
			std::wstring wem = m_db->GetLastDBErrorMessage();
			std::string em = SPA::Utilities::ToUTF8(wem.c_str(), wem.size());
			Trim(em);
			dbErr.set(PHP_ERR_MSG, em);
			return dbErr;
		}
		else if (name == "Connection") {
			std::wstring wc = m_db->GetConnection();
			std::string ac = SPA::Utilities::ToUTF8(wc.c_str(), wc.size());
			Trim(ac);
			return ac;
		}
		else if (name == "CallReturn") {
			return m_db->GetCallReturn();
		}
		else if (name == "Proc") {
			return m_db->IsProc();
		}
		else if (name == "RetVal" || name == "RetValue") {
			CPhpBuffer buff;
			*buff.GetBuffer() << m_db->GetRetValue();
			return buff.LoadObject();
		}
		else if (name == "ColMeta" || name == "ColumnInfo") {
			int index = 0;
			auto &cols = m_db->GetColumnInfo();
			Php::Array vMeta;
			for (auto &m : cols) {
				vMeta.set(index, Php::Object((SPA_CS_NS + PHP_DB_COLUMN_IFO).c_str(), new CPhpDBColumnInfo(m)));
				++index;
			}
			return vMeta;
		}
		return CPhpBaseHandler::__get(name);
	}
} //namespace PA
