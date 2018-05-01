
#include "stdafx.h"
#include "streamingserver.h"
#include "umysql_udf.h"
#include <algorithm>

bool PublishDBEvent_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    initid->maybe_null = 0;
    initid->const_item = 0;
    initid->ptr = nullptr;
    initid->decimals = 0;
    initid->max_length = 1024;
    if (args->arg_count < 5 || args->arg_type[0] != INT_RESULT || args->arg_type[1] != STRING_RESULT || args->arg_type[2] != STRING_RESULT || args->arg_type[3] != STRING_RESULT) {
#ifdef WIN32_64
        strcpy_s(message, 1023, "PublishDBEvent() requires database event type number, host, database, and one or more other values");
#else
        strcpy(message, "PublishDBEvent() requires database event type number, host, database, and one or more other values");
#endif
        return 1;
    }
    return 0;
}

void PublishDBEvent_deinit(UDF_INIT *initid) {

}

long long PublishDBEvent(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error) {
    if (!g_pStreamingServer)
        return 0;
    VARIANT *p = nullptr;
    long long eventType = *((long long*) (args->args[0]));
    unsigned int count = args->arg_count;
    SPA::UVariant vtArray;
    vtArray.vt = (VT_ARRAY | VT_VARIANT);
    SAFEARRAYBOUND sab[] = {count + 1, 0};
    vtArray.parray = SafeArrayCreate(VT_VARIANT, 1, sab);
    SafeArrayAccessData(vtArray.parray, (void**) &p);
    ::memset(p, 0, sizeof (VARIANT) * count);

    p[0].vt = VT_I4;
    p[0].intVal = (int) eventType;

    {
        char *s = nullptr;
        char host_name[128] = {0};
        int res = ::gethostname(host_name, sizeof (host_name));
        unsigned int len = (unsigned int) strlen(host_name);
        SAFEARRAYBOUND sab1[] = {len, 0};
        p[1].vt = (VT_ARRAY | VT_I1);
        p[1].parray = SafeArrayCreate(VT_I1, 1, sab1);
        SafeArrayAccessData(p[1].parray, (void**) &s);
        memcpy(s, host_name, len);
        SafeArrayUnaccessData(p[1].parray);
        ++p;
    }

    for (unsigned int n = 1; n < count; ++n) {
        bool bNull = (args->args[n]) ? false : true;
        if (bNull && args->maybe_null[n]) {
            p[n].vt = VT_NULL;
            continue;
        }
        Item_result type = args->arg_type[n];
        switch (type) {
            case STRING_RESULT:
            {
                char *s = nullptr;
                unsigned int len = (unsigned int) args->lengths[n];
                p[n].vt = (VT_ARRAY | VT_I1);
                SAFEARRAYBOUND sab1[] = {len, 0};
                p[n].parray = SafeArrayCreate(VT_I1, 1, sab1);
                SafeArrayAccessData(p[n].parray, (void**) &s);
                memcpy(s, args->args[n], len);
                SafeArrayUnaccessData(p[n].parray);
            }
                break;
            case REAL_RESULT:
                p[n].vt = VT_R8;
                p[n].dblVal = *((double*) (args->args[n]));
                break;
            case INT_RESULT:
                p[n].vt = VT_I8;
                p[n].llVal = *((SPA::INT64*) (args->args[n]));
                break;
            case DECIMAL_RESULT:
            {
                unsigned long len = args->lengths[n];
                if (len < 20) {
                    SPA::ParseDec(args->args[n], p[n].decVal);
                } else {
                    SPA::ParseDec_long(args->args[n], p[n].decVal);
                }
                p[n].vt = VT_DECIMAL;
            }
                break;
            default:
                assert(false);
                break;
        }
    }
    SafeArrayUnaccessData(vtArray.parray);
    if (g_pStreamingServer) {
        return SPA::ServerSide::CSocketProServer::PushManager::Publish(vtArray, &SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1) ? 1 : 0;
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

        std::wstring CMysqlImpl::GetCreateTriggerSQL(const wchar_t *db, const wchar_t *table, const CPriKeyArray &vPriKey, SPA::UDB::tagUpdateEvent eventType) {
            std::wstring sql;
            CPriKeyArray vDelKey;
            if (!vPriKey.size())
                return sql;
            const CPriKeyArray *pKey = &vPriKey;
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
            sql = L"CREATE TRIGGER ";
            sql += STREAMING_DB_TRIGGER_PREFIX;
            sql += (strDB + L"_");
            sql += (strTable + L"_");
            switch (eventType) {
                case SPA::UDB::ueDelete:
                    sql += L"DELETE AFTER DELETE ON `";
                    for (auto it = vPriKey.begin(), end = vPriKey.end(); it != end; ++it) {
                        if (it->Pri) {
                            vDelKey.push_back(*it);
                        }
                    }
                    if (vDelKey.size()) {
                        pKey = &vDelKey;
                    }
                    break;
                case SPA::UDB::ueInsert:
                    sql += L"INSERT AFTER INSERT ON `";
                    break;
                default: //update
                    sql += L"UPDATE AFTER UPDATE ON `";
                    break;
            }
            sql += db;
            sql += L"`.`";
            sql += table;
            sql += L"` FOR EACH ROW BEGIN DECLARE res BIGINT;";
            sql += L"SELECT PublishDBEvent(" + std::to_wstring((SPA::INT64)eventType);
            sql += L",USER(),DATABASE(),'";
            sql += table;
            sql += L"'";
            for (auto it = pKey->begin(), end = pKey->end(); it != end; ++it) {
                switch (eventType) {
                    case SPA::UDB::ueDelete:
                        sql += L",old.`";
                        break;
                    case SPA::UDB::ueInsert:
                        sql += L",new.`";
                        break;
                    default: //update
                        sql += L",old.`";
                        break;
                }
                sql += (SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size()) + L"`");
                if (eventType == SPA::UDB::ueUpdate) {
                    sql += L",new.`";
                    sql += (SPA::Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size()) + L"`");
                }
            }
            sql += L")INTO res;END";
            return sql;
        }

        void CMysqlImpl::CreateTriggers(const std::string &schema, const std::string & table) {
            bool bDelete = false, bInsert = false, bUpdate = false;
            std::wstring wSchema = SPA::Utilities::ToWide(schema.c_str(), schema.size());
            std::wstring wTable = SPA::Utilities::ToWide(table.c_str(), table.size());
            std::wstring prefix(STREAMING_DB_TRIGGER_PREFIX);
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
            std::wstring sql = L"SELECT COLUMN_NAME,COLUMN_KEY FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='";
            sql += (wSchema + L"' AND TABLE_NAME='");
            sql += (wTable + L"' ORDER BY TABLE_NAME,ORDINAL_POSITION");
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s failed for creating triggers(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return;
            }
            if (!m_qSend.GetSize()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create triggers for the table %s.%s because it has no table or primary key defined", schema.c_str(), table.c_str());
                return;
            }

            CPriKeyArray vKey;
            PriKey pk;
            while (m_qSend.GetSize()) {
                m_qSend >> vtName >> vtType;
                pk.ColumnName = ToString(vtName);
                std::string type = ToString(vtType);
                if (type == "PRI")
                    pk.Pri = true;
                else
                    pk.Pri = false;
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
            std::wstring prefix(STREAMING_DB_TRIGGER_PREFIX);
            std::wstring sql_existing = L"SELECT event_object_schema,trigger_name,EVENT_OBJECT_TABLE FROM INFORMATION_SCHEMA.TRIGGERS where TRIGGER_NAME like '" + prefix + L"%' order by event_object_schema,EVENT_OBJECT_TABLE";
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

