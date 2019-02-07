#include "stdafx.h"
#include "phpdb.h"
#include "phpdbcolumninfo.h"

namespace PA {

	const char* CPhpDb::PHP_DB_AFFECTED = "affected";
	const char* CPhpDb::PHP_DB_FAILS = "fails";
	const char* CPhpDb::PHP_DB_OKS = "oks";
	const char* CPhpDb::PHP_DB_LAST_ID = "id";

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
			Php::ByVal("sql", Php::Type::Null), //string or array of parameter data
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
		std::string aconn = params[0].stringValue();
		Trim(aconn);
		std::wstring conn = SPA::Utilities::ToWide(aconn.c_str(), aconn.size());
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[1];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for Open final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDR(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
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
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!m_db->Open(conn.c_str(), Dr, flags, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_db->Open(conn.c_str(), Dr, flags, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpDb::Execute(Php::Parameters &params) {
		SPA::UDB::CDBVariantArray vParam;
		std::wstring sql;
		if (params[0].isString()) {
			std::string asql = params[0].stringValue();
			Trim(asql);
			if (!asql.size()) {
				throw Php::Exception("SQL statement cannot be empty");
			}
			sql = SPA::Utilities::ToWide(asql.c_str(), asql.size());
		}
		else if (params[0].isArray()) {
			int count = params[0].length();
			for (int n = 0; n < count; ++n) {
				SPA::UDB::CDBVariant vt;
				ToVariant(params[0].get(n), vt);
				vParam.push_back(std::move(vt));
			}
		}
		else if (params[0].instanceOf((SPA_CS_NS + PHP_BUFFER).c_str())) {
			Php::Value vQueue = params[0];
			Php::Value bytes = vQueue.call("PopBytes");
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
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[1];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for BeginTrans final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DExecuteResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant& vtId) {
			unsigned int fails = (unsigned int)(fail_ok >> 32);
			unsigned int oks = (unsigned int)fail_ok;
			CPhpBuffer buff;
			*buff.GetBuffer() << vtId;
			if (sync) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				pV->set(PHP_DB_AFFECTED, affected);
				pV->set(PHP_DB_FAILS, (int64_t)fails);
				pV->set(PHP_DB_OKS, (int64_t)oks);
				pV->set(PHP_DB_LAST_ID, buff.LoadObject());
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
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
		size_t args = params.size();
		Php::Value phpRow;
		if (args > 2) {
			phpRow = params[2];
			if (phpRow.isNull()) {
			}
			else if (!phpRow.isCallable()) {
				throw Php::Exception("A callback required for Execute row event");
			}
		}
		CDBHandler::DRows r = [phpRow, this](CDBHandler &db, Php::Array &vData) {
			if (phpRow.isCallable()) {
				Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(this->GetPoolId(), &db, false));
				phpRow(vData, db.IsProc(), obj);
			}
		};
		Php::Value phpRh;
		if (args > 3) {
			phpRh = params[3];
			if (phpRh.isNull()) {
			}
			else if (!phpRh.isCallable()) {
				throw Php::Exception("A callback required for Execute rwoset header event");
			}
		}
		CDBHandler::DRowsetHeader rh = [phpRh, this](CDBHandler &db) {
			if (phpRh.isCallable()) {
				Php::Object obj((SPA_CS_NS + PHP_DB_HANDLER).c_str(), new CPhpDb(this->GetPoolId(), &db, false));
				phpRh(obj);
			}
		};
		Php::Value phpCanceled;
		if (args > 4) {
			phpCanceled = params[4];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!(sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded))) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return (sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded)) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpDb::ExecuteBatch(Php::Parameters &params) {
		return nullptr;
	}

	SPA::UDB::CParameterInfoArray CPhpDb::ConvertFrom(Php::Value vP) {
		SPA::UDB::CParameterInfoArray vPInfo;
		int count = vP.length();
		for (int n = 0; n < count; ++n) {
			Php::Value vP = vP.get(n);
			if (vP.instanceOf((SPA_CS_NS + PHP_DB_PARAMETER_INFO).c_str())) {
				SPA::UDB::CParameterInfo pi;
				pi.Direction = (SPA::UDB::tagParameterDirection)vP.get("Direction").numericValue();
				pi.DataType = (VARTYPE)vP.get("DataType").numericValue();
				pi.ColumnSize = (unsigned int)vP.get("ColumnSize").numericValue();
				pi.Precision = (unsigned char)vP.get("Precision").numericValue();
				pi.Scale = (unsigned char)vP.get("Scale").numericValue();
				std::string s = vP.get("ParameterName").stringValue();
				pi.ParameterName = SPA::Utilities::ToWide(s.c_str(), s.size());
				vPInfo.push_back(pi);
			}
			else {
				throw Php::Exception("A parameter info structure is expected");
			}
		}
		return vPInfo;
	}

	Php::Value CPhpDb::Prepare(Php::Parameters &params) {
		std::string asql = params[0].stringValue();
		Trim(asql);
		if (!asql.size()) {
			throw Php::Exception("SQL statement cannot be empty");
		}
		std::wstring sql = SPA::Utilities::ToWide(asql.c_str(), asql.size());
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[1];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for BeginTrans final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (sync) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDR(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
		SPA::UDB::CParameterInfoArray vPInfo;
		if (args > 3) {
			Php::Value vParamInfo = params[3];
			if (vParamInfo.isArray()) {
				vPInfo = ConvertFrom(vParamInfo);
			}
			else if (!vParamInfo.isNull()) {
				throw Php::Exception("An array of parameters required");
			}
		}
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpDb::Close(Php::Parameters &params) {
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[0];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for Close final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (sync) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDR(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 1) {
			phpCanceled = params[1];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!m_db->Close(Dr, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_db->Close(Dr, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpDb::BeginTrans(Php::Parameters &params) {
		int64_t iso = params[0].numericValue();
		if (iso < SPA::UDB::tiUnspecified || iso > SPA::UDB::tiIsolated) {
			throw Php::Exception("Bad transaction isolation value");
		}
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[1];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for BeginTrans final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (sync) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDR(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!m_db->BeginTrans((SPA::UDB::tagTransactionIsolation)iso, Dr, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_db->BeginTrans((SPA::UDB::tagTransactionIsolation)iso, Dr, discarded) ? rrsOk : rrsClosed;
	}

	Php::Value CPhpDb::EndTrans(Php::Parameters &params) {
		int64_t plan = params[0].numericValue();
		if (plan < SPA::UDB::rpDefault || plan > SPA::UDB::rpRollbackAlways) {
			throw Php::Exception("Bad rollback plan value");
		}
		unsigned int timeout = m_db->GetAttachedClientSocket()->GetRecvTimeout();
		bool sync = false;
		Php::Value phpDR = params[1];
		if (phpDR.isNumeric()) {
			sync = true;
			unsigned int t = (unsigned int)phpDR.numericValue();
			if (t < timeout) {
				timeout = t;
			}
		}
		else if (phpDR.isBool()) {
			sync = phpDR.boolValue();
		}
		else if (phpDR.isNull()) {
		}
		else if (!phpDR.isCallable()) {
			throw Php::Exception("A callback required for BeginTrans final result");
		}
		std::shared_ptr<Php::Value> pV;
		if (sync) {
			pV.reset(new Php::Value);
		}
		CDBHandler::DResult Dr = [sync, phpDR, pV, this](CDBHandler &db, int res, const std::wstring& errMsg) {
			if (sync) {
				pV->set(PHP_ERR_CODE, res);
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				pV->set(PHP_ERR_MSG, em);
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				this->m_db->m_cvPhp.notify_all();
			}
			else if (phpDR.isCallable()) {
				std::string em = SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size());
				Trim(em);
				Php::Value v;
				v.set(PHP_ERR_CODE, res);
				v.set(PHP_ERR_MSG, em);
				phpDR(v);
			}
		};
		size_t args = params.size();
		Php::Value phpCanceled;
		if (args > 2) {
			phpCanceled = params[2];
			if (phpCanceled.isNull()) {
			}
			else if (!phpCanceled.isCallable()) {
				throw Php::Exception("A callback required for Close event");
			}
		}
		tagRequestReturnStatus rrs = rrsOk;
		SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = [phpCanceled, sync, &rrs, this](SPA::ClientSide::CAsyncServiceHandler *ash, bool canceled) {
			if (phpCanceled.isCallable()) {
				phpCanceled(canceled);
			}
			if (sync) {
				std::unique_lock<std::mutex> lk(this->m_db->m_mPhp);
				rrs = canceled ? rrsCanceled : rrsClosed;
				this->m_db->m_cvPhp.notify_all();
			}
		};
		if (sync) {
			std::unique_lock<std::mutex> lk(m_db->m_mPhp);
			if (!m_db->EndTrans((SPA::UDB::tagRollbackPlan)plan, Dr, discarded)) {
				Unlock();
				throw Php::Exception(PHP_SOCKET_CLOSED);
			}
			Unlock();
			auto status = m_db->m_cvPhp.wait_for(lk, std::chrono::milliseconds(timeout));
			if (status == std::cv_status::timeout) {
				rrs = rrsTimeout;
			}
			switch (rrs) {
			case rrsServerException:
				throw Php::Exception(PHP_SERVER_EXCEPTION);
			case rrsCanceled:
				throw Php::Exception(PHP_REQUEST_CANCELED);
			case rrsClosed:
				throw Php::Exception(PHP_SOCKET_CLOSED);
			case rrsTimeout:
				throw Php::Exception(PHP_REQUEST_TIMEOUT);
			default:
				break;
			}
			return *pV;
		}
		return m_db->EndTrans((SPA::UDB::tagRollbackPlan)plan, Dr, discarded) ? rrsOk : rrsClosed;
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
				vMeta.set(index, Php::Object((SPA_CS_NS + PHP_DB_COLUMN_INFO).c_str(), new CPhpDBColumnInfo(m)));
				++index;
			}
			return vMeta;
		}
		return CPhpBaseHandler::__get(name);
	}
} //namespace PA
