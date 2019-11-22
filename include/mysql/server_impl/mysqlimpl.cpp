
#include "mysqlimpl.h"
#include <sstream>
#ifndef NDEBUG
#include <iostream>
#endif
#include "../umysql_server.h"
#include "../../scloader.h"
#include <cctype>

#ifdef MM_DB_SERVER_PLUGIN
#include "../../../stream_sql/mariadb/smysql/streamingserver.h"
#include "../include/mysqld_error.h"
#include "../../../stream_sql/mariadb/smysql/umysql_udf.h"
#endif

#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID < 50700
#define MYSQL_OPT_SSL_KEY ((mysql_option) 25)
#define MYSQL_OPT_SSL_CERT ((mysql_option) 26)
#define MYSQL_OPT_SSL_CA ((mysql_option) 27)
#define MYSQL_OPT_SSL_CAPATH ((mysql_option) 28)
#define MYSQL_OPT_SSL_CIPHER ((mysql_option) 29)
#endif

namespace SPA
{
    namespace ServerSide{

        CUCriticalSection CMysqlImpl::m_csPeer;
        my_bool CMysqlImpl::B_IS_NULL = 1;
        CDBString CMysqlImpl::m_strGlobalConnection;
        bool CMysqlImpl::m_bInitMysql = false;
#ifdef WIN32_64
        const UTF16 * CMysqlImpl::NO_DB_OPENED_YET = L"No mysql database opened yet";
        const UTF16 * CMysqlImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const UTF16 * CMysqlImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const UTF16 * CMysqlImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const UTF16 * CMysqlImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const UTF16 * CMysqlImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const UTF16 * CMysqlImpl::NO_DB_NAME_SPECIFIED = L"No mysql database name specified";
        const UTF16 * CMysqlImpl::MYSQL_LIBRARY_NOT_INITIALIZED = L"Mysql library not initialized";
        const UTF16 * CMysqlImpl::BAD_MANUAL_TRANSACTION_STATE = L"Bad manual transaction state";
        const UTF16 * CMysqlImpl::MYSQL_GLOBAL_CONNECTION_STRING = L"MYSQL_GLOBAL_CONNECTION_STRING";
#else
        const UTF16 * CMysqlImpl::NO_DB_OPENED_YET = u"No mysql database opened yet";
        const UTF16 * CMysqlImpl::BAD_END_TRANSTACTION_PLAN = u"Bad end transaction plan";
        const UTF16 * CMysqlImpl::NO_PARAMETER_SPECIFIED = u"No parameter specified";
        const UTF16 * CMysqlImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = u"Bad parameter data array length";
        const UTF16 * CMysqlImpl::BAD_PARAMETER_COLUMN_SIZE = u"Bad parameter column size";
        const UTF16 * CMysqlImpl::DATA_TYPE_NOT_SUPPORTED = u"Data type not supported";
        const UTF16 * CMysqlImpl::NO_DB_NAME_SPECIFIED = u"No mysql database name specified";
        const UTF16 * CMysqlImpl::MYSQL_LIBRARY_NOT_INITIALIZED = u"Mysql library not initialized";
        const UTF16 * CMysqlImpl::BAD_MANUAL_TRANSACTION_STATE = u"Bad manual transaction state";
        const UTF16 * CMysqlImpl::MYSQL_GLOBAL_CONNECTION_STRING = u"MYSQL_GLOBAL_CONNECTION_STRING";
#endif

        unsigned int CMysqlImpl::m_nParam = 0;

        CMysqlLoader CMysqlImpl::m_remMysql;
        CMysqlImpl::CMyMap CMysqlImpl::m_mapConnection;

        void CMysqlImpl::MYSQL_CONNECTION_STRING::Init() {
            timeout = 10;
            port = 3306;
            database.clear();
            host.clear();
            password.clear();
            ssl_ca.clear();
            ssl_capath.clear();
            ssl_cert.clear();
            ssl_cipher.clear();
            ssl_key.clear();
            user.clear();
        }

        void CMysqlImpl::MYSQL_CONNECTION_STRING::Parse(const char *s) {
            using namespace std;
            Init();
            if (!strstr(s, "="))
                return;
            stringstream ss(s ? s : "");
            string item;
            vector<string> tokens;
            while (getline(ss, item, ';')) {
                tokens.push_back(item);
            }
            for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
                auto pos = it->find('=');
                if (pos == string::npos)
                    continue;
                string left = it->substr(0, pos);
                string right = it->substr(pos + 1);
                Utilities::Trim(left);
                Utilities::Trim(right);
                transform(left.begin(), left.end(), left.begin(), ::tolower);
                if (left == "connect-timeout" || left == "timeout" || left == "connection-timeout")
                    timeout = (unsigned int) std::atoi(right.c_str());
                else if (left == "database" || left == "db")
                    database = right;
                else if (left == "port")
                    port = (unsigned int) std::atoi(right.c_str());
                else if (left == "pwd" || left == "password")
                    password = right;
                else if (left == "host" || left == "server")
                    host = right;
                else if (left == "user" || left == "uid")
                    user = right;
                else if (left == "socket")
                    socket = right;
                else if (left == "ssl-ca")
                    ssl_ca = right;
                else if (left == "ssl-capath")
                    ssl_capath = right;
                else if (left == "ssl-cert")
                    ssl_cert = right;
                else if (left == "ssl-cipher")
                    ssl_cipher = right;
                else if (left == "ssl-key")
                    ssl_key = right;
                else {
                    //!!! not implemented
                    assert(false);
                }
            }
        }

#ifdef MM_DB_SERVER_PLUGIN

        std::string CMysqlImpl::ToString(const CDBVariant & vtUTF8) {
            assert(vtUTF8.Type() == (VT_I1 | VT_ARRAY));
            const char *p;
            ::SafeArrayAccessData(vtUTF8.parray, (void**) &p);
            unsigned int len = vtUTF8.parray->rgsabound->cElements;
            std::string s(p, p + len);
            ::SafeArrayUnaccessData(vtUTF8.parray);
            return s;
        }

        bool CMysqlImpl::SetPublishDBEvent(CMysqlImpl & impl) {
#ifdef WIN32_64
            CDBString wsql = L"CREATE FUNCTION PublishDBEvent RETURNS INTEGER SONAME 'smysql.dll'";
#else
            CDBString wsql = u"CREATE FUNCTION PublishDBEvent RETURNS INTEGER SONAME 'libsmysql.so'";
#endif
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            CDBString errMsg;
            impl.Execute(wsql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
            //Setting streaming DB events failed(errCode=1125; errMsg=Function 'PublishDBEvent' already exists)
            if (res && res != ER_UDF_EXISTS) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Setting streaming DB events failed(errCode=%d; errMsg=%s)", res, Utilities::ToUTF8(errMsg).c_str());
                return false;
            }
            return true;
        }

        bool CMysqlImpl::CreateTriggers(CMysqlImpl &impl, const std::vector<std::string> &vecTables) {
            if (!impl.RemoveUnusedTriggers(vecTables))
                return false;
            for (auto it = vecTables.begin(), end = vecTables.end(); it != end; ++it) {
                auto pos = it->find_last_of('.');
                std::string schema = it->substr(0, pos);
                std::string table = it->substr(pos + 1);
                if (!impl.CreateTriggers(schema, table)) return false;
            }
            return true;
        }

