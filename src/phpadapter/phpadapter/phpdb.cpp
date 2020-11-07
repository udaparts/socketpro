#include "stdafx.h"
#include "phpdb.h"
#include "phpdbcolumninfo.h"
#include "phpbuffer.h"

namespace PA
{

    const char* CPhpDb::PHP_DB_AFFECTED = "affected";
    const char* CPhpDb::PHP_DB_FAILS = "fails";
    const char* CPhpDb::PHP_DB_OKS = "oks";
    const char* CPhpDb::PHP_DB_LAST_ID = "id";

    CPhpDb::CPhpDb(CDBHandler *db, bool locked) : CPhpBaseHandler(locked, db), m_db(db) {
    }

    void CPhpDb::RegisterInto(Php::Class<CPhpBaseHandler> &base, Php::Namespace & cs) {
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
        handler.property("Unknown", (int)SPA::UDB::tagManagementSystem::msUnknown, Php::Const);
        handler.property("Sqlite", (int)SPA::UDB::tagManagementSystem::msSqlite, Php::Const);
        handler.property("Mysql", (int)SPA::UDB::tagManagementSystem::msMysql, Php::Const);
        handler.property("ODBC", (int)SPA::UDB::tagManagementSystem::msODBC, Php::Const);
        handler.property("MsSQL", (int)SPA::UDB::tagManagementSystem::msMsSQL, Php::Const);
        handler.property("Oracle", (int)SPA::UDB::tagManagementSystem::msOracle, Php::Const);
        handler.property("DB2", (int)SPA::UDB::tagManagementSystem::msDB2, Php::Const);
        handler.property("PostgreSQL", (int)SPA::UDB::tagManagementSystem::msPostgreSQL, Php::Const);
        handler.property("MongoDB", (int)SPA::UDB::tagManagementSystem::msMongoDB, Php::Const);

        //tagTransactionIsolation
        handler.property("tiUnspecified", (int)SPA::UDB::tagTransactionIsolation::tiUnspecified, Php::Const);
        handler.property("tiChaos", (int)SPA::UDB::tagTransactionIsolation::tiChaos, Php::Const);
        handler.property("tiReadUncommited", (int)SPA::UDB::tagTransactionIsolation::tiReadUncommited, Php::Const);
        handler.property("tiBrowse", (int)SPA::UDB::tagTransactionIsolation::tiBrowse, Php::Const);
        handler.property("tiCursorStability", (int)SPA::UDB::tagTransactionIsolation::tiCursorStability, Php::Const);
        handler.property("tiReadCommited", (int)SPA::UDB::tagTransactionIsolation::tiReadCommited, Php::Const);
        handler.property("tiRepeatableRead", (int)SPA::UDB::tagTransactionIsolation::tiRepeatableRead, Php::Const);
        handler.property("tiSerializable", (int)SPA::UDB::tagTransactionIsolation::tiSerializable, Php::Const);
        handler.property("tiIsolated", (int)SPA::UDB::tagTransactionIsolation::tiIsolated, Php::Const);

        //tagRollbackPlan
        handler.property("rpDefault", (int)SPA::UDB::tagRollbackPlan::rpDefault, Php::Const);
        handler.property("rpRollbackErrorAny", (int)SPA::UDB::tagRollbackPlan::rpRollbackErrorAny, Php::Const);
        handler.property("rpRollbackErrorLess", (int)SPA::UDB::tagRollbackPlan::rpRollbackErrorLess, Php::Const);
        handler.property("rpRollbackErrorEqual", (int)SPA::UDB::tagRollbackPlan::rpRollbackErrorEqual, Php::Const);
        handler.property("rpRollbackErrorMore", (int)SPA::UDB::tagRollbackPlan::rpRollbackErrorMore, Php::Const);
        handler.property("rpRollbackErrorAll", (int)SPA::UDB::tagRollbackPlan::rpRollbackErrorAll, Php::Const);
        handler.property("rpRollbackAlways", (int)SPA::UDB::tagRollbackPlan::rpRollbackAlways, Php::Const);

        //tagUpdateEvent
        handler.property("ueUnknown", (int)SPA::UDB::tagUpdateEvent::ueUnknown, Php::Const);
        handler.property("ueInsert", (int)SPA::UDB::tagUpdateEvent::ueInsert, Php::Const);
        handler.property("ueUpdate", (int)SPA::UDB::tagUpdateEvent::ueUpdate, Php::Const);
        handler.property("ueDelete", (int)SPA::UDB::tagUpdateEvent::ueDelete, Php::Const);

        //DB Column info tags
        handler.property("NOT_NULL", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_NOT_NULL, Php::Const);
        handler.property("UNIQUE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_UNIQUE, Php::Const);
        handler.property("PRIMARY_KEY", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_PRIMARY_KEY, Php::Const);
        handler.property("AUTOINCREMENT", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_AUTOINCREMENT, Php::Const);
        handler.property("NOT_WRITABLE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_NOT_WRITABLE, Php::Const);
        handler.property("ROWID", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_ROWID, Php::Const);
        handler.property("XML", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_XML, Php::Const);
        handler.property("JSON", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_JSON, Php::Const);
        handler.property("CASE_SENSITIVE", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_CASE_SENSITIVE, Php::Const);
        handler.property("ENUM", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_ENUM, Php::Const);
        handler.property("SET", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_SET, Php::Const);
        handler.property("UNSIGNED", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_UNSIGNED, Php::Const);
        handler.property("BIT", (int64_t) SPA::UDB::CDBColumnInfo::FLAG_IS_BIT, Php::Const);

        handler.method<&CPhpDb::Open>("Open",{
            Php::ByVal("conn", Php::Type::String)
        });
        handler.method<&CPhpDb::Close>("Close");
        handler.method<&CPhpDb::Prepare>("Prepare",{
            Php::ByVal("sql", Php::Type::String)
        });
        handler.method<&CPhpDb::Execute>("Execute",{
            Php::ByVal("sql", Php::Type::Null) //string or array of parameter data
        });
        handler.method<&CPhpDb::BeginTrans>("BeginTrans",{
            Php::ByVal("isolation", Php::Type::Numeric, false)
        });
        handler.method<&CPhpDb::EndTrans>("EndTrans",{
            Php::ByVal("plan", Php::Type::Numeric, false)
        });
        handler.method<&CPhpDb::ExecuteBatch>("ExecuteBatch",{
            Php::ByVal("isolation", Php::Type::Numeric),
            Php::ByVal("sql", Php::Type::String),
            Php::ByVal("vParam", Php::Type::Null)
        });
        cs.add(handler);
    }

    void CPhpDb::PopTopCallbacks(PACallback & cb) {
        Php::Value v;
        Php::Value &callback = *cb.CallBack;
        switch (cb.CallbackType) {
            case enumCallbackType::ctDbExeRes:
                v = ToPhpValueEx(cb.Res);
                break;
            case enumCallbackType::ctDbR:
            {
                bool proc;
                int cols;
                int index = 0;
                *cb.Res >> proc >> cols;
                Php::Array vData;
                CPhpBuffer buff(cb.Res);
                while (cb.Res->GetSize()) {
                    vData.set(index, buff.LoadObject());
                    ++index;
                }
                callback(vData, proc, cols);
            }
                break;
            case enumCallbackType::ctDbRes:
                v = ToPhpValue(cb.Res);
                break;
            case enumCallbackType::ctDbRH:
            {
                int index = 0;
                v = Php::Array(); //convert to array
                SPA::UDB::CDBColumnInfoArray vCol;
                *cb.Res >> vCol;
                for (auto &p : vCol) {
                    v.set(index, From(p));
                    ++index;
                }
            }
                break;
            default:
                assert(false);
                break;
        }
        if (cb.CallbackType != enumCallbackType::ctDbR) {
            callback(v);
        }
    }

    Php::Value CPhpDb::Open(Php::Parameters & params) {
        unsigned int timeout = (~0);
        std::string aconn = params[0].stringValue();
        Trim(aconn);
        SPA::CDBString conn = SPA::Utilities::ToUTF16(aconn);
        CQPointer pV;
        size_t args = params.size();
        CDBHandler::DResult Dr;
        if (args > 1) {
            Dr = SetResCallback(params[1], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        auto discarded = SetAbortCallback(phpCanceled, SPA::UDB::idOpen, pV ? true : false);
        unsigned int flags = 0;
        if (args > 3) {
            Php::Value vFlags = params[3];
            if (vFlags.isNumeric()) {
                flags = (unsigned int) vFlags.numericValue();
            } else if (!vFlags.isNull()) {
                throw Php::Exception("The Open method flags must be an integer value");
            }
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->Open(conn.c_str(), Dr, flags, discarded), lk, timeout);
                return ToPhpValue(pV.get());
            }
            PopCallbacks();
        }
        return m_db->Open(conn.c_str(), Dr, flags, discarded);
    }

    Php::Value CPhpDb::ToPhpValueEx(SPA::CUQueue * q) {
        int res;
        std::string em;
        SPA::INT64 affected;
        unsigned int fails, oks;
        SPA::UDB::CDBVariant vtId;
        *q >> res >> em >> affected >> fails >> oks >> vtId;
        Php::Value v;
        v.set(PHP_ERR_CODE, res);
        v.set(PHP_ERR_MSG, em);
        v.set(PHP_DB_AFFECTED, affected);
        v.set(PHP_DB_FAILS, (int64_t) fails);
        v.set(PHP_DB_OKS, (int64_t) oks);
        if (vtId.vt > VT_NULL) {
            v.set(PHP_DB_LAST_ID, vtId.llVal);
        } else {
            v.set(PHP_DB_LAST_ID, nullptr);
        }
        return v;
    }

    Php::Value CPhpDb::ToPhpValue(SPA::CUQueue * q) {
        int res;
        std::string em;
        *q >> res >> em;
        Php::Value v;
        v.set(PHP_ERR_CODE, res);
        v.set(PHP_ERR_MSG, em);
        return v;
    }

    CDBHandler::DExecuteResult CPhpDb::SetExeResCallback(const Php::Value &phpDR, CQPointer &pV, unsigned int &timeout) {
        timeout = (~0);
        bool sync = false;
        if (phpDR.isNumeric()) {
            sync = true;
            timeout = (unsigned int) phpDR.numericValue();
        } else if (phpDR.isBool()) {
            sync = phpDR.boolValue();
        } else if (phpDR.isNull()) {
        } else if (!phpDR.isCallable()) {
            throw Php::Exception("A callback required for Execute final result");
        }
        if (sync) {
            pV.reset(SPA::CScopeUQueue::Lock(), [](SPA::CUQueue * q) {
                SPA::CScopeUQueue::Unlock(q);
            });
        } else {
            pV.reset();
        }
        CPVPointer callback;
        if (phpDR.isCallable()) {
            callback.reset(new Php::Value(phpDR));
        }
        CDBHandler::DExecuteResult Dr = [callback, pV, this](CDBHandler &db, int res, const std::wstring& errMsg, SPA::INT64 affected, SPA::UINT64 fail_ok, SPA::UDB::CDBVariant & vtId) {
            unsigned int fails = (unsigned int) (fail_ok >> 32);
            unsigned int oks = (unsigned int) fail_ok;
            std::string em = SPA::Utilities::ToUTF8(errMsg);
            Trim(em);
            if (pV) {
                *pV << res << em << affected << fails << oks << vtId;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << res << em << affected << fails << oks << vtId;
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctDbExeRes;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        return Dr;
    }

    CDBHandler::DRows CPhpDb::SetRCallback(const Php::Value & phpRow) {
        CDBHandler::DRows r;
        if (phpRow.isNull()) {
        } else if (!phpRow.isCallable()) {
            throw Php::Exception("A callback required for row data event");
        } else {
            CPVPointer callback(new Php::Value(phpRow));
            r = [callback, this](CDBHandler &db, SPA::CUQueue & vData) {
                SPA::CScopeUQueue sb;
                sb << db.IsProc();
                if (db.IsProc() && db.GetCallReturn()) {
                    sb << db.GetRetValue();
                }
                sb->Push(vData.GetBuffer(), vData.GetSize());
                sb << (int) db.GetColumnInfo().size();
                vData.SetSize(0);
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctDbR;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            };
        }
        return r;
    }

    CDBHandler::DRowsetHeader CPhpDb::SetRHCallback(const Php::Value &phpRh, bool batch) {
        CDBHandler::DRowsetHeader rh;
        if (phpRh.isNull()) {
        } else if (!phpRh.isCallable()) {
            throw Php::Exception(batch ? "A callback required for ExecuteBatch header event" : "A callback required for rowset header event");
        } else {
            CPVPointer callback(new Php::Value(phpRh));
            rh = [callback, this](CDBHandler & db, const unsigned char *start, unsigned int bytes) {
                SPA::CScopeUQueue sb;
                sb->Push(start, bytes);
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctDbRH;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            };
        }
        return rh;
    }

    Php::Value CPhpDb::Execute(Php::Parameters & params) {
        unsigned int timeout = (~0);
        SPA::UDB::CDBVariantArray vParam;
        SPA::CDBString sql;
        if (params[0].isString()) {
            std::string asql = params[0].stringValue();
            Trim(asql);
            if (!asql.size()) {
                throw Php::Exception("SQL statement cannot be empty");
            }
            sql = SPA::Utilities::ToUTF16(asql);
        } else {
            GetParams(params[0], vParam);
        }
        CQPointer pV;
        CDBHandler::DExecuteResult Dr;
        size_t args = params.size();
        if (args > 1) {
            Dr = SetExeResCallback(params[1], pV, timeout);
        }
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
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded), lk, timeout);
                return ToPhpValueEx(pV.get());
            }
            PopCallbacks();
        }
        return sql.size() ? m_db->Execute(sql.c_str(), Dr, r, rh, true, true, discarded) : m_db->Execute(vParam, Dr, r, rh, true, true, discarded);
    }

