
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
            impl.m_NoSending = true;
            impl.RemoveUnusedTriggers(vecTables);
            for (auto it = vecTables.begin(), end = vecTables.end(); it != end; ++it) {
                auto pos = it->find_last_of('.');
                std::string schema = it->substr(0, pos);
                std::string table = it->substr(pos + 1);
                impl.CreateTriggers(schema, table);
            }
        }

        void CMysqlImpl::CreateTriggers(const std::string &schema, const std::string & table) {
            bool bDelete = false, bInsert = false, bUpdate = false;
            std::wstring prefix(STREAMING_DB_TRIGGER_FIX);
            std::wstring sql_existing = L"SELECT EVENT_MANIPULATION, TRIGGER_NAME FROM INFORMATION_SCHEMA.TRIGGERS WHERE ";
            sql_existing += L"EVENT_OBJECT_SCHEMA='" + SPA::Utilities::ToWide(schema.c_str(), schema.size()) + L"'";
            sql_existing += L" AND EVENT_OBJECT_TABLE='" + SPA::Utilities::ToWide(table.c_str(), table.size()) + L"'";
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
            if (!bInsert) {

            }
            if (!bDelete) {

            }
            if (!bUpdate) {

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
            while (m_qSend.GetSize() && !res) {
                m_qSend >> vtSchema >> vtName >> vtTable;
                std::string schema = ToString(vtSchema);
                std::string name = ToString(vtName);
                std::string table = ToString(vtTable);

                std::string trigger_db_table = schema + "." + table;
                std::transform(trigger_db_table.begin(), trigger_db_table.end(), trigger_db_table.begin(), ::tolower);

                auto ret = std::find_if(vec.begin(), vec.end(), [&trigger_db_table](const std::string & s) {
                    return (trigger_db_table == s);
                });
                if (ret == vec.end()) {
                    //trigger not needed any more as it is not found inside sp_streaming_db.config.cached_tables
                    std::wstring wsql = L"drop trigger " + SPA::Utilities::ToWide(name.c_str(), name.size());
                    Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                    if (res) {
                        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Removing the unused trigger %s failed(errCode=%d; errMsg=%s)", name.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                    }
                }
            }
        }

    } //namespace ServerSide
} //namespace SPA