        bool CMysqlImpl::RemoveUnusedTriggers(const std::vector<std::string> &vecTables) {
#ifdef WIN32_64
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = L"SELECT event_object_schema,trigger_name,EVENT_OBJECT_TABLE FROM INFORMATION_SCHEMA.TRIGGERS where TRIGGER_NAME like '" + prefix + L"%' order by event_object_schema,EVENT_OBJECT_TABLE";
#else
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = u"SELECT event_object_schema,trigger_name,EVENT_OBJECT_TABLE FROM INFORMATION_SCHEMA.TRIGGERS where TRIGGER_NAME like '" + prefix + u"%' order by event_object_schema,EVENT_OBJECT_TABLE";
#endif
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            CDBString errMsg;
            SPA::UDB::CDBVariant vtSchema, vtName, vtTable;
            std::vector<std::string> vec = vecTables;
            SPA::CScopeUQueue sb;
            SPA::CUQueue &q = *sb;
            m_pNoSending = &q;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            m_pNoSending = nullptr;
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying SocketPro streaming db triggers failed(errCode=%d; errMsg=%s)", res, Utilities::ToUTF8(errMsg).c_str());
                return false;
            }
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
#ifdef WIN32_64
                    CDBString wsql = L"USE `" + Utilities::ToWide(schema) + L"`";
#else
                    CDBString wsql = u"USE `" + CDBString(Utilities::ToUTF16(schema)) + u"`";
#endif
                    Execute(wsql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                    res = 0;

                    //trigger not needed any more as it is not found inside sp_streaming_db.config.cached_tables
#ifdef WIN32_64
                    wsql = L"drop trigger " + Utilities::ToWide(name);
#else
                    wsql = u"drop trigger " + CDBString(Utilities::ToUTF16(name));
#endif
                    Execute(wsql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                    if (res) {
                        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Removing the unused trigger %s failed(errCode=%d; errMsg=%s)", name.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                        res = 0;
                        return false;
                    }
                }
            }
            return true;
        }

        bool CMysqlImpl::CreateTriggers(const std::string &schema, const std::string & table) {
            bool bDelete = false, bInsert = false, bUpdate = false;
            CDBString wSchema = Utilities::ToUTF16(schema);
            CDBString wTable = Utilities::ToUTF16(table);
#ifdef WIN32_64
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = L"SELECT EVENT_MANIPULATION, TRIGGER_NAME FROM INFORMATION_SCHEMA.TRIGGERS WHERE ";
            sql_existing += L"EVENT_OBJECT_SCHEMA='" + wSchema + L"'";
            sql_existing += L" AND EVENT_OBJECT_TABLE='" + wTable + L"'";
            sql_existing += L" AND ACTION_TIMING='AFTER'";
            sql_existing += L" AND TRIGGER_NAME LIKE '" + prefix + L"%' ORDER BY EVENT_MANIPULATION";
#else
            CDBString prefix(STREAMING_DB_TRIGGER_PREFIX);
            CDBString sql_existing = u"SELECT EVENT_MANIPULATION, TRIGGER_NAME FROM INFORMATION_SCHEMA.TRIGGERS WHERE ";
            sql_existing += u"EVENT_OBJECT_SCHEMA='" + wSchema + u"'";
            sql_existing += u" AND EVENT_OBJECT_TABLE='" + wTable + u"'";
            sql_existing += u" AND ACTION_TIMING='AFTER'";
            sql_existing += u" AND TRIGGER_NAME LIKE '" + prefix + u"%' ORDER BY EVENT_MANIPULATION";
#endif
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            CDBString errMsg;
            SPA::UDB::CDBVariant vtType, vtName;
            SPA::CScopeUQueue sb;
            SPA::CUQueue &q = *sb;
            m_pNoSending = &q;
            Execute(sql_existing, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            m_pNoSending = nullptr;
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s triggers failed(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                return false;
            }
            while (q.GetSize() && !res) {
                q >> vtType >> vtName;
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
                return false;
#ifdef WIN32_64
            CDBString sql = L"SELECT COLUMN_NAME,COLUMN_KEY FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='";
            sql += (wSchema + L"' AND TABLE_NAME='");
            sql += (wTable + L"' ORDER BY TABLE_NAME,ORDINAL_POSITION");
#else
            CDBString sql = u"SELECT COLUMN_NAME,COLUMN_KEY FROM information_schema.COLUMNS WHERE TABLE_SCHEMA='";
            sql += (wSchema + u"' AND TABLE_NAME='");
            sql += (wTable + u"' ORDER BY TABLE_NAME,ORDINAL_POSITION");
#endif
            m_pNoSending = &q;
            Execute(sql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            m_pNoSending = nullptr;
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Querying the table %s.%s failed for creating triggers(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                return false;
            }
            if (!q.GetSize()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create triggers for the table %s.%s because it has no table or primary key defined", schema.c_str(), table.c_str());
                return false;
            }
            CPriKeyArray vKey;
            PriKey pk;
            while (q.GetSize()) {
                q >> vtName >> vtType;
                pk.ColumnName = ToString(vtName);
                std::string type = ToString(vtType);
                if (type == "PRI")
                    pk.Pri = true;
                else
                    pk.Pri = false;
                vKey.push_back(pk);
            }
#ifdef WIN32_64
            sql = L"USE `" + wSchema + L"`";
#else
            sql = u"USE `" + wSchema + u"`";
#endif
            Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (!bInsert) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueInsert);
                Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create insert trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                    return false;
                }
            }
            if (!bDelete) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueDelete);
                Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create delete trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                    return false;
                }
            }
            if (!bUpdate) {
                sql = GetCreateTriggerSQL(wSchema.c_str(), wTable.c_str(), vKey, SPA::UDB::ueUpdate);
                Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Unable to create update trigger for the table %s.%s(errCode=%d; errMsg=%s)", schema.c_str(), table.c_str(), res, Utilities::ToUTF8(errMsg).c_str());
                    return false;
                }
            }
            return true;
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
                    *it = '_';
                }
            }
            for (auto it = strTable.begin(), end = strTable.end(); it != end; ++it) {
                if (isspace(*it)) {
                    *it = '_';
                }
            }
#ifdef WIN32_64
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
                sql += (Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size()) + L"`");
                if (eventType == SPA::UDB::ueUpdate) {
                    sql += L",new.`";
                    sql += (Utilities::ToWide(it->ColumnName.c_str(), it->ColumnName.size()) + L"`");
                }
            }
            sql += L")INTO res;END";
#else
            sql = u"CREATE TRIGGER ";
            sql += STREAMING_DB_TRIGGER_PREFIX;
            sql += (strDB + u"_");
            sql += (strTable + u"_");
            switch (eventType) {
                case SPA::UDB::ueDelete:
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
                case SPA::UDB::ueInsert:
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
                    case SPA::UDB::ueDelete:
                        sql += u",old.`";
                        break;
                    case SPA::UDB::ueInsert:
                        sql += u",new.`";
                        break;
                    default: //update
                        sql += u",old.`";
                        break;
                }
                sql += (CDBString(Utilities::ToUTF16(it->ColumnName)) + u"`");
                if (eventType == SPA::UDB::ueUpdate) {
                    sql += u",new.`";
                    sql += (CDBString(Utilities::ToUTF16(it->ColumnName)) + u"`");
                }
            }
            sql += u")INTO res;END";
#endif
            return sql;
        }
