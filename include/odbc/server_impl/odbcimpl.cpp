
#include "odbcimpl.h"
#include <algorithm>
#include <sstream>
#ifndef NDEBUG
#include <iostream>
#endif

namespace SPA
{
    namespace ServerSide{

        const wchar_t * COdbcImpl::NO_DB_OPENED_YET = L"No ODBC database opened yet";
        const wchar_t * COdbcImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const wchar_t * COdbcImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const wchar_t * COdbcImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const wchar_t * COdbcImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const wchar_t * COdbcImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const wchar_t * COdbcImpl::NO_DB_NAME_SPECIFIED = L"No database name specified";
        const wchar_t * COdbcImpl::ODBC_ENVIRONMENT_NOT_INITIALIZED = L"ODBC system library not initialized";
        const wchar_t * COdbcImpl::BAD_MANUAL_TRANSACTION_STATE = L"Bad manual transaction state";

        SQLHENV COdbcImpl::g_hEnv = nullptr;

        const wchar_t * COdbcImpl::ODBC_GLOBAL_CONNECTION_STRING = L"ODBC_GLOBAL_CONNECTION_STRING";

        CUCriticalSection COdbcImpl::m_csPeer;
        std::wstring COdbcImpl::m_strGlobalConnection;

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
            if (str)
                m_strGlobalConnection = str;
            else
                m_strGlobalConnection.clear();
            m_csPeer.unlock();
        }

        void COdbcImpl::ODBC_CONNECTION_STRING::Trim(std::wstring & s) {
            static const wchar_t *WHITESPACE = L" \r\n\t\v\f\v";
            auto pos = s.find_first_of(WHITESPACE);
            while (pos == 0) {
                s.erase(s.begin());
                pos = s.find_first_of(WHITESPACE);
            }
            pos = s.find_last_of(WHITESPACE);
            while (s.size() && pos == s.size() - 1) {
                s.pop_back();
                pos = s.find_last_of(WHITESPACE);
            }
        }

