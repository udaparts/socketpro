
#include "stdafx.h"
#include "streamingserver.h"
#include "umysql_udf.h"
#include <algorithm>

my_bool PublishDBEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 13;
    if (args->arg_count != 2 || args->arg_type[0] != INT_RESULT || args->arg_type[1] != STRING_RESULT) {
#ifdef WIN32_64
        strcpy_s(message, 1024, "PublishDBEvent() requires database event type number and a filter string");
#else
        strcpy(message, "PublishDBEvent() requires database event type number and a filter string");
#endif
        return 1;
    }
    args->maybe_null[0] = 0;
    args->maybe_null[1] = 0;
    return 0;
}

void PublishDBEvent_deinit(UDF_INIT *initid) {

}

long long PublishDBEvent(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    long long eventType = *((long long*) (args->args[0]));
    std::string s = std::to_string(eventType);
    s += "/";
    s.append(args->args[1], args->lengths[1]);
    SPA::UVariant vt(s.c_str());
    if (g_pStreamingServer) {
        return SPA::ServerSide::CSocketProServer::PushManager::Publish(vt, &SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1) ? 1 : 0;
    }
    return 0;
}

namespace SPA{
    namespace ServerSide
    {

        std::string CMysqlImpl::ToString(const CDBVariant & vtUTF8) {
            assert(vtUTF8.Type() == (VT_I1 | VT_ARRAY));
            const char *p;
            ::SafeArrayAccessData(vtUTF8.parray, (void**) &p);
            unsigned int len = vtUTF8.parray->rgsabound->cElements;
            std::string s(p, p + len);
            ::SafeArrayUnaccessData(vtUTF8.parray);
            return s;
        }

        void CMysqlImpl::CreateTriggers(CMysqlImpl &impl, const std::vector<std::string> &vecTables) {
            if (!impl.m_pMysql && !impl.OpenSession(L"root", "localhost"))
                return;
            impl.m_NoSending = true;
            impl.RemoveUnusedTriggers(vecTables);
            for (auto it = vecTables.begin(), end = vecTables.end(); it != end; ++it) {
                auto pos = it->find_last_of('.');
                std::string schema = it->substr(0, pos);
                std::string table = it->substr(pos + 1);
                impl.CreateTriggers(schema, table);
            }
        }

        std::wstring CMysqlImpl::GetCreateTriggerSQLDelete(const wchar_t *db, const wchar_t *table, const CPriKeyArray & vPriKey) {
            std::wstring sql(L"DELETE AFTER DELETE ON `");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"` FOR EACH ROW BEGIN DECLARE msg VARCHAR(2048); DECLARE res BIGINT; SET msg = CONCAT(";
            std::wstring str;
            for (auto it = vPriKey.begin(), end = vPriKey.end(); it != end; ++it) {
                if (str.size())
                    str += L", ' AND ', ";
                str += L"'`";
                str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                str += L"`=', ";
                if (it->Quoted) {
                    str += L"'\\'', ";
                    str += L"old.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`, '\\''";
                } else {
                    str += L"old.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`";
                }
            }
            sql += (str + L", '@`");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"`');";

            return sql + L"SELECT PublishDBEvent(" + std::to_wstring((SPA::INT64)SPA::UDB::ueDelete);
        }

        std::wstring CMysqlImpl::GetCreateTriggerSQLInsert(const wchar_t *db, const wchar_t *table, const CPriKeyArray & vPriKey) {
            std::wstring sql(L"INSERT AFTER INSERT ON `");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"` FOR EACH ROW BEGIN DECLARE msg VARCHAR(2048); DECLARE res BIGINT; SET msg = CONCAT(";
            std::wstring str;
            for (auto it = vPriKey.begin(), end = vPriKey.end(); it != end; ++it) {
                if (str.size())
                    str += L", ' AND ', ";
                str += L"'`";
                str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                str += L"`=', ";
                if (it->Quoted) {
                    str += L"'\\'', ";
                    str += L"new.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`, '\\''";
                } else {
                    str += L"new.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`";
                }
            }
            sql += (str + L", '@`");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"`');";

            return sql + L"SELECT PublishDBEvent(" + std::to_wstring((SPA::INT64)SPA::UDB::ueInsert);
        }

        std::wstring CMysqlImpl::GetCreateTriggerSQLUpdate(const wchar_t *db, const wchar_t *table, const CPriKeyArray & vPriKey) {
            std::wstring sql(L"UPDATE AFTER UPDATE ON `");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"` FOR EACH ROW BEGIN DECLARE msg VARCHAR(2048); DECLARE res BIGINT; SET msg = CONCAT(";
            std::wstring str;
            for (auto it = vPriKey.begin(), end = vPriKey.end(); it != end; ++it) {
                if (str.size())
                    str += L", ' AND ', ";
                str += L"'`";
                str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                str += L"`=', ";
                if (it->Quoted) {
                    str += L"'\\'', ";
                    str += L"new.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`, '\\''";
                } else {
                    str += L"new.`";
                    str += SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size());
                    str += L"`";
                }
            }
            sql += (str + L", '@`");
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"`');";

            return sql + L"SELECT PublishDBEvent(" + std::to_wstring((SPA::INT64)SPA::UDB::ueUpdate);
        }

