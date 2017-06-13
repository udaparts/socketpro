
#include "mysqlimpl.h"
#include <algorithm>
#ifndef NDEBUG
#include <iostream>
#endif
#include "streamingserver.h"

namespace SPA
{
    namespace ServerSide{

        my_bool CMysqlImpl::B_IS_NULL = 1;

        const wchar_t * CMysqlImpl::NO_DB_OPENED_YET = L"No mysql database opened yet";
        const wchar_t * CMysqlImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const wchar_t * CMysqlImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const wchar_t * CMysqlImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const wchar_t * CMysqlImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const wchar_t * CMysqlImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const wchar_t * CMysqlImpl::NO_DB_NAME_SPECIFIED = L"No mysql database name specified";
        const wchar_t * CMysqlImpl::MYSQL_LIBRARY_NOT_INITIALIZED = L"Mysql library not initialized";
        const wchar_t * CMysqlImpl::BAD_MANUAL_TRANSACTION_STATE = L"Bad manual transaction state";
        const wchar_t * CMysqlImpl::UNABLE_TO_SWITCH_TO_DATABASE = L"Unable to switch to database ";

        st_command_service_cbs CMysqlImpl::m_sql_cbs =
        {
            CMysqlImpl::sql_start_result_metadata,
            CMysqlImpl::sql_field_metadata,
            CMysqlImpl::sql_end_result_metadata,
            CMysqlImpl::sql_start_row,
            CMysqlImpl::sql_end_row,
            CMysqlImpl::sql_abort_row,
            CMysqlImpl::sql_get_client_capabilities,
            CMysqlImpl::sql_get_null,
            CMysqlImpl::sql_get_integer,
            CMysqlImpl::sql_get_longlong,
            CMysqlImpl::sql_get_decimal,
            CMysqlImpl::sql_get_double,
            CMysqlImpl::sql_get_date,
            CMysqlImpl::sql_get_time,
            CMysqlImpl::sql_get_datetime,
            CMysqlImpl::sql_get_string,
            CMysqlImpl::sql_handle_ok,
            CMysqlImpl::sql_handle_error,
            CMysqlImpl::sql_shutdown
        };

        CMysqlImpl::CMysqlImpl()
        : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_global(true),
        m_Blob(*m_sb), m_parameters(0), m_bCall(false), m_sql_errno(0),
        m_sc(nullptr), m_sql_resultcs(nullptr), m_sql_num_meta_rows(0),
        m_sql_num_rows(0), m_ColIndex(0), m_Cols(0),
        m_sql_flags(0), m_Rows(0), m_affected_rows(0), m_last_insert_id(0),
        m_server_status(0), m_statement_warn_count(0) {
            m_Blob.ToUtf8(true);
#ifdef WIN32_64
            m_UQueue.TimeEx(true); //use high-precision datetime
#endif
            m_UQueue.ToUtf8(true);
        }

        unsigned int CMysqlImpl::GetParameters() const {
            return (unsigned int) m_parameters;
        }

        bool CMysqlImpl::IsGloballyConnected() const {
            return m_global;
        }

        bool CMysqlImpl::IsStoredProcedure() const {
            return m_bCall;
        }

        const std::string & CMysqlImpl::GetProcedureName() const {
            return m_procName;
        }

        void CALLBACK CMysqlImpl::OnThreadEventEmbedded(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {

            } else {

            }
        }

        void CALLBACK CMysqlImpl::OnThreadEvent(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {

            } else {

            }
        }

        void CMysqlImpl::ReleaseArray() {
            for (auto it = m_vArray.begin(), end = m_vArray.end(); it != end; ++it) {
                SAFEARRAY *arr = *it;
                ::SafeArrayUnaccessData(arr);
            }
            m_vArray.clear();
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
        }

        void CMysqlImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I1_R0(idStartBLOB, StartBLOB, unsigned int)
            M_I0_R0(idChunk, Chunk)
            M_I0_R0(idEndBLOB, EndBLOB)
            M_I0_R0(idBeginRows, BeginRows)
            M_I0_R0(idTransferring, Transferring)
            M_I0_R0(idEndRows, EndRows)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            END_SWITCH
        }

        int CMysqlImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
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

        int CMysqlImpl::sql_start_result_metadata(void *ctx, uint num_cols, uint flags, const CHARSET_INFO * resultcs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_Rows = 0;
            impl->m_Cols = num_cols;
            impl->m_sql_resultcs = resultcs;
            impl->m_sql_flags = flags;
            return 0;
        }

