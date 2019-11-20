
#include "odbcimpl.h"
#include <sstream>
#ifndef NDEBUG
#include <iostream>
#endif
#include "../async_sqlncli.h"
#include "../../../include/scloader.h"
#include <cctype>

namespace SPA
{
    namespace ServerSide{

        SQLHENV COdbcImpl::g_hEnv = nullptr;

#ifdef WIN32_64
        const UTF16 * COdbcImpl::NO_DB_OPENED_YET = L"No ODBC database opened yet";
        const UTF16 * COdbcImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const UTF16 * COdbcImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const UTF16 * COdbcImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const UTF16 * COdbcImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const UTF16 * COdbcImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const UTF16 * COdbcImpl::NO_DB_NAME_SPECIFIED = L"No database name specified";
        const UTF16 * COdbcImpl::ODBC_ENVIRONMENT_NOT_INITIALIZED = L"ODBC system library not initialized";
        const UTF16 * COdbcImpl::BAD_MANUAL_TRANSACTION_STATE = L"Bad manual transaction state";
        const UTF16 * COdbcImpl::BAD_INPUT_PARAMETER_DATA_TYPE = L"Bad input parameter data type";
        const UTF16 * COdbcImpl::BAD_PARAMETER_DIRECTION_TYPE = L"Bad parameter direction type";
        const UTF16 * COdbcImpl::CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = L"Correct parameter information not provided yet";
        const UTF16 * COdbcImpl::ODBC_GLOBAL_CONNECTION_STRING = L"ODBC_GLOBAL_CONNECTION_STRING";
#else
        const UTF16 * COdbcImpl::NO_DB_OPENED_YET = u"No ODBC database opened yet";
        const UTF16 * COdbcImpl::BAD_END_TRANSTACTION_PLAN = u"Bad end transaction plan";
        const UTF16 * COdbcImpl::NO_PARAMETER_SPECIFIED = u"No parameter specified";
        const UTF16 * COdbcImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = u"Bad parameter data array length";
        const UTF16 * COdbcImpl::BAD_PARAMETER_COLUMN_SIZE = u"Bad parameter column size";
        const UTF16 * COdbcImpl::DATA_TYPE_NOT_SUPPORTED = u"Data type not supported";
        const UTF16 * COdbcImpl::NO_DB_NAME_SPECIFIED = u"No database name specified";
        const UTF16 * COdbcImpl::ODBC_ENVIRONMENT_NOT_INITIALIZED = u"ODBC system library not initialized";
        const UTF16 * COdbcImpl::BAD_MANUAL_TRANSACTION_STATE = u"Bad manual transaction state";
        const UTF16 * COdbcImpl::BAD_INPUT_PARAMETER_DATA_TYPE = u"Bad input parameter data type";
        const UTF16 * COdbcImpl::BAD_PARAMETER_DIRECTION_TYPE = u"Bad parameter direction type";
        const UTF16 * COdbcImpl::CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET = u"Correct parameter information not provided yet";
        const UTF16 * COdbcImpl::ODBC_GLOBAL_CONNECTION_STRING = u"ODBC_GLOBAL_CONNECTION_STRING";
#endif
        CUCriticalSection COdbcImpl::m_csPeer;
        CDBString COdbcImpl::m_strGlobalConnection;

        std::unordered_map<USocket_Server_Handle, SQLHDBC> COdbcImpl::m_mapConnection;

        bool COdbcImpl::DoSQLAuthentication(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *odbcDriver, const wchar_t * dsn) {
            SQLHDBC hdbc = nullptr;
            if (!g_hEnv)
                return false;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_DBC, g_hEnv, &hdbc);
            if (!SQL_SUCCEEDED(retcode)) {
                return false;
            }
            std::wstring conn;
            if (dsn && ::wcslen(dsn)) {
                conn = dsn;
            } else {
                //MS SQL Server
                conn = L"Server=";
                conn += L"(local)";
            }
            if (userId && ::wcslen(userId)) {
                conn += L";UID=";
                conn += userId;
            }
            if (password && ::wcslen(password)) {
                conn += L";PWD=";
                conn += password;
            }
            if (odbcDriver && ::wcslen(odbcDriver)) {
                conn += L";DRIVER=";
                conn += odbcDriver;
            }
            CScopeUQueue sb;
            SQLSMALLINT cbConnStrOut = 0;
            std::string strConn = Utilities::ToUTF8(conn);
            retcode = SQLDriverConnect(hdbc, nullptr, (SQLCHAR*) strConn.c_str(), (SQLSMALLINT) strConn.size(), (SQLCHAR*) sb->GetBuffer(), (SQLSMALLINT) sb->GetSize(), &cbConnStrOut, SQL_DRIVER_NOPROMPT);
            if (!SQL_SUCCEEDED(retcode)) {
                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                return false;
            }
            if (nSvsId == Odbc::sidOdbc) {
                m_csPeer.lock();
                m_mapConnection[hSocket] = hdbc;
                m_csPeer.unlock();
            } else {
                retcode = SQLDisconnect(hdbc);
                retcode = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            }
            return true;
        }