#endif

        CMysqlImpl::CMysqlImpl() : m_oks(0), m_fails(0), m_ti(tiUnspecified),
        m_global(true), m_Blob(*m_sb), m_parameters(0),
        m_bCall(false), m_bManual(false), m_EnableMessages(false), m_pNoSending(nullptr) {
            m_Blob.ToUtf8(true);
#ifdef WIN32_64
            m_UQueue.TimeEx(true); //use high-precision datetime
#endif
            m_UQueue.ToUtf8(true);
        }

        void CMysqlImpl::UnloadMysql() {
            SPA::CAutoLock al(m_csPeer);
            m_remMysql.Unload();
        }

        void CALLBACK CMysqlImpl::OnThreadEvent(tagThreadEvent te) {
            if (te == teStarted) {
                my_bool fail = m_remMysql.mysql_thread_init();
                assert(!fail);
            } else {
                m_remMysql.mysql_thread_end();
            }
        }

        bool CMysqlImpl::InitMySql() {
            SPA::CAutoLock al(m_csPeer);
            if (m_bInitMysql) {
                return true;
            }
            if (m_remMysql.LoadMysql()) {
                m_bInitMysql = (m_remMysql.mysql_server_init(0, nullptr, nullptr) == 0);
                ServerCoreLoader.SetThreadEvent(OnThreadEvent);
            }
            return m_bInitMysql;
        }

        void CMysqlImpl::SetDBGlobalConnectionString(const wchar_t *dbConnection, bool remote) {
            m_csPeer.lock();
            if (dbConnection) {
                m_strGlobalConnection = Utilities::ToUTF16(dbConnection);
            } else {
                m_strGlobalConnection.clear();
            }
            m_csPeer.unlock();
        }

        void CMysqlImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanDBObjects();
            m_global = true;
            MYSQL_BIND_RESULT_FIELD::ShrinkMemoryPool();
        }

        void CMysqlImpl::ResetMemories() {
            m_Blob.SetSize(0);
            if (m_Blob.GetMaxSize() > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_Blob.ReallocBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
        }

        void CMysqlImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_oks = 0;
            m_fails = 0;
            m_ti = tiUnspecified;
            m_bManual = false;
            USocket_Server_Handle hSocket = GetSocketHandle();
            CAutoLock al(m_csPeer);
            CMyMap::iterator it = m_mapConnection.find(hSocket);
            if (it != m_mapConnection.end()) {
                m_pMysql = it->second.Handle;
                m_dbNameOpened = it->second.DefaultDB;
                m_mapConnection.erase(hSocket);
            }
        }

        void CMysqlImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I1_R0(idStartBLOB, StartBLOB, unsigned int)
            M_I0_R0(idChunk, Chunk)
            M_I0_R0(idEndBLOB, EndBLOB)
            M_I0_R0(idBeginRows, BeginRows)
            M_I0_R0(idTransferring, Transferring)
            M_I0_R0(idEndRows, EndRows)
            END_SWITCH
        }

        int CMysqlImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I0_R2(idClose, CloseDb, int, CDBString)
            M_I2_R3(idOpen, Open, CDBString, unsigned int, int, CDBString, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, CDBString, unsigned int, int, CDBString, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, CDBString)
            M_I5_R5(idExecute, Execute, CDBString, bool, bool, bool, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, CDBString, CParameterInfoArray, int, CDBString, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            M_I10_R5(idExecuteBatch, ExecuteBatch, CDBString, CDBString, int, int, bool, bool, bool, CDBString, unsigned int, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            END_SWITCH
            if (reqId == idExecuteParameters || reqId == idExecuteBatch) {
                m_vParam.clear();
            }
            return 0;
        }

        void CMysqlImpl::Open(const CDBString &strConnection, unsigned int flags, int &res, CDBString &errMsg, int &ms) {
            ms = msMysql;
            if ((flags & ENABLE_TABLE_UPDATE_MESSAGES) == ENABLE_TABLE_UPDATE_MESSAGES) {
                m_EnableMessages = GetPush().Subscribe(&STREAMING_SQL_CHAT_GROUP_ID, 1);
            }
            if (!InitMySql()) {
                res = SPA::Mysql::ER_MYSQL_LIBRARY_NOT_INITIALIZED;
                errMsg = MYSQL_LIBRARY_NOT_INITIALIZED;
                return;
            }
            if (m_pMysql) {
                res = 0;
                CDBString db(strConnection);
                Utilities::Trim(db);
                if (m_dbNameOpened.size() && !db.size()) {
                    errMsg = m_dbNameOpened;
                } else {
#ifdef WIN32_64
                    std::transform(db.begin(), db.end(), db.begin(), ::tolower);
                    std::transform(m_dbNameOpened.begin(), m_dbNameOpened.end(), m_dbNameOpened.begin(), ::tolower);
#endif
                    if (!db.size()) {
#ifdef WIN32_64
                        db = L"mysql"; //default to mysql database
#else
                        db = u"mysql"; //default to mysql database
#endif
                    }
                    if (db == m_dbNameOpened) {
                        errMsg = db;
                        return;
                    }
                    INT64 affected;
                    CDBVariant vtId;
                    UINT64 fail_ok;
#ifdef WIN32_64
                    CDBString sql(L"USE ");
#else
                    CDBString sql(u"USE ");
#endif
                    sql += db;
                    Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                    if (res) {
                        int r;
                        CDBString err;
                        CloseDb(r, err);
                    } else {
                        errMsg = db;
                        m_dbNameOpened = db;
                    }
                }
            } else {
                MYSQL *mysql = m_remMysql.mysql_init(nullptr);
                do {
                    CDBString db(strConnection);
                    if (!db.size() || db == MYSQL_GLOBAL_CONNECTION_STRING) {
                        m_csPeer.lock();
                        db = m_strGlobalConnection;
                        m_csPeer.unlock();
                        m_global = true;
                    } else {
                        m_global = false;
                    }
                    m_remMysql.mysql_options(mysql, MYSQL_SET_CHARSET_NAME, "utf8");
                    MYSQL_CONNECTION_STRING conn;
                    conn.Parse(Utilities::ToUTF8(db).c_str());
                    int failed = m_remMysql.mysql_options(mysql, MYSQL_OPT_CONNECT_TIMEOUT, &conn.timeout);
                    assert(!failed);
#if 0 //def WIN32_64
                    unsigned int sm = MYSQL_PROTOCOL_MEMORY;
                    failed = m_remMysql.mysql_options(mysql, MYSQL_OPT_PROTOCOL, &sm);
                    assert(!failed);
                    failed = m_remMysql.mysql_options(mysql, MYSQL_SHARED_MEMORY_BASE_NAME, "MYSQL");
                    assert(!failed);
#endif
#ifndef WIN32_64
                    if (conn.socket.size()) {
                        unsigned int socket = MYSQL_PROTOCOL_SOCKET;
                        failed = m_remMysql.mysql_options(mysql, MYSQL_OPT_PROTOCOL, &socket);
                        assert(!failed);
                        conn.host = "localhost";
                    }
#else
                    conn.socket.clear();
#endif
                    if (conn.IsSSL()) {
                        if (m_remMysql.mysql_get_client_version() > 50700) {
                            if (conn.ssl_ca.size())
                                m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_CA, conn.ssl_ca.c_str());
                            if (conn.ssl_capath.size())
                                m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_CAPATH, conn.ssl_capath.c_str());
                            if (conn.ssl_cert.size())
                                m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_CERT, conn.ssl_cert.c_str());
                            if (conn.ssl_cipher.size())
                                m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_CIPHER, conn.ssl_cipher.c_str());
                            if (conn.ssl_key.size())
                                m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_KEY, conn.ssl_key.c_str());
                        } else {
                            my_bool ssl_enabled = 1;
                            m_remMysql.mysql_options(mysql, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_enabled);
                        }
                    }

                    char *socket = nullptr;
                    if (conn.socket.size())
                        socket = (char*) conn.socket.c_str();
                    MYSQL *ret = m_remMysql.mysql_real_connect(mysql, conn.host.c_str(), conn.user.c_str(),
                            conn.password.c_str(), conn.database.c_str(), conn.port, socket,
                            CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS | CLIENT_LOCAL_FILES | CLIENT_IGNORE_SIGPIPE);
                    if (!ret) {
                        res = m_remMysql.mysql_errno(mysql);
                        errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(mysql));
                        break;
                    } else {
                        res = 0;
                    }
                    m_dbNameOpened = Utilities::ToUTF16(conn.database);
                    if (!m_global) {
                        errMsg = db;
                    } else {
                        errMsg = MYSQL_GLOBAL_CONNECTION_STRING;
                    }
                } while (false);
                if (!res) {
                    m_pMysql.reset(mysql, [](MYSQL * mysql) {
                        if (mysql) {
                            m_remMysql.mysql_close(mysql);
                        }
                    });
                } else if (mysql) {
                    m_remMysql.mysql_close(mysql);
                }
            }
        }

        void CMysqlImpl::CloseDb(int &res, CDBString & errMsg) {
            if (m_EnableMessages) {
                GetPush().Unsubscribe();
            }
            CleanDBObjects();
            res = 0;
        }

        void CMysqlImpl::CleanDBObjects() {
            m_pPrepare.reset();
            m_pMysql.reset();
            m_vParam.clear();
            m_parameters = 0;
            ResetMemories();
            m_bManual = false;
            m_ti = tiUnspecified;
            m_EnableMessages = false;
            m_dbNameOpened.clear();
        }

        void CMysqlImpl::OnBaseRequestArrive(unsigned short requestId) {
            switch (requestId) {
                case idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                    do {
                        MYSQL *mysql = m_pMysql.get();
                        if (!mysql)
                            break;
                        int status = 0;
                        unsigned long id = m_remMysql.mysql_thread_id(mysql);
                        std::string sqlKill = "KILL QUERY " + std::to_string((UINT64) id);
                        status = m_remMysql.mysql_query(mysql, sqlKill.c_str());
                        if (!m_bManual)
                            break;
                        status = m_remMysql.mysql_rollback(mysql);
                    } while (false);
                    break;
                default:
                    break;
            }
        }

        void CMysqlImpl::BeginTrans(int isolation, const CDBString &dbConn, unsigned int flags, int &res, CDBString &errMsg, int &ms) {
            ms = msMysql;
            if (m_bManual) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (!m_pMysql) {
                Open(dbConn, flags, res, errMsg, ms);
                if (!m_pMysql) {
                    return;
                }
            }
            std::string sql;
            if ((int) m_ti != isolation) {
                switch ((tagTransactionIsolation) isolation) {
                    case tiReadUncommited:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ UNCOMMITTED";
                        break;
                    case tiRepeatableRead:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL REPEATABLE READ";
                        break;
                    case tiReadCommited:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED";
                        break;
                    case tiSerializable:
                        sql = "SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE";
                        break;
                    default:
                        errMsg = BAD_MANUAL_TRANSACTION_STATE;
                        res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                        return;
                }
            }
            if (sql.size()) {
                int status = m_remMysql.mysql_real_query(m_pMysql.get(), sql.c_str(), (unsigned long) sql.size());
                if (status) {
                    res = m_remMysql.mysql_errno(m_pMysql.get());
                    errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                    return;
                }
            }
            my_bool fail = m_remMysql.mysql_autocommit(m_pMysql.get(), 0);
            if (!fail) {
                res = 0;
                m_fails = 0;
                m_oks = 0;
                m_ti = (tagTransactionIsolation) isolation;
                if (!m_global) {
                    errMsg = dbConn;
                } else {
                    errMsg = MYSQL_GLOBAL_CONNECTION_STRING;
                }
                m_bManual = true;
            } else {
                res = m_remMysql.mysql_errno(m_pMysql.get());
                errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
            }
        }

        void CMysqlImpl::EndTrans(int plan, int &res, CDBString & errMsg) {
            if (!m_bManual) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (plan < 0 || plan > rpRollbackAlways) {
                res = SPA::Mysql::ER_BAD_END_TRANSTACTION_PLAN;
                errMsg = BAD_END_TRANSTACTION_PLAN;
                return;
            }
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            bool rollback = false;
            tagRollbackPlan rp = (tagRollbackPlan) plan;
            switch (rp) {
                case rpRollbackErrorAny:
                    rollback = m_fails ? true : false;
                    break;
                case rpRollbackErrorLess:
                    rollback = (m_fails < m_oks && m_fails) ? true : false;
                    break;
                case rpRollbackErrorEqual:
                    rollback = (m_fails >= m_oks) ? true : false;
                    break;
                case rpRollbackErrorMore:
                    rollback = (m_fails > m_oks) ? true : false;
                    break;
                case rpRollbackErrorAll:
                    rollback = (m_oks) ? false : true;
                    break;
                case rpRollbackAlways:
                    rollback = true;
                    break;
                default:
                    assert(false); //shouldn't come here
                    break;
            }
            my_bool fail;
            if (rollback) {
                fail = m_remMysql.mysql_rollback(m_pMysql.get());
            } else {
                fail = m_remMysql.mysql_commit(m_pMysql.get());
            }
            if (fail) {
                res = m_remMysql.mysql_errno(m_pMysql.get());
                errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
            } else {
                res = 0;
                m_fails = 0;
                m_oks = 0;
            }
            m_bManual = false;
        }

        void CMysqlImpl::ExecuteSqlWithoutRowset(int &res, CDBString &errMsg, INT64 & affected) {
            do {
                MYSQL_RES *result = m_remMysql.mysql_use_result(m_pMysql.get());
                if (result) {
                    m_remMysql.mysql_free_result(result);
                    ++m_oks;

                    //For SELECT statements, mysql_affected_rows() works like mysql_num_rows().
                    //affected += (INT64)m_remMysql.mysql_affected_rows(m_pMysql.get());
                } else {
                    int errCode = m_remMysql.mysql_errno(m_pMysql.get());
                    if (!errCode) {
                        ++m_oks;
                        affected += (INT64) m_remMysql.mysql_affected_rows(m_pMysql.get());
                    } else {
                        ++m_fails;
                        if (!res) {
                            res = errCode;
                            errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                        }
                    }
                }
                int status = m_remMysql.mysql_next_result(m_pMysql.get());
                if (status == -1) {
                    break; //Successful and there are no more results
                } else if (status > 0) {
                    ++m_fails;
                    if (!res) {
                        res = m_remMysql.mysql_errno(m_pMysql.get());
                        errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                    }
                    break;
                } else if (status == 0) {
                    //Successful and there are more results
                } else {
                    assert(false); //never come here
                }
            } while (true);
        }

        CDBColumnInfoArray CMysqlImpl::GetColInfo(MYSQL_RES *result, unsigned int cols, bool meta) {
            CDBColumnInfoArray vCols(cols);
            MYSQL_FIELD *field = m_remMysql.mysql_fetch_fields(result);
            for (unsigned int n = 0; n < cols; ++n) {
                CDBColumnInfo &info = vCols[n];
                MYSQL_FIELD &f = field[n];
                if (meta) {
                    if (f.name) {
                        info.DisplayName.assign(f.name, f.name + ::strlen(f.name));
                    } else {
                        info.DisplayName.clear();
                    }
                    if (f.org_name) {
                        info.OriginalName.assign(f.org_name, f.org_name + ::strlen(f.org_name));
                    } else {
                        info.OriginalName = info.DisplayName;
                    }

                    if (f.table) {
                        info.TablePath.assign(f.table, f.table + ::strlen(f.table));
                    } else if (f.org_table) {
                        info.TablePath.assign(f.org_table, f.org_table + ::strlen(f.org_table));
                    } else {
                        info.TablePath.clear();
                    }
                    if (f.db) {
                        info.DBPath.assign(f.db, f.db + ::strlen(f.db));
                    } else {
                        info.DBPath.clear();
                    }

                    if (f.org_table && (f.org_table != f.table)) {
                        info.Collation.assign(f.org_table, f.org_table + ::strlen(f.org_table));
                    } else if (f.catalog) {
                        info.Collation.assign(f.catalog, f.catalog + ::strlen(f.catalog));
                    } else if (f.def) {
                        info.Collation.assign(f.def, f.def + ::strlen(f.def));
                    } else {
                        info.Collation.clear();
                    }
                }
                if ((f.flags & NOT_NULL_FLAG) == NOT_NULL_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                }
                if ((f.flags & PRI_KEY_FLAG) == PRI_KEY_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                }
                if ((f.flags & UNIQUE_KEY_FLAG) == UNIQUE_KEY_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
                }
                if ((f.flags & AUTO_INCREMENT_FLAG) == AUTO_INCREMENT_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                    info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID > 50700
                } else if (f.type == MYSQL_TYPE_JSON) {
                    info.Flags |= CDBColumnInfo::FLAG_JSON;
#endif
                }
                if ((f.flags & ENUM_FLAG) == ENUM_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_ENUM;
                } else if ((f.flags & SET_FLAG) == SET_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_SET;
                }

                if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG) {
                    info.Flags |= CDBColumnInfo::FLAG_IS_UNSIGNED;
                }

                switch (f.type) {
                    case MYSQL_TYPE_NEWDECIMAL:
                    case MYSQL_TYPE_DECIMAL:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"DECIMAL"; //The maximum number of digits for DECIMAL is 65, but the actual range for a given DECIMAL column can be constrained by the precision or scale for a given column.
#else
                            info.DeclaredType = u"DECIMAL";
#endif
                        }
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        info.Scale = (unsigned char) f.decimals;
                        info.Precision = 29;
                        break;
                    case MYSQL_TYPE_TINY:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"TINYINT";
#else
                            info.DeclaredType = u"TINYINT";
#endif
                        }
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI1;
                        else
                            info.DataType = VT_I1;
                        break;
                    case MYSQL_TYPE_SHORT:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"SMALLINT";