        int CMysqlImpl::sql_field_metadata(void *ctx, struct st_send_field *field, const CHARSET_INFO * charset) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            return 0;
        }

        int CMysqlImpl::sql_end_result_metadata(void *ctx, uint server_status, uint warn_count) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_sql_num_meta_rows = (unsigned int) impl->m_Rows;
            impl->m_Rows = 0;
            return 0;
        }

        int CMysqlImpl::sql_start_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_ColIndex = 0;
            return 0;
        }

        int CMysqlImpl::sql_end_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_Rows;
            return 0;
        }

        void CMysqlImpl::sql_abort_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
        }

        ulong CMysqlImpl::sql_get_client_capabilities(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            return 0;
        }

        int CMysqlImpl::sql_get_null(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_integer(void * ctx, longlong value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_longlong(void * ctx, longlong value, uint is_unsigned) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_decimal(void * ctx, const decimal_t * value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_double(void * ctx, double value, uint32 decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_date(void * ctx, const MYSQL_TIME * value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_time(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_datetime(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_string(void * ctx, const char * const value, size_t length, const CHARSET_INFO * const valuecs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            ++impl->m_ColIndex;
            return 0;
        }

        void CMysqlImpl::sql_handle_ok(void * ctx, uint server_status, uint statement_warn_count, ulonglong affected_rows, ulonglong last_insert_id, const char * const message) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_sql_errno = 0;
            impl->m_server_status = server_status;
            impl->m_statement_warn_count = statement_warn_count;
            impl->m_affected_rows = affected_rows;
            impl->m_last_insert_id = last_insert_id;
            if (message)
                impl->m_err_msg = SPA::Utilities::ToWide(message);
            else
                impl->m_err_msg.clear();
        }

        void CMysqlImpl::sql_handle_error(void * ctx, uint sql_errno, const char * const err_msg, const char * const sqlstate) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_sql_errno = (int) sql_errno;
            impl->m_err_msg = SPA::Utilities::ToWide(err_msg);
            if (sqlstate)
                impl->m_sqlstate = sqlstate;
            else
                impl->m_sqlstate.clear();
        }

        void CMysqlImpl::sql_shutdown(void *ctx, int shutdown_server) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
        }

        void CMysqlImpl::srv_session_error_cb(void *ctx, unsigned int sql_errno, const char *err_msg) {
            CMysqlImpl *p = (CMysqlImpl *) ctx;
            p->m_sql_errno = (int) sql_errno;
            p->m_err_msg = SPA::Utilities::ToWide(err_msg);
        }

        void CMysqlImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            res = 0;
            ms = msMysql;
            CleanDBObjects();

            MYSQL_SESSION st_session = srv_session_open(srv_session_error_cb, this);
            m_pMysql.reset(st_session, [this](MYSQL_SESSION mysql) {
                if (mysql) {
                    srv_session_close(mysql);
                }
            });
            std::wstring user = GetUID();
            std::string userA = SPA::Utilities::ToUTF8(user.c_str(), user.size());
            std::string db = SPA::Utilities::ToUTF8(strConnection.c_str(), strConnection.size());
            my_svc_bool fail = thd_get_security_context(srv_session_info_get_thd(st_session), &m_sc);
            fail = security_context_lookup(m_sc, userA.c_str(), "localhost", "127.0.0.1", db.c_str());
            if (fail) {
                res = SPA::Mysql::ER_UNABLE_TO_SWITCH_TO_DATABASE;
                errMsg = UNABLE_TO_SWITCH_TO_DATABASE + strConnection;
                m_pMysql.reset();
            } else {
                res = 0;
                errMsg = strConnection;
            }
        }

        void CMysqlImpl::CloseDb(int &res, std::wstring & errMsg) {
            CleanDBObjects();
            res = 0;
        }

        void CMysqlImpl::CleanDBObjects() {
            //m_pPrepare.reset();
            m_pMysql.reset();
            m_vParam.clear();
            m_parameters = 0;
            ResetMemories();
        }

        void CMysqlImpl::OnBaseRequestArrive(unsigned short requestId) {
            switch (requestId) {
                case idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                    break;
                default:
                    break;
            }
        }

        void CMysqlImpl::BeginTrans(int isolation, const std::wstring &dbConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msMysql;
            if (m_ti != tiUnspecified || isolation == (int) tiUnspecified) {
                errMsg = BAD_MANUAL_TRANSACTION_STATE;
                res = SPA::Mysql::ER_BAD_MANUAL_TRANSACTION_STATE;
                return;
            }
            if (!m_pMysql) {
                Open(dbConn, flags, res, errMsg, ms);
                if (!m_pMysql) {
                    return;
                }
            } else {

            }
            res = 0;
        }

        void CMysqlImpl::EndTrans(int plan, int &res, std::wstring & errMsg) {
            if (m_ti == tiUnspecified) {
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
            res = 0;
        }

        void CMysqlImpl::ExecuteSqlWithoutRowset(int &res, std::wstring &errMsg, INT64 & affected) {

        }

        bool CMysqlImpl::SendRows(CScopeUQueue& sb, bool transferring) {
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

        const wchar_t * CMysqlImpl::fieldtype2str(enum_field_types type) {
            switch (type) {
                case MYSQL_TYPE_BIT:
                    return L"BIT";
                case MYSQL_TYPE_BLOB:
                    return L"BLOB";
                case MYSQL_TYPE_DATE:
                    return L"DATE";
                case MYSQL_TYPE_DATETIME:
                    return L"DATETIME";
                case MYSQL_TYPE_NEWDECIMAL:
                    return L"NEWDECIMAL";
                case MYSQL_TYPE_DECIMAL:
                    return L"DECIMAL";
                case MYSQL_TYPE_DOUBLE:
                    return L"DOUBLE";
                case MYSQL_TYPE_ENUM:
                    return L"ENUM";
                case MYSQL_TYPE_FLOAT:
                    return L"FLOAT";
                case MYSQL_TYPE_GEOMETRY:
                    return L"GEOMETRY";
                case MYSQL_TYPE_INT24:
                    return L"INT24";
                case MYSQL_TYPE_LONG:
                    return L"LONG";
                case MYSQL_TYPE_LONGLONG:
                    return L"LONGLONG";
                case MYSQL_TYPE_LONG_BLOB:
                    return L"LONG_BLOB";
                case MYSQL_TYPE_MEDIUM_BLOB:
                    return L"MEDIUM_BLOB";
                case MYSQL_TYPE_NEWDATE:
                    return L"NEWDATE";
                case MYSQL_TYPE_NULL:
                    return L"NULL";
                case MYSQL_TYPE_SET:
                    return L"SET";
                case MYSQL_TYPE_SHORT:
                    return L"SHORT";
                case MYSQL_TYPE_STRING:
                    return L"STRING";
                case MYSQL_TYPE_TIME:
                    return L"TIME";
                case MYSQL_TYPE_TIMESTAMP:
                    return L"TIMESTAMP";
                case MYSQL_TYPE_TINY:
                    return L"TINY";
                case MYSQL_TYPE_TINY_BLOB:
                    return L"TINY_BLOB";
                case MYSQL_TYPE_VARCHAR:
                    return L"VARCHAR";
                case MYSQL_TYPE_VAR_STRING:
                    return L"VAR_STRING";
                case MYSQL_TYPE_YEAR:
                    return L"YEAR";
                default:
                    break;
            }
            return L"?-unknown-?";
        }

        void CMysqlImpl::Trim(std::string & s) {
            static const char *WHITESPACE = " \r\n\t\v\f\v";
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

        void CMysqlImpl::ExecuteSqlWithRowset(bool meta, UINT64 index, int &res, std::wstring &errMsg, INT64 & affected) {

        }

        void CMysqlImpl::Execute(const std::wstring& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
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
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            COM_DATA cmd;
			::memset(&cmd, 0, sizeof(cmd));
			m_sql_errno = 0;
			m_last_insert_id = 0;
            cmd.com_query.query = sqlUtf8;
            cmd.com_query.length = sb->GetSize();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            if (fail || m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
				affected = 0;
				vtId = m_last_insert_id;
                ++m_fails;
            } else {
                ++m_oks;
                affected = (INT64) m_affected_rows;
                if (lastInsertId)
                    vtId = m_last_insert_id;
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
                CMysqlImpl::Trim(m_procName);
            } else {
                m_procName.clear();
            }
        }

        void CMysqlImpl::Prepare(const std::wstring& wsql, CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters) {
            ResetMemories();
            parameters = 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            res = 0;
            m_vParam.clear();
            m_parameters = 0;
            m_sqlPrepare = Utilities::ToUTF8(wsql.c_str(), wsql.size());
            CMysqlImpl::Trim(m_sqlPrepare);
            PreprocessPreparedStatement();
            my_bool fail;
        }

        int CMysqlImpl::Bind(CUQueue &qBufferSize, int row, std::wstring & errMsg) {
            int res = 0;
            if (!m_parameters) {
                return res;
            }

            return res;
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

        void CMysqlImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
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
            res = 0;
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

    } //namespace ServerSide
} //namespace SPA