        bool COdbcImpl::SetODBCEnv(int param) {
            SQLRETURN retcode = SQLSetEnvAttr(nullptr, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_ONE_PER_DRIVER, SQL_IS_INTEGER);
            retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &g_hEnv);
            if (!SQL_SUCCEEDED(retcode))
                return false;
            retcode = SQLSetEnvAttr(g_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*) SQL_OV_ODBC3, 0);
            if (!SQL_SUCCEEDED(retcode))
                return false;
            return true;
        }

        void COdbcImpl::FreeODBCEnv() {
            if (g_hEnv) {
                SQLFreeHandle(SQL_HANDLE_ENV, g_hEnv);
                g_hEnv = nullptr;
            }
        }

        void COdbcImpl::SetGlobalConnectionString(const wchar_t * str) {
            m_csPeer.lock();
            if (str) {
                m_strGlobalConnection = Utilities::ToUTF16(str);
            } else {
                m_strGlobalConnection.clear();
            }
            m_csPeer.unlock();
        }

        void COdbcImpl::ODBC_CONNECTION_STRING::Parse(const wchar_t * s) {
            using namespace std;
            remaining.clear();
            connection_string.clear();
            if (s)
                connection_string = s;
            if (!wcsstr(s, L"="))
                return;
            wstringstream ss(s ? s : L"");
            wstring item;
            vector<wstring> tokens;
            while (getline(ss, item, L';')) {
                tokens.push_back(item);
            }
            for (auto it = tokens.begin(), end = tokens.end(); it != end; ++it) {
                auto pos = it->find(L'=');
                if (pos == string::npos)
                    continue;
                wstring left = it->substr(0, pos);
                wstring right = it->substr(pos + 1);
                Utilities::Trim(left);
                Utilities::Trim(right);
                transform(left.begin(), left.end(), left.begin(), ::tolower);
                if (left == L"connect-timeout" || left == L"timeout" || left == L"connection-timeout") {
#ifdef WIN32_64
                    timeout = (unsigned int) _wtoi(right.c_str());
#else
                    wchar_t *tail = nullptr;
                    timeout = (unsigned int) wcstol(right.c_str(), &tail, 0);
#endif
                } else if (left == L"database" || left == L"db")
                    database = right;
                else if (left == L"port") {
#ifdef WIN32_64
                    port = (unsigned int) _wtoi(right.c_str());
#else
                    wchar_t *tail = nullptr;
                    port = (unsigned int) wcstol(right.c_str(), &tail, 0);
#endif
                } else if (left == L"pwd" || left == L"password") {
                    password = right;
                } else if (left == L"host" || left == L"server") {
                    host = right;
                } else if (left == L"dsn") {
                    dsn = right;
                } else if (left == L"user" || left == L"uid") {
                    user = right;
                } else if (left == L"driver") {
                    driver = right;
                } else if (left == L"filedsn") {
                    filedsn = right;
                } else if (left == L"savefile") {
                    savefile = right;
                } else if (left == L"async" || left == L"asynchronous") {
#ifdef WIN32_64
                    async = (_wtoi(right.c_str()) ? true : false);
#else
                    wchar_t *tail = nullptr;
                    async = (wcstol(right.c_str(), &tail, 0) ? true : false);
#endif
                } else {
                    if (remaining.size()) {
                        remaining += L";";
                    }
                    remaining += (left + L"=" + right);
                }
            }
        }

        COdbcImpl::COdbcImpl()
        : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_global(true),
        m_Blob(*m_sb), m_parameters(0), m_bCall(false), m_bReturn(false),
        m_outputs(0), m_nRecordSize(0), m_pNoSending(nullptr),
        m_msDriver(msODBC), m_EnableMessages(false),
        m_bPrimaryKeys(SQL_FALSE), m_bProcedureColumns(SQL_FALSE) {
        }

        void COdbcImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanDBObjects();
            m_global = true;
        }

        void COdbcImpl::ResetMemories() {
            m_Blob.SetSize(0);
            if (m_Blob.GetMaxSize() > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_Blob.ReallocBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
            m_UQueue.SetSize(0);
            if (m_UQueue.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_UQueue.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
        }

        void COdbcImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_oks = 0;
            m_fails = 0;
            m_ti = tiUnspecified;
            CAutoLock al(m_csPeer);
            SQLHDBC hdbc = m_mapConnection[GetSocketHandle()];
            if (hdbc) {
                m_pOdbc.reset(hdbc, [](SQLHDBC h) {
                    if (h) {
                        SQLRETURN ret = SQLDisconnect(h);
                        assert(ret == SQL_SUCCESS);
                        ret = SQLFreeHandle(SQL_HANDLE_DBC, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
            }
        }

        void COdbcImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I1_R0(idStartBLOB, StartBLOB, unsigned int)
            M_I0_R0(idChunk, Chunk)
            M_I0_R0(idEndBLOB, EndBLOB)
            M_I0_R0(idBeginRows, BeginRows)
            M_I0_R0(idTransferring, Transferring)
            M_I0_R0(idEndRows, EndRows)
            END_SWITCH
            m_pExcuting.reset();
        }

        int COdbcImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I0_R2(idClose, CloseDb, int, CDBString)
            M_I2_R3(idOpen, Open, CDBString, unsigned int, int, CDBString, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, CDBString, unsigned int, int, CDBString, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, CDBString)
            M_I5_R5(idExecute, Execute, CDBString, bool, bool, bool, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, CDBString, CParameterInfoArray, int, CDBString, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            M_I10_R5(idExecuteBatch, ExecuteBatch, CDBString, CDBString, int, int, bool, bool, bool, CDBString, unsigned int, UINT64, INT64, int, CDBString, CDBVariant, UINT64)
            M_I5_R3(Odbc::idSQLColumnPrivileges, DoSQLColumnPrivileges, CDBString, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I5_R3(Odbc::idSQLColumns, DoSQLColumns, CDBString, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I7_R3(Odbc::idSQLForeignKeys, DoSQLForeignKeys, CDBString, CDBString, CDBString, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I4_R3(Odbc::idSQLPrimaryKeys, DoSQLPrimaryKeys, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I5_R3(Odbc::idSQLProcedureColumns, DoSQLProcedureColumns, CDBString, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I4_R3(Odbc::idSQLProcedures, DoSQLProcedures, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I7_R3(Odbc::idSQLSpecialColumns, DoSQLSpecialColumns, SQLSMALLINT, CDBString, CDBString, CDBString, SQLSMALLINT, SQLSMALLINT, UINT64, int, CDBString, UINT64)
            M_I6_R3(Odbc::idSQLStatistics, DoSQLStatistics, CDBString, CDBString, CDBString, SQLUSMALLINT, SQLUSMALLINT, UINT64, int, CDBString, UINT64)
            M_I4_R3(Odbc::idSQLTablePrivileges, DoSQLTablePrivileges, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            M_I5_R3(Odbc::idSQLTables, DoSQLTables, CDBString, CDBString, CDBString, CDBString, UINT64, int, CDBString, UINT64)
            END_SWITCH
            m_pExcuting.reset();
            if (reqId == idExecuteParameters || reqId == idExecuteBatch) {
                m_vParam.clear();
            }
            return 0;
        }

        void COdbcImpl::OnBaseRequestArrive(unsigned short requestId) {
            switch (requestId) {
                case idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                    do {
                        std::shared_ptr<void> pExcuting = m_pExcuting;
                        if (pExcuting)
                            SQLCancel(pExcuting.get());
                        if (m_ti == tiUnspecified)
                            break;
                        SQLEndTran(SQL_HANDLE_DBC, m_pOdbc.get(), SQL_ROLLBACK);
                        m_ti = tiUnspecified;
                    } while (false);
                    break;
                default:
                    break;
            }
        }

        void COdbcImpl::CleanDBObjects() {
            m_pExcuting.reset();
            m_pPrepare.reset();
            m_pOdbc.reset();
            m_vParam.clear();
            ResetMemories();
            m_msDriver = msODBC;
            m_EnableMessages = false;
            m_bPrimaryKeys = SQL_FALSE;
            m_bProcedureColumns = SQL_FALSE;
        }

        void COdbcImpl::Open(const CDBString &strConnection, unsigned int flags, int &res, CDBString &errMsg, int &ms) {
            res = 0;
            if ((flags & ENABLE_TABLE_UPDATE_MESSAGES) == ENABLE_TABLE_UPDATE_MESSAGES)
                m_EnableMessages = GetPush().Subscribe(&STREAMING_SQL_CHAT_GROUP_ID, 1);
            if (m_pOdbc.get()) {
                PushInfo(m_pOdbc.get());
                if (strConnection.size()) {
                    CDBString sql;
#ifdef WIN32_64
                    if (m_dbms == L"microsoft sql server") {
                        sql = L"USE [" + strConnection + L"]";
                    } else if (m_dbms == L"mysql") {
                        sql = L"USE " + strConnection;
                    } else if (m_dbms == L"oracle") {
                        sql = L"ALTER SESSION SET current_schema=" + strConnection;
                    } else if (m_dbms.find(L"db2") != CDBString::npos) {
                        sql = L"SET SCHEMA " + strConnection;
                    } else if (m_dbms.find(L"postgre") == 0) {
                        sql = L"SET search_path=" + strConnection;
                    }
#else
                    if (m_dbms == u"microsoft sql server") {
                        sql = u"USE [" + strConnection + u"]";
                    } else if (m_dbms == u"mysql") {
                        sql = u"USE " + strConnection;
                    } else if (m_dbms == u"oracle") {
                        sql = u"ALTER SESSION SET current_schema=" + strConnection;
                    } else if (m_dbms.find(u"db2") != CDBString::npos) {
                        sql = u"SET SCHEMA " + strConnection;
                    } else if (m_dbms.find(u"postgre") == 0) {
                        sql = u"SET search_path=" + strConnection;
                    }
#endif
                    if (sql.size()) {
                        INT64 affected = 0;
                        UDB::CDBVariant vtId;
                        UINT64 fail_ok = 0;
                        Execute(sql, false, false, false, 0, affected, res, errMsg, vtId, fail_ok);
                        if (!res) {
                            errMsg = strConnection;
                            m_dbName = strConnection;
                        }
                    }
                }
                if (!res && !errMsg.size()) {
                    errMsg = m_dbName;
                }
            } else {
                if (!g_hEnv) {
                    res = Odbc::ER_ODBC_ENVIRONMENT_NOT_INITIALIZED;
                    errMsg = ODBC_ENVIRONMENT_NOT_INITIALIZED;
                    return;
                } else {
                    res = SQL_SUCCESS;
                }
                do {
                    CDBString db(strConnection);
                    if (!db.size() || db == ODBC_GLOBAL_CONNECTION_STRING) {
                        m_csPeer.lock();
                        db = m_strGlobalConnection;
                        m_csPeer.unlock();
                        m_global = true;
                    } else {
                        m_global = false;
                    }
                    ODBC_CONNECTION_STRING ocs;
#ifdef WIN32_64
                    ocs.Parse(db.c_str());
#else
                    ocs.Parse(Utilities::ToWide(db).c_str());
#endif
                    SQLHDBC hdbc = nullptr;
                    SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_DBC, g_hEnv, &hdbc);
                    if (!SQL_SUCCEEDED(retcode)) {
                        res = Odbc::ER_ERROR;
                        GetErrMsg(SQL_HANDLE_ENV, g_hEnv, errMsg);
                        break;
                    }

                    if (ocs.timeout) {
                        SQLPOINTER rgbValue = &ocs.timeout;
                        retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, rgbValue, 0);
                    }

                    if (ocs.database.size()) {
                        std::string db = Utilities::ToUTF8(ocs.database.c_str(), ocs.database.size());
                        retcode = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER) db.c_str(), (SQLINTEGER) (db.size() * sizeof (SQLCHAR)));
                    }

                    //retcode = SQLSetConnectAttr(hdbc, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER) (ocs.async ? SQL_ASYNC_ENABLE_ON : SQL_ASYNC_ENABLE_OFF), 0);
                    //std::string host = Utilities::ToUTF8(ocs.host.c_str(), ocs.host.size());
                    //std::string user = Utilities::ToUTF8(ocs.user.c_str(), ocs.user.size());
                    //std::string pwd = Utilities::ToUTF8(ocs.password.c_str(), ocs.password.size());

                    std::string conn = Utilities::ToUTF8(ocs.connection_string.c_str(), ocs.connection_string.size());

                    CScopeUQueue sb;
                    SQLCHAR *ConnStrIn = (SQLCHAR *) conn.c_str();
                    SQLCHAR *ConnStrOut = (SQLCHAR *) sb->GetBuffer();
                    SQLSMALLINT cbConnStrOut;
                    retcode = SQLDriverConnect(hdbc, nullptr, ConnStrIn, (SQLSMALLINT) conn.size(), ConnStrOut, (SQLSMALLINT) sb->GetSize(), &cbConnStrOut, SQL_DRIVER_NOPROMPT);
                    //retcode = SQLConnect(hdbc, (SQLCHAR*) host.c_str(), (SQLSMALLINT) host.size(), (SQLCHAR *) user.c_str(), (SQLSMALLINT) user.size(), (SQLCHAR *) pwd.c_str(), (SQLSMALLINT) pwd.size());
                    if (!SQL_SUCCEEDED(retcode)) {
                        res = Odbc::ER_ERROR;
                        GetErrMsg(SQL_HANDLE_DBC, hdbc, errMsg);
                        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
                        break;
                    }
                    m_pOdbc.reset(hdbc, [](SQLHDBC h) {
                        if (h) {
                            SQLRETURN ret = SQLDisconnect(h);
                            assert(ret == SQL_SUCCESS);
                            ret = SQLFreeHandle(SQL_HANDLE_DBC, h);
                            assert(ret == SQL_SUCCESS);
                        }
                    });
                    PushInfo(hdbc);
                    if (!m_global) {
                        errMsg = db;
                    } else {
                        errMsg = ODBC_GLOBAL_CONNECTION_STRING;
                    }
                } while (false);
            }
            ms = m_msDriver;
            if (m_pOdbc) {
                SQLGetFunctions(m_pOdbc.get(), SQL_API_SQLPRIMARYKEYS, &m_bPrimaryKeys);
                SQLGetFunctions(m_pOdbc.get(), SQL_API_SQLPROCEDURECOLUMNS, &m_bProcedureColumns);
            }
        }

        void COdbcImpl::CloseDb(int &res, CDBString & errMsg) {
            if (m_EnableMessages) {
                GetPush().Unsubscribe();
            }
            CleanDBObjects();
            res = SQL_SUCCESS;
        }

        void COdbcImpl::BeginTrans(int isolation, const CDBString &dbConn, unsigned int flags, int &res, CDBString &errMsg, int &ms) {
            ms = msODBC;
            if (m_ti != tiUnspecified || isolation == (int) tiUnspecified) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = Odbc::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (!m_pOdbc) {
                Open(dbConn, flags, res, errMsg, ms);
                if (!m_pOdbc) {
                    return;
                }
            }
            ms = m_msDriver;
            SQLINTEGER attr;
            switch ((tagTransactionIsolation) isolation) {
                case tiReadUncommited:
                    attr = SQL_TXN_READ_UNCOMMITTED;
                    break;
                case tiRepeatableRead:
                    attr = SQL_TXN_REPEATABLE_READ;
                    break;
                case tiReadCommited:
                    attr = SQL_TXN_READ_COMMITTED;
                    break;
                case tiSerializable:
                    attr = SQL_TXN_SERIALIZABLE;
                    break;
                default:
                    attr = 0;
                    break;
            }
            SQLRETURN retcode;
            if (attr) {
                retcode = SQLSetConnectAttr(m_pOdbc.get(), SQL_ATTR_TXN_ISOLATION, (SQLPOINTER) attr, 0);
                //ignore errors
            }
            retcode = SQLSetConnectAttr(m_pOdbc.get(), SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_FALSE, 0);
            if (!SQL_SUCCEEDED(retcode)) {
                res = Odbc::ER_ERROR;
                GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
            } else {
                res = SQL_SUCCESS;
                m_fails = 0;
                m_oks = 0;
                m_ti = (tagTransactionIsolation) isolation;
                if (!m_global) {
                    errMsg = dbConn;
                } else {
                    errMsg = ODBC_GLOBAL_CONNECTION_STRING;
                }
            }
        }

        void COdbcImpl::EndTrans(int plan, int &res, CDBString & errMsg) {
            if (m_ti == tiUnspecified) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = Odbc::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (plan < 0 || plan > rpRollbackAlways) {
                res = Odbc::ER_BAD_END_TRANSTACTION_PLAN;
                errMsg = BAD_END_TRANSTACTION_PLAN;
                return;
            }
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
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
            SQLRETURN retcode = SQLEndTran(SQL_HANDLE_DBC, m_pOdbc.get(), rollback ? SQL_ROLLBACK : SQL_COMMIT);
            if (!SQL_SUCCEEDED(retcode)) {
                res = Odbc::ER_ERROR;
                GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
            } else {
                res = SQL_SUCCESS;
                m_ti = tiUnspecified;
                m_fails = 0;
                m_oks = 0;
            }
        }

        bool COdbcImpl::SendRows(CUQueue& sb, bool transferring) {
            if (m_pNoSending) {
                m_pNoSending->Push(sb.GetBuffer(), sb.GetSize());
                sb.SetSize(0);
                return true;
            }
            bool batching = (GetBytesBatched() >= DEFAULT_RECORD_BATCH_SIZE);
            if (batching) {
                CommitBatching();
            }
            unsigned int ret = SendResult(transferring ? idTransferring : idEndRows, sb.GetBuffer(), sb.GetSize());
            sb.SetSize(0);
            if (batching) {
                StartBatching();
            }
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool COdbcImpl::SendUText(SQLHSTMT hstmt, SQLUSMALLINT index, CUQueue &qTemp, CUQueue &q, bool & blob) {
            assert(!m_pNoSending);
            assert((qTemp.GetMaxSize() % sizeof (SQLWCHAR)) == 0);
            qTemp.SetSize(0);
            SQLLEN len_or_null = 0;
            SQLRETURN retcode = SQLGetData(hstmt, index, SQL_C_WCHAR, (SQLPOINTER) qTemp.GetBuffer(), qTemp.GetMaxSize(), &len_or_null);
            assert(SQL_SUCCEEDED(retcode));
            if (len_or_null == SQL_NULL_DATA) {
                q << (VARTYPE) VT_NULL;
                blob = false;
                return true;
            } else if ((unsigned int) len_or_null + sizeof (SQLWCHAR) <= qTemp.GetMaxSize()) {
                q << (VARTYPE) VT_BSTR << (unsigned int) len_or_null;
                q.Push(qTemp.GetBuffer(), (unsigned int) len_or_null);
                blob = false;
                return true;
            }
            if (q.GetSize() && !SendRows(q, true)) {
                return false;
            }
            unsigned int bytes = (unsigned int) len_or_null;
            unsigned int ret = SendResult(idStartBLOB, (unsigned int) (bytes + sizeof (VARTYPE) + sizeof (unsigned int) + sizeof (unsigned int)), (VARTYPE) VT_BSTR, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            blob = true;
            while (retcode == SQL_SUCCESS_WITH_INFO) {
                if (bytes > qTemp.GetMaxSize()) {
                    bytes = qTemp.GetMaxSize();
                }
                bytes -= sizeof (SQLWCHAR);
                ret = SendResult(idChunk, qTemp.GetBuffer(), bytes);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                retcode = SQLGetData(hstmt, index, SQL_C_WCHAR, (SQLPOINTER) qTemp.GetBuffer(), qTemp.GetMaxSize(), &len_or_null);
                bytes = (unsigned int) len_or_null;
            }
            ret = SendResult(idEndBLOB, qTemp.GetBuffer(), bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool COdbcImpl::SendBlob(SQLHSTMT hstmt, SQLUSMALLINT index, VARTYPE vt, CUQueue &qTemp, CUQueue &q, bool & blob) {
            assert(!m_pNoSending);
            qTemp.SetSize(0);
            SQLLEN len_or_null = 0;
            SQLRETURN retcode = SQLGetData(hstmt, index, SQL_C_BINARY, (SQLPOINTER) qTemp.GetBuffer(), qTemp.GetMaxSize(), &len_or_null);
            assert(SQL_SUCCEEDED(retcode));
            if (len_or_null == SQL_NULL_DATA) {
                q << (VARTYPE) VT_NULL;
                blob = false;
                return true;
            } else if ((unsigned int) len_or_null <= qTemp.GetMaxSize()) {
                q << vt << (unsigned int) len_or_null;
                q.Push(qTemp.GetBuffer(), (unsigned int) len_or_null);
                blob = false;
                return true;
            }
            if (q.GetSize() && !SendRows(q, true)) {
                return false;
            }
            unsigned int bytes = (unsigned int) len_or_null;
            unsigned int ret = SendResult(idStartBLOB, (unsigned int) (bytes + sizeof (VARTYPE) + sizeof (unsigned int) + sizeof (unsigned int)), vt, bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            blob = true;
            bool isBatching = IsBatching();
            if (isBatching)
                CommitBatching();
            while (retcode == SQL_SUCCESS_WITH_INFO) {
                if (bytes > qTemp.GetMaxSize()) {
                    bytes = qTemp.GetMaxSize();
                }
                ret = SendResult(idChunk, qTemp.GetBuffer(), bytes);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                retcode = SQLGetData(hstmt, index, SQL_C_BINARY, (SQLPOINTER) qTemp.GetBuffer(), qTemp.GetMaxSize(), &len_or_null);
                bytes = (unsigned int) len_or_null;
            }
            if (isBatching)
                StartBatching();
            ret = SendResult(idEndBLOB, qTemp.GetBuffer(), bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        CDBColumnInfoArray COdbcImpl::GetColInfo(SQLHSTMT hstmt, SQLSMALLINT columns, bool meta) {
            bool primary_key_set = false;
            m_vBindInfo.clear();
            bool hasBlob = false;
            bool hasVariant = false;
            SQLCHAR colname[256] =
            {0}; // column name
            m_nRecordSize = 0;
            SQLSMALLINT colnamelen = 0; // length of column name
            SQLSMALLINT nullable = 0; // whether column can have NULL value
            SQLULEN collen = 0; // column lengths
            SQLSMALLINT coltype = 0; // column type
            SQLSMALLINT decimaldigits = 0; // no of digits if column is numeric
            SQLLEN displaysize = 0; // drivers column display size
            CDBColumnInfoArray vCols((size_t) columns);
            bool bPostgres = (m_msDriver == msPostgreSQL);
            for (SQLSMALLINT n = 0; n < columns; ++n) {
                CDBColumnInfo &info = vCols[n];
                SQLRETURN retcode = SQLDescribeCol(hstmt, (SQLUSMALLINT) (n + 1), colname, sizeof (colname) / sizeof (SQLCHAR), &colnamelen, &coltype, &collen, &decimaldigits, &nullable);
                assert(SQL_SUCCEEDED(retcode));
                if (nullable == SQL_NO_NULLS) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                }
                if (bPostgres && collen > 8000)
                    collen = 0; //make it to long text or binary
                if (meta) {
                    info.DisplayName = Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR)); //display column name
                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_BASE_COLUMN_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                    assert(SQL_SUCCEEDED(retcode));
                    info.OriginalName = Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR)); //original column name
                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_SCHEMA_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                    assert(SQL_SUCCEEDED(retcode));
                    if (colnamelen) {
                        info.TablePath = Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR));
                        Utilities::Trim(info.TablePath);
                        info.TablePath.push_back('.');
                    }
                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_BASE_TABLE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                    assert(SQL_SUCCEEDED(retcode));
                    info.TablePath += Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR)); //schema.table_name
                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_TYPE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                    assert(SQL_SUCCEEDED(retcode));
                    info.DeclaredType = Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR)); //native data type
                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CATALOG_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                    assert(SQL_SUCCEEDED(retcode));
                    info.DBPath = Utilities::ToUTF16((const char*) colname, (size_t) colnamelen / sizeof (SQLCHAR)); //database name
                }
                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_UNSIGNED, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));

                CBindInfo bindinfo;

                switch (coltype) {
                    case SQL_CLOB: //IBM DB2
                    case SQL_CHAR:
                    case SQL_VARCHAR:
                    case SQL_LONGVARCHAR:
                        if (collen == 0)
                            info.ColumnSize = (~0);
                        else
                            info.ColumnSize = (unsigned int) collen;
                        info.DataType = (VT_ARRAY | VT_I1);
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        if (displaysize == SQL_TRUE) {
                            info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
                        }
                        bindinfo.BufferSize = (info.ColumnSize + 1);
                        break;
                    case SQL_GRAPHIC: //IBM DB2
                    case SQL_VARGRAPHIC: //IBM DB2
                    case SQL_LONGVARGRAPHIC: //IBM DB2
                    case SQL_DBCLOB: //IBM DB2
                    case SQL_WCHAR:
                    case SQL_WVARCHAR:
                    case SQL_WLONGVARCHAR:
                        if (collen == 0)
                            info.ColumnSize = (~0);
                        else
                            info.ColumnSize = (unsigned int) collen;
                        info.DataType = VT_BSTR;
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        if (displaysize == SQL_TRUE) {
                            info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
                        }
                        bindinfo.BufferSize = (info.ColumnSize + 1) * sizeof (SQLWCHAR);
                        break;
                    case SQL_BLOB:
                    case SQL_BINARY:
                    case SQL_VARBINARY:
                    case SQL_LONGVARBINARY:
                        if (collen == 0)
                            info.ColumnSize = (~0);
                        else
                            info.ColumnSize = (unsigned int) collen;
                        info.DataType = (VT_ARRAY | VT_UI1);
                        bindinfo.BufferSize = info.ColumnSize;
                        break;
                    case SQL_DECIMAL:
                    case SQL_NUMERIC:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_DECIMAL;
                        info.Scale = (unsigned char) decimaldigits;
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_PRECISION, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        info.Precision = (unsigned char) displaysize;
                        bindinfo.BufferSize = DECIMAL_STRING_BUFFER_SIZE;
                        if (decimaldigits == 0 && displaysize < 10) {
                            //Hack for Oracle as its driver doesn't support SQL_C_SBIGINT or SQL_C_UBIGINT binding
                            if (displaysize >= 5) {
                                info.DataType = VT_I4;
                                bindinfo.BufferSize = sizeof (int);
                            } else {
                                info.DataType = VT_I2;
                                bindinfo.BufferSize = sizeof (short);
                            }
                        }
                        break;
                    case SQL_SMALLINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI2;
                        } else {
                            info.DataType = VT_I2;
                        }
                        bindinfo.BufferSize = sizeof (short);
                        break;
                    case SQL_INTEGER:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI4;
                        } else {
                            info.DataType = VT_I4;
                        }
                        bindinfo.BufferSize = sizeof (int);
                        break;
                    case SQL_REAL:
                        info.DataType = VT_R4;
                        bindinfo.BufferSize = sizeof (float);
                        break;
                    case SQL_FLOAT:
                    case SQL_DOUBLE:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_R8;
                        bindinfo.BufferSize = sizeof (double);
                        break;
                    case SQL_TINYINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI1;
                        } else {
                            info.DataType = VT_I1;
                        }
                        bindinfo.BufferSize = sizeof (unsigned char);
                        break;
                    case SQL_BIGINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI8;
                        } else {
                            info.DataType = VT_I8;
                        }
                        bindinfo.BufferSize = sizeof (INT64);
                        break;
                    case SQL_BIT:
                        info.DataType = VT_BOOL;
                        info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
                        bindinfo.BufferSize = sizeof (unsigned char);
                        break;
                    case SQL_GUID:
                        info.DataType = VT_CLSID;
                        bindinfo.BufferSize = sizeof (GUID);
                        break;
                    case SQL_TYPE_DATE:
                    case SQL_TYPE_TIME:
                    case SQL_TYPE_TIMESTAMP:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_DATE;
                        info.Scale = (unsigned char) decimaldigits;
                        bindinfo.BufferSize = DATETIME_STRING_BUFFER_SIZE;
                        break;
                    case SQL_INTERVAL_MONTH:
                    case SQL_INTERVAL_YEAR:
                    case SQL_INTERVAL_YEAR_TO_MONTH:
                    case SQL_INTERVAL_DAY:
                    case SQL_INTERVAL_HOUR:
                    case SQL_INTERVAL_MINUTE:
                    case SQL_INTERVAL_SECOND:
                    case SQL_INTERVAL_DAY_TO_HOUR:
                    case SQL_INTERVAL_DAY_TO_MINUTE:
                    case SQL_INTERVAL_DAY_TO_SECOND:
                    case SQL_INTERVAL_HOUR_TO_MINUTE:
                    case SQL_INTERVAL_HOUR_TO_SECOND:
                    case SQL_INTERVAL_MINUTE_TO_SECOND:
                        info.ColumnSize = 32; //remember SQL data type
                        info.DataType = (VT_ARRAY | VT_I1);
                        info.Precision = (unsigned char) coltype;
                        bindinfo.BufferSize = info.ColumnSize;
                        break;
                    case SQL_XML: //IBM DB2
                    case SQL_SS_XML: //SQL Server
                        info.DataType = VT_XML;
                        info.ColumnSize = (~0);
                        bindinfo.BufferSize = 0;
                        break;
                    case SQL_SS_VARIANT:
                        hasVariant = true;
                        info.DataType = VT_VARIANT;
                        info.ColumnSize = DEFAULT_OUTPUT_BUFFER_SIZE;
                        bindinfo.BufferSize = DEFAULT_OUTPUT_BUFFER_SIZE;
                        break;
                    default:
                        info.DataType = VT_BSTR;
                        info.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                        bindinfo.BufferSize = DEFAULT_UNICODE_CHAR_SIZE * sizeof (SQLWCHAR);
                        break;
                }
                if (info.ColumnSize > DEFAULT_OUTPUT_BUFFER_SIZE) {
                    hasBlob = true;
                }
                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_AUTO_UNIQUE_VALUE, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                if (displaysize == SQL_TRUE) {
                    info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                    info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
                    info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                    primary_key_set = true;
                } else if (!primary_key_set) {
                    switch (m_msDriver) {
                        case msMsSQL:
                            retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_CA_SS_COLUMN_KEY, nullptr, 0, nullptr, &displaysize);
                            if (displaysize) {
                                info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                            }
                            break;
                        default:
                            break;
                    }
                }
                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_UPDATABLE, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                if (displaysize == SQL_ATTR_READONLY) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
                }
                if (!hasBlob) {
                    bindinfo.DataType = info.DataType;
                    bindinfo.Offset = m_nRecordSize;
                    m_nRecordSize += (bindinfo.BufferSize + sizeof (SQLULEN));
                    m_vBindInfo.push_back(bindinfo);
                }
            }
            if (hasBlob || hasVariant) {
                m_vBindInfo.clear();
                m_nRecordSize = 0;
            }
            return vCols;
        }

        unsigned int COdbcImpl::ToCTime(const TIMESTAMP_STRUCT &d, std::tm & tm) {
            tm.tm_isdst = 0;
            tm.tm_wday = 0;
            tm.tm_yday = 0;
            tm.tm_year = d.year - 1900;
            tm.tm_mon = d.month - 1;
            tm.tm_mday = d.day;
            tm.tm_hour = d.hour;
            tm.tm_min = d.minute;
            tm.tm_sec = d.second;
            return (unsigned int) (d.fraction / 1000);
        }

        unsigned int COdbcImpl::ToCTime(const TIME_STRUCT &d, std::tm & tm) {
            //start from 01/01/1900
            memset(&tm, 0, sizeof (tm));
            tm.tm_mday = 0;
            tm.tm_hour = d.hour;
            tm.tm_min = d.minute;
            tm.tm_sec = d.second;
            return 0;
        }

        unsigned int COdbcImpl::ToCTime(const DATE_STRUCT &d, std::tm & tm) {
            memset(&tm, 0, sizeof (tm));
            tm.tm_year = d.year - 1900;
            tm.tm_mon = d.month - 1;
            tm.tm_mday = d.day;
            return 0;
        }

        void COdbcImpl::SetStringInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo) {
            SQLSMALLINT bufferLen = 0;
            SQLCHAR buffer[1024] =
            {0};
            SQLRETURN retcode = SQLGetInfo(hdbc, infoType, buffer, (SQLSMALLINT) sizeof (buffer), &bufferLen);
            if (SQL_SUCCEEDED(retcode)) {
                std::wstring s(buffer, buffer + bufferLen);
                mapInfo[infoType] = s.c_str();
            }
        }

        void COdbcImpl::SetUIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo) {
            SQLSMALLINT bufferLen = 0;
            unsigned int d = 0;
            SQLRETURN retcode = SQLGetInfo(hdbc, infoType, &d, (SQLSMALLINT)sizeof (d), &bufferLen);
            if (SQL_SUCCEEDED(retcode)) {
                mapInfo[infoType] = d;
            }
        }

        void COdbcImpl::SetIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo) {
            SQLSMALLINT bufferLen = 0;
            int d = 0;
            SQLRETURN retcode = SQLGetInfo(hdbc, infoType, &d, (SQLSMALLINT)sizeof (d), &bufferLen);
            if (SQL_SUCCEEDED(retcode)) {
                mapInfo[infoType] = d;
            }
        }

        void COdbcImpl::SetUInt64Info(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo) {
            SQLSMALLINT bufferLen = 0;
            SQLULEN d = 0;
            SQLRETURN retcode = SQLGetInfo(hdbc, infoType, &d, (SQLSMALLINT)sizeof (d), &bufferLen);
            if (SQL_SUCCEEDED(retcode)) {
                mapInfo[infoType] = d;
            }
        }

        void COdbcImpl::SetUShortInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo) {
            SQLSMALLINT bufferLen = 0;
            SQLUSMALLINT d = 0;
            SQLRETURN retcode = SQLGetInfo(hdbc, infoType, &d, (SQLSMALLINT)sizeof (d), &bufferLen);
            if (SQL_SUCCEEDED(retcode)) {
                mapInfo[infoType] = d;
            }
        }

        bool COdbcImpl::PushInfo(SQLHDBC hdbc) {
            std::unordered_map<SQLUSMALLINT, CComVariant> mapInfo;

            SetStringInfo(hdbc, SQL_ACCESSIBLE_PROCEDURES, mapInfo);
            SetStringInfo(hdbc, SQL_ACCESSIBLE_TABLES, mapInfo);
            SetStringInfo(hdbc, SQL_DATABASE_NAME, mapInfo);
            if (mapInfo.find(SQL_DATABASE_NAME) != mapInfo.end()) {
                m_dbName = Utilities::ToUTF16(mapInfo[SQL_DATABASE_NAME].bstrVal);
            } else {
                m_dbName.clear();
            }
            SetStringInfo(hdbc, SQL_USER_NAME, mapInfo);
            if (mapInfo.find(SQL_USER_NAME) != mapInfo.end()) {
                m_userName = Utilities::ToUTF16(mapInfo[SQL_USER_NAME].bstrVal);
            } else {
                m_userName.clear();
            }
            SetStringInfo(hdbc, SQL_DATA_SOURCE_NAME, mapInfo);
            SetStringInfo(hdbc, SQL_DRIVER_NAME, mapInfo);
            SetStringInfo(hdbc, SQL_DRIVER_VER, mapInfo);
            SetStringInfo(hdbc, SQL_DRIVER_ODBC_VER, mapInfo);
            SetStringInfo(hdbc, SQL_ODBC_VER, mapInfo);
            SetStringInfo(hdbc, SQL_SERVER_NAME, mapInfo);
            SetStringInfo(hdbc, SQL_CATALOG_NAME, mapInfo);
            SetStringInfo(hdbc, SQL_CATALOG_TERM, mapInfo);
            SetStringInfo(hdbc, SQL_CATALOG_NAME_SEPARATOR, mapInfo);
            SetStringInfo(hdbc, SQL_COLLATION_SEQ, mapInfo);
            SetStringInfo(hdbc, SQL_COLUMN_ALIAS, mapInfo);
            SetStringInfo(hdbc, SQL_DATA_SOURCE_READ_ONLY, mapInfo);
            SetStringInfo(hdbc, SQL_DBMS_NAME, mapInfo);
            if (mapInfo.find(SQL_DBMS_NAME) != mapInfo.end()) {
                m_dbms = Utilities::ToUTF16(mapInfo[SQL_DBMS_NAME].bstrVal);
                std::transform(m_dbms.begin(), m_dbms.end(), m_dbms.begin(), ::tolower); //microsoft sql server, oracle, mysql
#ifdef WIN32_64
                if (m_dbms == L"microsoft sql server") {
                    m_msDriver = msMsSQL;
                } else if (m_dbms == L"mysql") {
                    m_msDriver = msMysql;
                } else if (m_dbms == L"oracle") {
                    m_msDriver = msOracle;
                } else if (m_dbms.find(L"db2") != std::wstring::npos) {
                    m_msDriver = msDB2;
                } else if (m_dbms.find(L"postgre") == 0) {
                    m_msDriver = msPostgreSQL;
                }
#else
                if (m_dbms == u"microsoft sql server") {
                    m_msDriver = msMsSQL;
                } else if (m_dbms == u"mysql") {
                    m_msDriver = msMysql;
                } else if (m_dbms == u"oracle") {
                    m_msDriver = msOracle;
                } else if (m_dbms.find(u"db2") != CDBString::npos) {
                    m_msDriver = msDB2;
                } else if (m_dbms.find(u"postgre") == 0) {
                    m_msDriver = msPostgreSQL;
                }
#endif
            } else {
                m_dbms.clear();
            }
            SetStringInfo(hdbc, SQL_DBMS_VER, mapInfo);
            SetStringInfo(hdbc, SQL_DESCRIBE_PARAMETER, mapInfo);
            SetStringInfo(hdbc, SQL_DM_VER, mapInfo);
            SetStringInfo(hdbc, SQL_EXPRESSIONS_IN_ORDERBY, mapInfo);
            SetStringInfo(hdbc, SQL_IDENTIFIER_QUOTE_CHAR, mapInfo);
            SetStringInfo(hdbc, SQL_INTEGRITY, mapInfo);
            SetStringInfo(hdbc, SQL_KEYWORDS, mapInfo);
            SetStringInfo(hdbc, SQL_LIKE_ESCAPE_CLAUSE, mapInfo);
            SetStringInfo(hdbc, SQL_MULT_RESULT_SETS, mapInfo);
            SetStringInfo(hdbc, SQL_MULTIPLE_ACTIVE_TXN, mapInfo);
            SetStringInfo(hdbc, SQL_NEED_LONG_DATA_LEN, mapInfo);
            SetStringInfo(hdbc, SQL_ORDER_BY_COLUMNS_IN_SELECT, mapInfo);
            SetStringInfo(hdbc, SQL_PROCEDURE_TERM, mapInfo);
            SetStringInfo(hdbc, SQL_PROCEDURES, mapInfo);
            SetStringInfo(hdbc, SQL_ROW_UPDATES, mapInfo);
            SetStringInfo(hdbc, SQL_SCHEMA_TERM, mapInfo);
            SetStringInfo(hdbc, SQL_SEARCH_PATTERN_ESCAPE, mapInfo);
            SetStringInfo(hdbc, SQL_SPECIAL_CHARACTERS, mapInfo);
            SetStringInfo(hdbc, SQL_SCHEMA_TERM, mapInfo);
            SetStringInfo(hdbc, SQL_TABLE_TERM, mapInfo);
            SetStringInfo(hdbc, SQL_XOPEN_CLI_YEAR, mapInfo);

            SetUShortInfo(hdbc, SQL_ACTIVE_ENVIRONMENTS, mapInfo);
            SetUShortInfo(hdbc, SQL_CATALOG_LOCATION, mapInfo);
            SetUShortInfo(hdbc, SQL_CONCAT_NULL_BEHAVIOR, mapInfo);
            SetUShortInfo(hdbc, SQL_CORRELATION_NAME, mapInfo);
            SetUShortInfo(hdbc, SQL_CURSOR_COMMIT_BEHAVIOR, mapInfo);
            SetUShortInfo(hdbc, SQL_CURSOR_ROLLBACK_BEHAVIOR, mapInfo);
            SetUShortInfo(hdbc, SQL_FILE_USAGE, mapInfo);
            SetUShortInfo(hdbc, SQL_GROUP_BY, mapInfo);
            SetUShortInfo(hdbc, SQL_IDENTIFIER_CASE, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_CATALOG_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMN_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMNS_IN_GROUP_BY, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMNS_IN_INDEX, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMNS_IN_ORDER_BY, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMNS_IN_SELECT, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_COLUMNS_IN_TABLE, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_CONCURRENT_ACTIVITIES, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_CURSOR_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_DRIVER_CONNECTIONS, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_IDENTIFIER_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_PROCEDURE_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_TABLE_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_TABLES_IN_SELECT, mapInfo);
            SetUShortInfo(hdbc, SQL_MAX_USER_NAME_LEN, mapInfo);
            SetUShortInfo(hdbc, SQL_NULL_COLLATION, mapInfo);
            SetUShortInfo(hdbc, SQL_QUOTED_IDENTIFIER_CASE, mapInfo);
            SetUShortInfo(hdbc, SQL_TXN_CAPABLE, mapInfo);

            SetIntInfo(hdbc, SQL_POS_OPERATIONS, mapInfo);

            SetUIntInfo(hdbc, SQL_AGGREGATE_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_ALTER_DOMAIN, mapInfo);
            SetUIntInfo(hdbc, SQL_ALTER_TABLE, mapInfo);
#ifdef WIN32_64
            SetUIntInfo(hdbc, SQL_ASYNC_DBC_FUNCTIONS, mapInfo);
#endif
            SetUIntInfo(hdbc, SQL_ASYNC_MODE, mapInfo);
            SetUIntInfo(hdbc, SQL_BATCH_ROW_COUNT, mapInfo);
            SetUIntInfo(hdbc, SQL_BATCH_SUPPORT, mapInfo);
            SetUIntInfo(hdbc, SQL_CATALOG_USAGE, mapInfo);
            SetUIntInfo(hdbc, SQL_CONVERT_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_ASSERTION, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_CHARACTER_SET, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_COLLATION, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_DOMAIN, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_SCHEMA, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_TABLE, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_TRANSLATION, mapInfo);
            SetUIntInfo(hdbc, SQL_CREATE_VIEW, mapInfo);
            SetUIntInfo(hdbc, SQL_CURSOR_SENSITIVITY, mapInfo);
            SetUIntInfo(hdbc, SQL_DATETIME_LITERALS, mapInfo);
            SetUIntInfo(hdbc, SQL_DDL_INDEX, mapInfo);
            SetUIntInfo(hdbc, SQL_DEFAULT_TXN_ISOLATION, mapInfo);
            //SetUIntInfo(hdbc, SQL_DRIVER_AWARE_POOLING_SUPPORTED, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_ASSERTION, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_COLLATION, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_DOMAIN, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_SCHEMA, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_TABLE, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_TRANSLATION, mapInfo);
            SetUIntInfo(hdbc, SQL_DROP_VIEW, mapInfo);
            SetUIntInfo(hdbc, SQL_DYNAMIC_CURSOR_ATTRIBUTES1, mapInfo);
            SetUIntInfo(hdbc, SQL_DYNAMIC_CURSOR_ATTRIBUTES2, mapInfo);
            SetUIntInfo(hdbc, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, mapInfo);
            SetUIntInfo(hdbc, SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, mapInfo);
            SetUIntInfo(hdbc, SQL_GETDATA_EXTENSIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_INDEX_KEYWORDS, mapInfo);
            SetUIntInfo(hdbc, SQL_INFO_SCHEMA_VIEWS, mapInfo);
            SetUIntInfo(hdbc, SQL_INSERT_STATEMENT, mapInfo);
            SetUIntInfo(hdbc, SQL_KEYSET_CURSOR_ATTRIBUTES1, mapInfo);
            SetUIntInfo(hdbc, SQL_KEYSET_CURSOR_ATTRIBUTES2, mapInfo);
            SetUIntInfo(hdbc, SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, mapInfo);
            SetUIntInfo(hdbc, SQL_MAX_BINARY_LITERAL_LEN, mapInfo);
            SetUIntInfo(hdbc, SQL_MAX_INDEX_SIZE, mapInfo);
            SetUIntInfo(hdbc, SQL_MAX_ROW_SIZE, mapInfo);
            SetUIntInfo(hdbc, SQL_MAX_STATEMENT_LEN, mapInfo);
            SetUIntInfo(hdbc, SQL_NUMERIC_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_ODBC_INTERFACE_CONFORMANCE, mapInfo);
            SetUIntInfo(hdbc, SQL_OJ_CAPABILITIES, mapInfo);
            SetUIntInfo(hdbc, SQL_PARAM_ARRAY_SELECTS, mapInfo);
            SetUIntInfo(hdbc, SQL_SCHEMA_USAGE, mapInfo);
            SetUIntInfo(hdbc, SQL_SCROLL_OPTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL_CONFORMANCE, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_DATETIME_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_FOREIGN_KEY_DELETE_RULE, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_FOREIGN_KEY_UPDATE_RULE, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_GRANT, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_NUMERIC_VALUE_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_PREDICATES, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_RELATIONAL_JOIN_OPERATORS, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_REVOKE, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_ROW_VALUE_CONSTRUCTOR, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_STRING_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_SQL92_VALUE_EXPRESSIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_STANDARD_CLI_CONFORMANCE, mapInfo);
            SetUIntInfo(hdbc, SQL_STATIC_CURSOR_ATTRIBUTES1, mapInfo);
            SetUIntInfo(hdbc, SQL_STATIC_CURSOR_ATTRIBUTES2, mapInfo);
            SetUIntInfo(hdbc, SQL_STRING_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_SUBQUERIES, mapInfo);
            SetUIntInfo(hdbc, SQL_SYSTEM_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_TIMEDATE_ADD_INTERVALS, mapInfo);
            SetUIntInfo(hdbc, SQL_TIMEDATE_DIFF_INTERVALS, mapInfo);
            SetUIntInfo(hdbc, SQL_TIMEDATE_FUNCTIONS, mapInfo);
            SetUIntInfo(hdbc, SQL_TXN_ISOLATION_OPTION, mapInfo);
            SetUIntInfo(hdbc, SQL_UNION, mapInfo);
            SetUIntInfo(hdbc, SQL_TXN_ISOLATION_OPTION, mapInfo);

            //SetUInt64Info(hdbc, SQL_DRIVER_HDBCSQL_DRIVER_HENV, mapInfo);
            SetUInt64Info(hdbc, SQL_DRIVER_HDESC, mapInfo);
            SetUInt64Info(hdbc, SQL_DRIVER_HLIB, mapInfo);
            SetUInt64Info(hdbc, SQL_DRIVER_HSTMT, mapInfo);

            CScopeUQueue sb;
            CUQueue &q = *sb;
            for (auto it = mapInfo.begin(), end = mapInfo.end(); it != end; ++it) {
                q << it->first << it->second;
            }
            unsigned int ret = SendResult(Odbc::idSQLGetInfo, q.GetBuffer(), q.GetSize());
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool COdbcImpl::PreprocessPreparedStatement() {
            CDBString s = m_sqlPrepare;
            if (s.size() && s.front() == '{' && s.back() == '}') {
                s.pop_back();
                s.erase(s.begin(), s.begin() + 1);
                Utilities::Trim(s);
            }
            m_bReturn = (s.front() == '?');
            if (m_bReturn) {
                s.erase(s.begin(), s.begin() + 1); //remove '?'
                Utilities::Trim(s);
                if (s.front() != '=')
                    return false;
                s.erase(s.begin(), s.begin() + 1); //remove '='
                Utilities::Trim(s);
            }
            CDBString s_copy = s;
            transform(s.begin(), s.end(), s.begin(), ::tolower);
#ifdef WIN32_64
            m_bCall = (s.find(L"call ") == 0);
#else
            m_bCall = (s.find(u"call ") == 0);
#endif
            if (m_bCall) {
                auto pos = s_copy.find('(');
                if (pos != CDBString::npos) {
                    if (s_copy.back() != ')')
                        return false;
                    m_procName.assign(s_copy.begin() + 5, s_copy.begin() + pos);
                } else {
                    if (s_copy.back() == ')')
                        return false;
                    m_procName = s_copy.substr(5);
                }
                Utilities::Trim(m_procName);
                pos = m_procName.rfind('.');
                if (pos != CDBString::npos) {
                    m_procCatalogSchema = m_procName.substr(0, pos);
                    Utilities::Trim(m_procCatalogSchema);
                    m_procName = m_procName.substr(pos + 1);
                    Utilities::Trim(m_procName);
                }
            } else {
                return false;
            }
            return true;
        }

        void COdbcImpl::ToDecimal(const SQL_NUMERIC_STRUCT &num, DECIMAL & dec) {
            dec.wReserved = 0;
            if (!num.sign)
                dec.sign = 0x80;
            else
                dec.sign = 0;
            dec.scale = num.scale;
            UINT64 *pll = (UINT64 *) num.val;
            dec.Lo64 = *pll;
            if (pll[1]) {
                dec.Hi32 = (unsigned int) pll[1];
            } else {
                dec.Hi32 = 0;
            }
        }

        void COdbcImpl::SaveSqlServerVariant(const unsigned char *buffer, unsigned int bytes, SQLSMALLINT c_type, CUQueue & q) {
            switch (c_type) {
                case SQL_C_WCHAR:
                    q << (VARTYPE) VT_BSTR;
                    q << bytes;
                    q.Push(buffer, bytes);
                    break;
                case SQL_C_CHAR:
                    q << (VARTYPE) (VT_ARRAY | VT_I1) << bytes;
                    q.Push(buffer, bytes);
                    break;
                case SQL_C_GUID:
                    q << (VARTYPE) VT_CLSID;
                    assert(bytes == sizeof (GUID));
                    q.Push(buffer, bytes);
                    break;
                case SQL_C_BINARY:
                    q << (VARTYPE) (VT_ARRAY | VT_UI1) << bytes;
                    q.Push(buffer, bytes);
                    break;
                case SQL_NUMERIC:
                    if (bytes == sizeof (SQL_NUMERIC_STRUCT)) {
                        DECIMAL dec;
                        SQL_NUMERIC_STRUCT *num = (SQL_NUMERIC_STRUCT*) buffer;
                        ToDecimal(*num, dec);
                        q << (VARTYPE) VT_DECIMAL << dec;
                    } else {
                        DECIMAL dec;
                        memset(&dec, 0, sizeof (dec));
                        q << (VARTYPE) VT_DECIMAL << dec;
                    }
                    break;
                case SQL_C_SSHORT:
                    q << (VARTYPE) VT_I2 << *((short*) buffer);
                    break;
                case SQL_C_USHORT:
                    q << (VARTYPE) VT_UI2 << *((unsigned short*) buffer);
                    break;
                case SQL_C_FLOAT:
                    q << (VARTYPE) VT_R4 << *((float*) buffer);
                    break;
                case SQL_C_DOUBLE:
                    q << (VARTYPE) VT_R8 << *((double*) buffer);
                    break;
                case SQL_C_STINYINT:
                    q << (VARTYPE) VT_I1 << *((char*) buffer);
                    break;
                case SQL_C_UTINYINT:
                    q << (VARTYPE) VT_UI1 << *((unsigned char*) buffer);
                    break;
                case SQL_C_SBIGINT:
                    q << (VARTYPE) VT_I8 << *((INT64*) buffer);
                    break;
                case SQL_C_UBIGINT:
                    q << (VARTYPE) VT_UI8 << *((UINT64*) buffer);
                    break;
                case SQL_C_SLONG:
                    q << (VARTYPE) VT_I4 << *((int*) buffer);
                    break;
                case SQL_C_ULONG:
                    q << (VARTYPE) VT_UI4 << *((unsigned int*) buffer);
                    break;
                case SQL_C_TIMESTAMP:
                {
                    std::tm tm;
                    TIMESTAMP_STRUCT *dt = (TIMESTAMP_STRUCT*) buffer;
                    unsigned int us = ToCTime(*dt, tm);
                    UDateTime udt(tm, us);
                    q << (VARTYPE) VT_DATE << udt.time;
                }
                    break;
                case SQL_C_TYPE_TIMESTAMP:
                {
                    std::tm tm;
                    SQL_TIMESTAMP_STRUCT *dt = (SQL_TIMESTAMP_STRUCT*) buffer;
                    unsigned int us = ToCTime(*dt, tm);
                    UDateTime udt(tm, us);
                    q << (VARTYPE) VT_DATE << udt.time;
                }
                    break;
                case SQL_C_TYPE_DATE:
                {
                    std::tm tm;
                    SQL_DATE_STRUCT *dt = (SQL_DATE_STRUCT*) buffer;
                    unsigned int us = ToCTime(*dt, tm);
                    UDateTime udt(tm, us);
                    q << (VARTYPE) VT_DATE << udt.time;
                }
                    break;
                case SQL_C_TYPE_TIME:
                {
                    std::tm tm;
                    SQL_TIME_STRUCT *dt = (SQL_TIME_STRUCT*) buffer;
                    unsigned int us = ToCTime(*dt, tm);
                    UDateTime udt(tm, us);
                    q << (VARTYPE) VT_DATE << udt.time;
                }
                    break;
                case SQL_C_BIT:
                    q << (VARTYPE) VT_BOOL << (VARIANT_BOOL) (buffer[0] ? VARIANT_TRUE : VARIANT_FALSE);
                    break;
                default:
                    assert(false);
                    break;
            }
        }

        bool COdbcImpl::PushRecords(SQLHSTMT hstmt, int &res, CDBString & errMsg) {
            assert(!m_Blob.GetSize());
            m_Blob.SetSize(0);
            unsigned int size = DEFAULT_BIG_FIELD_CHUNK_SIZE;
            if (size < m_nRecordSize)
                size = m_nRecordSize;
            unsigned int rowset_size = size / m_nRecordSize;
            if (m_Blob.GetMaxSize() < size) {
                m_Blob.ReallocBuffer(size);
            }
            SQLRETURN retcode = SQLSetStmtAttr(hstmt, SQL_ROWSET_SIZE, (void*) rowset_size, 0);
            if (!SQL_SUCCEEDED(retcode)) {
                res = Odbc::ER_ERROR;
                GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                return false;
            }
            retcode = SQLSetStmtAttr(hstmt, SQL_ATTR_ROW_BIND_TYPE, (void*) m_nRecordSize, 0);
            if (!SQL_SUCCEEDED(retcode)) {
                res = Odbc::ER_ERROR;
                GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                return false;
            }
            SQLUSMALLINT cols = (SQLUSMALLINT) m_vBindInfo.size();
            SQLUSMALLINT col = 1;
            unsigned char *start = (unsigned char*) m_Blob.GetBuffer();
            for (auto it = m_vBindInfo.begin(), end = m_vBindInfo.end(); it != end; ++it) {
                unsigned char *p = start + it->Offset;
                SQLLEN *ind = (SQLLEN*) (start + it->Offset + it->BufferSize);
                switch (it->DataType) {
                    case VT_VARIANT:
                    case VT_BSTR:
                    case VT_XML:
                        retcode = SQLBindCol(hstmt, col, SQL_C_WCHAR, p, it->BufferSize, ind);
                        break;
                    case (VT_I1 | VT_ARRAY):
                        retcode = SQLBindCol(hstmt, col, SQL_C_CHAR, p, it->BufferSize, ind);
                        break;
                    case VT_DATE:
                        retcode = SQLBindCol(hstmt, col, SQL_C_CHAR, p, it->BufferSize, ind);
                        break;
                    case VT_DECIMAL:
                        retcode = SQLBindCol(hstmt, col, SQL_C_CHAR, p, it->BufferSize, ind);
                        break;
                    case VT_I1:
                        retcode = SQLBindCol(hstmt, col, SQL_C_TINYINT, p, it->BufferSize, ind);
                        break;
                    case VT_UI1:
                        retcode = SQLBindCol(hstmt, col, SQL_C_UTINYINT, p, it->BufferSize, ind);
                        break;
                    case VT_I2:
                        retcode = SQLBindCol(hstmt, col, SQL_C_SHORT, p, it->BufferSize, ind);
                        break;
                    case VT_UI2:
                        retcode = SQLBindCol(hstmt, col, SQL_C_USHORT, p, it->BufferSize, ind);
                        break;
                    case VT_I4:
                        retcode = SQLBindCol(hstmt, col, SQL_C_LONG, p, it->BufferSize, ind);
                        break;
                    case VT_UI4:
                        retcode = SQLBindCol(hstmt, col, SQL_C_ULONG, p, it->BufferSize, ind);
                        break;
                    case VT_R4:
                        retcode = SQLBindCol(hstmt, col, SQL_C_FLOAT, p, it->BufferSize, ind);
                        break;
                    case VT_R8:
                        retcode = SQLBindCol(hstmt, col, SQL_C_DOUBLE, p, it->BufferSize, ind);
                        break;
                    case VT_I8:
                        retcode = SQLBindCol(hstmt, col, SQL_C_SBIGINT, p, it->BufferSize, ind);
                        break;
                    case VT_UI8:
                        retcode = SQLBindCol(hstmt, col, SQL_C_UBIGINT, p, it->BufferSize, ind);
                        break;
                    case VT_BOOL:
                        retcode = SQLBindCol(hstmt, col, SQL_C_BIT, p, it->BufferSize, ind);
                        break;
                    case VT_CLSID:
                        retcode = SQLBindCol(hstmt, col, SQL_C_GUID, p, it->BufferSize, ind);
                        break;
                    case (VT_UI1 | VT_ARRAY):
                        retcode = SQLBindCol(hstmt, col, SQL_C_BINARY, p, it->BufferSize, ind);
                        break;
                    default:
                        assert(false);
                        break;
                }
                ++col;
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    SQLFreeStmt(hstmt, SQL_UNBIND);
                    return false;
                }
            }
            SQLULEN rows = 0;
            m_UQueue.SetSize(0);
            if (m_UQueue.GetMaxSize() < rowset_size * sizeof (SQLUSMALLINT)) {
                m_UQueue.ReallocBuffer(rowset_size * sizeof (SQLUSMALLINT));
            }
            SQLUSMALLINT *pRowStatus = (SQLUSMALLINT *) m_UQueue.GetBuffer();
            CScopeUQueue sb;
            CUQueue &q = *sb;
            while ((retcode = SQLExtendedFetch(hstmt, SQL_FETCH_NEXT, 0, &rows, pRowStatus)) == SQL_SUCCESS) {
                for (SQLULEN r = 0; r < rows; ++r) {
                    unsigned char *beginning = start + r * m_nRecordSize;
                    for (SQLUSMALLINT c = 0; c < cols; ++c) {
                        CBindInfo &info = m_vBindInfo[c];
                        SQLLEN *ind = (SQLLEN *) (beginning + info.Offset + info.BufferSize);
                        if (*ind == SQL_NULL_DATA) {
                            q << (VARTYPE) VT_NULL;
                            continue;
                        }
                        unsigned char *header = beginning + info.Offset;
                        switch (info.DataType) {
                            case VT_BSTR:
                            case VT_VARIANT:
                            case VT_XML:
                            {
                                unsigned int len = (unsigned int) (*ind);
                                q << (VARTYPE) VT_BSTR;
                                const SQLWCHAR *s = (const SQLWCHAR*) header;
                                q << len;
                                q.Push((const unsigned char*) s, len);
                            }
                                break;
                            case (VT_UI1 | VT_ARRAY):
                            {
                                const unsigned char *s = (const unsigned char*) header;
                                unsigned int len = (unsigned int) (*ind);
                                q << info.DataType << len;
                                q.Push(s, len);
                            }
                                break;
                            case VT_BOOL:
                            {
                                const unsigned char *s = (const unsigned char*) header;
                                q << info.DataType;
                                VARIANT_BOOL b = (*s) ? VARIANT_TRUE : VARIANT_FALSE;
                                q << b;
                            }
                                break;
                            case (VT_I1 | VT_ARRAY):
                            {
                                q << info.DataType;
                                const char *s = (const char*) header;
                                unsigned int len = (unsigned int) (*ind);
                                q << len;
                                q.Push((const unsigned char*) s, len);
                            }
                                break;
                            case VT_DECIMAL:
                            {
                                q << info.DataType;
                                const char *s = (const char*) header;
                                unsigned int len = (unsigned int) (*ind);
                                DECIMAL dec;
                                if (len <= 19)
                                    ParseDec(s, dec);
                                else
                                    ParseDec_long(s, dec);
                                q << dec;
                            }
                                break;
                            case VT_DATE:
                            {
                                q << info.DataType;
                                const char *s = (const char*) header;
                                UDateTime dt(s);
                                q << dt.time;
                            }
                                break;
                            default:
                                q << info.DataType;
                                q.Push((const unsigned char *) header, info.BufferSize);
                                break;
                        }
                    }
                }
                if (q.GetSize() && !SendRows(q)) {
                    SQLFreeStmt(hstmt, SQL_UNBIND);
                    return false;
                }
                if (rows < rowset_size)
                    break;
            }
            if (!SQL_SUCCEEDED(retcode) && retcode != SQL_NO_DATA) {
                res = Odbc::ER_ERROR;
                GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                SQLFreeStmt(hstmt, SQL_UNBIND);
                return false;
            }
            retcode = SQLFreeStmt(hstmt, SQL_UNBIND);
            return true;
        }

        bool COdbcImpl::PushRecords(SQLHSTMT hstmt, const CDBColumnInfoArray &vColInfo, bool output, int &res, CDBString & errMsg) {
            SQLRETURN retcode;
            CScopeUQueue sbTemp(MY_OPERATION_SYSTEM, IsBigEndian(), DEFAULT_BIG_FIELD_CHUNK_SIZE);
            size_t fields = vColInfo.size();
            CScopeUQueue sb;
            CUQueue &q = *sb;
            while (true) {
                retcode = SQLFetch(hstmt);
                if (retcode == SQL_NO_DATA) {
                    break;
                }
                bool blob = false;
                if (SQL_SUCCEEDED(retcode)) {
                    for (size_t i = 0; i < fields; ++i) {
                        SQLLEN len_or_null = 0;
                        const CDBColumnInfo &colInfo = vColInfo[i];
                        VARTYPE vt = colInfo.DataType;
                        switch (vt) {
                            case VT_BOOL:
                            {
                                unsigned char boolean = 0;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_BIT, &boolean, sizeof (boolean), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    VARIANT_BOOL ok = boolean ? VARIANT_TRUE : VARIANT_FALSE;
                                    q << vt << ok;
                                }
                            }
                                break;
                            case VT_XML:
                            case VT_BSTR:
                                if (vt == VT_XML) {
                                    vt = VT_BSTR;
                                }
                                if (colInfo.ColumnSize >= DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                    if (!SendUText(hstmt, (SQLUSMALLINT) (i + 1), *sbTemp, q, blob)) {
                                        return false;
                                    }
                                } else {
                                    unsigned int max = (colInfo.ColumnSize * sizeof (SQLWCHAR));
                                    if (q.GetTailSize() < sizeof (unsigned int) + sizeof (VARTYPE) + max + sizeof (SQLWCHAR)) {
                                        q.ReallocBuffer(q.GetMaxSize() + max + sizeof (unsigned int) + sizeof (VARTYPE) + sizeof (SQLWCHAR));
                                    }
                                    VARTYPE *pvt = (VARTYPE *) q.GetBuffer(q.GetSize());
                                    unsigned int *plen = (unsigned int*) (pvt + 1);
                                    unsigned char *pos = (unsigned char*) (plen + 1);
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_WCHAR, pos, q.GetTailSize() - sizeof (unsigned int) - sizeof (VARTYPE), &len_or_null);
                                    if (SQL_NULL_DATA == len_or_null) {
                                        q << (VARTYPE) VT_NULL;
                                    } else {
                                        *pvt = vt;
                                        *plen = (unsigned int) len_or_null;
                                        q.SetSize(q.GetSize() + *plen + sizeof (unsigned int) + sizeof (VARTYPE));
                                    }
                                }
                                break;
                            case VT_DATE:
                                switch ((SQLSMALLINT) colInfo.ColumnSize) {
                                    case SQL_TYPE_DATE:
                                    {
                                        DATE_STRUCT d;
                                        retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_TYPE_DATE, &d, sizeof (d), &len_or_null);
                                        if (len_or_null == SQL_NULL_DATA) {
                                            q << (VARTYPE) VT_NULL;
                                        } else {
                                            q << vt;
                                            std::tm st;
                                            unsigned int us = ToCTime(d, st);
                                            UDateTime dt(st, us);
                                            q << dt.time;
                                        }
                                    }
                                        break;
                                    case SQL_TYPE_TIME:
                                    {
                                        TIME_STRUCT d;
                                        retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_TYPE_TIME, &d, sizeof (d), &len_or_null);
                                        if (len_or_null == SQL_NULL_DATA) {
                                            q << (VARTYPE) VT_NULL;
                                        } else {
                                            q << vt;
                                            std::tm st;
                                            unsigned int us = ToCTime(d, st);
                                            UDateTime dt(st, us);
                                            q << dt.time;
                                        }
                                    }
                                        break;
                                    case SQL_TYPE_TIMESTAMP:
                                    {
                                        TIMESTAMP_STRUCT d;
                                        retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_TYPE_TIMESTAMP, &d, sizeof (d), &len_or_null);
                                        if (len_or_null == SQL_NULL_DATA) {
                                            q << (VARTYPE) VT_NULL;
                                        } else {
                                            q << vt;
                                            std::tm st;
                                            unsigned int us = ToCTime(d, st);
                                            UDateTime dt(st, us);
                                            q << dt.time;
                                        }
                                    }
                                        break;
                                    case SQL_INTERVAL_MONTH:
                                        break;
                                    case SQL_INTERVAL_YEAR:
                                        break;
                                    case SQL_INTERVAL_YEAR_TO_MONTH:
                                        break;
                                    case SQL_INTERVAL_DAY:
                                        break;
                                    case SQL_INTERVAL_HOUR:
                                        break;
                                    case SQL_INTERVAL_MINUTE:
                                        break;
                                    case SQL_INTERVAL_SECOND:
                                        break;
                                    case SQL_INTERVAL_DAY_TO_HOUR:
                                        break;
                                    case SQL_INTERVAL_DAY_TO_MINUTE:
                                        break;
                                    case SQL_INTERVAL_DAY_TO_SECOND:
                                        break;
                                    case SQL_INTERVAL_HOUR_TO_MINUTE:
                                        break;
                                    case SQL_INTERVAL_HOUR_TO_SECOND:
                                        break;
                                    case SQL_INTERVAL_MINUTE_TO_SECOND:
                                        break;
                                    default:
                                        assert(false); //shouldn't come here
                                        break;
                                }
                                break;
                            case VT_I1:
                            {
                                char d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_TINYINT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt;
                                    q.Push((const unsigned char*) &d, sizeof (d));
                                }
                            }
                                break;
                            case VT_UI1:
                            {
                                unsigned char d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_UTINYINT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt;
                                    q.Push((const unsigned char*) &d, sizeof (d));
                                }
                            }
                                break;
                            case VT_I2:
                            {
                                short d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_SHORT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_UI2:
                            {
                                unsigned short d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_USHORT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_I4:
                            {
                                int d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_LONG, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_UI4:
                            {
                                unsigned int d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_ULONG, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_R4:
                            {
                                float d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_FLOAT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_I8:
                            {
                                INT64 d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_SBIGINT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_UI8:
                            {
                                UINT64 d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_UBIGINT, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_CLSID:
                            {
                                GUID d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_GUID, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case VT_R8:
                            {
                                double d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_DOUBLE, &d, sizeof (d), &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    q << vt << d;
                                }
                            }
                                break;
                            case (VT_ARRAY | VT_I1):
                                if (colInfo.ColumnSize < 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_CHAR, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                    if (SQL_NULL_DATA == len_or_null) {
                                        q << (VARTYPE) VT_NULL;
                                    } else {
                                        q << vt << (unsigned int) len_or_null;
                                        q.Push(sbTemp->GetBuffer(), (unsigned int) len_or_null);
                                    }
                                } else {
                                    if (!SendBlob(hstmt, (SQLUSMALLINT) (i + 1), vt, *sbTemp, q, blob)) {
                                        return false;
                                    }
                                }
                                break;
                            case (VT_ARRAY | VT_UI1):
                                if (colInfo.Precision == sizeof (SQLGUID)) {
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_GUID, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                    if (SQL_NULL_DATA == len_or_null) {
                                        q << (VARTYPE) VT_NULL;
                                    } else {
                                        q << vt;
                                        q.Push(sbTemp->GetBuffer(), sizeof (SQLGUID));
                                    }
                                } else if (colInfo.ColumnSize < 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_BINARY, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                    if (SQL_NULL_DATA == len_or_null) {
                                        q << (VARTYPE) VT_NULL;
                                    } else {
                                        q << vt << (unsigned int) len_or_null;
                                        q.Push(sbTemp->GetBuffer(), (unsigned int) len_or_null);
                                    }
                                } else {
                                    if (!SendBlob(hstmt, (SQLUSMALLINT) (i + 1), vt, *sbTemp, q, blob)) {
                                        return false;
                                    }
                                }
                                break;
                            case VT_DECIMAL:
                                switch ((SQLSMALLINT) colInfo.ColumnSize) {
                                    case SQL_NUMERIC:
                                    case SQL_DECIMAL:
                                    {
                                        char str[DECIMAL_STRING_BUFFER_SIZE] = {0};
                                        retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_CHAR, (SQLPOINTER) str, sizeof (str), &len_or_null);
                                        if (len_or_null == SQL_NULL_DATA) {
                                            q << (VARTYPE) VT_NULL;
                                        } else {
                                            DECIMAL dec;
                                            if (len_or_null <= 19)
                                                ParseDec(str, dec);
                                            else
                                                ParseDec_long(str, dec);
                                            q << vt << dec;
                                        }
                                    }
                                        break;
                                    default:
                                        assert(false); //shouldn't come here
                                        break;
                                }
                                break;
                            case VT_VARIANT:
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_BINARY, (SQLPOINTER) sbTemp->GetBuffer(), 0, &len_or_null);
                                if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    assert(retcode == SQL_SUCCESS_WITH_INFO);
                                    SQLLEN c_type = 0;
                                    retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (i + 1), SQL_CA_SS_VARIANT_TYPE, nullptr, 0, nullptr, &c_type);
                                    assert(SQL_SUCCEEDED(retcode));
                                    SQLLEN mylen;
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), (SQLSMALLINT) c_type, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &mylen);
                                    assert(SQL_SUCCEEDED(retcode));
                                    SaveSqlServerVariant(sbTemp->GetBuffer(), (unsigned int) mylen, (SQLSMALLINT) c_type, q);
                                }
                                break;
                            default:
                            {
                                if (sb->GetMaxSize() < 16 * 1024) {
                                    sb->ReallocBuffer(16 * 1024);
                                }
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_WCHAR, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                if (!SQL_SUCCEEDED(retcode)) {
                                    break;
                                } else if (len_or_null == SQL_NULL_DATA) {
                                    q << (VARTYPE) VT_NULL;
                                } else {
                                    unsigned int len = (unsigned int) len_or_null;
                                    q << (VARTYPE) VT_BSTR << len;
                                    q.Push(sbTemp->GetBuffer(), len);
                                    sbTemp->SetSize(0);
                                }
                            }
                                break;
                        } //for loop
                        if (!SQL_SUCCEEDED(retcode)) {
                            res = Odbc::ER_ERROR;
                            GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                        }
                    }
                } else {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    break;
                }
                if ((q.GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(q)) {
                    return false;
                }
            } //while loop
            if (SQL_NO_DATA == retcode || SQL_SUCCEEDED(retcode)) {
                if (output) {
                    //tell output parameter data
                    unsigned int res = SendResult(idOutputParameter, q.GetBuffer(), q.GetSize());
                    if (res == REQUEST_CANCELED || res == SOCKET_NOT_FOUND) {
                        return false;
                    }
                } else if (q.GetSize()) {
                    return SendRows(q);
                }
            }
            return true;
        }

        void COdbcImpl::DoSQLColumnPrivileges(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                std::string coln = Utilities::ToUTF8(columnName.c_str(), columnName.size());
                retcode = SQLColumnPrivileges(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size(),
                        (SQLCHAR*) coln.c_str(), (SQLSMALLINT) coln.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLTables(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& tableType, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                std::string tt = Utilities::ToUTF8(tableType.c_str(), tableType.size());
                retcode = SQLTables(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size(),
                        (SQLCHAR*) tt.c_str(), (SQLSMALLINT) tt.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLColumns(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                std::string coln = Utilities::ToUTF8(columnName.c_str(), columnName.size());
                retcode = SQLColumns(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size(),
                        (SQLCHAR*) coln.c_str(), (SQLSMALLINT) coln.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLProcedureColumns(const CDBString& catalogName, const CDBString& schemaName, const CDBString& procName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string pn = Utilities::ToUTF8(procName.c_str(), procName.size());
                std::string coln = Utilities::ToUTF8(columnName.c_str(), columnName.size());
                size_t col_size = coln.size();
                retcode = SQLProcedureColumns(hstmt, cn.size() ? (SQLCHAR*) cn.c_str() : nullptr, (SQLSMALLINT) cn.size(),
                        sn.size() ? (SQLCHAR*) sn.c_str() : nullptr, (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) pn.c_str(), (SQLSMALLINT) pn.size(),
                        col_size ? (SQLCHAR*) coln.c_str() : nullptr, (SQLSMALLINT) col_size);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                if (!m_pNoSending) {
                    unsigned int ret = SendResult(idRowsetHeader, vInfo, index);
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return;
                    }
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLPrimaryKeys(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                retcode = SQLPrimaryKeys(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                if (!m_pNoSending) {
                    unsigned int ret = SendResult(idRowsetHeader, vInfo, index);
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return;
                    }
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLTablePrivileges(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                retcode = SQLTablePrivileges(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLStatistics(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, SQLUSMALLINT unique, SQLUSMALLINT reserved, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                retcode = SQLStatistics(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size(), unique, reserved);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLProcedures(const CDBString& catalogName, const CDBString& schemaName, const CDBString& procName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string pn = Utilities::ToUTF8(procName.c_str(), procName.size());
                retcode = SQLProcedures(hstmt, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) pn.c_str(), (SQLSMALLINT) pn.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLSpecialColumns(SQLSMALLINT identifierType, const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, SQLSMALLINT scope, SQLSMALLINT nullable, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string cn = Utilities::ToUTF8(catalogName.c_str(), catalogName.size());
                std::string sn = Utilities::ToUTF8(schemaName.c_str(), schemaName.size());
                std::string tn = Utilities::ToUTF8(tableName.c_str(), tableName.size());
                retcode = SQLSpecialColumns(hstmt, identifierType, (SQLCHAR*) cn.c_str(), (SQLSMALLINT) cn.size(),
                        (SQLCHAR*) sn.c_str(), (SQLSMALLINT) sn.size(),
                        (SQLCHAR*) tn.c_str(), (SQLSMALLINT) tn.size(), scope, nullable);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLForeignKeys(const CDBString& pkCatalogName, const CDBString& pkSchemaName, const CDBString& pkTableName, const CDBString& fkCatalogName, const CDBString& fkSchemaName, const CDBString& fkTableName, UINT64 index, int &res, CDBString &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                std::string pk_cn = Utilities::ToUTF8(pkCatalogName.c_str(), pkCatalogName.size());
                std::string pk_sn = Utilities::ToUTF8(pkSchemaName.c_str(), pkSchemaName.size());
                std::string pk_tn = Utilities::ToUTF8(pkTableName.c_str(), pkTableName.size());
                std::string fk_cn = Utilities::ToUTF8(fkCatalogName.c_str(), fkCatalogName.size());
                std::string fk_sn = Utilities::ToUTF8(fkSchemaName.c_str(), fkSchemaName.size());
                std::string fk_tn = Utilities::ToUTF8(fkTableName.c_str(), fkTableName.size());
                retcode = SQLForeignKeys(hstmt, (SQLCHAR*) pk_cn.c_str(), (SQLSMALLINT) pk_cn.size(),
                        (SQLCHAR*) pkSchemaName.c_str(), (SQLSMALLINT) pkSchemaName.size(),
                        (SQLCHAR*) pk_tn.c_str(), (SQLSMALLINT) pk_tn.size(),
                        (SQLCHAR*) fk_cn.c_str(), (SQLSMALLINT) fk_cn.size(),
                        (SQLCHAR*) fk_sn.c_str(), (SQLSMALLINT) fk_sn.size(),
                        (SQLCHAR*) fk_sn.c_str(), (SQLSMALLINT) fk_tn.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, false);
                ret = SendResult(idRowsetHeader, vInfo, index);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok;
                if (m_nRecordSize)
                    ok = PushRecords(hstmt, res, errMsg);
                else
                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        CDBString COdbcImpl::GenerateMsSqlForCachedTables() {
            CScopeUQueue sb;
            CUQueue &q = *sb;
            bool rowset = true, lastInsertId = true;
            UINT64 index = 0, fail_ok = 0;
            INT64 affected = 0;
            int res = 0;
            CDBString errMsg, strSqlCache;
            CDBVariant vtId;
#ifdef WIN32_64
            CDBString sql = L"SELECT name FROM master.dbo.sysdatabases where name NOT IN('master','tempdb','model','msdb')";
#else
            CDBString sql = u"SELECT name FROM master.dbo.sysdatabases where name NOT IN('master','tempdb','model','msdb')";
#endif
            m_pNoSending = &q;
            do {
                Execute(sql, rowset, false, lastInsertId, index, affected, res, errMsg, vtId, fail_ok);
                if (res)
                    break;
                UDB::CDBVariantArray vDb;
                while (q.GetSize()) {
                    CDBVariant vt;
                    q >> vt;
                    vDb.push_back(std::move(vt));
                }
                for (auto it = vDb.cbegin(), end = vDb.cend(); it != end; ++it) {
#ifdef WIN32_64
                    sql = L"USE [";
                    sql += it->bstrVal;
                    sql += L"];";
                    sql += L"select object_schema_name(parent_id),OBJECT_NAME(parent_id)from sys.assembly_modules as am,sys.triggers as t where t.object_id=am.object_id and assembly_method like 'PublishDMLEvent%' and assembly_class='USqlStream'";
#else
                    sql = u"USE [";
                    sql += Utilities::ToUTF16(it->bstrVal);
                    sql += u"];";
                    sql += u"select object_schema_name(parent_id),OBJECT_NAME(parent_id)from sys.assembly_modules as am,sys.triggers as t where t.object_id=am.object_id and assembly_method like 'PublishDMLEvent%' and assembly_class='USqlStream'";
#endif
                    Execute(sql, rowset, false, lastInsertId, index, affected, res, errMsg, vtId, fail_ok);
                    if (res)
                        continue;
                    if (strSqlCache.size())
                        strSqlCache.push_back(';');
                    CDBVariant vtSchema, vtTable;
                    while (q.GetSize()) {
                        q >> vtSchema >> vtTable;
#ifdef WIN32_64
                        strSqlCache += L"SELECT * FROM [";
                        strSqlCache += it->bstrVal;
                        strSqlCache += L"].[";
                        strSqlCache += vtSchema.bstrVal;
                        strSqlCache += L"].[";
                        strSqlCache += vtTable.bstrVal;
                        strSqlCache += L"] FOR BROWSE";
#else
                        strSqlCache += u"SELECT * FROM [";
                        strSqlCache += Utilities::ToUTF16(it->bstrVal);
                        strSqlCache += u"].[";
                        strSqlCache += Utilities::ToUTF16(vtSchema.bstrVal);
                        strSqlCache += u"].[";
                        strSqlCache += Utilities::ToUTF16(vtTable.bstrVal);
                        strSqlCache += u"] FOR BROWSE";
#endif
                        if (q.GetSize())
                            strSqlCache.push_back(';');
                    }
                }
            } while (false);
#ifdef WIN32_64
            sql = L"USE [" + m_dbName + L"]";
#else
            sql = u"USE [" + m_dbName + u"]";
#endif
            Execute(sql, false, false, lastInsertId, index, affected, res, errMsg, vtId, fail_ok);
            m_pNoSending = nullptr;
            return strSqlCache;
        }

        void COdbcImpl::Execute(const CDBString& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            affected = 0;
            fail_ok = 0;
            ResetMemories();
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            CDBString sql = wsql;
            if (m_EnableMessages && !sql.size()) {
                switch (m_msDriver) {
                    case msMsSQL:
                        sql = GenerateMsSqlForCachedTables();
                        break;
                    default:
                        break;
                }
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                std::shared_ptr<void> pStmt(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                m_pExcuting = pStmt;
                if (meta) {
                    switch (m_msDriver) {
                        case msMsSQL:
                            retcode = SQLSetStmtAttr(hstmt, SQL_SOPT_SS_HIDDEN_COLUMNS, (SQLPOINTER) SQL_HC_ON, 0);
                            break;
                        default:
                            break;
                    }
                }
                retcode = SQLExecDirectW(hstmt, (SQLWCHAR*) sql.c_str(), (SQLINTEGER) sql.size());
                if (!SQL_SUCCEEDED(retcode) && retcode != SQL_NO_DATA) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                do {
                    SQLSMALLINT columns = 0;
                    retcode = SQLNumResultCols(hstmt, &columns);
                    if (!SQL_SUCCEEDED(retcode)) {
                        break;
                    }
                    if (columns > 0) {
                        if (rowset || meta) {
                            CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, meta);
                            if (!m_pNoSending) {
                                unsigned int ret = SendResult(idRowsetHeader, vInfo, index);
                                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                                    return;
                                }
                            }
                            bool ok;
                            if (rowset) {
                                if (m_nRecordSize) {
                                    ok = PushRecords(hstmt, res, errMsg);
                                } else {
                                    ok = PushRecords(hstmt, vInfo, false, res, errMsg);
                                }
                            } else {
                                ok = true;
                            }
                            ++m_oks;
                            if (!ok) {
                                return;
                            }
                        }
                    } else {
                        SQLLEN rows = 0;
                        retcode = SQLRowCount(hstmt, &rows);
                        assert(SQL_SUCCEEDED(retcode));
                        if (rows > 0) {
                            affected += rows;
                        }
                        ++m_oks;
                    }
                } while ((retcode = SQLMoreResults(hstmt)) == SQL_SUCCESS);
                if (!SQL_SUCCEEDED(retcode) && retcode != SQL_NO_DATA) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                } else {
                    assert(retcode == SQL_NO_DATA || retcode == SQL_SUCCESS_WITH_INFO);
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::SetOracleCallParams(const std::vector<tagParameterDirection> &vPD, int &res, CDBString & errMsg) {
#ifdef WIN32_64
            CDBString sql(L"SELECT in_out,data_type FROM SYS.ALL_ARGUMENTS WHERE data_type<>'REF CURSOR' AND owner='");
            std::transform(m_userName.begin(), m_userName.end(), m_userName.begin(), ::toupper);
            sql += m_userName;
            sql += L"' AND object_name='";
            std::transform(m_procName.begin(), m_procName.end(), m_procName.begin(), ::toupper);
            sql += m_procName;
            if (m_procCatalogSchema.size()) {
                sql += L"' AND package_name='";
                std::transform(m_procCatalogSchema.begin(), m_procCatalogSchema.end(), m_procCatalogSchema.begin(), ::toupper);
                sql += m_procCatalogSchema;
                sql += L"'";
            } else {
                sql += L"' AND package_name IS NULL";
            }
            sql += L" ORDER BY position";
#else
            CDBString sql(u"SELECT in_out,data_type FROM SYS.ALL_ARGUMENTS WHERE data_type<>'REF CURSOR' AND owner='");
            std::transform(m_userName.begin(), m_userName.end(), m_userName.begin(), ::toupper);
            sql += m_userName;
            sql += u"' AND object_name='";
            std::transform(m_procName.begin(), m_procName.end(), m_procName.begin(), ::toupper);
            sql += m_procName;
            if (m_procCatalogSchema.size()) {
                sql += u"' AND package_name='";
                std::transform(m_procCatalogSchema.begin(), m_procCatalogSchema.end(), m_procCatalogSchema.begin(), ::toupper);
                sql += m_procCatalogSchema;
                sql += u"'";
            } else {
                sql += u"' AND package_name IS NULL";
            }
            sql += u" ORDER BY position";
#endif
            CDBVariant vtId;
            UINT64 index = 0, fail_ok = 0;
            INT64 affected = 0;
            CScopeUQueue sb;
            CUQueue &q = *sb;
            m_pNoSending = &q;
            Execute(sql, true, true, false, index, affected, res, errMsg, vtId, fail_ok);
            m_pNoSending = nullptr;
            if (res) {
                return;
            }
            CDBVariantArray vData;
            while (q.GetSize()) {
                CDBVariant vt;
                q >> vt;
                vData.push_back(std::move(vt));
            }

            static CDBVariant vtIN(L"IN");
            static CDBVariant vtOUT(L"OUT");
            static CDBVariant vtINOUT(L"IN/OUT");
            size_t cols = m_vBindInfo.size();
            SQLSMALLINT parameters = (SQLSMALLINT) (vData.size() / cols);
            for (SQLSMALLINT r = 0, k = 0; r < parameters && k < (SQLSMALLINT) vPD.size(); ++r, ++k) {
                CParameterInfo pi;
                unsigned int pos = (unsigned int) (r * cols);
                CDBVariant &vt = vData[pos];
                if (vt == vtIN) {
                    if (vPD[k] == pdUnknown)
                        continue;
                } else if (vt == vtOUT) {
                    if (vPD[k] == pdUnknown)
                        continue;
                    pi.Direction = pdOutput;
                } else if (vt == vtINOUT) {
                    if (vPD[k] == pdUnknown)
                        continue;
                    pi.Direction = pdInputOutput;
                } else {
                    assert(false); //shouldn't come here
                }
                pos = (unsigned int) (r * cols + 1);
                std::wstring dt = vData[pos].bstrVal;
                if (dt == L"NUMBER") {
                    pi.DataType = VT_DECIMAL;
                    pi.Precision = MAX_DECIMAL_PRECISION;
                } else if (dt == L"BINARY_FLOAT") {
                    pi.DataType = VT_R4;
                } else if (dt == L"BINARY_DOUBLE") {
                    pi.DataType = VT_R8;
                } else if (dt == L"CHAR") {
                    pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                    pi.DataType = (VT_ARRAY | VT_I1);
                } else if (dt == L"VARCHAR") {
                    pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                    pi.DataType = (VT_ARRAY | VT_I1);
                } else if (dt == L"VARCHAR2") {
                    pi.ColumnSize = MAX_ORACLE_VARCHAR2;
                    pi.DataType = (VT_ARRAY | VT_I1);
                } else if (dt == L"NCHAR") {
                    pi.DataType = VT_BSTR;
                    pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                } else if (dt == L"NVARCHAR2") {
                    pi.DataType = VT_BSTR;
                    pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                } else if (dt == L"DATE") {
                    pi.DataType = VT_DATE;
                } else if (dt == L"TIMESTAMP") {
                    pi.DataType = VT_DATE;
                    pi.Scale = MAX_TIME_DIGITS;
                } else if (dt == L"TIMESTAMP WITH TIME ZONE") {
                    pi.DataType = VT_DATE;
                    pi.Scale = MAX_TIME_DIGITS;
                } else if (dt == L"TIMESTAMP WITH LOCAL TIME ZONE") {
                    pi.DataType = VT_DATE;
                    pi.Scale = MAX_TIME_DIGITS;
                } else if (dt == L"LONG") {
                    pi.ColumnSize = (~0);
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else if (dt == L"BLOB") {
                    pi.ColumnSize = (~0);
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else if (dt == L"CLOB") {
                    pi.ColumnSize = (~0);
                    pi.DataType = (VT_ARRAY | VT_I1);
                } else if (dt == L"NCLOB") {
                    pi.ColumnSize = (~0);
                    pi.DataType = VT_BSTR;
                } else if (dt == L"RAW") {
                    pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else if (dt == L"LONG RAW") {
                    pi.ColumnSize = (~0);
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else if (dt == L"ROWID") {
                    pi.ColumnSize = DECIMAL_STRING_BUFFER_SIZE; //set to 32
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else if (dt == L"UROWID") {
                    pi.ColumnSize = DECIMAL_STRING_BUFFER_SIZE; //set to 32
                    pi.DataType = (VT_ARRAY | VT_UI1);
                } else {
                    continue; //not supported
                }
                m_vPInfo.push_back(pi);
            }
        }

        void COdbcImpl::SetCallParams(const std::vector<tagParameterDirection> &vPD, int &res, CDBString & errMsg) {
            res = 0;
            UINT64 index = 0, fail_ok = 0;
            CDBString column, schema, catalog;
            std::size_t pos = m_procCatalogSchema.find(L'.');
            if (pos != CDBString::npos) {
                schema = m_procCatalogSchema.substr(pos + 1);
                catalog = m_procCatalogSchema.substr(0, pos);
            } else {
                catalog = m_procCatalogSchema;
            }
            CScopeUQueue sb;
            CUQueue &q = *sb;
            m_pNoSending = &q;
            CDBVariantArray vData;
            DoSQLProcedureColumns(catalog, schema, m_procName, column, index, res, errMsg, fail_ok);
            m_pNoSending = nullptr;
            if (res) {
                return;
            }
            while (q.GetSize()) {
                CDBVariant vt;
                q >> vt;
                vData.push_back(std::move(vt));
            }
            size_t cols = m_vBindInfo.size();
            SQLSMALLINT parameters = (SQLSMALLINT) (vData.size() / cols);
            for (SQLSMALLINT r = 0, k = 0; r < parameters && k < (SQLSMALLINT) vPD.size(); ++r, ++k) {
                CParameterInfo pi;
                unsigned int pos = (unsigned int) (r * cols + 4);
                CDBVariant &vt = vData[pos];
                switch (vt.iVal) {
                    case SQL_PARAM_INPUT:
                        if (vPD[k] == pdUnknown)
                            continue;
                        break;
                    case SQL_PARAM_INPUT_OUTPUT:
                        if (vPD[k] == pdUnknown)
                            continue;
                        pi.Direction = pdInputOutput;
                        break;
                    case SQL_PARAM_OUTPUT:
                        if (vPD[k] == pdUnknown)
                            continue;
                        pi.Direction = pdOutput;
                        break;
                    case SQL_RETURN_VALUE:
                        pi.Direction = pdReturnValue;
                        if (!m_bReturn) {
                            --k;
                            continue;
                        }
                        break;
                    default: //SQL_PARAM_TYPE_UNKNOWN, SQL_RESULT_COL
                        assert(false); //unexpected
                        break;
                }
                pos = (unsigned int) (r * cols + 7);
                CDBVariant &vtSize = vData[pos];
                if (vtSize.Type() > VT_NULL) {
                    pi.ColumnSize = vtSize.uintVal;
                }
                pos = (unsigned int) (r * cols + 5);
                CDBVariant &dt = vData[pos];
                SQLSMALLINT coltype = dt.iVal;
                switch (coltype) {
                    case SQL_CLOB: //IBM DB2
                    case SQL_CHAR:
                    case SQL_VARCHAR:
                    case SQL_LONGVARCHAR:
                        pi.DataType = (VT_ARRAY | VT_I1);
                        if (!pi.ColumnSize)
                            pi.ColumnSize = (~0);
                        break;
                    case SQL_GRAPHIC: //IBM DB2
                    case SQL_VARGRAPHIC: //IBM DB2
                    case SQL_LONGVARGRAPHIC: //IBM DB2
                    case SQL_DBCLOB: //IBM DB2
                    case SQL_WCHAR:
                    case SQL_WVARCHAR:
                    case SQL_WLONGVARCHAR:
                        pi.DataType = VT_BSTR;
                        if (!pi.ColumnSize)
                            pi.ColumnSize = (~0);
                        break;
                    case SQL_BLOB:
                    case SQL_BINARY:
                    case SQL_VARBINARY:
                    case SQL_LONGVARBINARY:
                        pi.DataType = (VT_ARRAY | VT_UI1);
                        break;
                    case SQL_DECIMAL:
                    case SQL_NUMERIC:
                        pi.DataType = VT_DECIMAL;
                        break;
                    case SQL_SMALLINT:
                        pi.DataType = VT_I2;
                        break;
                    case SQL_INTEGER:
                        pi.DataType = VT_I4;
                        break;
                    case SQL_REAL:
                        pi.DataType = VT_R4;
                        break;
                    case SQL_FLOAT:
                    case SQL_DOUBLE:
                        pi.DataType = VT_R8;
                        break;
                    case SQL_TINYINT:
                        pi.DataType = VT_I1;
                        break;
                    case SQL_BIGINT:
                        pi.DataType = VT_I8;
                        break;
                    case SQL_BIT:
                        pi.DataType = VT_BOOL;
                        break;
                    case SQL_GUID:
                        pi.DataType = VT_CLSID;
                        break;
                    case SQL_TYPE_DATE:
                    case SQL_TYPE_TIME:
                    case SQL_TYPE_TIMESTAMP:
                        pi.DataType = VT_DATE;
                        break;
                    case SQL_INTERVAL_MONTH:
                    case SQL_INTERVAL_YEAR:
                    case SQL_INTERVAL_YEAR_TO_MONTH:
                    case SQL_INTERVAL_DAY:
                    case SQL_INTERVAL_HOUR:
                    case SQL_INTERVAL_MINUTE:
                    case SQL_INTERVAL_SECOND:
                    case SQL_INTERVAL_DAY_TO_HOUR:
                    case SQL_INTERVAL_DAY_TO_MINUTE:
                    case SQL_INTERVAL_DAY_TO_SECOND:
                    case SQL_INTERVAL_HOUR_TO_MINUTE:
                    case SQL_INTERVAL_HOUR_TO_SECOND:
                    case SQL_INTERVAL_MINUTE_TO_SECOND:
                        pi.DataType = (VT_ARRAY | VT_I1);
                        break;
                    case SQL_XML: //IBM DB2
                    case SQL_SS_XML: //SQL Server
                        pi.DataType = VT_XML;
                        pi.ColumnSize = (~0);
                        break;
                    case SQL_SS_VARIANT:
                        pi.DataType = VT_VARIANT;
                        pi.ColumnSize = DEFAULT_OUTPUT_BUFFER_SIZE;
                        break;
                    default:
                        pi.DataType = VT_BSTR;
                        pi.ColumnSize = DEFAULT_UNICODE_CHAR_SIZE;
                        break;
                }
                pos = (unsigned int) (r * cols + 9);
                CDBVariant &vtDigit = vData[pos];
                if (vtDigit.Type() > VT_NULL) {
                    pi.Scale = vtDigit.bVal;
                }
                m_vPInfo.push_back(pi);
            }
        }

        void COdbcImpl::Prepare(const CDBString& wsql, CParameterInfoArray& params, int &res, CDBString &errMsg, unsigned int &parameters) {
            ResetMemories();
            m_vPInfo = params;
            parameters = 0;
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            } else {
                res = SQL_SUCCESS;
            }
            m_parameters = 0;
            m_outputs = 0;
            m_pPrepare.reset();
            m_sqlPrepare = wsql;
            m_bCall = false;
            m_procName.clear();
            m_bReturn = false;
            m_procCatalogSchema.clear();
            Utilities::Trim(m_sqlPrepare);
            PreprocessPreparedStatement();
            SQLHSTMT hstmt = nullptr;
            SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_pOdbc.get(), &hstmt);
            do {
                m_pPrepare.reset(hstmt, [](SQLHSTMT h) {
                    if (h) {
                        SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, h);
                        assert(ret == SQL_SUCCESS);
                    }
                });
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    break;
                }
                m_pExcuting = m_pPrepare;
                retcode = SQLPrepareW(hstmt, (SQLWCHAR*) m_sqlPrepare.c_str(), (SQLINTEGER) m_sqlPrepare.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    break;
                }
                retcode = SQLNumParams(hstmt, &m_parameters);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    break;
                } else if (!m_parameters) {
                    res = Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
                    errMsg = BAD_PARAMETER_COLUMN_SIZE;
                    break;
                } else {
                    if (m_vPInfo.size()) {
                        if (m_bCall && m_parameters != (SQLSMALLINT) m_vPInfo.size()) {
                            res = Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
                            errMsg = BAD_PARAMETER_COLUMN_SIZE;
                            break;
                        }
                    } else if (m_bCall && m_bProcedureColumns) {
                        m_vPD = GetCallDirections(m_sqlPrepare);
                        if (m_bReturn) {
                            //{?=CALL .....}
                            m_vPD.insert(m_vPD.begin(), pdInput);
                        }
                        switch (m_msDriver) {
                            case msOracle:
                                SetOracleCallParams(m_vPD, res, errMsg);
                                break;
                            default:
                                if (m_bProcedureColumns) {
                                    SetCallParams(m_vPD, res, errMsg);
                                }
                                break;
                        }
                        if (res) {
                            break;
                        }
                        if (m_bCall && m_parameters != (SQLSMALLINT) m_vPInfo.size()) {
                            res = Odbc::ER_BAD_PARAMETER_COLUMN_SIZE;
                            errMsg = BAD_PARAMETER_COLUMN_SIZE;
                            break;
                        }
                    }
                }
                for (auto it = m_vPInfo.begin(), end = m_vPInfo.end(); it != end; ++it) {
                    if (m_bReturn && it == m_vPInfo.begin()) {
                        it->Direction = pdReturnValue;
                    }
                    switch (it->DataType) {
                        case VT_I1:
                        case VT_UI1:
                        case VT_I2:
                        case VT_UI2:
                        case VT_I4:
                        case VT_UI4:
                        case VT_INT:
                        case VT_UINT:
                        case VT_I8:
                        case VT_UI8:
                        case VT_R4:
                        case VT_R8:
                        case VT_DATE:
                        case VT_DECIMAL:
                        case VT_CLSID:
                        case VT_BSTR:
                        case (VT_UI1 | VT_ARRAY):
                        case (VT_I1 | VT_ARRAY):
                        case VT_BOOL:
                        case VT_VARIANT:
                        case VT_XML:
                            break;
                        default:
                            res = Odbc::ER_BAD_INPUT_PARAMETER_DATA_TYPE;
                            errMsg = BAD_INPUT_PARAMETER_DATA_TYPE;
                            break;
                    }
                    switch (it->Direction) {
                        case pdUnknown:
                            res = Odbc::ER_BAD_PARAMETER_DIRECTION_TYPE;
                            errMsg = BAD_PARAMETER_DIRECTION_TYPE;
                            break;
                        case pdInput:
                            break;
                        case pdInputOutput:
                            ++m_outputs;
                            break;
                        default:
                            ++m_outputs;
                            break;
                    }
                    if (res)
                        break;
                }
                if ((m_outputs || m_bReturn) && m_vPInfo.size() != (size_t) m_parameters) {
                    res = Odbc::ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET;
                    errMsg = CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET;
                    break;
                }
                parameters = m_outputs;
                parameters <<= 16;
                parameters += (unsigned short) m_parameters;
            } while (false);
            if (res) {
                m_pExcuting.reset();
                m_pPrepare.reset();
                m_vPInfo.clear();
                m_parameters = 0;
                m_outputs = 0;
                parameters = 0;
            }
        }

        bool COdbcImpl::SetInputParamInfo() {
            CParameterInfo info;
            unsigned int rows = (unsigned int) (m_vParam.size() / (unsigned short) m_parameters);
            for (SQLSMALLINT col = 0; col < m_parameters; ++col) {
                info.DataType = (VT_I1 | VT_ARRAY);
                for (unsigned int r = 0; r < rows; ++r) {
                    VARTYPE vt = m_vParam[r * (unsigned short) m_parameters + (unsigned short) col].vt;
                    if (vt > VT_NULL) {
                        switch (vt) {
                            case VT_I1:
                            case VT_UI1:
                            case VT_I2:
                            case VT_UI2:
                            case VT_I4:
                            case VT_UI4:
                            case VT_INT:
                            case VT_UINT:
                            case VT_I8:
                            case VT_UI8:
                            case VT_R4:
                            case VT_R8:
                            case VT_DATE:
                            case VT_DECIMAL:
#ifdef WIN32_64
                            case VT_BSTR:
#else
                            case (VT_UI2 | VT_ARRAY):
#endif
                            case (VT_I1 | VT_ARRAY):
                            case VT_BOOL:
                                info.DataType = vt;
                                break;
                            case (VT_UI1 | VT_ARRAY):
                                info.DataType = vt;
                            {
                                CDBVariant &d = m_vParam[r * (unsigned short) m_parameters + (unsigned short) col];
                                if (d.VtExt == vteGuid && d.parray->rgsabound->cElements == sizeof (GUID)) {
                                    info.DataType = VT_CLSID;
                                }
                            }
                                break;
                            default:
                                return false;
                                break;
                        }
                        break;
                    } //if
                } //for (unsigned int r
                m_vPInfo.push_back(info);
            } //for (SQLSMALLINT col
            return true;
        }

        unsigned int COdbcImpl::ComputeOutputMaxSize() {
            unsigned int max_size = 0;
            for (SQLSMALLINT col = 0; col < m_parameters; ++col) {
                CParameterInfo &info = m_vPInfo[col];
                switch (info.Direction) {
                    case UDB::pdOutput:
                    case UDB::pdReturnValue:
                    case UDB::pdInputOutput:
                        switch (info.DataType) {
                            case VT_CLSID:
                                info.ColumnSize = sizeof (GUID);
                                max_size += sizeof (GUID);
                                break;
                            case VT_DATE:
                                info.ColumnSize = DATETIME_STRING_BUFFER_SIZE;
                                max_size += DATETIME_STRING_BUFFER_SIZE;
                                break;
                            case VT_DECIMAL:
                                info.ColumnSize = DECIMAL_STRING_BUFFER_SIZE;
                                max_size += DECIMAL_STRING_BUFFER_SIZE;
                                break;
                            case VT_XML:
                            case VT_BSTR:
                                if (info.ColumnSize == 0) {
                                    info.ColumnSize = (DEFAULT_UNICODE_CHAR_SIZE + 1);
                                    max_size += info.ColumnSize * sizeof (SQLWCHAR);
                                } else if (info.ColumnSize > MAX_OUTPUT_BLOB_BUFFER_SIZE / sizeof (SQLWCHAR) + 1) {
                                    max_size += (MAX_OUTPUT_BLOB_BUFFER_SIZE + sizeof (SQLWCHAR));
                                    info.ColumnSize = (MAX_OUTPUT_BLOB_BUFFER_SIZE / sizeof (SQLWCHAR) + 1);
                                } else {
                                    max_size += info.ColumnSize * sizeof (SQLWCHAR);
                                }
                                break;
                            case (VT_I1 | VT_ARRAY):
                                if (info.ColumnSize == 0) {
                                    info.ColumnSize = (DEFAULT_OUTPUT_BUFFER_SIZE + 1);
                                } else if (info.ColumnSize >= MAX_OUTPUT_BLOB_BUFFER_SIZE) {
                                    info.ColumnSize = (MAX_OUTPUT_BLOB_BUFFER_SIZE + 1);
                                }
                                max_size += info.ColumnSize;
                                break;
                            case (VT_UI1 | VT_ARRAY):
                                if (info.ColumnSize == 0) {
                                    info.ColumnSize = DEFAULT_OUTPUT_BUFFER_SIZE;
                                } else if (info.ColumnSize > MAX_OUTPUT_BLOB_BUFFER_SIZE) {
                                    info.ColumnSize = MAX_OUTPUT_BLOB_BUFFER_SIZE;
                                }
                                max_size += info.ColumnSize;
                                break;
                            case VT_VARIANT:
                                info.ColumnSize = (unsigned int) ((DEFAULT_UNICODE_CHAR_SIZE + 1) * sizeof (SQLWCHAR));
                                max_size += info.ColumnSize;
                                break;
                            default:
                                break;
                        } //switch (info.DataType)
                        break;
                    default:
                        break;
                } //switch (info.Direction)
            }
            return max_size;
        }

        bool COdbcImpl::PushOutputParameters(unsigned int r, UINT64 index) {
            CScopeUQueue sb;
            unsigned int ret = SendResult(idBeginRows, (const unsigned char*) &index, (unsigned int) sizeof (index));
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            SQLLEN *pLenInd = (SQLLEN*) m_UQueue.GetBuffer();
            const unsigned char *start = m_Blob.GetBuffer();
            for (SQLSMALLINT n = 0; n < m_parameters; ++n) {
                CParameterInfo &info = m_vPInfo[n];
                if (info.Direction == UDB::pdInput)
                    continue;
                VARTYPE vt = VT_NULL;
                if (pLenInd[n] == SQL_NULL_DATA) {
                    sb << vt;
                    switch (info.DataType) {
                        case VT_CLSID:
                        case VT_DATE:
                        case VT_XML:
                        case VT_BSTR:
                        case (VT_UI1 | VT_ARRAY):
                        case (VT_I1 | VT_ARRAY):
                        case VT_VARIANT:
                            start += info.ColumnSize;
                            break;
                        default:
                            break;
                    }
                    continue;
                }
                UDB::CDBVariant &vtD = m_vParam[(unsigned int) n + r * ((unsigned int) m_parameters)];
                if (info.DataType == VT_VARIANT || info.DataType == VT_XML) {
                    sb << (VARTYPE) VT_BSTR;
                } else {
                    sb << info.DataType;
                }
                switch (info.DataType) {
                    case VT_I1:
                    case VT_UI1:
                        sb->Push((const unsigned char*) &vtD.bVal, sizeof (vtD.bVal));
                        break;
                    case VT_I2:
                    case VT_UI2:
                        sb->Push((const unsigned char*) &vtD.uiVal, sizeof (vtD.uiVal));
                        break;
                    case VT_BOOL:
                        if (vtD.boolVal) {
                            vtD.boolVal = VARIANT_TRUE;
                        }
                        sb->Push((const unsigned char*) &vtD.boolVal, sizeof (vtD.boolVal));
                        break;
                    case VT_I4:
                    case VT_UI4:
                    case VT_INT:
                    case VT_UINT:
                        sb->Push((const unsigned char*) &vtD.uintVal, sizeof (unsigned int));
                        break;
                    case VT_I8:
                    case VT_UI8:
                        sb->Push((const unsigned char*) &vtD.ullVal, sizeof (vtD.ullVal));
                        break;
                    case VT_R4:
                        sb->Push((const unsigned char*) &vtD.fltVal, sizeof (vtD.fltVal));
                        break;
                    case VT_R8:
                        sb->Push((const unsigned char*) &vtD.dblVal, sizeof (vtD.dblVal));
                        break;
                    case VT_CLSID:
                    {
                        sb->Push(start, (unsigned int) sizeof (GUID));
                        start += info.ColumnSize;
                    }
                        break;
                    case VT_DATE:
                    {
                        UDateTime dt;
                        dt.ParseFromDBString((const char*) start);
                        sb << dt.time;
                        start += info.ColumnSize;
                    }
                        break;
                    case VT_DECIMAL:
                    {
                        DECIMAL dec;
                        const char* s = (const char*) start;
                        if (pLenInd[n] <= 19)
                            ParseDec(s, dec);
                        else
                            ParseDec_long(s, dec);
                        sb << dec;
                        start += info.ColumnSize;
                    }
                        break;
                    case VT_XML:
                    case VT_BSTR:
                    {
                        sb << (const SQLWCHAR*) start;
                        start += (info.ColumnSize * sizeof (SQLWCHAR));
                    }
                        break;
                    case (VT_UI1 | VT_ARRAY):
                    {
                        sb->Push(start, (unsigned int) (pLenInd[n]));
                        start += info.ColumnSize;
                    }
                        break;
                    case (VT_I1 | VT_ARRAY):
                    {
                        sb << (const char*) start;
                        start += info.ColumnSize;
                    }
                        break;
                    case VT_VARIANT:
                    {
                        const SQLWCHAR *ws = (const SQLWCHAR*) start;
                        sb << ws;
                        start += info.ColumnSize;
                    }
                        break;
                    default:
                        assert(false);
                        break;
                }
                if (m_bReturn && !n) {
                    ret = SendResult(idCallReturn, sb->GetBuffer(), sb->GetSize());
                    sb->SetSize(0);
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return false;
                    }
                }
            }

            ret = SendResult(idOutputParameter, sb->GetBuffer(), sb->GetSize());
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        bool COdbcImpl::BindParameters(unsigned int r, SQLLEN * pLenInd) {
            unsigned int output_pos = 0;
            for (SQLSMALLINT col = 0; col < m_parameters; ++col) {
                CDBVariant &vtD = m_vParam[r * (unsigned int) m_parameters + (unsigned int) col];
                SQLSMALLINT InputOutputType = SQL_PARAM_TYPE_UNKNOWN;
                CParameterInfo &info = m_vPInfo[col];
                switch (info.Direction) {
                    case UDB::pdInput:
                        InputOutputType = SQL_PARAM_INPUT;
                        break;
                    case UDB::pdOutput:
                        InputOutputType = SQL_PARAM_OUTPUT;
                        break;
                    case UDB::pdReturnValue:
                        InputOutputType = SQL_PARAM_OUTPUT;
                        break;
                    case UDB::pdInputOutput:
                        InputOutputType = SQL_PARAM_INPUT_OUTPUT;
                        break;
                    default:
                        assert(false);
                        break;
                }
                if (InputOutputType == SQL_PARAM_INPUT_OUTPUT && info.DataType == VT_VARIANT && vtD.vt > VT_NULL && vtD.vt != VT_BSTR) {
                    CDBVariant vt;
                    HRESULT hr = VariantChangeType(&vt, &vtD, 0, VT_BSTR);
                    if (FAILED(hr))
                        return false;
                    vtD = vt;
                }
                pLenInd[col] = 0;
                SQLULEN ColumnSize = 0;
                SQLPOINTER ParameterValuePtr = nullptr;
                SQLLEN BufferLength = 0;
                SQLSMALLINT c_type = 0, sql_type = 0;
                SQLSMALLINT DecimalDigits = 0;
                VARTYPE my_vt = vtD.vt;
                if (InputOutputType == SQL_PARAM_OUTPUT || SQL_RETURN_VALUE == InputOutputType) {
                    vtD.Clear();
                    my_vt = VT_NULL;
                }
                switch (my_vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        pLenInd[col] = SQL_NULL_DATA;
                        if (InputOutputType == SQL_PARAM_OUTPUT || InputOutputType == SQL_PARAM_INPUT_OUTPUT || SQL_RETURN_VALUE == InputOutputType) {
                            switch (info.DataType) {
                                case VT_I1:
                                case VT_UI1:
                                    ParameterValuePtr = &vtD.bVal;
                                    c_type = SQL_C_CHAR;
                                    sql_type = SQL_TINYINT;
                                    break;
                                case VT_I2:
                                    ParameterValuePtr = &vtD.iVal;
                                    c_type = SQL_C_SSHORT;
                                    sql_type = SQL_SMALLINT;
                                    break;
                                case VT_UI2:
                                    ParameterValuePtr = &vtD.uiVal;
                                    c_type = SQL_C_USHORT;
                                    sql_type = SQL_SMALLINT;
                                    break;
                                case VT_I4:
                                case VT_INT:
                                    c_type = SQL_C_SLONG;
                                    sql_type = SQL_INTEGER;
                                    ParameterValuePtr = &vtD.lVal;
                                    break;
                                case VT_UI4:
                                case VT_UINT:
                                    c_type = SQL_C_ULONG;
                                    sql_type = SQL_INTEGER;
                                    ParameterValuePtr = &vtD.ulVal;
                                    break;
                                case VT_I8:
                                    c_type = SQL_C_SBIGINT;
                                    sql_type = SQL_BIGINT;
                                    ParameterValuePtr = &vtD.llVal;
                                    break;
                                case VT_UI8:
                                    c_type = SQL_C_UBIGINT;
                                    sql_type = SQL_BIGINT;
                                    ParameterValuePtr = &vtD.ullVal;
                                    break;
                                case VT_R4:
                                    c_type = SQL_C_FLOAT;
                                    sql_type = SQL_REAL;
                                    ParameterValuePtr = &vtD.fltVal;
                                    break;
                                case VT_R8:
                                    c_type = SQL_C_DOUBLE;
                                    sql_type = SQL_DOUBLE;
                                    ParameterValuePtr = &vtD.dblVal;
                                    break;
                                case VT_BOOL:
                                    ParameterValuePtr = &vtD.boolVal;
                                    c_type = SQL_C_USHORT;
                                    sql_type = SQL_BIT;
                                    break;
                                case VT_DECIMAL:
                                    sql_type = SQL_NUMERIC;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize;
                                    c_type = SQL_C_CHAR;
                                    ColumnSize = info.Precision;
                                    output_pos += (unsigned int) BufferLength;
                                    DecimalDigits = info.Scale;
                                    if (!ColumnSize) {
                                        ColumnSize = MAX_DECIMAL_PRECISION; //max length for UINT64 in case a client ignores precision 
                                    }
                                    break;
                                case VT_DATE:
                                    sql_type = SQL_TYPE_TIMESTAMP;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize;
                                    ColumnSize = 20 + info.Scale;
                                    c_type = SQL_C_CHAR;
                                    output_pos += (unsigned int) BufferLength;
                                    if (info.Scale)
                                        DecimalDigits = info.Scale;
                                    else
                                        DecimalDigits = MAX_TIME_DIGITS;
                                    break;
                                case (VT_I1 | VT_ARRAY):
                                    sql_type = SQL_LONGVARCHAR;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize;
                                    c_type = SQL_C_CHAR;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                case (VT_UI1 | VT_ARRAY):
                                    sql_type = SQL_LONGVARBINARY;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize;
                                    c_type = SQL_C_BINARY;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                case VT_BSTR:
                                    if (info.DataType == VT_VARIANT) {
                                        sql_type = SQL_WVARCHAR;
                                    } else {
                                        sql_type = SQL_WLONGVARCHAR;
                                    }
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize * sizeof (SQLWCHAR);
                                    c_type = SQL_C_WCHAR;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                case VT_XML:
                                    if (m_msDriver == msDB2)
                                        sql_type = SQL_XML;
                                    else
                                        sql_type = SQL_SS_XML;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize * sizeof (SQLWCHAR);
                                    c_type = SQL_C_WCHAR;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                case VT_CLSID:
                                    c_type = SQL_C_GUID;
                                    sql_type = SQL_GUID;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    BufferLength = info.ColumnSize;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                case VT_VARIANT:
                                    c_type = SQL_C_WCHAR;
                                    sql_type = SQL_SS_VARIANT;
                                    ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                                    memset(ParameterValuePtr, 0, (unsigned int) BufferLength);
                                    BufferLength = info.ColumnSize;
                                    output_pos += (unsigned int) BufferLength;
                                    break;
                                default:
                                    assert(false);
                                    break;
                            }
                        } else {
                            ParameterValuePtr = nullptr;
                            switch (info.DataType) {
                                case VT_I1:
                                case VT_UI1:
                                    c_type = SQL_C_CHAR;
                                    sql_type = SQL_TINYINT;
                                    break;
                                case VT_I2:
                                    c_type = SQL_C_SSHORT;
                                    sql_type = SQL_SMALLINT;
                                    break;
                                case VT_UI2:
                                    c_type = SQL_C_USHORT;
                                    sql_type = SQL_SMALLINT;
                                    break;
                                case VT_I4:
                                case VT_INT:
                                    c_type = SQL_C_SLONG;
                                    sql_type = SQL_INTEGER;
                                    break;
                                case VT_UI4:
                                case VT_UINT:
                                    c_type = SQL_C_ULONG;
                                    sql_type = SQL_INTEGER;
                                    break;
                                case VT_I8:
                                    c_type = SQL_C_SBIGINT;
                                    sql_type = SQL_BIGINT;
                                    break;
                                case VT_UI8:
                                    c_type = SQL_C_UBIGINT;
                                    sql_type = SQL_BIGINT;
                                    break;
                                case VT_R4:
                                    c_type = SQL_C_FLOAT;
                                    sql_type = SQL_REAL;
                                    break;
                                case VT_R8:
                                    c_type = SQL_C_DOUBLE;
                                    sql_type = SQL_DOUBLE;
                                    break;
                                case VT_BOOL:
                                    c_type = SQL_C_USHORT;
                                    sql_type = SQL_BIT;
                                    break;
                                case VT_DECIMAL:
                                    sql_type = SQL_NUMERIC;
                                    c_type = SQL_C_NUMERIC;
                                    break;
                                case VT_DATE:
                                    sql_type = SQL_TYPE_TIMESTAMP;
                                    c_type = SQL_C_TIMESTAMP;
                                    DecimalDigits = MAX_TIME_DIGITS;
                                    break;
                                case (VT_I1 | VT_ARRAY):
                                    sql_type = SQL_LONGVARCHAR;
                                    c_type = SQL_C_CHAR;
                                    break;
                                case (VT_UI1 | VT_ARRAY):
                                    sql_type = SQL_LONGVARBINARY;
                                    c_type = SQL_C_BINARY;
                                    break;
                                case VT_BSTR:
                                    sql_type = SQL_WLONGVARCHAR;
                                    c_type = SQL_C_WCHAR;
                                    break;
                                case VT_CLSID:
                                    c_type = SQL_C_GUID;
                                    sql_type = SQL_GUID;
                                    break;
                                case VT_VARIANT:
                                    c_type = SQL_C_WCHAR;
                                    sql_type = SQL_SS_VARIANT;
                                    break;
                                case VT_XML:
                                    c_type = SQL_C_WCHAR;
                                    if (m_msDriver == msDB2)
                                        sql_type = SQL_XML;
                                    else
                                        sql_type = SQL_SS_XML;
                                    break;
                                default:
                                    assert(false);
                                    break;
                            }
                        }
                        break;
                    case VT_I1:
                    case VT_UI1:
                        ParameterValuePtr = &vtD.bVal;
                        c_type = SQL_C_CHAR;
                        sql_type = SQL_TINYINT;
                        break;
                    case VT_I2:
                        ParameterValuePtr = &vtD.iVal;
                        c_type = SQL_C_SSHORT;
                        sql_type = SQL_SMALLINT;
                        break;
                    case VT_UI2:
                        ParameterValuePtr = &vtD.uiVal;
                        c_type = SQL_C_USHORT;
                        sql_type = SQL_SMALLINT;
                        break;
                    case VT_I4:
                    case VT_INT:
                        c_type = SQL_C_SLONG;
                        sql_type = SQL_INTEGER;
                        ParameterValuePtr = &vtD.lVal;
                        break;
                    case VT_UI4:
                    case VT_UINT:
                        c_type = SQL_C_ULONG;
                        sql_type = SQL_INTEGER;
                        ParameterValuePtr = &vtD.ulVal;
                        break;
                    case VT_I8:
                        c_type = SQL_C_SBIGINT;
                        sql_type = SQL_BIGINT;
                        ParameterValuePtr = &vtD.llVal;
                        break;
                    case VT_UI8:
                        c_type = SQL_C_UBIGINT;
                        sql_type = SQL_BIGINT;
                        ParameterValuePtr = &vtD.ullVal;
                        break;
                    case VT_R4:
                        c_type = SQL_C_FLOAT;
                        sql_type = SQL_REAL;
                        ParameterValuePtr = &vtD.fltVal;
                        break;
                    case VT_R8:
                        c_type = SQL_C_DOUBLE;
                        sql_type = SQL_DOUBLE;
                        ParameterValuePtr = &vtD.dblVal;
                        break;
                    case VT_DATE:
                        c_type = SQL_C_CHAR;
                    {
                        UDateTime dt(vtD.ullVal);
                        if (dt.HasMicrosecond()) {
                            DecimalDigits = MAX_TIME_DIGITS;
                        }
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            BufferLength = info.ColumnSize;
                            dt.ToDBString((char*) ParameterValuePtr, (unsigned int) BufferLength);
                            ColumnSize = ::strlen((const char*) ParameterValuePtr);
                            switch (ColumnSize) {
                                case 10:
                                    sql_type = SQL_TYPE_DATE;
                                    DecimalDigits = 0;
                                    break;
                                case 8:
                                    sql_type = SQL_TYPE_TIME;
                                    if (info.Scale)
                                        DecimalDigits = info.Scale;
                                    break;
                                default:
                                    sql_type = SQL_TYPE_TIMESTAMP;
                                    if (info.Scale) {
                                        DecimalDigits = info.Scale;
                                        ColumnSize = 20 + info.Scale;
                                    }
                                    break;
                            }
                            pLenInd[col] = (SQLLEN) ColumnSize;
                            output_pos += (unsigned int) BufferLength;
                        } else {
                            char str[32] = {0};
                            dt.ToDBString(str, sizeof (str));
                            vtD = (const char*) str;
                            ::SafeArrayAccessData(vtD.parray, &ParameterValuePtr);
                            ::SafeArrayUnaccessData(vtD.parray);
                            ColumnSize = vtD.parray->rgsabound->cElements;
                            BufferLength = (SQLLEN) ColumnSize;
                            switch (vtD.parray->rgsabound->cElements) {
                                case 10:
                                    sql_type = SQL_TYPE_DATE;
                                    DecimalDigits = 0;
                                    break;
                                case 8:
                                    sql_type = SQL_TYPE_TIME;
                                    if (info.Scale)
                                        DecimalDigits = info.Scale;
                                    break;
                                default:
                                    sql_type = SQL_TYPE_TIMESTAMP;
                                    if (info.Scale) {
                                        DecimalDigits = info.Scale;
                                        ColumnSize = 20 + info.Scale;
                                    }
                                    break;
                            }
                            pLenInd[col] = (SQLLEN) ColumnSize;
                        }
                    }
                        break;
#ifdef WIN32_64
                    case VT_BSTR:
                        c_type = SQL_C_WCHAR;
                        if (info.DataType == VT_VARIANT) {
                            sql_type = SQL_SS_VARIANT;
                        } else if (info.DataType == VT_XML) {
                            if (m_msDriver == msDB2)
                                sql_type = SQL_XML;
                            else
                                sql_type = SQL_SS_XML;
                        } else {
                            if (SysStringLen(vtD.bstrVal) < DEFAULT_UNICODE_CHAR_SIZE)
                                sql_type = SQL_WVARCHAR;
                            else
                                sql_type = SQL_WLONGVARCHAR;
                        }
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            ColumnSize = ::SysStringLen(vtD.bstrVal);
                            if (info.DataType == VT_VARIANT) {
                                if (ColumnSize >= info.ColumnSize / sizeof (SQLWCHAR)) {
                                    ColumnSize = info.ColumnSize / sizeof (SQLWCHAR) - 1;
                                }
                                ::memcpy(ParameterValuePtr, (const void*) vtD.bstrVal, (ColumnSize + 1) * sizeof (SQLWCHAR));
                                BufferLength = info.ColumnSize;
                                ColumnSize *= sizeof (SQLWCHAR);
                            } else {
                                if (ColumnSize >= info.ColumnSize) {
                                    ColumnSize = info.ColumnSize - 1;
                                }
                                ::memcpy(ParameterValuePtr, (const void*) vtD.bstrVal, (ColumnSize + 1) * sizeof (SQLWCHAR));
                                BufferLength = (SQLULEN) info.ColumnSize * sizeof (SQLWCHAR);
                            }
                            output_pos += (unsigned int) BufferLength;
                        } else {
                            ParameterValuePtr = vtD.bstrVal;
                            ColumnSize = ::SysStringLen(vtD.bstrVal);
                            BufferLength = (SQLLEN) ColumnSize * sizeof (SQLWCHAR);
                        }
                        pLenInd[col] = SQL_NTS;
#else
                    case (VT_UI2 | VT_ARRAY):
                        c_type = SQL_C_WCHAR;
                        if (info.DataType == VT_VARIANT) {
                            sql_type = SQL_SS_VARIANT;
                        } else if (info.DataType == VT_XML) {
                            if (m_msDriver == msDB2)
                                sql_type = SQL_XML;
                            else
                                sql_type = SQL_SS_XML;
                        } else {
                            if (SysStringLen(vtD.bstrVal) < DEFAULT_UNICODE_CHAR_SIZE)
                                sql_type = SQL_WVARCHAR;
                            else
                                sql_type = SQL_WLONGVARCHAR;
                        }
                        ColumnSize = vtD.parray->rgsabound->cElements;
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            unsigned short *data = nullptr;
                            ::SafeArrayAccessData(vtD.parray, (void**) &data);
                            if (info.DataType == VT_VARIANT) {
                                if (ColumnSize >= info.ColumnSize / sizeof (SQLWCHAR)) {
                                    ColumnSize = info.ColumnSize / sizeof (SQLWCHAR) - 1;
                                }
                                ::memcpy(ParameterValuePtr, data, (ColumnSize + 1) * sizeof (SQLWCHAR));
                                BufferLength = info.ColumnSize;
                                ColumnSize *= sizeof (SQLWCHAR);
                            } else {
                                if (ColumnSize >= info.ColumnSize) {
                                    ColumnSize = info.ColumnSize - 1;
                                }
                                ::memcpy(ParameterValuePtr, (const void*) data, (ColumnSize + 1) * sizeof (SQLWCHAR));
                                BufferLength = (SQLULEN) info.ColumnSize * sizeof (SQLWCHAR);
                            }
                            ::SafeArrayUnaccessData(vtD.parray);
                            output_pos += (unsigned int) BufferLength;
                        } else {
                            ::SafeArrayAccessData(vtD.parray, &ParameterValuePtr);
                            BufferLength = (SQLLEN) ColumnSize * sizeof (SQLWCHAR);
                            ::SafeArrayUnaccessData(vtD.parray);
                        }
                        pLenInd[col] = SQL_NTS;
#endif
                        break;
                    case VT_DECIMAL:
                        sql_type = SQL_NUMERIC;
                        DecimalDigits = info.Scale;
                        if (DecimalDigits < (SQLSMALLINT) (vtD.decVal.scale)) {
                            DecimalDigits = (SQLSMALLINT) (vtD.decVal.scale);
                        }
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            c_type = SQL_C_CHAR;
                            BufferLength = (SQLULEN) info.ColumnSize;
                            const DECIMAL &decVal = vtD.decVal;
                            std::string s;
                            if (decVal.Hi32)
                                s = ToString_long(decVal);
                            else
                                s = ToString(decVal);
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            memset(ParameterValuePtr, 0, BufferLength);
                            ::memcpy(ParameterValuePtr, s.c_str(), s.size());
                            pLenInd[col] = (SQLLEN) s.size();
                            ColumnSize = info.Precision;
                            if (!ColumnSize) {
                                ColumnSize = MAX_DECIMAL_PRECISION;
                            }
                            output_pos += (unsigned int) BufferLength;
                        } else {
                            c_type = SQL_C_CHAR;
                            bool neg = vtD.decVal.sign ? true : false;
                            bool point = vtD.decVal.scale ? true : false;
                            ConvertDecimalAString(vtD);
                            ::SafeArrayAccessData(vtD.parray, &ParameterValuePtr);
                            ::SafeArrayUnaccessData(vtD.parray);
                            ColumnSize = vtD.parray->rgsabound->cElements;
                            pLenInd[col] = (SQLLEN) ColumnSize;
                            BufferLength = (SQLLEN) ColumnSize;
                            ColumnSize = ColumnSize - neg - point;
                        }
                        break;
                    case (VT_I1 | VT_ARRAY):
                        ColumnSize = vtD.parray->rgsabound->cElements;
                        c_type = SQL_C_CHAR;
                        if (info.DataType == VT_VARIANT) {
                            sql_type = SQL_VARCHAR;
                        } else {
                            if (ColumnSize < DEFAULT_UNICODE_CHAR_SIZE)
                                sql_type = SQL_VARCHAR;
                            else
                                sql_type = SQL_LONGVARCHAR;
                        }
                        ::SafeArrayAccessData(vtD.parray, &ParameterValuePtr);
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            if (ColumnSize > info.ColumnSize) {
                                ColumnSize = info.ColumnSize;
                            }
                            BufferLength = (SQLLEN) info.ColumnSize;
                            void *pSrc = ParameterValuePtr;
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            ::memcpy(ParameterValuePtr, (const void*) pSrc, (size_t) ColumnSize);
                            output_pos += (unsigned int) BufferLength;
                            ::SafeArrayUnaccessData(vtD.parray);
                        } else {
                            ::SafeArrayUnaccessData(vtD.parray);
                            BufferLength = (SQLLEN) ColumnSize;
                        }
                        pLenInd[col] = (SQLLEN) ColumnSize;
                        break;
                    case (VT_UI1 | VT_ARRAY):
                        c_type = SQL_C_BINARY;
                        ColumnSize = vtD.parray->rgsabound->cElements;
                        if (ColumnSize == sizeof (GUID) && info.DataType == VT_CLSID) {
                            sql_type = SQL_GUID;
                            c_type = SQL_C_GUID;
                        } else {
                            if (info.DataType == VT_VARIANT)
                                sql_type = SQL_VARBINARY;
                            else
                                sql_type = SQL_LONGVARBINARY;
                        }
                        if (InputOutputType == SQL_PARAM_INPUT_OUTPUT) {
                            if (ColumnSize > info.ColumnSize) {
                                ColumnSize = info.ColumnSize;
                            }
                            BufferLength = (SQLLEN) info.ColumnSize;
                            void *pSrc = nullptr;
                            ::SafeArrayAccessData(vtD.parray, &pSrc);
                            ParameterValuePtr = (SQLPOINTER) (m_Blob.GetBuffer() + output_pos);
                            ::memcpy(ParameterValuePtr, (const void*) pSrc, (size_t) ColumnSize);
                            output_pos += (unsigned int) BufferLength;
                            ::SafeArrayUnaccessData(vtD.parray);
                        } else {
                            ::SafeArrayAccessData(vtD.parray, &ParameterValuePtr);
                            ::SafeArrayUnaccessData(vtD.parray);
                            BufferLength = (SQLLEN) ColumnSize;
                            pLenInd[col] = (SQLLEN) ColumnSize;
                        }
                        break;
                    case VT_BOOL:
                        ParameterValuePtr = &vtD.boolVal;
                        c_type = SQL_C_USHORT;
                        sql_type = SQL_BIT;
                        break;
                    default:
                        assert(false);
                        break;
                }
                SQLRETURN retcode = SQLBindParameter(m_pPrepare.get(), (SQLUSMALLINT) (col + 1), InputOutputType,
                        c_type, sql_type, ColumnSize, DecimalDigits, ParameterValuePtr, BufferLength,
                        pLenInd + col);
                if (!SQL_SUCCEEDED(retcode)) {
                    return false;
                }
            }
            return true;
        }

        void COdbcImpl::ConvertDecimalAString(CDBVariant & vt) {
            if (vt.vt == VT_DECIMAL) {
                std::string s;
                if (vt.decVal.Hi32)
                    s = ToString_long(vt.decVal);
                else
                    s = ToString(vt.decVal);
                vt = s.c_str();
            }
        }

        std::vector<CDBString> COdbcImpl::Split(const CDBString &sql, const CDBString & delimiter) {
            std::vector<CDBString> v;
            size_t d_len = delimiter.size();
            if (d_len) {
                const UTF16 quote = '\'', slash = '\\', done = delimiter[0];
                size_t params = 0, len = sql.size();
                bool b_slash = false, balanced = true;
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

        std::vector<tagParameterDirection> COdbcImpl::GetCallDirections(const CDBString & sql) {
            bool quest = false;
            bool parenthesis = false;
            bool not_empty = false;
            std::vector<tagParameterDirection> vPD;
            const UTF16 quote = L'\'', slash = L'\\', coma = L',', question = L'?';
            bool b_slash = false, balanced = true;
            size_t len = sql.size();
            for (size_t n = 0; n < len; ++n) {
                const UTF16 &c = sql[n];
                if (!parenthesis && c == L'(') {
                    parenthesis = true;
                    quest = false;
                    continue;
                }
                if (!not_empty && parenthesis && c != L')' && !::isspace(c))
                    not_empty = true;
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
                    if (c == coma) {
                        vPD.push_back(quest ? pdInput : pdUnknown);
                        quest = false;
                    } else if (c == question) {
                        quest = true;
                    }
                }
            }
            if (not_empty) {
                vPD.push_back(quest ? pdInput : pdUnknown);
            }
            return vPD;
        }

        size_t COdbcImpl::ComputeParameters(const CDBString & sql) {
            const UTF16 quote = L'\'', slash = L'\\', question = L'?';
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

        void COdbcImpl::SetVParam(CDBVariantArray& vAll, size_t parameters, size_t pos, size_t ps) {
            m_vParam.clear();
            size_t rows = vAll.size() / parameters;
            for (size_t r = 0; r < rows; ++r) {
                for (size_t p = 0; p < ps; ++p) {
                    CDBVariant &vt = vAll[parameters * r + pos + p];
                    m_vParam.push_back(std::move(vt));
                }
            }
        }

        CParameterInfoArray COdbcImpl::GetVInfo(const CParameterInfoArray& vPInfo, size_t pos, size_t ps) {
            CParameterInfoArray v;
            size_t count = vPInfo.size();
            for (size_t n = 0; n < ps; ++n) {
                v.push_back(vPInfo[pos + n]);
            }
            return v;
        }

        void COdbcImpl::ExecuteBatch(const CDBString& sql, const CDBString& delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, const CDBString &dbConn, unsigned int flags, UINT64 callIndex, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            CParameterInfoArray vPInfo;
            m_UQueue >> vPInfo;
            res = 0;
            fail_ok = 0;
            affected = 0;
            int ms = 0;
            if (!m_pOdbc) {
                Open(dbConn, flags, res, errMsg, ms);
            }
            size_t parameters = 0;
            std::vector<CDBString> vSql = Split(sql, delimiter);
            for (auto it = vSql.cbegin(), end = vSql.cend(); it != end; ++it) {
                parameters += ComputeParameters(*it);
            }
            if (!m_pOdbc) {
                res = Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                fail_ok = vSql.size();
                fail_ok <<= 32;
                SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
                return;
            }
            size_t rows = 0;
            if (parameters) {
                if (!m_vParam.size()) {
                    res = Odbc::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                    errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
                    return;
                }
                if ((m_vParam.size() % (unsigned short) parameters)) {
                    res = Odbc::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                    errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
                    return;
                }
                rows = m_vParam.size() / parameters;
                if (vPInfo.size() && parameters != vPInfo.size()) {
                    res = Odbc::ER_CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET;
                    errMsg = CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET;
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
                    return;
                }
            }
            if (isolation != (int) tiUnspecified) {
                int ms;
                BeginTrans(isolation, dbConn, flags, res, errMsg, ms);
                if (res) {
                    m_fails += vSql.size();
                    fail_ok = vSql.size();
                    fail_ok <<= 32;
                    SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
                    return;
                } else if (IsCanceled() || !IsOpened())
                    return;
            } else {
                if (!m_global) {
                    errMsg = dbConn;
                } else {
                    errMsg = ODBC_GLOBAL_CONNECTION_STRING;
                }
            }
            unsigned int ret = SendResult(idSqlBatchHeader, res, errMsg, (int) msODBC, (unsigned int) parameters, callIndex);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return;
            }
            errMsg.clear();
            CDBVariantArray vAll;
            m_vParam.swap(vAll);
            INT64 aff = 0;
            int r = 0;
            CDBString err;
            UINT64 fo = 0;
            size_t pos = 0;
            for (auto it = vSql.begin(), end = vSql.end(); it != end; ++it) {
                Utilities::Trim(*it);
                if (!it->size()) {
                    continue;
                }
                size_t ps = ComputeParameters(*it);
                if (ps) {
                    //prepared statements
                    CParameterInfoArray vP;
                    if (vPInfo.size())
                        vP = GetVInfo(vPInfo, pos, ps);
                    unsigned int my_ps = 0;
                    Prepare(*it, vP, r, err, my_ps);
                    if (!IsOpened() || IsCanceled())
                        return;
                    if (r) {
                        fail_ok += (((UINT64) rows) << 32);
                    } else {
                        assert(ps == (my_ps & 0xffff));
                        m_vParam.clear();
                        SetVParam(vAll, parameters, pos, ps);
                        unsigned int nParamPos = (unsigned int) ((pos << 16) + ps);
                        ret = SendResult(idParameterPosition, nParamPos);
                        if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                            return;
                        }
                        ExecuteParameters(rowset, meta, lastInsertId, callIndex, aff, r, err, vtId, fo);
                    }
                    pos += ps;
                } else {
                    Execute(*it, rowset, meta, lastInsertId, callIndex, aff, r, err, vtId, fo);
                }
                if (!IsOpened() || IsCanceled())
                    return;
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

        void COdbcImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            do {
                if (!m_pOdbc) {
                    res = Odbc::ER_NO_DB_OPENED_YET;
                    errMsg = NO_DB_OPENED_YET;
                    ++m_fails;
                    break;
                }
                if (!m_pPrepare) {
                    res = Odbc::ER_NO_PARAMETER_SPECIFIED;
                    errMsg = NO_PARAMETER_SPECIFIED;
                    ++m_fails;
                    break;
                }
                if (m_parameters) {
                    if ((m_vParam.size() % (unsigned short) m_parameters) || m_vParam.size() == 0) {
                        res = Odbc::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                        errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                        ++m_fails;
                        break;
                    }
                    if (!m_outputs && !m_vPInfo.size()) {
                        if (!SetInputParamInfo()) {
                            res = Odbc::ER_BAD_INPUT_PARAMETER_DATA_TYPE;
                            errMsg = BAD_INPUT_PARAMETER_DATA_TYPE;
                            ++m_fails;
                            break;
                        }
                    }
                }
                res = SQL_SUCCESS;
                if (m_parameters) {
                    bool output_sent = false;
                    unsigned int rows = (unsigned int) (m_vParam.size() / (unsigned short) m_parameters);
                    m_UQueue.SetSize(0);
                    CUQueue &qLenInd = m_UQueue;
                    unsigned int len_ind = (unsigned int) (sizeof (SQLLEN) * m_parameters);
                    if (qLenInd.GetMaxSize() < len_ind) {
                        qLenInd.ReallocBuffer(len_ind);
                    }
                    unsigned int max_size = 0;
                    if (m_outputs) {
                        max_size = ComputeOutputMaxSize();
                    }
                    CUQueue &qOut = m_Blob;
                    qOut.SetSize(0);
                    if (qOut.GetMaxSize() < max_size) {
                        qOut.ReallocBuffer(max_size);
                    }
                    SQLLEN *pLenInd = (SQLLEN*) qLenInd.GetBuffer();
                    for (unsigned int r = 0; r < rows; ++r) {
                        output_sent = false;
                        if (!BindParameters(r, pLenInd)) {
                            res = Odbc::ER_ERROR;
                            GetErrMsg(SQL_HANDLE_STMT, m_pPrepare.get(), errMsg);
                            ++m_fails;
                            continue;
                        }
                        SQLRETURN retcode;
                        if (meta) {
                            switch (m_msDriver) {
                                case msMsSQL:
                                    retcode = SQLSetStmtAttr(m_pPrepare.get(), SQL_SOPT_SS_HIDDEN_COLUMNS, (SQLPOINTER) SQL_HC_ON, 0);
                                    break;
                                default:
                                    break;
                            }
                        }
                        retcode = SQLExecute(m_pPrepare.get());
                        if (!SQL_SUCCEEDED(retcode)) {
                            if (!res) {
                                res = Odbc::ER_ERROR;
                                GetErrMsg(SQL_HANDLE_STMT, m_pPrepare.get(), errMsg);
                            }
                            ++m_fails;
                            continue;
                        }
                        int temp;
                        do {
                            temp = 0;
                            CDBString errTemp;
                            SQLSMALLINT columns = 0;
                            retcode = SQLNumResultCols(m_pPrepare.get(), &columns);
                            assert(SQL_SUCCEEDED(retcode));
                            if (columns) {
                                CDBColumnInfoArray vInfo = GetColInfo(m_pPrepare.get(), columns, (meta || m_bCall));
                                bool output = (m_bCall && vInfo[0].TablePath == m_procName && (size_t) m_parameters >= vInfo.size());
                                if (output || rowset || meta) {
                                    unsigned int outputs = output ? ((unsigned int) vInfo.size()) : 0;
                                    unsigned int ret = SendResult(idRowsetHeader, vInfo, index, outputs);
                                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                                        m_vParam.clear();
                                        return;
                                    }
                                    bool ok;
                                    if (m_nRecordSize && !output) {
                                        if (rowset) {
                                            ok = PushRecords(m_pPrepare.get(), res, errMsg);
                                        } else {
                                            ok = true;
                                        }
                                    } else if (output || rowset) {
                                        ok = PushRecords(m_pPrepare.get(), vInfo, output, temp, errTemp);
                                    } else {
                                        ok = true;
                                    }
                                    output_sent = output;
                                    if (!ok) {
                                        m_vParam.clear();
                                        return;
                                    }
                                    if (temp && !res) {
                                        res = temp;
                                        errMsg = errTemp;
                                    }
                                }
                            } else {
                                SQLLEN rows = 0;
                                retcode = SQLRowCount(m_pPrepare.get(), &rows);
                                assert(SQL_SUCCEEDED(retcode));
                                if (rows > 0) {
                                    affected += rows;
                                }
                            }
                        } while (SQLMoreResults(m_pPrepare.get()) == SQL_SUCCESS);
                        if (temp) {
                            ++m_fails;
                        } else {
                            ++m_oks;
                        }
                        if (m_outputs && !output_sent && !PushOutputParameters(r, index)) {
                            break;
                        }
                    }
                } else {
                    SQLRETURN retcode = SQLExecute(m_pPrepare.get());
                    if (!SQL_SUCCEEDED(retcode)) {
                        res = Odbc::ER_ERROR;
                        GetErrMsg(SQL_HANDLE_STMT, m_pPrepare.get(), errMsg);
                        ++m_fails;
                        break;
                    }
                    do {
                        SQLSMALLINT columns = 0;
                        retcode = SQLNumResultCols(m_pPrepare.get(), &columns);
                        assert(SQL_SUCCEEDED(retcode));
                        if (columns) {
                            if (rowset || meta) {
                                unsigned int ret;
                                CDBColumnInfoArray vInfo = GetColInfo(m_pPrepare.get(), columns, meta);
                                ret = SendResult(idRowsetHeader, vInfo, index);
                                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                                    m_vParam.clear();
                                    return;
                                }
                                bool ok;
                                if (rowset) {
                                    ok = PushRecords(m_pPrepare.get(), vInfo, false, res, errMsg);
                                } else {
                                    ok = true;
                                }
                                if (!ok) {
                                    m_vParam.clear();
                                    return;
                                }
                                if (res) {
                                    ++m_fails;
                                } else {
                                    ++m_oks;
                                }
                            }
                        } else {
                            SQLLEN rows = 0;
                            retcode = SQLRowCount(m_pPrepare.get(), &rows);
                            assert(SQL_SUCCEEDED(retcode));
                            if (rows > 0) {
                                affected += rows;
                            }
                            ++m_oks;
                        }
                    } while (SQLMoreResults(m_pPrepare.get()) == SQL_SUCCESS);
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
            m_vParam.clear();
        }

        void COdbcImpl::StartBLOB(unsigned int lenExpected) {
            m_Blob.SetSize(0);
            if (lenExpected > m_Blob.GetMaxSize()) {
                m_Blob.ReallocBuffer(lenExpected);
            }
            CUQueue &q = m_UQueue;
            m_Blob.Push(q.GetBuffer(), q.GetSize());
            assert(q.GetSize() > sizeof (unsigned short) + sizeof (unsigned int));
            q.SetSize(0);
        }

        void COdbcImpl::Chunk() {
            CUQueue &q = m_UQueue;
            if (q.GetSize()) {
                m_Blob.Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
            }
        }

        void COdbcImpl::EndBLOB() {
            Chunk();
            m_vParam.push_back(CDBVariant());
            CDBVariant &vt = m_vParam.back();
#ifndef WIN32_64
            VARTYPE *pvt = (VARTYPE*) m_Blob.GetBuffer();
            if (*pvt == VT_BSTR) {
                unsigned int *plen = (unsigned int*) m_Blob.GetBuffer(sizeof (VARTYPE));
                *pvt = (VT_UI2 | VT_ARRAY);
                *plen /= sizeof (short);
            }
#endif
            m_Blob >> vt;
            assert(m_Blob.GetSize() == 0);
        }

        void COdbcImpl::BeginRows() {
            Transferring();
        }

        void COdbcImpl::EndRows() {
            Transferring();
        }

        void COdbcImpl::Transferring() {
            CUQueue &q = m_UQueue;
            while (q.GetSize()) {
                m_vParam.push_back(CDBVariant());
                CDBVariant &vt = m_vParam.back();
#ifndef WIN32_64
                VARTYPE *pvt = (VARTYPE*) q.GetBuffer();
                if (*pvt == VT_BSTR) {
                    unsigned int *plen = (unsigned int*) q.GetBuffer(sizeof (VARTYPE));
                    *pvt = (VT_UI2 | VT_ARRAY);
                    *plen /= sizeof (short);
                }
#endif
                q >> vt;
            }
            assert(q.GetSize() == 0);
        }

        void COdbcImpl::GetErrMsg(SQLSMALLINT HandleType, SQLHANDLE Handle, CDBString & errMsg) {
#ifdef WIN32_64
            static CDBString SQLSTATE(L"SQLSTATE=");
            static CDBString NATIVE_ERROR(L":NATIVE=");
            static CDBString ERROR_MESSAGE(L":ERROR_MESSAGE=");
#else
            static CDBString SQLSTATE(u"SQLSTATE=");
            static CDBString NATIVE_ERROR(u":NATIVE=");
            static CDBString ERROR_MESSAGE(u":ERROR_MESSAGE=");
#endif
            errMsg.clear();
            SQLSMALLINT i = 1, MsgLen = 0;
            SQLINTEGER NativeError = 0;
            SQLCHAR SqlState[6] =
            {0}, Msg[SQL_MAX_MESSAGE_LENGTH + 1] =
            {0};
            SQLRETURN res = SQLGetDiagRec(HandleType, Handle, i, SqlState, &NativeError, Msg, sizeof (Msg) / sizeof (SQLCHAR), &MsgLen);
            while (res != SQL_NO_DATA) {
                if (errMsg.size()) {
                    errMsg.push_back(';');
                }
                errMsg += (SQLSTATE + Utilities::ToUTF16((const char*) SqlState));
                errMsg += (NATIVE_ERROR + Utilities::ToUTF16(std::to_string((INT64) NativeError)));
                errMsg += (ERROR_MESSAGE + Utilities::ToUTF16((const char*) Msg));
                ::memset(Msg, 0, sizeof (Msg));
                ++i;
                MsgLen = 0;
                NativeError = 0;
                res = SQLGetDiagRec(HandleType, Handle, i, SqlState, &NativeError, Msg, sizeof (Msg), &MsgLen);
            }
        }
    } //namespace ServerSide
} //namespace SPA