#else
                            info.DeclaredType = u"SMALLINT";
#endif
                        }
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI2;
                        else
                            info.DataType = VT_I2;
                        break;
                    case MYSQL_TYPE_LONG:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"INT";
#else
                            info.DeclaredType = u"INT";
#endif
                        }
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI4;
                        else
                            info.DataType = VT_I4;
                        break;
                    case MYSQL_TYPE_FLOAT:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"FLOAT";
#else
                            info.DeclaredType = u"FLOAT";
#endif
                        }
                        info.DataType = VT_R4;
                        break;
                    case MYSQL_TYPE_DOUBLE:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"DOUBLE";
#else
                            info.DeclaredType = u"DOUBLE";
#endif
                        }
                        info.DataType = VT_R8;
                        break;
                    case MYSQL_TYPE_TIMESTAMP:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"TIMESTAMP";
#else
                            info.DeclaredType = u"TIMESTAMP";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_LONGLONG:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"BIGINT";
#else
                            info.DeclaredType = u"BIGINT";
#endif
                        }
                        if ((f.flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                            info.DataType = VT_UI8;
                        else
                            info.DataType = VT_I8;
                        break;
                    case MYSQL_TYPE_INT24:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"MEDIUMINT";
#else
                            info.DeclaredType = u"MEDIUMINT";
#endif
                        }
                        info.DataType = VT_I4;
                        break;
                    case MYSQL_TYPE_DATE:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"DATE";
#else
                            info.DeclaredType = u"DATE";
#endif
                        }
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_TIME:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"TIME";
#else
                            info.DeclaredType = u"TIME";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_DATETIME: //#define DATETIME_MAX_DECIMALS 6
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"DATETIME";
#else
                            info.DeclaredType = u"DATETIME";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_YEAR:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"YEAR";
#else
                            info.DeclaredType = u"YEAR";
#endif
                        }
                        info.DataType = VT_I2;
                        break;
                    case MYSQL_TYPE_NEWDATE:
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"NEWDATE";
#else
                            info.DeclaredType = u"NEWDATE";
#endif
                        }
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_VARCHAR:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"VARBINARY";
#else
                                info.DeclaredType = u"VARBINARY";
#endif
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"VARCHAR";
#else
                                info.DeclaredType = u"VARCHAR";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        }
                        break;
                    case MYSQL_TYPE_BIT:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"BIT";
#else
                            info.DeclaredType = u"BIT";
#endif
                        }
                        info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
                        if (f.length == 1)
                            info.DataType = VT_BOOL;
                        else if (f.length <= 8)
                            info.DataType = VT_UI1;
                        else if (f.length <= 16)
                            info.DataType = VT_UI2;
                        else if (f.length <= 32)
                            info.DataType = VT_UI4;
                        else if (f.length <= 64)
                            info.DataType = VT_UI8;
                        else {
                            assert(false); //not implemented
                        }
                        break;
#if defined(MYSQL_VERSION_ID) && MYSQL_VERSION_ID > 50700
                    case MYSQL_TYPE_TIMESTAMP2:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"TIMESTAMP2";
#else
                            info.DeclaredType = u"TIMESTAMP2";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_DATETIME2:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"DATETIME2";
#else
                            info.DeclaredType = u"DATETIME2";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_TIME2:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"TIME2";
#else
                            info.DeclaredType = u"TIME2";
#endif
                        }
                        info.Scale = (unsigned char) f.decimals;
                        info.DataType = VT_DATE;
                        break;
                    case MYSQL_TYPE_JSON:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"JSON";
#else
                            info.DeclaredType = u"JSON";
#endif
                        }
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
#endif
                    case MYSQL_TYPE_ENUM:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"ENUM";
#else
                            info.DeclaredType = u"ENUM";
#endif
                        }
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
                    case MYSQL_TYPE_SET:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"SET";
#else
                            info.DeclaredType = u"SET";
#endif
                        }
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                        break;
                    case MYSQL_TYPE_TINY_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"TINYBLOB";
#else
                                info.DeclaredType = u"TINYBLOB";
#endif
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"TINYTEXT";
#else
                                info.DeclaredType = u"TINYTEXT";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_MEDIUM_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"MEDIUMBLOB";
#else
                                info.DeclaredType = u"MEDIUMBLOB";
#endif
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"MEDIUMTEXT";
#else
                                info.DeclaredType = u"MEDIUMTEXT";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_LONG_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"LONGBLOB";
#else
                                info.DeclaredType = u"LONGBLOB";
#endif
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"LONGTEXT";
#else
                                info.DeclaredType = u"LONGTEXT";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_BLOB:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
                                if (f.length == MYSQL_TINYBLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"TINYBLOB";
#else
                                    info.DeclaredType = u"TINYBLOB";
#endif
                                } else if (f.length == MYSQL_MIDBLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"MEDIUMBLOB";
#else
                                    info.DeclaredType = u"MEDIUMBLOB";
#endif
                                } else if (f.length == MYSQL_BLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"BLOB";
#else
                                    info.DeclaredType = u"BLOB";
#endif
                                } else {
#ifdef WIN32_64
                                    info.DeclaredType = L"LONGBLOB";
#else
                                    info.DeclaredType = u"LONGBLOB";
#endif
                                }
                            }
                            info.DataType = (VT_UI1 | VT_ARRAY); //binary
                        } else {
                            if (meta) {
                                if (f.length == MYSQL_TINYBLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"TINYTEXT";
#else
                                    info.DeclaredType = u"TINYTEXT";
#endif
                                } else if (f.length == MYSQL_MIDBLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"MEDIUMTEXT";
#else
                                    info.DeclaredType = u"MEDIUMTEXT";
#endif
                                } else if (f.length == MYSQL_BLOB) {
#ifdef WIN32_64
                                    info.DeclaredType = L"TEXT";
#else
                                    info.DeclaredType = u"TEXT";
#endif
                                } else {
#ifdef WIN32_64
                                    info.DeclaredType = L"LONGTEXT";
#else
                                    info.DeclaredType = u"LONGTEXT";
#endif
                                }
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //text
                        }
                        break;
                    case MYSQL_TYPE_VAR_STRING:
                        info.ColumnSize = f.length;
                        if (f.charsetnr == IS_BINARY) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"VARBINARY";
#else
                                info.DeclaredType = u"VARBINARY";
#endif
                            }

                            info.DataType = (VT_UI1 | VT_ARRAY);
                        } else {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"VARCHAR";
#else
                                info.DeclaredType = u"VARCHAR";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        }
                        break;
                    case MYSQL_TYPE_STRING:
                        info.ColumnSize = f.length;
                        if ((f.flags & ENUM_FLAG) == ENUM_FLAG) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"ENUM";
