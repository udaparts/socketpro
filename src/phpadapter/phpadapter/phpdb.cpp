#include "stdafx.h"
#include "phpdb.h"
#include "phpdbcolumninfo.h"
#include "phpbuffer.h"

namespace PA {

	const char* CPhpDb::PHP_DB_AFFECTED = "affected";
	const char* CPhpDb::PHP_DB_FAILS = "fails";
	const char* CPhpDb::PHP_DB_OKS = "oks";
	const char* CPhpDb::PHP_DB_LAST_ID = "id";

	CPhpDb::CPhpDb(unsigned int poolId, CDBHandler *db, bool locked)
		: CPhpBaseHandler(locked, db, poolId), m_db(db) {
	}

	void CPhpDb::RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace &cs) {
		Php::Class<CPhpDb> handler(PHP_DB_HANDLER);
		handler.extends(base);

		//database-related request ids
		handler.property("idOpen", SPA::UDB::idOpen, Php::Const);
		handler.property("idClose", SPA::UDB::idClose, Php::Const);
		handler.property("idBeginTrans", SPA::UDB::idBeginTrans, Php::Const);
		handler.property("idEndTrans", SPA::UDB::idEndTrans, Php::Const);
		handler.property("idExecute", SPA::UDB::idExecute, Php::Const);
		handler.property("idPrepare", SPA::UDB::idPrepare, Php::Const);
		handler.property("idExecuteParameters", SPA::UDB::idExecuteParameters, Php::Const);
		handler.property("idDBUpdate", SPA::UDB::idDBUpdate, Php::Const);
		handler.property("idRowsetHeader", SPA::UDB::idRowsetHeader, Php::Const);
		handler.property("idOutputParameter", SPA::UDB::idOutputParameter, Php::Const);
		handler.property("idBeginRows", SPA::UDB::idBeginRows, Php::Const);
		handler.property("idTransferring", SPA::UDB::idTransferring, Php::Const);
		handler.property("idStartBLOB", SPA::UDB::idStartBLOB, Php::Const);
		handler.property("idChunk", SPA::UDB::idChunk, Php::Const);
		handler.property("idEndBLOB", SPA::UDB::idEndBLOB, Php::Const);
		handler.property("idEndRows", SPA::UDB::idEndRows, Php::Const);
		handler.property("idCallReturn", SPA::UDB::idCallReturn, Php::Const);
		handler.property("idGetCachedTables", SPA::UDB::idGetCachedTables, Php::Const);
		handler.property("idSqlBatchHeader", SPA::UDB::idSqlBatchHeader, Php::Const);
		handler.property("idExecuteBatch", SPA::UDB::idExecuteBatch, Php::Const);
		handler.property("idParameterPosition", SPA::UDB::idParameterPosition, Php::Const);

		//tagManagementSystem
		handler.property("Unknown", SPA::UDB::msUnknown, Php::Const);
		handler.property("Sqlite", SPA::UDB::msSqlite, Php::Const);
		handler.property("Mysql", SPA::UDB::msMysql, Php::Const);
		handler.property("ODBC", SPA::UDB::msODBC, Php::Const);
		handler.property("MsSQL", SPA::UDB::msMsSQL, Php::Const);
		handler.property("Oracle", SPA::UDB::msOracle, Php::Const);
		handler.property("DB2", SPA::UDB::msDB2, Php::Const);
		handler.property("PostgreSQL", SPA::UDB::msPostgreSQL, Php::Const);
		handler.property("MongoDB", SPA::UDB::msMongoDB, Php::Const);

		//tagTransactionIsolation
		handler.property("tiUnspecified", SPA::UDB::tiUnspecified, Php::Const);
		handler.property("tiChaos", SPA::UDB::tiChaos, Php::Const);
		handler.property("tiReadUncommited", SPA::UDB::tiReadUncommited, Php::Const);
		handler.property("tiBrowse", SPA::UDB::tiBrowse, Php::Const);
		handler.property("tiCursorStability", SPA::UDB::tiCursorStability, Php::Const);
		handler.property("tiReadCommited", SPA::UDB::tiReadCommited, Php::Const);
		handler.property("tiRepeatableRead", SPA::UDB::tiRepeatableRead, Php::Const);
		handler.property("tiSerializable", SPA::UDB::tiSerializable, Php::Const);
		handler.property("tiIsolated", SPA::UDB::tiIsolated, Php::Const);

		//tagRollbackPlan
		handler.property("rpDefault", SPA::UDB::rpDefault, Php::Const);
		handler.property("rpRollbackErrorAny", SPA::UDB::rpRollbackErrorAny, Php::Const);
		handler.property("rpRollbackErrorLess", SPA::UDB::rpRollbackErrorLess, Php::Const);
		handler.property("rpRollbackErrorEqual", SPA::UDB::rpRollbackErrorEqual, Php::Const);
		handler.property("rpRollbackErrorMore", SPA::UDB::rpRollbackErrorMore, Php::Const);
		handler.property("rpRollbackErrorAll", SPA::UDB::rpRollbackErrorAll, Php::Const);
		handler.property("rpRollbackAlways", SPA::UDB::rpRollbackAlways, Php::Const);

		//tagUpdateEvent
		handler.property("ueUnknown", SPA::UDB::ueUnknown, Php::Const);
		handler.property("ueInsert", SPA::UDB::ueInsert, Php::Const);
		handler.property("ueUpdate", SPA::UDB::ueUpdate, Php::Const);
		handler.property("ueDelete", SPA::UDB::ueDelete, Php::Const);

		handler.method<&CPhpDb::Open>("Open", {
			Php::ByVal("conn", Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::Close>("Close", {
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::Prepare>("Prepare", {
			Php::ByVal("sql", Php::Type::String),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::Execute>("Execute", {
			Php::ByVal("sql", Php::Type::Null), //string or array of parameter data
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::BeginTrans>("BeginTrans", {
			Php::ByVal("isolation", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::EndTrans>("EndTrans", {
			Php::ByVal("plan", Php::Type::Numeric),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		handler.method<&CPhpDb::ExecuteBatch>("ExecuteBatch", {
			Php::ByVal("isolation", Php::Type::Numeric),
			Php::ByVal("sql", Php::Type::String),
			Php::ByVal("vParam", Php::Type::Null),
			Php::ByVal(PHP_SENDREQUEST_SYNC, Php::Type::Null)
		});
		cs.add(handler);
	}

	void CPhpDb::PopTopCallbacks(PACallback &cb) {

	}

	Php::Value CPhpDb::Open(Php::Parameters &params) {
		unsigned int timeout;
		std::string aconn = params[0].stringValue();
		Trim(aconn);
		std::wstring conn = SPA::Utilities::ToWide(aconn);
		std::shared_ptr<Php::Value> pV;
		auto Dr = SetResCallback(params[1], pV, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::UDB::idOpen, pV ? true : false);
		unsigned int flags = 0;
		if (args > 3) {
			Php::Value vFlags = params[3];
			if (vFlags.isNumeric()) {
				flags = (unsigned int)vFlags.numericValue();
			}
			else if (!vFlags.isNull()) {
				throw Php::Exception("The Open method flags must be an integer value");
			}
		}
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->Open(conn.c_str(), Dr, flags, discarded), lk, timeout);
			return *pV;
		}
		return m_db->Open(conn.c_str(), Dr, flags, discarded);
	}

	CDBHandler::DExecuteResult CPhpDb::SetExeResCallback(const Php::Value &phpDR, std::shared_ptr<Php::Value> &pV, unsigned int &timeout) {
		timeout = (~0);
		bool sync = false;
		if (phpDR.isNumeric()) {
			sync = true;
			timeout = (unsigned int)phpDR.numericValue();
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for Execute final result");
		}
		if (sync) {
			pV.reset(new Php::Value);
		}
		else {
			pV.reset();
		}
		CDBHandler::DExecuteResult Dr = [phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant& vtId) {
			unsigned int fails = (unsigned int)(fail_ok >> 32);
			unsigned int oks = (unsigned int)fail_ok;
			std::string em = SPA::Utilities::ToUTF8(errMsg);
			Trim(em);
			CPhpBuffer buff;
			*buff.GetBuffer() << vtId;
			if (phpDR) {
				pV->set(PHP_ERR_CODE, res);
				pV->set(PHP_ERR_MSG, em);
				pV->set(PHP_DB_AFFECTED, affected);
				pV->set(PHP_DB_FAILS, (int64_t)fails);
				pV->set(PHP_DB_OKS, (int64_t)oks);
				pV->set(PHP_DB_LAST_ID, buff.LoadObject());
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				v.set(PHP_DB_AFFECTED, affected);
				v.set(PHP_DB_FAILS, (int64_t)fails);
				v.set(PHP_DB_OKS, (int64_t)oks);
				v.set(PHP_DB_LAST_ID, buff.LoadObject());
				phpDR(v);
			}
		};
		return Dr;
	}

	CDBHandler::DRows CPhpDb::SetRCallback(const Php::Value &phpRow) {
		if (phpRow.isNull()) {
		}
		else if (!phpRow.isCallable()) {
			throw Php::Exception("A callback required for row data event");
		}
		CDBHandler::DRows r = [phpRow, this](CDBHandler &db, Php::Value &vData) {
			if (phpRow.isCallable()) {
				Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(this->GetPoolId(), &db, false));
				phpRow(vData, db.IsProc(), obj);
			}
		};
		return r;
	}

	CDBHandler::DRowsetHeader CPhpDb::SetRHCallback(const Php::Value &phpRh, bool batch) {
		if (phpRh.isNull()) {
		}
		else if (!phpRh.isCallable()) {
			throw Php::Exception(batch ? "A callback required for ExecuteBatch header event" : "A callback required for rowset header event");
		}
		CDBHandler::DRowsetHeader rh = [phpRh, this](CDBHandler &db) {
			if (phpRh.isCallable()) {
				Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(this->GetPoolId(), &db, false));
				phpRh(obj);
			}
		};
		return rh;
	}

	Php::Value CPhpDb::Execute(Php::Parameters &params) {
		unsigned int timeout;
		SPA::UDB::CDBVariantArray vParam;
		std::wstring sql;
		if (params[0].isString()) {
			std::string asql = params[0].stringValue();
			Trim(asql);
			if (!asql.size()) {
				throw Php::Exception("SQL statement cannot be empty");
			}
			sql = SPA::Utilities::ToWide(asql);
		}
		else {
			GetParams(params[0], vParam);
		}

		std::shared_ptr<Php::Value> pV;
		auto Dr = SetExeResCallback(params[1], pV, timeout);

		size_t args = params.size();
		Php::Value phpRow;
		if (args > 2) {
			phpRow = params[2];
		}
		CDBHandler::DRows r = SetRCallback(phpRow);

		Php::Value phpRh;
		if (args > 3) {
			phpRh = params[3];
		}
		CDBHandler::DRowsetHeader rh = SetRHCallback(phpRh);

		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
		}
		auto discarded = SetAbortCallback(phpCanceled, SPA::UDB::idExecute, pV ? true : false);

		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded), lk, timeout);
			return *pV;
		}
		return sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded);
	}

	void CPhpDb::GetParams(const Php::Value &vP, SPA::UDB::CDBVariantArray &vParam) {
		if (vP.isArray()) {
			int count = vP.length();
			for (int n = 0; n < count; ++n) {
				SPA::UDB::CDBVariant vt;
				ToVariant(vP.get(n), vt);
				vParam.push_back(std::move(vt));
			}
		}
		else if (vP.instanceOf(SPA_CS_NS + PHP_BUFFER)) {
			Php::Value bytes = vP.call("PopBytes");
			const char *raw = bytes.rawValue();
			int len = bytes.length();
			SPA::CScopeUQueue sb;
			sb->Push((const unsigned char*)raw, (unsigned int)len);
			while (sb->GetSize()) {
				try {
					SPA::UDB::CDBVariant vt;
					*sb >> vt;
					vParam.push_back(std::move(vt));
				}
				catch (SPA::CUException &err) {
					throw Php::Exception(err.what());
				}
			}
		}
		else {
			throw Php::Exception("A SQL statement or an array of parameter data expected");
		}
	}

	Php::Value CPhpDb::ExecuteBatch(Php::Parameters &params) {
		unsigned int timeout;
		int64_t iso = params[0].numericValue();
		if (iso < SPA::UDB::tiUnspecified || iso > SPA::UDB::tiIsolated) {
			throw Php::Exception("Bad transaction isolation value");
		}
		SPA::UDB::tagTransactionIsolation ti = (SPA::UDB::tagTransactionIsolation)iso;

		std::string asql = params[1].stringValue();
		Trim(asql);
		if (!asql.size()) {
			throw Php::Exception("SQL statement cannot be empty");
		}
		std::wstring sql = SPA::Utilities::ToWide(asql);
		
		SPA::UDB::CDBVariantArray vParam;
		GetParams(params[2], vParam);

		std::shared_ptr<Php::Value> pV;
		CDBHandler::DExecuteResult Dr = SetExeResCallback(params[3], pV, timeout);
		size_t args = params.size();
		Php::Value phpRow;
		if (args > 4) {
			phpRow = params[4];
		}
		CDBHandler::DRows r = SetRCallback(phpRow);

		Php::Value phpRh;
		if (args > 5) {
			phpRh = params[5];
		}
		CDBHandler::DRowsetHeader rh = SetRHCallback(phpRh);

		Php::Value phpBh;
		if (args > 6) {
			phpBh = params[6];
		}
		CDBHandler::DRowsetHeader bh = SetRHCallback(phpBh, true);

		SPA::UDB::tagRollbackPlan plan = SPA::UDB::rpDefault;
		if (args > 7) {
			if (params[7].isNumeric()) {
				int64_t p = params[7].numericValue();
				if (p < SPA::UDB::rpDefault || p > SPA::UDB::rpRollbackAlways) {
					throw Php::Exception("Bad rollback plan value");
				}
				plan = (SPA::UDB::tagRollbackPlan)p;
			}
			else if (!params[7].isNull()) {
				throw Php::Exception("An integer required for ExecuteBatch rollback plan");
			}
		}
		Php::Value phpCanceled;
		if (args > 8) {
			phpCanceled = params[8];
		}
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idExecuteBatch, pV ? true : false);
		
		std::wstring delimiter(L";");
		if (args > 9) {
			if (params[9].isString()) {
				std::string s = params[9].stringValue();
				Trim(s);
				if (!s.size()) {
					throw Php::Exception("Delimiter string cannot be empty");
				}
				delimiter = SPA::Utilities::ToWide(s);
			}
			else if (!params[9].isNull()) {
				throw Php::Exception("A string required for delimiter");
			}
		}

		SPA::UDB::CParameterInfoArray vPInfo;
		if (args > 10) {
			Php::Value vParamInfo = params[10];
			if (vParamInfo.isArray()) {
				vPInfo = ConvertFrom(vParamInfo);
			}
			else if (!vParamInfo.isNull()) {
				throw Php::Exception("An array of parameter info structures required");
			}
		}
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->ExecuteBatch(ti, sql.c_str(), vParam, Dr, r, rh, bh, vPInfo, plan, discarded, delimiter.c_str()), lk, timeout);
			return *pV;
		}
		return m_db->ExecuteBatch(ti, sql.c_str(), vParam, Dr, r, rh, bh, vPInfo, plan, discarded, delimiter.c_str());
	}

	SPA::UDB::CParameterInfoArray CPhpDb::ConvertFrom(Php::Value vP) {
		SPA::UDB::CParameterInfoArray vPInfo;
		int count = vP.length();
		for (int n = 0; n < count; ++n) {
			Php::Value vP = vP.get(n);
			if (vP.instanceOf(SPA_CS_NS + PHP_DB_PARAMETER_INFO)) {
				SPA::UDB::CParameterInfo pi;
				pi.Direction = (SPA::UDB::tagParameterDirection)vP.get("Direction").numericValue();
				pi.DataType = (VARTYPE)vP.get(PHP_DATATYPE).numericValue();
				pi.ColumnSize = (unsigned int)vP.get(PHP_COLUMN_SIZE).numericValue();
				pi.Precision = (unsigned char)vP.get(PHP_COLUMN_PRECSISON).numericValue();
				pi.Scale = (unsigned char)vP.get(PHP_COLUMN_SCALE).numericValue();
				std::string s = vP.get("ParameterName").stringValue();
				pi.ParameterName = SPA::Utilities::ToWide(s);
				vPInfo.push_back(pi);
			}
			else {
				throw Php::Exception("A parameter info structure is expected");
			}
		}
		return vPInfo;
	}

	Php::Value CPhpDb::Prepare(Php::Parameters &params) {
		unsigned int timeout;
		std::string asql = params[0].stringValue();
		Trim(asql);
		if (!asql.size()) {
			throw Php::Exception("SQL statement cannot be empty");
		}
		std::wstring sql = SPA::Utilities::ToWide(asql);
		std::shared_ptr<Php::Value> pV;
		CDBHandler::DResult Dr = SetResCallback(params[1], pV, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
		}
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idPrepare, pV ? true : false);
		SPA::UDB::CParameterInfoArray vPInfo;
		if (args > 3) {
			Php::Value vParamInfo = params[3];
			if (vParamInfo.isArray()) {
				vPInfo = ConvertFrom(vParamInfo);
			}
			else if (!vParamInfo.isNull()) {
				throw Php::Exception("An array of parameter info structures required");
			}
		}
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded), lk, timeout);
			return *pV;
		}
		return m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded);
	}

	CDBHandler::DResult CPhpDb::SetResCallback(const Php::Value &phpRes, std::shared_ptr<Php::Value> &pV, unsigned int &timeout) {
		timeout = (~0);
		bool sync = false;
		if (phpRes.isNumeric()) {
			sync = true;
			timeout = (unsigned int)phpRes.numericValue();
		}
		else if (phpRes.isBool()) {
			sync = phpRes.boolValue();
		}
		else if (phpRes.isNull()) {
		}
		else if (!phpRes.isCallable()) {
			throw Php::Exception("A callback required for final result");
		}
		if (sync) {
			pV.reset(new Php::Value);
		}
		else {
			pV.reset();
		}
		CDBHandler::DResult Dr = [phpRes, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (phpRes) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg);
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_mPhp);
				this->m_cvPhp.notify_all();
			}
			else if (phpRes.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg);
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpRes(v);
			}
		};
		return Dr;
	}

	Php::Value CPhpDb::Close(Php::Parameters &params) {
		unsigned int timeout;
		std::shared_ptr<Php::Value> pV;
		CDBHandler::DResult Dr = SetResCallback(params[0], pV, timeout);
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 1) {
			phpCanceled = params[1];
		}
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idClose, pV ? true : false);
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->Close(Dr, discarded), lk, timeout);
			return *pV;
		}
		return m_db->Close(Dr, discarded);
	}

	Php::Value CPhpDb::BeginTrans(Php::Parameters &params) {
		unsigned int timeout;
		int64_t iso = params[0].numericValue();
		if (iso < SPA::UDB::tiUnspecified || iso > SPA::UDB::tiIsolated) {
			throw Php::Exception("Bad transaction isolation value");
		}
		SPA::UDB::tagTransactionIsolation ti = (SPA::UDB::tagTransactionIsolation)iso;
		std::shared_ptr<Php::Value> pV;
		CDBHandler::DResult Dr = SetResCallback(params[1], pV, timeout);
		Php::Value phpCanceled;
		if (params.size() > 2) {
			phpCanceled = params[2];
		}
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idBeginTrans, pV ? true : false);
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->BeginTrans(ti, Dr, discarded), lk, timeout);
			return *pV;
		}
		return m_db->BeginTrans(ti, Dr, discarded);
	}

	Php::Value CPhpDb::EndTrans(Php::Parameters &params) {
		unsigned int timeout;
		int64_t plan = params[0].numericValue();
		if (plan < SPA::UDB::rpDefault || plan > SPA::UDB::rpRollbackAlways) {
			throw Php::Exception("Bad rollback plan value");
		}
		SPA::UDB::tagRollbackPlan p = (SPA::UDB::tagRollbackPlan)plan;
		std::shared_ptr<Php::Value> pV;
		CDBHandler::DResult Dr = SetResCallback(params[1], pV, timeout);
		Php::Value phpCanceled;
		if (params.size() > 2) {
			phpCanceled = params[2];
		}
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idEndTrans, pV ? true : false);
		if (pV) {
			std::unique_lock<std::mutex> lk(m_mPhp);
			ReqSyncEnd(m_db->EndTrans(p, Dr, discarded), lk, timeout);
			return *pV;
		}
		return m_db->EndTrans(p, Dr, discarded);
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
		else if (name == "Error" || name == "DBError" || name == "LastDBError") {
			Php::Value dbErr;
			dbErr.set(PHP_ERR_CODE, m_db->GetLastDBErrorCode());
			std::wstring wem = m_db->GetLastDBErrorMessage();
			std::string em = SPA::Utilities::ToUTF8(wem);
			Trim(em);
			dbErr.set(PHP_ERR_MSG, em);
			return dbErr;
		}
		else if (name == "Connection") {
			std::wstring wc = m_db->GetConnection();
			std::string ac = SPA::Utilities::ToUTF8(wc);
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
		else if (name == "Meta" || name == "ColMeta" || name == "ColumnInfo") {
			int index = 0;
			auto &cols = m_db->GetColumnInfo();
			Php::Array vMeta;
			for (auto &m : cols) {
				vMeta.set(index, Php::Object((SPA_CS_NS + PHP_DB_COLUMN_INFO).c_str(), new CPhpDBColumnInfo(m)));
				++index;
			}
			return vMeta;
		}
		return CPhpBaseHandler::__get(name);
	}
} //namespace PA