        void COdbcImpl::ODBC_CONNECTION_STRING::Parse(const wchar_t * s) {
            using namespace std;
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
                Trim(left);
                Trim(right);
                transform(left.begin(), left.end(), left.begin(), ::tolower);
                if (left == L"connect-timeout" || left == L"timeout" || left == L"connection-timeout")
                    timeout = (unsigned int) _wtoi(right.c_str());
                else if (left == L"database" || left == L"db")
                    database = right;
                else if (left == L"port")
                    port = (unsigned int) _wtoi(right.c_str());
                else if (left == L"pwd" || left == L"password")
                    password = right;
                else if (left == L"host" || left == L"server" || left == L"dsn")
                    host = right;
                else if (left == L"user" || left == L"uid")
                    user = right;
                else if (left == L"async" || left == L"asynchronous")
                    async = (_wtoi(right.c_str()) ? true : false);
                else {
                    //!!! not implemented
                    assert(false);
                }
            }
        }

        COdbcImpl::COdbcImpl() : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_global(true), m_Blob(*m_sb), m_parameters(0), m_bCall(false) {

        }

        void COdbcImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanDBObjects();
            m_Blob.SetSize(0);
            if (m_Blob.GetMaxSize() > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_Blob.ReallocBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
            m_global = true;
        }

        void COdbcImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_oks = 0;
            m_fails = 0;
            m_ti = tiUnspecified;
        }

        void COdbcImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I1_R0(idStartBLOB, StartBLOB, unsigned int)
            M_I0_R0(idChunk, Chunk)
            M_I0_R0(idEndBLOB, EndBLOB)
            M_I0_R0(idBeginRows, BeginRows)
            M_I0_R0(idTransferring, Transferring)
            M_I0_R0(idEndRows, EndRows)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            M_I2_R3(idOpen, Open, std::wstring, unsigned int, int, std::wstring, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, std::wstring, unsigned int, int, std::wstring, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, std::wstring)
            M_I5_R5(idExecute, Execute, std::wstring, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, std::wstring, CParameterInfoArray, int, std::wstring, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
#if 0
            M_I5_R3(SPA::Odbc::idSQLColumnPrivileges, DoSQLColumnPrivileges, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLColumns, DoSQLColumns, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I7_R3(SPA::Odbc::idSQLForeignKeys, DoSQLForeignKeys, std::wstring, std::wstring, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLPrimaryKeys, DoSQLPrimaryKeys, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLProcedureColumns, DoSQLProcedureColumns, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLProcedures, DoSQLProcedures, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I7_R3(SPA::Odbc::idSQLSpecialColumns, DoSQLSpecialColumns, SQLSMALLINT, std::wstring, std::wstring, std::wstring, SQLSMALLINT, SQLSMALLINT, UINT64, int, std::wstring, UINT64)
            M_I6_R3(SPA::Odbc::idSQLStatistics, DoSQLStatistics, std::wstring, std::wstring, std::wstring, SQLUSMALLINT, SQLUSMALLINT, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLTablePrivileges, DoSQLTablePrivileges, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLTables, DoSQLTables, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
#endif
            END_SWITCH
            if (reqId == idExecuteParameters) {
                ReleaseArray();
                m_vParam.clear();
            }
        }

        int COdbcImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            M_I2_R3(idOpen, Open, std::wstring, unsigned int, int, std::wstring, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, std::wstring, unsigned int, int, std::wstring, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, std::wstring)
            M_I5_R5(idExecute, Execute, std::wstring, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, std::wstring, CParameterInfoArray, int, std::wstring, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I5_R3(SPA::Odbc::idSQLColumnPrivileges, DoSQLColumnPrivileges, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLColumns, DoSQLColumns, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I7_R3(SPA::Odbc::idSQLForeignKeys, DoSQLForeignKeys, std::wstring, std::wstring, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLPrimaryKeys, DoSQLPrimaryKeys, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLProcedureColumns, DoSQLProcedureColumns, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLProcedures, DoSQLProcedures, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I7_R3(SPA::Odbc::idSQLSpecialColumns, DoSQLSpecialColumns, SQLSMALLINT, std::wstring, std::wstring, std::wstring, SQLSMALLINT, SQLSMALLINT, UINT64, int, std::wstring, UINT64)
            M_I6_R3(SPA::Odbc::idSQLStatistics, DoSQLStatistics, std::wstring, std::wstring, std::wstring, SQLUSMALLINT, SQLUSMALLINT, UINT64, int, std::wstring, UINT64)
            M_I4_R3(SPA::Odbc::idSQLTablePrivileges, DoSQLTablePrivileges, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            M_I5_R3(SPA::Odbc::idSQLTables, DoSQLTables, std::wstring, std::wstring, std::wstring, std::wstring, UINT64, int, std::wstring, UINT64)
            END_SWITCH
            if (reqId == idExecuteParameters) {
                ReleaseArray();
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
                        if (m_ti == tiUnspecified)
                            break;
                        m_ti = tiUnspecified;
                    } while (false);
                    break;
                default:
                    break;
            }
        }

        void COdbcImpl::ReleaseArray() {
            for (auto it = m_vArray.begin(), end = m_vArray.end(); it != end; ++it) {
                SAFEARRAY *arr = *it;
                ::SafeArrayUnaccessData(arr);
            }
            m_vArray.clear();
        }

        void COdbcImpl::CleanDBObjects() {
            m_pPrepare.reset();
            m_pOdbc.reset();
            m_vParam.clear();
        }

        void COdbcImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msODBC;
            CleanDBObjects();
            if (!g_hEnv) {
                res = SPA::Odbc::ER_ODBC_ENVIRONMENT_NOT_INITIALIZED;
                errMsg = ODBC_ENVIRONMENT_NOT_INITIALIZED;
                return;
            } else {
                res = 0;
            }
            do {
                std::wstring db(strConnection);
                if (!db.size() || db == ODBC_GLOBAL_CONNECTION_STRING) {
                    m_csPeer.lock();
                    db = m_strGlobalConnection;
                    m_csPeer.unlock();
                    m_global = true;
                } else {
                    m_global = false;
                }
                ODBC_CONNECTION_STRING ocs;
                ocs.Parse(db.c_str());
                SQLHDBC hdbc = nullptr;
                SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_DBC, g_hEnv, &hdbc);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_ENV, g_hEnv, errMsg);
                    break;
                }

                if (ocs.timeout) {
                    SQLPOINTER rgbValue = &ocs.timeout;
                    retcode = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, rgbValue, 0);
                }

                if (ocs.database.size()) {
                    retcode = SQLSetConnectAttr(hdbc, SQL_ATTR_CURRENT_CATALOG, (SQLPOINTER) ocs.database.c_str(), (SQLINTEGER) (ocs.database.size() * sizeof (SQLWCHAR)));
                }

                retcode = SQLSetConnectAttr(hdbc, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER) (ocs.async ? SQL_ASYNC_ENABLE_ON : SQL_ASYNC_ENABLE_OFF), 0);

                retcode = SQLConnect(hdbc, (SQLWCHAR*) ocs.host.c_str(), (SQLSMALLINT) ocs.host.size(), (SQLWCHAR *) ocs.user.c_str(), (SQLSMALLINT) ocs.user.size(), (SQLWCHAR *) ocs.password.c_str(), (SQLSMALLINT) ocs.password.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
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
            } while (false);
        }

        void COdbcImpl::CloseDb(int &res, std::wstring & errMsg) {
            CleanDBObjects();
            res = 0;
        }

        void COdbcImpl::BeginTrans(int isolation, const std::wstring &dbConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            res = 0;
            ms = msODBC;

        }

        void COdbcImpl::EndTrans(int plan, int &res, std::wstring & errMsg) {
            res = 0;
        }

        bool COdbcImpl::SendRows(CUQueue& sb, bool transferring) {
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

        bool COdbcImpl::SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) {
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

        bool COdbcImpl::SendUText(SQLHSTMT hstmt, SQLUSMALLINT index, CUQueue &qTemp, CUQueue &q, bool & blob) {
            qTemp.SetSize(0);
            SQLLEN len_or_null = 0;
            SQLRETURN retcode = SQLGetData(hstmt, index, SQL_C_WCHAR, (SQLPOINTER) qTemp.GetBuffer(), qTemp.GetMaxSize(), &len_or_null);
            assert(SQL_SUCCEEDED(retcode));
            if (len_or_null == SQL_NULL_DATA) {
                q << (VARTYPE) VT_NULL;
                blob = false;
                return true;
            } else if ((unsigned int) len_or_null < qTemp.GetMaxSize()) {
#ifdef WIN32_64
                q << (VARTYPE) VT_BSTR << (unsigned int) len_or_null;
                q.Push(qTemp.GetBuffer(), (unsigned int) len_or_null);
#else

#endif
                blob = false;
                return true;
            }
            if (q.GetSize() && !SendRows(q, true)) {
                return false;
            }
#ifdef WIN32_64
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
                bytes -= sizeof (wchar_t);
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
#else

#endif
            return true;
        }

        bool COdbcImpl::SendBlob(SQLHSTMT hstmt, SQLUSMALLINT index, VARTYPE vt, CUQueue &qTemp, CUQueue &q, bool & blob) {
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
            ret = SendResult(idEndBLOB, qTemp.GetBuffer(), bytes);
            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                return false;
            }
            return true;
        }

        CDBColumnInfoArray COdbcImpl::GetColInfo(SQLHSTMT hstmt, SQLSMALLINT columns, bool meta) {
            bool primary_key_set = false;
            SQLWCHAR colname[128 + 1] =
            {0}; // column name
            SQLSMALLINT colnamelen = 0; // length of column name
            SQLSMALLINT nullable = 0; // whether column can have NULL value
            SQLULEN collen = 0; // column lengths
            SQLSMALLINT coltype = 0; // column type
            SQLSMALLINT decimaldigits = 0; // no of digits if column is numeric
            SQLLEN displaysize = 0; // drivers column display size
            CDBColumnInfoArray vCols;
            for (SQLSMALLINT n = 0; n < columns; ++n) {
                vCols.push_back(CDBColumnInfo());
                CDBColumnInfo &info = vCols.back();
                SQLRETURN retcode = SQLDescribeCol(hstmt, (SQLUSMALLINT) (n + 1), colname, sizeof (colname) / sizeof (SQLWCHAR), &colnamelen, &coltype, &collen, &decimaldigits, &nullable);
                assert(SQL_SUCCEEDED(retcode));
                info.DisplayName.assign(colname, colnamelen); //display column name
                if (nullable == SQL_NO_NULLS) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                }

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_BASE_COLUMN_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                info.OriginalName.assign(colname, colnamelen / sizeof (SQLWCHAR)); //original column name

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_SCHEMA_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                if (colnamelen) {
                    info.TablePath.assign(colname, colnamelen / sizeof (SQLWCHAR));
                    info.TablePath += L".";
                }
                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_BASE_TABLE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                info.TablePath += colname; //schema.table_name

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_TYPE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                info.DeclaredType.assign(colname, colnamelen / sizeof (SQLWCHAR)); //native data type

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CATALOG_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                info.DBPath.assign(colname, colnamelen / sizeof (SQLWCHAR)); //database name

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_UNSIGNED, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                switch (coltype) {
                    case SQL_CHAR:
                    case SQL_VARCHAR:
                    case SQL_LONGVARCHAR:
                        info.ColumnSize = (unsigned int) collen;
                        info.DataType = (VT_ARRAY | VT_I1);
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        if (displaysize == SQL_TRUE) {
                            info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
                        }
                        break;
                    case SQL_WCHAR:
                    case SQL_WVARCHAR:
                    case SQL_WLONGVARCHAR:
                        info.ColumnSize = (unsigned int) collen;
                        info.DataType = VT_BSTR;
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        if (displaysize == SQL_TRUE) {
                            info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
                        }
                        break;
                    case SQL_BINARY:
                    case SQL_VARBINARY:
                    case SQL_LONGVARBINARY:
                        info.ColumnSize = (unsigned int) collen;
                        info.DataType = (VT_ARRAY | VT_UI1);
                        break;
                    case SQL_DECIMAL:
                    case SQL_NUMERIC:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_DECIMAL;
                        info.Scale = (unsigned char) decimaldigits;
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_PRECISION, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        info.Precision = (unsigned char) displaysize;
                        break;
                    case SQL_SMALLINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI2;
                        } else {
                            info.DataType = VT_I2;
                        }
                        break;
                    case SQL_INTEGER:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI4;
                        } else {
                            info.DataType = VT_I4;
                        }
                        break;
                    case SQL_REAL:
                        info.DataType = VT_R4;
                        break;
                    case SQL_FLOAT:
                    case SQL_DOUBLE:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_R8;
                        break;
                    case SQL_TINYINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI1;
                        } else {
                            info.DataType = VT_I1;
                        }
                        break;
                        break;
                    case SQL_BIGINT:
                        if (displaysize == SQL_TRUE) {
                            info.DataType = VT_UI8;
                        } else {
                            info.DataType = VT_I8;
                        }
                        break;
                    case SQL_BIT:
                        info.DataType = VT_BOOL;
                        info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
                        break;
                    case SQL_GUID:
                        info.ColumnSize = sizeof (GUID);
                        info.DataType = (VT_ARRAY | VT_UI1);
                        info.Precision = ((unsigned char) 16);
                        break;
                    case SQL_TYPE_DATE:
                    case SQL_TYPE_TIME:
                    case SQL_TYPE_TIMESTAMP:
                        info.ColumnSize = coltype; //remember SQL data type
                        info.DataType = VT_DATE;
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
                        info.ColumnSize = coltype; //remember SQL data type
                        info.Scale = (unsigned char) decimaldigits;
                        retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_PRECISION, nullptr, 0, nullptr, &displaysize);
                        assert(SQL_SUCCEEDED(retcode));
                        info.Precision = (unsigned char) displaysize;
                        info.DataType = VT_DATE;
                        break;
                    default:
                        assert(false); //not supported
                        break;
                }

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_AUTO_UNIQUE_VALUE, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                if (displaysize == SQL_TRUE) {
                    info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                    info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
                    info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                    info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                    primary_key_set = true;
                }

                retcode = SQLColAttribute(hstmt, (SQLUSMALLINT) (n + 1), SQL_DESC_UPDATABLE, nullptr, 0, nullptr, &displaysize);
                assert(SQL_SUCCEEDED(retcode));
                if (displaysize == SQL_ATTR_READONLY) {
                    info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
                }
            }

            if (meta && !primary_key_set) {

            }
            return vCols;
        }

        unsigned short COdbcImpl::ToSystemTime(const TIMESTAMP_STRUCT &d, SYSTEMTIME & st) {
            st.wYear = (unsigned short) d.year;
            st.wMonth = d.month;
            st.wDay = d.day;
            st.wHour = d.hour;
            st.wMinute = d.minute;
            st.wSecond = d.second;
            st.wMilliseconds = (unsigned short) (d.fraction / 1000000);
            return (unsigned short) ((d.fraction / 1000) % 1000);
        }

        void COdbcImpl::ToSystemTime(const TIME_STRUCT &d, SYSTEMTIME & st) {
            //start from 01/01/1900
            st.wYear = 1900;
            st.wMonth = 1;
            st.wDay = 1;
            st.wHour = d.hour;
            st.wMinute = d.minute;
            st.wSecond = d.second;
            st.wMilliseconds = 0;
        }

        void COdbcImpl::ToSystemTime(const DATE_STRUCT &d, SYSTEMTIME & st) {
            memset(&st, 0, sizeof (st));
            st.wYear = (unsigned short) d.year;
            st.wMonth = d.month;
            st.wDay = d.day;
        }

        bool COdbcImpl::PushRecords(SQLHSTMT hstmt, const CDBColumnInfoArray &vColInfo, int &res, std::wstring & errMsg) {
            SQLRETURN retcode;
            VARTYPE vt;
            CScopeUQueue sbTemp(MY_OPERATION_SYSTEM, SPA::IsBigEndian(), 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
            SQLLEN len_or_null = 0;
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
                        const CDBColumnInfo &colInfo = vColInfo[i];
                        vt = colInfo.DataType;
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
                            case VT_BSTR:
                                if (colInfo.ColumnSize >= DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                    if (!SendUText(hstmt, (SQLUSMALLINT) (i + 1), *sbTemp, q, blob)) {
                                        return false;
                                    }
                                } else {
#ifdef WIN32_64
                                    unsigned int max = (colInfo.ColumnSize << 1);
                                    if (q.GetTailSize() < sizeof (unsigned int) + sizeof (VARTYPE) + max + sizeof (wchar_t)) {
                                        q.ReallocBuffer(q.GetMaxSize() + max + sizeof (unsigned int) + sizeof (VARTYPE) + sizeof (wchar_t));
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
#else
                                    unsigned int max = (colInfo.ColumnSize << 2 + sizeof (wchar_t));
                                    if (max > sbTemp->GetMaxSize()) {
                                        sbTemp->ReallocBuffer(max);
                                    }
                                    retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_WCHAR, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                    if (SQL_NULL_DATA == len_or_null) {
                                        q << (VARTYPE) VT_NULL;
                                    } else {
                                        sbTemp->SetSize(len_or_null);
                                        sbTemp->SetNull();
                                        q << vt;
                                        q << (const wchar_t *) sbTemp->GetBuffer();
                                    }
                                    sbTemp->SetSize(0);
#endif
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
                                            SYSTEMTIME st;
                                            ToSystemTime(d, st);
                                            SPA::UDateTime dt(st);
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
                                            SYSTEMTIME st;
                                            ToSystemTime(d, st);
                                            SPA::UDateTime dt(st);
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
                                            SYSTEMTIME st;
                                            unsigned short us = ToSystemTime(d, st);
                                            SPA::UDateTime dt(st, us);
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
                                SPA::INT64 d;
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
                                SPA::UINT64 d;
                                retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_UBIGINT, &d, sizeof (d), &len_or_null);
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
                                        retcode = SQLGetData(hstmt, (SQLUSMALLINT) (i + 1), SQL_C_CHAR, (SQLPOINTER) sbTemp->GetBuffer(), sbTemp->GetMaxSize(), &len_or_null);
                                        if (len_or_null == SQL_NULL_DATA) {
                                            q << (VARTYPE) VT_NULL;
                                        } else {
                                            q << vt;
                                            sbTemp->SetSize((unsigned int) len_or_null);
                                            sbTemp->SetNull();
                                            DECIMAL dec;
                                            SPA::ParseDec((const char*) sbTemp->GetBuffer(), dec);
                                            q << dec;
                                            sbTemp->SetSize(0);
                                        }
                                    }
                                        break;
                                    default:
                                        assert(false); //shouldn't come here
                                        break;
                                }
                                break;
                            default:
                                assert(false);
                                break;
                        } //for loop
                        assert(SQL_SUCCEEDED(retcode));
                    }
                } else {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                if ((q.GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(q)) {
                    return false;
                }
            } //while loop
            assert(SQL_NO_DATA == retcode);
            if (q.GetSize()) {
                return SendRows(q);
            }
            return true;
        }

        void COdbcImpl::DoSQLColumnPrivileges(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLColumnPrivileges(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size(),
                        (SQLWCHAR*) columnName.c_str(), (SQLSMALLINT) columnName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLTables(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& tableType, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLTables(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size(),
                        (SQLWCHAR*) tableType.c_str(), (SQLSMALLINT) tableType.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLColumns(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLColumns(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size(),
                        (SQLWCHAR*) columnName.c_str(), (SQLSMALLINT) columnName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLProcedureColumns(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& procName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLProcedureColumns(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) procName.c_str(), (SQLSMALLINT) procName.size(),
                        (SQLWCHAR*) columnName.c_str(), (SQLSMALLINT) columnName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLPrimaryKeys(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLPrimaryKeys(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLTablePrivileges(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLTablePrivileges(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLStatistics(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, SQLUSMALLINT unique, SQLUSMALLINT reserved, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLStatistics(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size(), unique, reserved);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLProcedures(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& procName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLProcedures(hstmt, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) procName.c_str(), (SQLSMALLINT) procName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLSpecialColumns(SQLSMALLINT identifierType, const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, SQLSMALLINT scope, SQLSMALLINT nullable, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLSpecialColumns(hstmt, identifierType, (SQLWCHAR*) catalogName.c_str(), (SQLSMALLINT) catalogName.size(),
                        (SQLWCHAR*) schemaName.c_str(), (SQLSMALLINT) schemaName.size(),
                        (SQLWCHAR*) tableName.c_str(), (SQLSMALLINT) tableName.size(), scope, nullable);
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::DoSQLForeignKeys(const std::wstring& pkCatalogName, const std::wstring& pkSchemaName, const std::wstring& pkTableName, const std::wstring& fkCatalogName, const std::wstring& fkSchemaName, const std::wstring& fkTableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 & fail_ok) {
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLForeignKeys(hstmt, (SQLWCHAR*) pkCatalogName.c_str(), (SQLSMALLINT) pkCatalogName.size(),
                        (SQLWCHAR*) pkSchemaName.c_str(), (SQLSMALLINT) pkSchemaName.size(),
                        (SQLWCHAR*) pkTableName.c_str(), (SQLSMALLINT) pkTableName.size(),
                        (SQLWCHAR*) fkCatalogName.c_str(), (SQLSMALLINT) fkCatalogName.size(),
                        (SQLWCHAR*) fkSchemaName.c_str(), (SQLSMALLINT) fkSchemaName.size(),
                        (SQLWCHAR*) fkTableName.c_str(), (SQLSMALLINT) fkTableName.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                unsigned int ret;
                SQLSMALLINT columns = 0;
                retcode = SQLNumResultCols(hstmt, &columns);
                assert(SQL_SUCCEEDED(retcode));
                CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, true);
                {
                    CScopeUQueue sbRowset;
                    sbRowset << vInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                }
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return;
                }
                bool ok = PushRecords(hstmt, vInfo, res, errMsg);
                ++m_oks;
                if (!ok) {
                    return;
                }
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::Execute(const std::wstring& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            affected = 0;
            fail_ok = 0;
            if (!m_pOdbc) {
                res = SPA::Odbc::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else {
                res = 0;
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
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_DBC, m_pOdbc.get(), errMsg);
                    ++m_fails;
                    break;
                }
                retcode = SQLExecDirect(hstmt, (SQLWCHAR*) wsql.c_str(), (SQLINTEGER) wsql.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    ++m_fails;
                    break;
                }
                do {
                    SQLSMALLINT columns = 0;
                    retcode = SQLNumResultCols(hstmt, &columns);
                    assert(SQL_SUCCEEDED(retcode));
                    if (columns > 0) {
                        if (rowset) {
                            unsigned int ret;
                            CDBColumnInfoArray vInfo = GetColInfo(hstmt, columns, meta);
                            {
                                CScopeUQueue sbRowset;
                                sbRowset << vInfo << index;
                                ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                            }
                            if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                                return;
                            }
                            bool ok = PushRecords(hstmt, vInfo, res, errMsg);
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
                assert(retcode == SQL_NO_DATA);
            } while (false);
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void COdbcImpl::Prepare(const std::wstring& wsql, CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters) {
            res = 0;
            parameters = 0;
        }

        void COdbcImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            affected = 0;
            res = 0;
            fail_ok = 0;
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
                q >> vt;
                ConvertToUTF8OrDouble(vt);
            }
            assert(q.GetSize() == 0);
        }

        void COdbcImpl::GetErrMsg(SQLSMALLINT HandleType, SQLHANDLE Handle, std::wstring & errMsg) {
            static std::wstring SQLSTATE(L"SQLSTATE=");
            static std::wstring NATIVE_ERROR(L":NATIVE=");
            static std::wstring ERROR_MESSAGE(L":ERROR_MESSAGE=");
            errMsg.clear();
            SQLSMALLINT i = 1, MsgLen = 0;
            SQLINTEGER NativeError = 0;
            SQLWCHAR SqlState[6] =
            {0}, Msg[SQL_MAX_MESSAGE_LENGTH + 1] =
            {0};
            SQLRETURN res = SQLGetDiagRec(HandleType, Handle, i, SqlState, &NativeError, Msg, sizeof (Msg) / sizeof (SQLWCHAR), &MsgLen);
            while (res != SQL_NO_DATA) {
                if (errMsg.size())
                    errMsg += L";";
                errMsg += (SQLSTATE + SqlState);
                errMsg += (NATIVE_ERROR + std::to_wstring((INT64) NativeError));
                errMsg += (ERROR_MESSAGE + Msg);
                ::memset(Msg, 0, sizeof (Msg));
                ++i;
                MsgLen = 0;
                NativeError = 0;
                res = SQLGetDiagRec(HandleType, Handle, i, SqlState, &NativeError, Msg, sizeof (Msg), &MsgLen);
            }
        }

        void COdbcImpl::ConvertToUTF8OrDouble(CDBVariant & vt) {
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
                    const DECIMAL &decVal = vt.decVal;
                    std::string s = std::to_string(decVal.Lo64);
                    unsigned char len = (unsigned char) s.size();
                    if (len <= decVal.scale) {
                        s.insert(0, (decVal.scale - len) + 1, '0');
                    }
                    if (decVal.sign) {
                        s.insert(0, 1, '-');
                    }
                    if (decVal.scale) {
                        size_t pos = s.length() - decVal.scale;
                        s.insert(pos, 1, '.');
                    }
                    vt = s.c_str();
                }
                    break;
                default:
                    break;
            }
        }

    } //namespace ServerSide
} //namespace SPA