#else
                                info.DeclaredType = u"ENUM";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        } else if ((f.flags & SET_FLAG) == SET_FLAG) {
                            if (meta) {
#ifdef WIN32_64
                                info.DeclaredType = L"SET";
#else
                                info.DeclaredType = u"SET";
#endif
                            }
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        } else {
                            if (f.charsetnr == IS_BINARY) {
                                if (meta) {
#ifdef WIN32_64
                                    info.DeclaredType = L"BINARY";
#else
                                    info.DeclaredType = u"BINARY";
#endif
                                }
                                info.DataType = (VT_UI1 | VT_ARRAY);
                            } else {
                                if (meta) {
#ifdef WIN32_64
                                    info.DeclaredType = L"CHAR";
#else
                                    info.DeclaredType = u"CHAR";
#endif
                                }
                                info.DataType = (VT_I1 | VT_ARRAY); //string
                            }
                        }
                        info.ColumnSize = f.length;
                        break;
                    case MYSQL_TYPE_GEOMETRY:
                        info.ColumnSize = f.length;
                        if (meta) {
#ifdef WIN32_64
                            info.DeclaredType = L"GEOMETRY";
#else
                            info.DeclaredType = u"GEOMETRY";
#endif
                        }
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary array
                        break;
                    default:
                        assert(false); //not implemented
                        break;
                }
            }
            return vCols;
        }

        bool CMysqlImpl::SendRows(CScopeUQueue& sb, bool transferring) {
            if (m_pNoSending) {
                m_pNoSending->Push(sb->GetBuffer(), sb->GetSize());
                sb->SetSize(0);
                return true;
            }
            bool batching = (GetBytesBatched() >= DEFAULT_RECORD_BATCH_SIZE);
            if (batching) {
                CommitBatching();
            }
            unsigned int ret = SendResult(transferring ? idTransferring : idEndRows, sb->GetBuffer(), sb->GetSize());
            sb->SetSize(0);
            if (batching) {
                StartBatching();
            }
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool CMysqlImpl::SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) {
            assert(!m_pNoSending);
            unsigned int ret = SendResult(idStartBLOB,
            (unsigned int) (bytes + sizeof (unsigned short) + sizeof (unsigned int) + sizeof (unsigned int))/* extra 4 bytes for string null termination*/,
            data_type, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            while (bytes > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                ret = SendResult(idChunk, buffer, DEFAULT_BIG_FIELD_CHUNK_SIZE);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                assert(ret == DEFAULT_BIG_FIELD_CHUNK_SIZE);
                buffer += DEFAULT_BIG_FIELD_CHUNK_SIZE;
                bytes -= DEFAULT_BIG_FIELD_CHUNK_SIZE;
            }
            ret = SendResult(idEndBLOB, buffer, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        void CMysqlImpl::ConvertToUTF8OrDouble(CDBVariant & vt) {
            switch (vt.Type()) {
                case VT_DATE:
                {
                    char str[32] = {0};
                    UDateTime d(vt.ullVal);
                    d.ToDBString(str, sizeof (str));
                    vt = (const char*) str;
                }
                    break;
                case VT_CY:
                {
                    double d = (double) vt.cyVal.int64;
                    d /= 10000.0;
                    vt = d;
                }
                    break;
                case VT_DECIMAL:
                {
                    std::string s;
                    const DECIMAL &decVal = vt.decVal;
                    if (decVal.Hi32)
                        s = SPA::ToString_long(decVal);
                    else
                        s = SPA::ToString(decVal);
                    vt = s.c_str();
                }
                    break;
                default:
                    break;
            }
        }

        UINT64 CMysqlImpl::ConvertBitsToInt(const unsigned char *s, unsigned int bytes) {
            UINT64 n = 0;
            for (unsigned int i = 0; i < bytes; ++i) {
                if (i) {
                    n <<= 8;
                }
                n += s[i];
            }
            return n;
        }

        bool CMysqlImpl::PushRecords(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, CDBString & errMsg) {
            VARTYPE vt;
            CScopeUQueue sb;
            size_t fields = vColInfo.size();
            MYSQL_ROW ppData = m_remMysql.mysql_fetch_row(result);
            while (ppData) {
                bool blob = false;
                unsigned long *lengths = m_remMysql.mysql_fetch_lengths(result);
                for (size_t i = 0; i < fields; ++i) {
                    const CDBColumnInfo &colInfo = vColInfo[i];
                    vt = colInfo.DataType;
                    unsigned int len = (unsigned int) (lengths[i]);
                    char *data = ppData[i];
                    if (data) {
                        if ((vt & VT_ARRAY) != VT_ARRAY) {
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                        }
                        switch (vt) {
                            case (VT_I1 | VT_ARRAY):
                            case (VT_UI1 | VT_ARRAY):
                            {
                                if (colInfo.Precision && colInfo.Precision <= 29) {
                                    DECIMAL dec;
                                    if (len <= 19)
                                        ParseDec(data, dec);
                                    else
                                        ParseDec_long(data, dec);
                                    vt = VT_DECIMAL;
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb << dec;
                                } else if (len > (2 * DEFAULT_BIG_FIELD_CHUNK_SIZE)) {
                                    if (sb->GetSize() && !SendRows(sb, true)) {
                                        return false;
                                    }
                                    bool batching = IsBatching();
                                    if (batching) {
                                        CommitBatching();
                                    }

                                    if (!SendBlob(vt, (const unsigned char *) data, len)) {
                                        return false;
                                    }
                                    if (batching) {
                                        StartBatching();
                                    }
                                    blob = true;
                                } else {
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb->Push((const unsigned char*) &len, sizeof (len));
                                    sb->Push((const unsigned char*) data, len);
                                }
                            }
                                break;
                            case VT_BOOL:
                            {
                                unsigned short c = *data ? (~0) : 0;
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I1:
                            {
                                const char *end;
                                char c = (char) SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI1:
                            {
                                unsigned char c;
                                if (colInfo.ColumnSize) {
                                    //bit
                                    c = (unsigned char) (data[0]);
                                } else {
                                    const char *end;
                                    c = (unsigned char) SPA::atoui(data, end);
                                }
                                sb->Push(&c, sizeof (c));
                            }
                                break;
                            case VT_UI2:
                            {
                                unsigned short c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = (unsigned short) ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = (unsigned short) SPA::atoui(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I2:
                            {
                                const char *end;
                                short c = (short) SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI4:
                            {
                                unsigned int c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = (unsigned int) ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = SPA::atoui(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I4:
                            {
                                const char *end;
                                int c = SPA::atoi(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_UI8:
                            {
                                UINT64 c;
                                if (colInfo.ColumnSize)
                                    //bit
                                    c = ConvertBitsToInt((unsigned char*) data, len);
                                else {
                                    const char *end;
                                    c = SPA::atoull(data, end);
                                }
                                sb->Push((const unsigned char*) &c, sizeof (UINT64));
                            }
                                break;
                            case VT_I8:
                            {
                                const char *end;
                                INT64 c = SPA::atoll(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (INT64));
                            }
                                break;
                            case VT_R4:
                            {
                                const char *end;
                                float c = (float) SPA::atof(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_R8:
                            {
                                const char *end;
                                double c = SPA::atof(data, end);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_DATE:
                            {
                                UDateTime udt(data);
                                sb << udt.time;
                            }
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    } else {
                        vt = VT_NULL;
                        sb->Push((const unsigned char*) &vt, sizeof (vt));
                    }
                }
                if ((sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(sb)) {
                    return false;
                }
                ppData = m_remMysql.mysql_fetch_row(result);
            }
            if (sb->GetSize()) {
                return SendRows(sb);
            }
            return true;
        }

        void CMysqlImpl::ExecuteSqlWithRowset(bool rowset, bool meta, UINT64 index, int &res, CDBString &errMsg, INT64 & affected) {
            do {
                MYSQL_RES *result = m_remMysql.mysql_use_result(m_pMysql.get());
                if (result) {
                    unsigned int cols = m_remMysql.mysql_num_fields(result);
                    CDBColumnInfoArray vInfo = GetColInfo(result, cols, meta);
                    if (!m_pNoSending) {
                        unsigned int ret = SendResult(idRowsetHeader, vInfo, index);
                        if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                            m_remMysql.mysql_free_result(result);
                            return;
                        }
                    }
                    bool ok;
                    if (rowset) {
                        ok = PushRecords(result, vInfo, res, errMsg);
                    } else {
                        ok = true;
                    }
                    m_remMysql.mysql_free_result(result);
                    ++m_oks;

                    //For SELECT statements, mysql_affected_rows() works like mysql_num_rows().
                    //affected += (INT64)m_remMysql.mysql_affected_rows(m_pMysql.get());

                    if (!ok) {
                        return;
                    }
                } else {
                    if (!m_pNoSending) {
                        CDBColumnInfoArray vInfo;
                        unsigned int ret = SendResult(idRowsetHeader, vInfo, index);
                        if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                            return;
                        }
                    }
                    int errCode = m_remMysql.mysql_errno(m_pMysql.get());
                    if (!errCode) {
                        ++m_oks;
                        affected += (INT64) m_remMysql.mysql_affected_rows(m_pMysql.get());
                    } else {
                        ++m_fails;
                        if (!res) {
                            res = errCode;
                            errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                        }
                    }
                }
                int status = m_remMysql.mysql_next_result(m_pMysql.get());
                if (status == -1) {
                    break; //Successful and there are no more results
                } else if (status > 0) {
                    ++m_fails;
                    if (!res) {
                        res = m_remMysql.mysql_errno(m_pMysql.get());
                        errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                    }
                    break;
                } else if (status == 0) {
                    //Successful and there are more results
                } else {
                    assert(false); //never come here
                }
            } while (true);
        }

        void CMysqlImpl::Execute(const CDBString& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            ResetMemories();
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
            }
            vtId = (INT64) 0;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            std::string sql = Utilities::ToUTF8(wsql);
            Utilities::Trim(sql);
#ifdef MM_DB_SERVER_PLUGIN
            if (m_EnableMessages && !sql.size()) {
                //client side is asking for data from cached tables
                for (auto it = CSetGlobals::Globals.cached_tables.begin(), end = CSetGlobals::Globals.cached_tables.end(); it != end; ++it) {
                    if (sql.size())
                        sql += ";";
                    std::string s = *it;
                    auto pos = s.find('.');
                    sql += "select * from `";
                    sql += s.substr(0, pos);
                    sql += "`.`";
                    sql += s.substr(pos + 1);
                    sql += "`";
                }
            }
#endif
            const char *sqlUtf8 = (const char*) sql.c_str();
            int status = m_remMysql.mysql_real_query(m_pMysql.get(), sqlUtf8, (unsigned long) sql.size());
            if (status) {
                res = m_remMysql.mysql_errno(m_pMysql.get());
                errMsg = Utilities::ToUTF16(m_remMysql.mysql_error(m_pMysql.get()));
                ++m_fails;
            } else {
                if (rowset || meta) {
                    ExecuteSqlWithRowset(rowset, meta, index, res, errMsg, affected);
                } else {
                    ExecuteSqlWithoutRowset(res, errMsg, affected);
                }
                if (lastInsertId) {
                    if (affected) {
                        vtId = (INT64) m_remMysql.mysql_insert_id(m_pMysql.get());
                    }
                }
            }
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CMysqlImpl::PreprocessPreparedStatement() {
            std::string s = m_sqlPrepare;
            transform(s.begin(), s.end(), s.begin(), ::tolower);
            m_bCall = (s.find("call ") == 0);
            if (m_bCall) {
                auto pos = m_sqlPrepare.find('(');
                if (pos != std::string::npos) {
                    m_procName.assign(m_sqlPrepare.begin() + 5, m_sqlPrepare.begin() + pos);
                } else {
                    m_procName = m_sqlPrepare.substr(5);
                }
                auto dot = m_procName.rfind('.');
                if (dot != std::string::npos) {
                    m_procName = m_procName.substr(dot + 1);
                }
                if (m_procName.back() == '`') {
                    m_procName.pop_back();
                    m_procName.erase(m_procName.begin());
                }
                Utilities::Trim(m_procName);
            } else {
                m_procName.clear();
            }
        }

        void CMysqlImpl::Prepare(const CDBString& wsql, CParameterInfoArray& params, int &res, CDBString &errMsg, unsigned int &parameters) {
            ResetMemories();
            parameters = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            m_pPrepare.reset();
            m_vParam.clear();
            m_parameters = 0;
            m_sqlPrepare = Utilities::ToUTF8(wsql);
            Utilities::Trim(m_sqlPrepare);
            MYSQL_STMT *stmt = m_remMysql.mysql_stmt_init(m_pMysql.get());
            PreprocessPreparedStatement();
            my_bool fail = m_remMysql.mysql_stmt_prepare(stmt, m_sqlPrepare.c_str(), (unsigned long) m_sqlPrepare.size());
            if (fail) {
                res = m_remMysql.mysql_stmt_errno(stmt);
                errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(stmt));
                m_remMysql.mysql_stmt_close(stmt);
            } else {
                res = 0;
                m_parameters = m_remMysql.mysql_stmt_param_count(stmt);
                m_pPrepare.reset(stmt, [](MYSQL_STMT * stmt) {
                    if (stmt) {
                        m_remMysql.mysql_stmt_close(stmt);
                    }
                });
                parameters = (unsigned int) m_parameters;
            }
        }

        int CMysqlImpl::Bind(CUQueue &qBufferSize, int row, CDBString & errMsg) {
            int res = 0;
            if (!m_parameters) {
                return res;
            }
            unsigned int size = sizeof (MYSQL_BIND);
            size *= (unsigned int) m_parameters;
            SPA::CScopeUQueue sb;
            if (size > sb->GetMaxSize()) {
                sb->ReallocBuffer(size);
            }
            sb->CleanTrack();
            qBufferSize.SetSize(0);
            if ((m_parameters + 1) * sizeof (unsigned long) > qBufferSize.GetMaxSize()) {
                qBufferSize.ReallocBuffer((unsigned int) ((m_parameters + 1) * sizeof (unsigned long)));
            }
            unsigned int indexBS = 0;
            MYSQL_BIND *pBind = (MYSQL_BIND*) sb->GetBuffer();
            for (size_t n = 0; n < m_parameters; ++n) {
                CDBVariant &data = m_vParam[row * m_parameters + n];
                unsigned short vt = data.Type();
                MYSQL_BIND &bind = pBind[n];
                switch (vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        bind.buffer_type = MYSQL_TYPE_NULL;
                        bind.is_null = (my_bool*) & CMysqlImpl::B_IS_NULL;
                        break;
                    case VT_I1:
                        bind.buffer_type = MYSQL_TYPE_TINY;
                        bind.buffer = (void*) &data.cVal;
                        bind.buffer_length = sizeof (char);
                        break;
                    case VT_UI1:
                        bind.buffer_type = MYSQL_TYPE_TINY;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.bVal;
                        bind.buffer_length = sizeof (unsigned char);
                        break;
                    case VT_UI2:
                        bind.buffer_type = MYSQL_TYPE_SHORT;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.uiVal;
                        bind.buffer_length = sizeof (unsigned short);
                        break;
                    case VT_I2:
                        bind.buffer_type = MYSQL_TYPE_SHORT;
                        bind.buffer = (void*) &data.iVal;
                        bind.buffer_length = sizeof (short);
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        bind.buffer_type = MYSQL_TYPE_LONG;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.ulVal;
                        bind.buffer_length = sizeof (unsigned int);
                        break;
                    case VT_INT:
                    case VT_I4:
                        bind.buffer_type = MYSQL_TYPE_LONG;
                        bind.buffer = (void*) &data.lVal;
                        bind.buffer_length = sizeof (int);
                        break;
                    case VT_UI8:
                        bind.buffer_type = MYSQL_TYPE_LONGLONG;
                        bind.is_unsigned = true;
                        bind.buffer = (void*) &data.ullVal;
                        bind.buffer_length = sizeof (UINT64);
                        break;
                    case VT_I8:
                        bind.buffer_type = MYSQL_TYPE_LONGLONG;
                        bind.buffer = (void*) &data.llVal;
                        bind.buffer_length = sizeof (INT64);
                        break;
                    case VT_BOOL:
                        bind.buffer_type = MYSQL_TYPE_BIT;
                        bind.buffer_length = sizeof (bool);
                        data.cVal = data.boolVal ? 1 : 0;
                        bind.buffer = (void*) &data.cVal;
                        break;
                    case VT_R4:
                        bind.buffer_type = MYSQL_TYPE_FLOAT;
                        bind.buffer = (void*) &data.fltVal;
                        bind.buffer_length = sizeof (float);
                        break;
                    case VT_R8:
                        bind.buffer_type = MYSQL_TYPE_DOUBLE;
                        bind.buffer = (void*) &data.dblVal;
                        bind.buffer_length = sizeof (double);
                        break;

                    case VT_STR:
                    case (VT_ARRAY | VT_I1):
                        bind.buffer_type = MYSQL_TYPE_STRING;
                        bind.buffer_length = data.parray->rgsabound->cElements;
                        ::SafeArrayAccessData(data.parray, &bind.buffer);
                        ::SafeArrayUnaccessData(data.parray);
                        qBufferSize << bind.buffer_length;
                        bind.length = (unsigned long*) qBufferSize.GetBuffer(indexBS * sizeof (unsigned long));
                        ++indexBS;
                        break;
                    case VT_BYTES:
                    case (VT_ARRAY | VT_UI1):
                        bind.buffer_type = MYSQL_TYPE_BLOB;
                        bind.buffer_length = data.parray->rgsabound->cElements;
                        ::SafeArrayAccessData(data.parray, &bind.buffer);
                        ::SafeArrayUnaccessData(data.parray);
                        qBufferSize << bind.buffer_length;
                        bind.length = (unsigned long*) qBufferSize.GetBuffer(indexBS * sizeof (unsigned long));
                        ++indexBS;
                        break;
                    default:
                        assert(false); //not implemented
                        if (!res) {
                            res = SPA::Mysql::ER_DATA_TYPE_NOT_SUPPORTED;
                            errMsg = DATA_TYPE_NOT_SUPPORTED;
                        }
                        break;
                }
            }
            if (!res && m_remMysql.mysql_stmt_bind_param(m_pPrepare.get(), pBind)) {
                res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                if (!errMsg.size()) {
                    errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                }
            }
            return res;
        }

        std::shared_ptr<MYSQL_BIND> CMysqlImpl::PrepareBindResultBuffer(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, CDBString &errMsg, std::shared_ptr<MYSQL_BIND_RESULT_FIELD> &field) {
            std::shared_ptr<MYSQL_BIND> p(new MYSQL_BIND[vColInfo.size()], [](MYSQL_BIND * b) {
                if (b) {
                    delete []b;
                }
            });
            field.reset(new MYSQL_BIND_RESULT_FIELD[vColInfo.size()], [](MYSQL_BIND_RESULT_FIELD * f) {
                if (f) {
                    delete []f;
                }
            });
            MYSQL_BIND *ps_params = p.get();
            MYSQL_BIND_RESULT_FIELD *ps_fields = field.get();
            size_t index = 0;
            for (auto it = vColInfo.begin(), end = vColInfo.end(); it != end; ++it, ++index) {
                MYSQL_BIND &b = ps_params[index];
                ::memset(&b, 0, sizeof (b));
                MYSQL_BIND_RESULT_FIELD &f = ps_fields[index];
                VARTYPE vt = it->DataType;
                switch (vt) {
                    case VT_ARRAY | VT_I1:
                    case VT_ARRAY | VT_UI1:
                        if (it->ColumnSize > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                            if (f.buffer_length < 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                f.ResetMaxBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
                            }
                        } else {
                            if (it->ColumnSize > f.buffer_length) {
                                f.ResetMaxBuffer((unsigned int) it->ColumnSize);
                            }
                        }
                        if (vt == (VT_ARRAY | VT_I1)) {
                            b.buffer_type = MYSQL_TYPE_STRING;
                        } else {
                            b.buffer_type = MYSQL_TYPE_BLOB;
                        }
                        break;
                    case VT_UI1:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I1:
                        b.buffer_type = MYSQL_TYPE_TINY;
                        break;
                    case VT_UI2:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I2:
                        b.buffer_type = MYSQL_TYPE_SHORT;
                        break;
                    case VT_BOOL:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_INT:
                    case VT_I4:
                        b.buffer_type = MYSQL_TYPE_LONG;
                        break;
                    case VT_UI8:
                        b.buffer_type = MYSQL_TYPE_BIT;
                        break;
                    case VT_I8:
                        b.buffer_type = MYSQL_TYPE_LONGLONG;
                        break;
                    case VT_R4:
                        b.buffer_type = MYSQL_TYPE_FLOAT;
                        break;
                    case VT_R8:
                        b.buffer_type = MYSQL_TYPE_DOUBLE;
                        break;
                    case VT_DATE:
                        b.buffer_type = MYSQL_TYPE_TIMESTAMP;
                        break;
                    default:
                        assert(false);
                        break;
                }
                b.buffer = f.GetBuffer();
                b.buffer_length = f.buffer_length;
                b.length = &f.length;
                b.is_null = &f.is_null;
            }
            my_bool fail = m_remMysql.mysql_stmt_bind_result(m_pPrepare.get(), ps_params);
            if (fail) {
                if (!res) {
                    res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                    errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                }
                p.reset();
                field.reset();
            }
            return p;
        }

        bool CMysqlImpl::PushRecords(UINT64 index, MYSQL_BIND *binds, MYSQL_BIND_RESULT_FIELD *fields, const CDBColumnInfoArray &vColInfo, bool rowset, bool output, int &res, CDBString & errMsg) {
            unsigned int sent;
            size_t cols = vColInfo.size();
            if (output) {
                sent = SendResult(idBeginRows, index);
                if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                    return false;
                }
            }
            CScopeUQueue sb;
            //successful binding
            int ret = m_remMysql.mysql_stmt_fetch(m_pPrepare.get());
            while (ret != MYSQL_NO_DATA && ret != 1) {
                if (output || rowset) {
                    bool blob = false;
                    VARTYPE vt;
                    for (size_t i = 0; i < cols; ++i) {
                        MYSQL_BIND_RESULT_FIELD &f = fields[i];
                        if (f.is_null) {
                            vt = VT_NULL;
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                            continue;
                        }
                        const CDBColumnInfo &colInfo = vColInfo[i];
                        MYSQL_BIND &b = binds[i];
                        vt = colInfo.DataType;
                        if ((vt & VT_ARRAY) != VT_ARRAY) {
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                        }
                        switch (vt) {
                            case (VT_I1 | VT_ARRAY):
                            case (VT_UI1 | VT_ARRAY):
                            {
                                unsigned int len = (unsigned int) *b.length;
                                if (len >= b.buffer_length) {
                                    if (sb->GetSize() && !SendRows(sb, true)) {
                                        return false;
                                    }
                                    sent = SendResult(idStartBLOB,
                                            (unsigned int) (len + sizeof (unsigned short) + sizeof (unsigned int) + sizeof (unsigned int))/* extra 4 bytes for string null termination*/,
                                            vt, len);
                                    if (ret == (int) REQUEST_CANCELED || ret == (int) SOCKET_NOT_FOUND) {
                                        return false;
                                    }
                                    bool batching = IsBatching();
                                    if (batching) {
                                        CommitBatching();
                                    }
                                    unsigned long offset = 0;
                                    unsigned int remain = len;
                                    unsigned int obtain = b.buffer_length;
                                    while (remain) {
                                        ret = SendResult((obtain >= b.buffer_length) ? idChunk : idEndBLOB, (const unsigned char*) b.buffer, obtain);
                                        if (ret == (int) REQUEST_CANCELED || ret == (int) SOCKET_NOT_FOUND) {
                                            return false;
                                        }
                                        remain -= obtain;
                                        offset += obtain;
                                        if (!remain) {
                                            break;
                                        }
                                        ret = m_remMysql.mysql_stmt_fetch_column(m_pPrepare.get(), &b, (unsigned int) i, offset);
                                        if (ret) {
                                            if (!res) {
                                                //res == CR_NO_DATA or 2051, a libmariadb bug
                                                res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                                                errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                                            }
                                            return true;
                                        }
                                        if (remain < b.buffer_length) {
                                            obtain = remain;
                                        }
                                    }
                                    if (batching) {
                                        StartBatching();
                                    }
                                } else if (colInfo.Precision && colInfo.Precision <= 29) {
                                    DECIMAL dec;
                                    if (len <= 19)
                                        ParseDec((const char*) b.buffer, dec);
                                    else
                                        ParseDec_long((const char*) b.buffer, dec);
                                    vt = VT_DECIMAL;
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb << dec;
                                } else {
                                    sb->Push((const unsigned char*) &vt, sizeof (vt));
                                    sb->Push((const unsigned char*) &len, sizeof (len));
                                    sb->Push((const unsigned char*) b.buffer, len);
                                }
                            }
                                break;
                            case VT_I1:
                                assert(*b.length == sizeof (char));
                                sb->Push((const unsigned char*) b.buffer, sizeof (char));
                                break;
                            case VT_UI1:
                            {
                                sb->Push((const unsigned char*) b.buffer, sizeof (unsigned char));
                            }
                                break;
                            case VT_I2:
                                assert(*b.length == sizeof (short));
                                sb->Push((const unsigned char*) b.buffer, sizeof (short));
                                break;
                            case VT_UI2:
                            {
                                unsigned int len = *b.length;
                                unsigned short c = (unsigned short) ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_BOOL:
                                assert(*b.length == sizeof (char));
                            {
                                unsigned short s = *((char*) b.buffer) ? (~0) : 0;
                                sb->Push((const unsigned char*) &s, sizeof (unsigned short));
                            }
                                break;
                            case VT_UI4:
                            case VT_UINT:
                            {
                                unsigned int len = *b.length;
                                unsigned int c = (unsigned int) ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_I4:
                            case VT_INT:
                            case VT_R4:
                                assert(*b.length == sizeof (int));
                                sb->Push((const unsigned char*) b.buffer, sizeof (int));
                                break;
                            case VT_UI8:
                            {
                                unsigned int len = *b.length;
                                UINT64 c = ConvertBitsToInt((unsigned char*) b.buffer, len);
                                sb->Push((const unsigned char*) &c, sizeof (c));
                            }
                                break;
                            case VT_R8:
                            case VT_I8:
                                assert(*b.length == sizeof (INT64));
                                sb->Push((const unsigned char*) b.buffer, sizeof (INT64));
                                break;
                            case VT_DATE:
                            {
                                const MYSQL_TIME *date = (const MYSQL_TIME*) b.buffer;
                                sb << ToUDateTime(*date);
                            }
                                break;
                            default:
                                assert(false);
                                break;
                        }
                    }
                    if (rowset) {
                        if ((sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(sb)) {
                            return false;
                        }
                    }
                }
                ret = m_remMysql.mysql_stmt_fetch(m_pPrepare.get());
            }
            assert(ret != MYSQL_DATA_TRUNCATED);
            if (ret == 1 && !res) {
                res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
            }
            if (output) {
                //tell output parameter data
                sent = SendResult(idOutputParameter, sb->GetBuffer(), sb->GetSize());
                if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                    return false;
                }
            } else if (rowset) {
                if (sb->GetSize()) {
                    return SendRows(sb);
                }
            }
            return true;
        }

        UINT64 CMysqlImpl::ToUDateTime(const MYSQL_TIME & td) {
            std::tm date;
            if (td.time_type == MYSQL_TIMESTAMP_TIME) {
                date.tm_year = 0;
                date.tm_mon = 0;
                date.tm_mday = 0;
            } else {
                date.tm_year = td.year - 1900;
                date.tm_mon = td.month - 1;
                date.tm_mday = td.day;
            }
            date.tm_hour = td.hour;
            date.tm_min = td.minute;
            date.tm_sec = td.second;
            return SPA::UDateTime(date, td.second_part).time;
        }

        size_t CMysqlImpl::ComputeParameters(const CDBString & sql) {
            const UTF16 quote = '\'', slash = '\\', question = '?';
            bool b_slash = false, balanced = true;
            size_t params = 0, len = sql.size();
            for (size_t n = 0; n < len; ++n) {
                const UTF16 &c = sql[n];
                if (c == slash) {
                    b_slash = true;
                    continue;
                }
                if (c == quote && b_slash) {
                    b_slash = false;
                    continue; //ignore a quote if there is a slash ahead
                }
                b_slash = false;
                if (c == quote) {
                    balanced = (!balanced);
                    continue;
                }
                if (balanced) {
                    params += ((c == question) ? 1 : 0);
                }
            }
            return params;
        }

        void CMysqlImpl::SetVParam(CDBVariantArray& vAll, size_t parameters, size_t pos, size_t ps) {
            m_vParam.clear();
            size_t rows = vAll.size() / parameters;
            for (size_t r = 0; r < rows; ++r) {
                for (size_t p = 0; p < ps; ++p) {
                    CDBVariant &vt = vAll[parameters * r + pos + p];
                    m_vParam.push_back(std::move(vt));
                }
            }
        }

        std::vector<CDBString> CMysqlImpl::Split(const CDBString &sql, const CDBString & delimiter) {
            std::vector<CDBString> v;
            size_t d_len = delimiter.size();
            if (d_len) {
                const UTF16 quote = '\'', slash = '\\', done = delimiter[0];
                size_t params = 0, len = sql.size();
                bool b_slash = false, balanced = true;
                for (size_t n = 0; n < len; ++n) {
                    const wchar_t &c = sql[n];
                    if (c == slash) {
                        b_slash = true;
                        continue;
                    }
                    if (c == quote && b_slash) {
                        b_slash = false;
                        continue; //ignore a quote if there is a slash ahead
                    }
                    b_slash = false;
                    if (c == quote) {
                        balanced = (!balanced);
                        continue;
                    }
                    if (balanced && c == done) {
                        size_t pos = sql.find(delimiter, n);
                        if (pos == n) {
                            v.push_back(sql.substr(params, n - params));
                            n += d_len;
                            params = n;
                        }
                    }
                }
                v.push_back(sql.substr(params));
            } else {
                v.push_back(sql);
            }
            return v;
        }

        void CMysqlImpl::ExecuteBatch(const CDBString& sql, const CDBString& delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, const CDBString &dbConn, unsigned int flags, UINT64 callIndex, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            CParameterInfoArray vPInfo;
            m_UQueue >> vPInfo;
            vPInfo.clear();
            res = 0;
            fail_ok = 0;
            affected = 0;
            int ms = 0;
            if (!m_pMysql) {
                Open(dbConn, flags, res, errMsg, ms);
            }
            size_t parameters = 0;
            std::vector<CDBString> vSql = Split(sql, delimiter);
            for (auto it = vSql.cbegin(), end = vSql.cend(); it != end; ++it) {
                parameters += ComputeParameters(*it);
            }
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                fail_ok = vSql.size();
                fail_ok <<= 32;
                SendResult(idSqlBatchHeader, res, errMsg, (int) msMysql, (unsigned int) parameters, callIndex);
                return;
            }
            size_t rows = 0;
            if (parameters) {
                if (!m_vParam.size()) {
                    res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                    errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msMysql, (unsigned int) parameters, callIndex);
                    return;
                }
                if ((m_vParam.size() % (unsigned short) parameters)) {
                    res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                    errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msMysql, (unsigned int) parameters, callIndex);
                    return;
                }
                rows = m_vParam.size() / parameters;
            }
            if (isolation != (int) tiUnspecified) {
                int ms;
                BeginTrans(isolation, dbConn, flags, res, errMsg, ms);
                if (res) {
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msMysql, (unsigned int) parameters, callIndex);
                    return;
                }
            } else {
                errMsg = dbConn;
            }
            SendResult(idSqlBatchHeader, res, errMsg, (int) msMysql, (unsigned int) parameters, callIndex);
            errMsg.clear();
            CDBVariantArray vAll;
            m_vParam.swap(vAll);
            INT64 aff = 0;
            int r = 0;
            CDBString err;
            UINT64 fo = 0;
            size_t pos = 0;
            vtId = aff;
            CDBVariant last_id;
            for (auto it = vSql.begin(), end = vSql.end(); it != end; ++it) {
                Utilities::Trim(*it);
                if (!it->size()) {
                    continue;
                }
                size_t ps = ComputeParameters(*it);
                if (ps) {
                    //prepared statements
                    unsigned int my_ps = 0;
                    Prepare(*it, vPInfo, r, err, my_ps);
                    if (r) {
                        fail_ok += (((UINT64) rows) << 32);
                    } else {
                        assert(ps == (my_ps & 0xffff));
                        SetVParam(vAll, parameters, pos, ps);
                        unsigned int nParamPos = (unsigned int) ((pos << 16) + ps);
                        SendResult(idParameterPosition, nParamPos);
                        last_id = (INT64) 0;
                        ExecuteParameters(rowset, meta, lastInsertId, callIndex, aff, r, err, last_id, fo);
                        m_vParam.clear();
                        if (last_id.llVal != 0)
                            vtId = last_id;
                    }
                    pos += ps;
                } else {
                    last_id = (INT64) 0;
                    Execute(*it, rowset, meta, lastInsertId, callIndex, aff, r, err, last_id, fo);
                    if (last_id.llVal != 0)
                        vtId = last_id;
                }
                if (r && !res) {
                    res = r;
                    errMsg = err;
                }
                if (r && isolation != (int) tiUnspecified && plan == (int) rpDefault)
                    break;
                affected += aff;
                fail_ok += fo;
            }
            if (isolation != (int) tiUnspecified) {
                EndTrans(plan, r, err);
                if (r && !res) {
                    res = r;
                    errMsg = err;
                }
            }
        }

        void CMysqlImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            assert(!m_pNoSending);
            fail_ok = 0;
            affected = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            if (!m_pPrepare) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            if (!m_parameters) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else if ((m_vParam.size() % m_parameters) || (m_vParam.size() == 0)) {
                res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }

            res = 0;
            CScopeUQueue sb;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            bool header_sent = false;
            int rows = (int) (m_vParam.size() / m_parameters);
            for (int row = 0; row < rows; ++row) {
                int ret = Bind(*sb, row, errMsg);
                if (ret) {
                    if (!res) {
                        res = ret;
                    }
                    ++m_fails;
                    continue;
                }
                ret = m_remMysql.mysql_stmt_execute(m_pPrepare.get());
                if (ret) {
                    if (!res) {
                        res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                        errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                    }
                    ++m_fails;
                    continue;
                }

                //For SELECT statements, mysql_stmt_affected_rows() works like mysql_num_rows().
                my_ulonglong affected_rows = m_remMysql.mysql_stmt_affected_rows(m_pPrepare.get());
                if (affected_rows != (my_ulonglong) (~0) && affected_rows) {
                    affected += affected_rows;
                }

                unsigned int cols = m_remMysql.mysql_stmt_field_count(m_pPrepare.get());
                bool output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
                MYSQL_RES *result = m_remMysql.mysql_stmt_result_metadata(m_pPrepare.get());
#ifndef NDEBUG
                if (cols) {
                    assert(result);
                }
#endif
                while (result && cols) {
                    CDBColumnInfoArray vInfo = GetColInfo(result, cols, (meta || m_bCall));

                    if (!output) {
                        //Mysql + Mariadb server_status & SERVER_PS_OUT_PARAMS does NOT work correctly for an unknown reason
                        //This is a hack solution for detecting output result, which may be wrong if a table name is EXACTLY the same as stored procedure name
                        output = (m_bCall && (vInfo[0].TablePath == Utilities::ToUTF16(m_procName)));
                    }

                    //we push stored procedure output parameter meta data onto client to follow common approach for output parameter data
                    if (output || rowset || meta) {
                        unsigned int outputs = 0;
                        if (output) {
                            outputs = (unsigned int) vInfo.size();
                        }
                        unsigned int sent = SendResult(idRowsetHeader, vInfo, index, outputs);
                        header_sent = true;
                        if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                            m_remMysql.mysql_stmt_free_result(m_pPrepare.get());
                            return;
                        }
                    }
                    std::shared_ptr<MYSQL_BIND_RESULT_FIELD> fields;
                    std::shared_ptr<MYSQL_BIND> pBinds = PrepareBindResultBuffer(result, vInfo, res, errMsg, fields);
                    MYSQL_BIND *mybind = pBinds.get();
                    MYSQL_BIND_RESULT_FIELD *myfield = fields.get();
                    if (pBinds && (output || rowset)) {
                        int my_res = 0;
                        CDBString err;
                        if (!PushRecords(index, mybind, myfield, vInfo, rowset, output, my_res, err)) {
                            ret = m_remMysql.mysql_stmt_free_result(m_pPrepare.get());
                            return;
                        }
                        if (my_res) {
                            ret = m_remMysql.mysql_stmt_free_result(m_pPrepare.get());
                            if (!res) {
                                res = my_res;
                                errMsg = err;
                            }
                            ret = 1;
                            break;
                        }
                    }
                    m_remMysql.mysql_stmt_free_result(m_pPrepare.get());
                    pBinds.reset();
                    fields.reset();
                    ret = m_remMysql.mysql_stmt_next_result(m_pPrepare.get());
                    if (ret == 0) {
                        //continue for the next set
                    } else if (ret == -1) {
                        //no more result
                        ret = 0;
                        break;
                    } else if (ret > 0) {
                        //error
                        if (!res) {
                            res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                            errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                        }
                        break;
                    } else {
                        //should never come here
                        assert(false);
                    }
                    cols = m_remMysql.mysql_stmt_field_count(m_pPrepare.get());
                    output = (m_pMysql.get()->server_status & SERVER_PS_OUT_PARAMS) ? true : false;
                    result = m_remMysql.mysql_stmt_result_metadata(m_pPrepare.get());
                }
                int ret2 = m_remMysql.mysql_stmt_free_result(m_pPrepare.get());
                if (ret2) {
                    ret = 1;
                    if (!res) {
                        res = m_remMysql.mysql_stmt_errno(m_pPrepare.get());
                        errMsg = Utilities::ToUTF16(m_remMysql.mysql_stmt_error(m_pPrepare.get()));
                    }
                }
                if (ret)
                    ++m_fails;
                else
                    ++m_oks;
                my_bool myfail = m_remMysql.mysql_stmt_reset(m_pPrepare.get());
                assert(!myfail);
            }
            if (!header_sent && (rowset || meta)) {
                CDBColumnInfoArray vInfo;
                SendResult(idRowsetHeader, vInfo, index);
            }
            if (lastInsertId) {
                vtId = (INT64) m_remMysql.mysql_stmt_insert_id(m_pPrepare.get());
            }
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CMysqlImpl::StartBLOB(unsigned int lenExpected) {
            m_Blob.SetSize(0);
            if (lenExpected > m_Blob.GetMaxSize()) {
                m_Blob.ReallocBuffer(lenExpected);
            }
            CUQueue &q = m_UQueue;
            m_Blob.Push(q.GetBuffer(), q.GetSize());
            assert(q.GetSize() > sizeof (unsigned short) + sizeof (unsigned int));
            q.SetSize(0);
        }

        void CMysqlImpl::Chunk() {
            CUQueue &q = m_UQueue;
            if (q.GetSize()) {
                m_Blob.Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
            }
        }

        void CMysqlImpl::EndBLOB() {
            Chunk();
            m_vParam.push_back(CDBVariant());
            CDBVariant &vt = m_vParam.back();
            m_Blob >> vt;
            assert(m_Blob.GetSize() == 0);
        }

        void CMysqlImpl::BeginRows() {
            Transferring();
        }

        void CMysqlImpl::EndRows() {
            Transferring();
        }

        void CMysqlImpl::Transferring() {
            CUQueue &q = m_UQueue;
            while (q.GetSize()) {
                m_vParam.push_back(CDBVariant());
                CDBVariant &vt = m_vParam.back();
                q >> vt;
                ConvertToUTF8OrDouble(vt);
            }
            assert(q.GetSize() == 0);
        }

        bool CMysqlImpl::DoSQLAuthentication(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t * dbConnection) {
            CMysqlImpl impl;
#ifdef WIN32_64
            CDBString db(dbConnection ? dbConnection : L"host=localhost;port=3306;timeout=30");
            if (userId && ::wcslen(userId)) {
                db += L";uid=";
                db += userId;
            }
            if (password && ::wcslen(password)) {
                db += L";pwd=";
                db += password;
            }
#else
            CDBString db(dbConnection ? Utilities::ToUTF16(dbConnection) : u"host=localhost;port=3306;timeout=30");
            if (userId && ::wcslen(userId)) {
                db += u";uid=";
                db += Utilities::ToUTF16(userId);
            }
            if (password && ::wcslen(password)) {
                db += u";pwd=";
                db += Utilities::ToUTF16(password);
            }
#endif
            int res = 0, ms = 0;
            CDBString errMsg;
            impl.Open(db, 0, res, errMsg, ms);
            if (res) {
                return false;
            }
            if (nSvsId == SPA::Mysql::sidMysql) {
                MyStruct ms;
                ms.Handle = impl.m_pMysql;
                ms.DefaultDB = impl.m_dbNameOpened;
                CAutoLock al(m_csPeer);
                m_mapConnection[hSocket] = ms;
            } else {
                impl.m_pMysql.reset();
            }
            return true;
        }

    } //namespace ServerSide
} //namespace SPA