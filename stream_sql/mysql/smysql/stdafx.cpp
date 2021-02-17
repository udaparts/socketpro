
#include "stdafx.h"
#include "streamingserver.h"
#include "umysql_udf.h"

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
                if (len <= 19) {
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
            if (!impl.m_pMysql && !impl.OpenSession(CSetGlobals::Globals.Config.auth_account.c_str(), "localhost"))
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

        CDBString CMysqlImpl::GetCreateTriggerSQL(const UTF16 *db, const UTF16 *table, const CPriKeyArray &vPriKey, SPA::UDB::tagUpdateEvent eventType) {
            CDBString sql;
            CPriKeyArray vDelKey;
            if (!vPriKey.size())
                return sql;
            const CPriKeyArray *pKey = &vPriKey;
            CDBString strDB(db), strTable(table);
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
            sql = u"CREATE TRIGGER ";
            sql += STREAMING_DB_TRIGGER_PREFIX;
            sql += (strDB + u"_");
            sql += (strTable + u"_");
            switch (eventType) {
                case tagUpdateEvent::ueDelete:
                    sql += u"DELETE AFTER DELETE ON `";
                    for (auto it = vPriKey.begin(), end = vPriKey.end(); it != end; ++it) {
                        if (it->Pri) {
                            vDelKey.push_back(*it);
                        }
                    }
                    if (vDelKey.size()) {
                        pKey = &vDelKey;
                    }
                    break;
                case tagUpdateEvent::ueInsert:
                    sql += u"INSERT AFTER INSERT ON `";
                    break;
                default: //update
                    sql += u"UPDATE AFTER UPDATE ON `";
                    break;
            }
            sql += db;
            sql += u"`.`";
            sql += table;
            sql += u"` FOR EACH ROW BEGIN DECLARE res BIGINT;";
            sql += u"SELECT PublishDBEvent(" + CDBString(Utilities::ToUTF16(std::to_string((SPA::INT64)eventType)));
            sql += u",USER(),DATABASE(),'";
            sql += table;
            sql += u"'";
            for (auto it = pKey->begin(), end = pKey->end(); it != end; ++it) {
                switch (eventType) {
                    case tagUpdateEvent::ueDelete:
                        sql += u",old.`";
                        break;
                    case tagUpdateEvent::ueInsert:
                        sql += u",new.`";
                        break;
                    default: //update
                        sql += u",old.`";
                        break;
                }
                sql += (it->ColumnName + u"`");
                if (eventType == tagUpdateEvent::ueUpdate) {
                    sql += u",new.`";
                    sql += (it->ColumnName + u"`");
                }
            }
            sql += u")INTO res;END";
            return sql;
        }

        void CMysqlImpl::CreateTriggers(const std::string &schema, const std::string & table) {
            bool bDelete = false, bInsert = false, bUpdate = false;
            CDBString wSchema = Utilities::ToUTF16(schema);
            CDBString wTable = Utilities::ToUTF16(table);
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = u"SELECT EVENT_MANIPULATION, TRIGGER_NAME FROM INFORMATION_SCHEMA.TRIGGERS WHERE ";
            sql_existing += (u"EVENT_OBJECT_SCHEMA='" + wSchema + u"'");
            sql_existing += (u" AND EVENT_OBJECT_TABLE='" + wTable + u"'");
            sql_existing += u" AND ACTION_TIMING='AFTER'";
            sql_existing += (u" AND TRIGGER_NAME LIKE '" + prefix + u"%' ORDER BY EVENT_MANIPULATION");
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            CDBString errMsg;
            SPA::UDB::CDBVariant vtType, vtName;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s triggers failed(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
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
            CDBString sql = u"SELECT COLUMN_NAME,COLUMN_KEY FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='";
            sql += (wSchema + u"' AND TABLE_NAME='");
            sql += (wTable + u"' ORDER BY TABLE_NAME,ORDINAL_POSITION");
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s failed for creating triggers(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
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
                pk.ColumnName = Utilities::ToUTF16(ToString(vtName));
                std::string type = ToString(vtType);
                if (type == "PRI")
                    pk.Pri = true;
                else
                    pk.Pri = false;
                vKey.push_back(pk);
            }
            sql = u"USE `" + wSchema + u"`";
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);

            if (!bInsert) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, tagUpdateEvent::ueInsert);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create insert trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                }
            }
            if (!bDelete) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, tagUpdateEvent::ueDelete);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create delete trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                }
            }
            if (!bUpdate) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, tagUpdateEvent::ueUpdate);
                Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create update trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                }
            }
        }

        void CMysqlImpl::RemoveUnusedTriggers(const std::vector<std::string> &vecTables) {
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = u"SELECT event_object_schema,trigger_name,EVENT_OBJECT_TABLE FROM INFORMATION_SCHEMA.TRIGGERS where TRIGGER_NAME like '" + prefix + u"%' order by event_object_schema,EVENT_OBJECT_TABLE";
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            CDBString errMsg;
            SPA::UDB::CDBVariant vtSchema, vtName, vtTable;
            std::vector<std::string> vec = vecTables;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying SocketPro streaming db triggers failed(errCode=%d; errMsg=%s)", res, Utilities::ToUTF8(errMsg).c_str());
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
                    CDBString wsql = CDBString(u"USE `") + Utilities::ToUTF16(schema) + u"`";
                    Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                    res = 0;

                    //trigger not needed any more as it is not found inside sp_streaming_db.config.cached_tables
                    wsql = CDBString(u"drop trigger ") + Utilities::ToUTF16(name);
                    Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                    if (res) {
                        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Removing the unused trigger %s failed(errCode=%d; errMsg=%s)", name.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                        res = 0;
                    }
                }
            }
        }
    } //namespace ServerSide
} //namespace SPA