        std::wstring CMysqlImpl::GetCreateTriggerSQL(const wchar_t *db, const wchar_t *table, const CPriKeyArray &vPriKey, SPA::UDB::tagUpdateEvent eventType) {
            std::wstring sql;
            if (!vPriKey.size())
                return sql;
            std::wstring strDB(db), strTable(table);
            for (auto it = strDB.begin(), end = strDB.end(); it != end; ++it) {
                if (isspace(*it)) {
                    *it = L'_';
                }
            }
            for (auto it = strTable.begin(), end = strTable.end(); it != end; ++it) {
                if (isspace(*it)) {
                    *it = L'_';
                }
            }
            sql.append(L"CREATE TRIGGER ");
            sql.append(STREAMING_DB_TRIGGER_FIX);
            sql.append(strDB + L"_");
            sql.append(strTable + L"_");

            switch (eventType) {
                case SPA::UDB::ueDelete:
                    sql += GetCreateTriggerSQLDelete(db, table, vPriKey);
                    break;
                case SPA::UDB::ueInsert:
                    sql += GetCreateTriggerSQLInsert(db, table, vPriKey);
                    break;
                default:
                    sql += GetCreateTriggerSQLUpdate(db, table, vPriKey);
                    break;
            }
            sql += L",msg)INTO res;END";
            return sql;
        }

        void CMysqlImpl::CreateTriggers(const std::string &schema, const std::string & table) {
            bool bDelete = false, bInsert = false, bUpdate = false;
            std::wstring wSchema = SPA::Utilities::ToWide(schema.c_str(), schema.size());
            std::wstring wTable = SPA::Utilities::ToWide(table.c_str(), table.size());
            std::wstring prefix(STREAMING_DB_TRIGGER_FIX);
            std::wstring sql_existing = L"SELECT EVENT_MANIPULATION, TRIGGER_NAME FROM INFORMATION_SCHEMA.TRIGGERS WHERE ";
            sql_existing += L"EVENT_OBJECT_SCHEMA='" + wSchema + L"'";
            sql_existing += L" AND EVENT_OBJECT_TABLE='" + wTable + L"'";
            sql_existing += L" AND ACTION_TIMING='AFTER'";
            sql_existing += L" AND TRIGGER_NAME LIKE '" + prefix + L"%' ORDER BY EVENT_MANIPULATION";
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            std::wstring errMsg;
            SPA::UDB::CDBVariant vtType, vtName;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s triggers failed(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return;
            }
            while (m_qSend.GetSize() && !res) {
                m_qSend >> vtType >> vtName;
                std::string type = ToString(vtType);
                std::string name = ToString(vtName);
                if (type == "INSERT")
                    bInsert = true;
                else if (type == "DELETE")
                    bDelete = true;
                else if (type == "UPDATE")
                    bUpdate = true;
            }
            if (bInsert && bDelete && bUpdate)
                return;
            std::wstring sql = L"SELECT COLUMN_NAME,DATA_TYPE FROM information_schema.COLUMNS WHERE COLUMN_KEY='PRI' AND TABLE_SCHEMA='";
            sql += (wSchema + L"' AND TABLE_NAME='");
            sql += (wTable + L"'");
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s failed for creating triggers(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return;
            }
            if (!m_qSend.GetSize()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create triggers for the table %s.%s because it has no primary key defined", schema.c_str(), table.c_str());
                return;
            }

            CPriKeyArray vKey;
            PriKey pk;
            while (m_qSend.GetSize()) {
                m_qSend >> vtName >> vtType;
                pk.ColumnName = ToString(vtName);
                std::string type = ToString(vtType);
                if (type == "varchar" || type == "char" || type == "datetime" || type == "date" || type == "time" || type == "binary" || type == "varbinary" || type == "enum" || type == "set")
                    pk.Quoted = true;
                else
                    pk.Quoted = false;
                vKey.push_back(pk);
            }

            sql = L"USE `" + wSchema + L"`";
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);

            if (!bInsert) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueInsert);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create insert trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                }
            }
            if (!bDelete) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueDelete);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create delete trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                }
            }
            if (!bUpdate) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueUpdate);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create update trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                }
            }
        }

        void CMysqlImpl::RemoveUnusedTriggers(const std::vector<std::string> &vecTables) {
            std::wstring prefix(STREAMING_DB_TRIGGER_FIX);
            std::wstring sql_existing = L"SELECT event_object_schema, trigger_name, EVENT_OBJECT_TABLE FROM INFORMATION_SCHEMA.TRIGGERS where TRIGGER_NAME like '" + prefix + L"%' order by event_object_schema, EVENT_OBJECT_TABLE";
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            std::wstring errMsg;
            SPA::UDB::CDBVariant vtSchema, vtName, vtTable;
            std::vector<std::string> vec = vecTables;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying SocketPro streaming db triggers failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
            }

            SPA::CScopeUQueue sb;
            sb->Push(m_qSend.GetBuffer(), m_qSend.GetSize());
            SPA::CUQueue &q = *sb;

            while (q.GetSize() && !res) {
                q >> vtSchema >> vtName >> vtTable;
                std::string schema = ToString(vtSchema);
                std::string name = ToString(vtName);
                std::string table = ToString(vtTable);

                std::string trigger_db_table = schema + "." + table;
                std::transform(trigger_db_table.begin(), trigger_db_table.end(), trigger_db_table.begin(), ::tolower);

                auto ret = std::find_if(vec.begin(), vec.end(), [&trigger_db_table](const std::string & s) {
                    return (trigger_db_table == s);
                });
                if (ret == vec.end()) {
                    std::wstring wsql = L"USE `" + SPA::Utilities::ToWide(schema.c_str(), schema.size()) + L"`";
                    Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                    res = 0;

                    //trigger not needed any more as it is not found inside sp_streaming_db.config.cached_tables
                    wsql = L"drop trigger " + SPA::Utilities::ToWide(name.c_str(), name.size());
                    Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                    if (res) {
                        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Removing the unused trigger %s failed(errCode=%d; errMsg=%s)", name.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                        res = 0;
                    }
                }
            }
        }

    } //namespace ServerSide
} //namespace SPA