    void CPhpDb::GetParams(const Php::Value &vP, SPA::UDB::CDBVariantArray & vParam) {
        if (vP.isArray()) {
            int count = vP.length();
            for (int n = 0; n < count; ++n) {
                SPA::UDB::CDBVariant vt;
                ToVariant(vP.get(n), vt);
                vParam.push_back(std::move(vt));
            }
        } else if (vP.instanceOf(SPA_NS + PHP_BUFFER)) {
            Php::Value bytes = vP.call(PHP_POPBYTES);
            const char *raw = bytes.rawValue();
            int len = bytes.length();
            SPA::CScopeUQueue sb;
            sb->Push((const unsigned char*) raw, (unsigned int) len);
            while (sb->GetSize()) {
                try{
                    SPA::UDB::CDBVariant vt;
                    *sb >> vt;
                    vParam.push_back(std::move(vt));
                }

                catch(SPA::CUException & err) {
                    throw Php::Exception(err.what());
                }
            }
        } else {
            throw Php::Exception("A SQL statement or an array of parameter data expected");
        }
    }

    Php::Value CPhpDb::ExecuteBatch(Php::Parameters & params) {
        unsigned int timeout = (~0);
        int64_t iso = params[0].numericValue();
        if (iso < (int)SPA::UDB::tagTransactionIsolation::tiUnspecified || iso > (int)SPA::UDB::tagTransactionIsolation::tiIsolated) {
            throw Php::Exception("Bad transaction isolation value");
        }
        SPA::UDB::tagTransactionIsolation ti = (SPA::UDB::tagTransactionIsolation)iso;
        std::string asql = params[1].stringValue();
        Trim(asql);
        if (!asql.size()) {
            throw Php::Exception("SQL statement cannot be empty");
        }
        SPA::CDBString sql = SPA::Utilities::ToUTF16(asql);
        SPA::UDB::CDBVariantArray vParam;
        GetParams(params[2], vParam);
        CQPointer pV;
        CDBHandler::DExecuteResult Dr;
        size_t args = params.size();
        if (args > 3) {
            Dr = SetExeResCallback(params[3], pV, timeout);
        }
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
        SPA::CDBString delimiter(u";");
        if (args > 6) {
            if (params[6].isString()) {
                std::string s = params[6].stringValue();
                Trim(s);
                if (!s.size()) {
                    throw Php::Exception("Delimiter string cannot be empty");
                }
                delimiter = SPA::Utilities::ToUTF16(s);
            }
            else if (!params[6].isNull()) {
                throw Php::Exception("A string required for delimiter");
            }
        }
        Php::Value phpBh;
        if (args > 7) {
            phpBh = params[7];
        }
        CDBHandler::DRowsetHeader bh = SetRHCallback(phpBh, true);

        Php::Value phpCanceled;
        if (args > 8) {
            phpCanceled = params[8];
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idExecuteBatch, pV ? true : false);
        bool meta = true;
        if (args > 9) {
            if (params[9].isBool()) {
                meta = params[9].boolValue();
            }
            else if (params[9].isNumeric()) {
                meta = (params[9].numericValue() != 0);
            }
            else if (!params[9].isNull()) {
                throw Php::Exception("A boolean required for ExecuteBatch meta");
            }
        }
        SPA::UDB::tagRollbackPlan plan = SPA::UDB::tagRollbackPlan::rpDefault;
        if (args > 10) {
            if (params[10].isNumeric()) {
                int64_t p = params[10].numericValue();
                if (p < (int)SPA::UDB::tagRollbackPlan::rpDefault || p > (int)SPA::UDB::tagRollbackPlan::rpRollbackAlways) {
                    throw Php::Exception("Bad rollback plan value");
                }
                plan = (SPA::UDB::tagRollbackPlan)p;
            } else if (!params[10].isNull()) {
                throw Php::Exception("An integer required for ExecuteBatch rollback plan");
            }
        }
        SPA::UDB::CParameterInfoArray vPInfo;
        if (args > 11) {
            Php::Value vParamInfo = params[11];
            if (vParamInfo.isArray()) {
                vPInfo = ConvertFrom(vParamInfo);
            } else if (!vParamInfo.isNull()) {
                throw Php::Exception("An array of parameter info structures required");
            }
        }
        bool lastInsertId = true;
        if (args > 12) {
            if (params[12].isBool()) {
                lastInsertId = params[12].boolValue();
            }
            else if (params[12].isNumeric()) {
                lastInsertId = (params[12].numericValue() != 0);
            }
            else if (!params[12].isNull()) {
                throw Php::Exception("A boolean required for ExecuteBatch lastInsertId");
            }
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->ExecuteBatch(ti, sql.c_str(), vParam, Dr, r, rh, delimiter.c_str(), bh, discarded, meta, plan, vPInfo, lastInsertId, nullptr), lk, timeout);
                return ToPhpValueEx(pV.get());
            }
            PopCallbacks();
        }
        return m_db->ExecuteBatch(ti, sql.c_str(), vParam, Dr, r, rh, delimiter.c_str(), bh, discarded, meta, plan, vPInfo, lastInsertId, nullptr);
    }

    SPA::UDB::CParameterInfoArray CPhpDb::ConvertFrom(Php::Value vP) {
        SPA::UDB::CParameterInfoArray vPInfo;
        int count = vP.length();
        for (int n = 0; n < count; ++n) {
            Php::Value vP = vP.get(n);
            if (vP.instanceOf(SPA_CS_NS + PHP_DB_PARAMETER_INFO)) {
                SPA::UDB::CParameterInfo pi;
                pi.Direction = (SPA::UDB::tagParameterDirection)vP.get("Direction").numericValue();
                pi.DataType = (VARTYPE) vP.get(PHP_DATATYPE).numericValue();
                pi.ColumnSize = (unsigned int) vP.get(PHP_COLUMN_SIZE).numericValue();
                pi.Precision = (unsigned char) vP.get(PHP_COLUMN_PRECSISON).numericValue();
                pi.Scale = (unsigned char) vP.get(PHP_COLUMN_SCALE).numericValue();
                std::string s = vP.get("ParameterName").stringValue();
                pi.ParameterName = SPA::Utilities::ToUTF16(s.c_str(), s.size());
                vPInfo.push_back(pi);
            } else {
                throw Php::Exception("A parameter info structure is expected");
            }
        }
        return vPInfo;
    }

    Php::Value CPhpDb::Prepare(Php::Parameters & params) {
        unsigned int timeout = (~0);
        std::string asql = params[0].stringValue();
        Trim(asql);
        if (!asql.size()) {
            throw Php::Exception("SQL statement cannot be empty");
        }
        SPA::CDBString sql = SPA::Utilities::ToUTF16(asql);
        CQPointer pV;
        CDBHandler::DResult Dr;
        size_t args = params.size();
        if (args > 1) {
            Dr = SetResCallback(params[1], pV, timeout);
        }
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
            } else if (!vParamInfo.isNull()) {
                throw Php::Exception("An array of parameter info structures required");
            }
        }
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded), lk, timeout);
                return ToPhpValue(pV.get());
            }
            PopCallbacks();
        }
        return m_db->Prepare(sql.c_str(), Dr, vPInfo, discarded);
    }

    CDBHandler::DResult CPhpDb::SetResCallback(const Php::Value &phpRes, CQPointer &pV, unsigned int &timeout) {
        timeout = (~0);
        bool sync = false;
        if (phpRes.isNumeric()) {
            sync = true;
            timeout = (unsigned int) phpRes.numericValue();
        } else if (phpRes.isBool()) {
            sync = phpRes.boolValue();
        } else if (phpRes.isNull()) {
        } else if (!phpRes.isCallable()) {
            throw Php::Exception("A callback required for final result");
        }
        if (sync) {
            pV.reset(SPA::CScopeUQueue::Lock(), [](SPA::CUQueue * q) {
                SPA::CScopeUQueue::Unlock(q);
            });
        } else {
            pV.reset();
        }
        CPVPointer callback;
        if (phpRes.isCallable()) {
            callback.reset(new Php::Value(phpRes));
        }
        CDBHandler::DResult Dr = [callback, pV, this](CDBHandler &db, int res, const std::wstring & errMsg) {
            std::string em = SPA::Utilities::ToUTF8(errMsg);
            Trim(em);
            if (pV) {
                *pV << res << em;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_cvPhp.notify_all();
            } else if (callback) {
                SPA::CScopeUQueue sb;
                sb << res << em;
                PACallback cb;
                cb.CallbackType = enumCallbackType::ctDbRes;
                cb.Res = sb.Detach();
                cb.CallBack = callback;
                std::unique_lock<std::mutex> lk(this->m_mPhp);
                this->m_vCallback.push_back(cb);
            }
        };
        return Dr;
    }

    Php::Value CPhpDb::Close(Php::Parameters & params) {
        unsigned int timeout = (~0);
        CQPointer pV;
        CDBHandler::DResult Dr;
        size_t args = params.size();
        if (args > 0) {
            Dr = SetResCallback(params[0], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 1) {
            phpCanceled = params[1];
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idClose, pV ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->Close(Dr, discarded), lk, timeout);
                return ToPhpValue(pV.get());
            }
            PopCallbacks();
        }
        return m_db->Close(Dr, discarded);
    }

    Php::Value CPhpDb::BeginTrans(Php::Parameters & params) {
        unsigned int timeout = (~0);
        SPA::UDB::tagTransactionIsolation ti = SPA::UDB::tagTransactionIsolation::tiReadCommited;
        size_t args = params.size();
        if (args > 0) {
            int64_t iso = params[0].numericValue();
            if (iso < (int)SPA::UDB::tagTransactionIsolation::tiUnspecified || iso > (int)SPA::UDB::tagTransactionIsolation::tiIsolated) {
                throw Php::Exception("Bad transaction isolation value");
            }
            ti = (SPA::UDB::tagTransactionIsolation)iso;
        }
        CQPointer pV;
        CDBHandler::DResult Dr;
        if (args > 1) {
            Dr = SetResCallback(params[1], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idBeginTrans, pV ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->BeginTrans(ti, Dr, discarded), lk, timeout);
                return ToPhpValue(pV.get());
            }
            PopCallbacks();
        }
        return m_db->BeginTrans(ti, Dr, discarded);
    }

    Php::Value CPhpDb::EndTrans(Php::Parameters & params) {
        unsigned int timeout = (~0);
        SPA::UDB::tagRollbackPlan p = SPA::UDB::tagRollbackPlan::rpDefault;
        size_t args = params.size();
        if (args > 0) {
            int64_t plan = params[0].numericValue();
            if (plan < (int)SPA::UDB::tagRollbackPlan::rpDefault || plan > (int)SPA::UDB::tagRollbackPlan::rpRollbackAlways) {
                throw Php::Exception("Bad rollback plan value");
            }
            p = (SPA::UDB::tagRollbackPlan)plan;
        }
        CQPointer pV;
        CDBHandler::DResult Dr;
        if (args > 1) {
            Dr = SetResCallback(params[1], pV, timeout);
        }
        Php::Value phpCanceled;
        if (args > 2) {
            phpCanceled = params[2];
        }
        SPA::ClientSide::CAsyncServiceHandler::DDiscarded discarded = SetAbortCallback(phpCanceled, SPA::UDB::idEndTrans, pV ? true : false);
        {
            std::unique_lock<std::mutex> lk(m_mPhp);
            if (pV) {
                ReqSyncEnd(m_db->EndTrans(p, Dr, discarded), lk, timeout);
                return ToPhpValue(pV.get());
            }
            PopCallbacks();
        }
        return m_db->EndTrans(p, Dr, discarded);
    }

    Php::Value CPhpDb::__get(const Php::Value & name) {
        if (name == "Opened") {
            return m_db->IsOpened();
        } else if (name == "Outputs") {
            return (int64_t) m_db->GetOutputs();
        } else if (name == "Parameters") {
            return (int64_t) m_db->GetParameters();
        } else if (name == "LastAffected") {
            return m_db->GetLastAffected();
        } else if (name == "DBMS" || name == "DBManagementSystem") {
            return (int)m_db->GetDBManagementSystem();
        } else if (name == "Error" || name == "DBError" || name == "LastDBError") {
            Php::Value dbErr;
            dbErr.set(PHP_ERR_CODE, m_db->GetLastDBErrorCode());
            std::wstring wem = m_db->GetLastDBErrorMessage();
            std::string em = SPA::Utilities::ToUTF8(wem);
            Trim(em);
            dbErr.set(PHP_ERR_MSG, em);
            return dbErr;
        } else if (name == "Connection") {
            std::wstring wc = m_db->GetConnection();
            std::string ac = SPA::Utilities::ToUTF8(wc);
            Trim(ac);
            return ac;
        } else if (name == "CallReturn") {
            return m_db->GetCallReturn();
        } else if (name == "Proc") {
            return m_db->IsProc();
        } else if (name == "RetVal" || name == "RetValue") {
            CPhpBuffer buff;
            *buff.GetBuffer() << m_db->GetRetValue();
            return buff.LoadObject();
        } else if (name == "Meta" || name == "ColMeta" || name == "ColumnInfo") {
            int index = 0;
            auto &cols = m_db->GetColumnInfo();
            Php::Array vMeta;
            for (auto &m : cols) {
                vMeta.set(index, From(m));
                ++index;
            }
            return vMeta;
        }
        return CPhpBaseHandler::__get(name);
    }
} //namespace PA
