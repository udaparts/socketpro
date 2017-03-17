
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

        COdbcImpl::COdbcImpl() : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_global(true), m_Blob(*m_sb), m_parameters(0) {

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

		CDBColumnInfoArray COdbcImpl::GetColInfo(SQLHSTMT hstmt, SQLSMALLINT columns, bool meta) {
			bool primary_key_set = false;
			SQLWCHAR colname[128 + 1] = {0};	// column name
			SQLSMALLINT colnamelen = 0;			// length of column name
			SQLSMALLINT nullable = 0;			// whether column can have NULL value
			SQLULEN collen = 0;					// column lengths
			SQLSMALLINT coltype = 0;			// column type
			SQLSMALLINT decimaldigits = 0;		// no of digits if column is numeric
			SQLLEN displaysize = 0;				// drivers column display size
			CDBColumnInfoArray vCols;
			for (SQLSMALLINT n = 0; n < columns; ++n) {
				vCols.push_back(CDBColumnInfo());
                CDBColumnInfo &info = vCols.back();
				SQLRETURN retcode = SQLDescribeCol(hstmt, (SQLUSMALLINT)n+1, colname, sizeof (colname) / sizeof(SQLWCHAR), &colnamelen, &coltype, &collen, &decimaldigits, &nullable);
				assert(SQL_SUCCEEDED(retcode));
				info.DisplayName.assign(colname, colnamelen);
				if (nullable == SQL_NO_NULLS)
					info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_BASE_COLUMN_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				info.OriginalName.assign(colname, colnamelen / sizeof(SQLWCHAR));

				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_SCHEMA_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				if (colnamelen) {
					info.TablePath.assign(colname, colnamelen / sizeof(SQLWCHAR));
					info.TablePath += L".";
				}
				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_BASE_TABLE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				info.TablePath += colname;

				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_TYPE_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				info.DeclaredType.assign(colname, colnamelen / sizeof(SQLWCHAR));

				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_CATALOG_NAME, colname, sizeof (colname), &colnamelen, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				info.DBPath.assign(colname, colnamelen / sizeof(SQLWCHAR)); //database name

				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_UNSIGNED, nullptr, 0, nullptr, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				switch(coltype) {
				case SQL_CHAR:
				case SQL_VARCHAR:
				case SQL_LONGVARCHAR:
					info.ColumnSize = (unsigned int)collen;
					info.DataType = (VT_ARRAY | VT_I1);
					retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
					assert(SQL_SUCCEEDED(retcode));
					if (displaysize == SQL_TRUE) {
						info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
					}
					break;
				case SQL_WCHAR:
				case SQL_WVARCHAR:
				case SQL_WLONGVARCHAR:
					info.ColumnSize = (unsigned int)collen;
					info.DataType = VT_BSTR;
					retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_CASE_SENSITIVE, nullptr, 0, nullptr, &displaysize);
					assert(SQL_SUCCEEDED(retcode));
					if (displaysize == SQL_TRUE) {
						info.Flags |= CDBColumnInfo::FLAG_CASE_SENSITIVE;
					}
					break;
				case SQL_BINARY:
				case SQL_VARBINARY:
				case SQL_LONGVARBINARY:
					info.ColumnSize = (unsigned int)collen;
					info.DataType = (VT_ARRAY | VT_UI1);
					break;
				case SQL_DECIMAL:
				case SQL_NUMERIC:
					info.ColumnSize = coltype; //remember SQL data type
					info.DataType = VT_DECIMAL;
					info.Scale = (unsigned char)decimaldigits;
					retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_PRECISION, nullptr, 0, nullptr, &displaysize);
					assert(SQL_SUCCEEDED(retcode));
					info.Precision = (unsigned char)displaysize;
					break;
				case SQL_SMALLINT:
					if (displaysize == SQL_TRUE)
						info.DataType = VT_UI2;
					else
						info.DataType = VT_I2;
					break;
				case SQL_INTEGER:
					if (displaysize == SQL_TRUE)
						info.DataType = VT_UI4;
					else
						info.DataType = VT_I4;
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
					if (displaysize == SQL_TRUE)
						info.DataType = VT_UI1;
					else
						info.DataType = VT_I1;
					break;
					break;
				case SQL_BIGINT:
					if (displaysize == SQL_TRUE)
						info.DataType = VT_UI8;
					else
						info.DataType = VT_I8;
					break;
				case SQL_BIT:
					info.DataType = VT_BOOL;
					info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
					break;
				case SQL_GUID:
					info.ColumnSize = sizeof(GUID);
					info.DataType = (VT_ARRAY | VT_UI1);
					info.Collation = L"GUID"; //remember SQL data type
					break;
				case SQL_TYPE_DATE:
				case SQL_TYPE_TIME:
				case SQL_TYPE_TIMESTAMP:
					info.ColumnSize = coltype; //remember SQL data type
					info.DataType = VT_DATE;
					break;
				case SQL_INTERVAL_MONTH:
					break;
				default:
					assert(false); //not supported
					break;
				}
				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_AUTO_UNIQUE_VALUE, nullptr, 0, nullptr, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				if (displaysize == SQL_TRUE) {
					info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
					info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
					info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
					info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
					primary_key_set = true;
				}

				retcode = SQLColAttribute(hstmt, (SQLUSMALLINT)n+1, SQL_DESC_UPDATABLE, nullptr, 0, nullptr, &displaysize);
				assert(SQL_SUCCEEDED(retcode));
				if (displaysize == SQL_ATTR_READONLY) {
					info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
				}
			}
			if (meta && !primary_key_set) {

			}
			return vCols;
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
                    break;
                }
                retcode = SQLExecDirect(hstmt, (SQLWCHAR*) wsql.c_str(), (SQLINTEGER) wsql.size());
                if (!SQL_SUCCEEDED(retcode)) {
                    res = SPA::Odbc::ER_ERROR;
                    GetErrMsg(SQL_HANDLE_STMT, hstmt, errMsg);
                    break;
                }
                do {
                    SQLSMALLINT columns;
                    retcode = SQLNumResultCols(hstmt, &columns);
                    assert(SQL_SUCCEEDED(retcode));
                    if (columns > 0) {
                        if (rowset) {
							CDBColumnInfoArray vCols = GetColInfo(hstmt, columns, meta);
							vCols.clear();
                        }
                    } else {
                        SQLLEN rows = 0;
                        retcode = SQLRowCount(hstmt, &rows);
                        assert(SQL_SUCCEEDED(retcode));
                        if (rows > 0) {
                            affected += rows;
                        }
                    }
                } while ((retcode = SQLMoreResults(hstmt)) == SQL_SUCCESS);
                assert(retcode == SQL_NO_DATA);
            } while (false);
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