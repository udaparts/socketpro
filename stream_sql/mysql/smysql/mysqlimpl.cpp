
#include "mysqlimpl.h"
#include <algorithm>
#ifndef NDEBUG
#include <iostream>
#endif
#include "streamingserver.h"
#include "include/mysqld_error.h"

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
        const wchar_t * CMysqlImpl::SERVICE_COMMAND_ERROR = L"Service command error";

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
        : m_EnableMessages(false), m_oks(0), m_fails(0), m_ti(tiUnspecified),
        m_qSend(*m_sb), m_stmt(0, false), m_bCall(false), m_bExecutingParameters(false), m_NoSending(false), m_sql_errno(0),
        m_sc(nullptr), m_sql_resultcs(nullptr), m_ColIndex(0),
        m_sql_flags(0), m_affected_rows(0), m_last_insert_id(0),
        m_server_status(0), m_statement_warn_count(0), m_indexCall(0), m_bBlob(false) {
            m_qSend.ToUtf8(true);
#ifdef WIN32_64
            m_UQueue.TimeEx(true); //use high-precision datetime
#endif
            m_UQueue.ToUtf8(true);
        }

        CMysqlImpl::~CMysqlImpl() {
            CleanDBObjects();
        }

        unsigned int CMysqlImpl::GetParameters() const {
            return (unsigned int) m_stmt.parameters;
        }

        bool CMysqlImpl::IsStoredProcedure() const {
            return m_bCall;
        }

        const std::string & CMysqlImpl::GetProcedureName() const {
            return m_procName;
        }

        void CALLBACK CMysqlImpl::OnThreadEvent(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {
                my_bool fail = srv_session_init_thread(CSetGlobals::Globals.Plugin);
                assert(!fail);
            } else {
                srv_session_deinit_thread();
            }
        }

        void CMysqlImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            CleanDBObjects();
        }

        void CMysqlImpl::ResetMemories() {
            m_qSend.SetSize(0);
            if (m_qSend.GetMaxSize() > 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_qSend.ReallocBuffer(2 * DEFAULT_BIG_FIELD_CHUNK_SIZE);
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
            END_SWITCH
        }

        int CMysqlImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            m_NoSending = false;
            BEGIN_SWITCH(reqId)
            M_I2_R3(idOpen, Open, std::wstring, unsigned int, int, std::wstring, int)
            M_I3_R3(idBeginTrans, BeginTrans, int, std::wstring, unsigned int, int, std::wstring, int)
            M_I1_R2(idEndTrans, EndTrans, int, int, std::wstring)
            M_I5_R5(idExecute, Execute, std::wstring, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I2_R3(idPrepare, Prepare, std::wstring, CParameterInfoArray, int, std::wstring, unsigned int)
            M_I4_R5(idExecuteParameters, ExecuteParameters, bool, bool, bool, UINT64, INT64, int, std::wstring, CDBVariant, UINT64)
            M_I0_R2(idClose, CloseDb, int, std::wstring)
            END_SWITCH
            if (m_pMysql) {
                my_bool fail = srv_session_detach(m_pMysql.get());
            }
            return 0;
        }

        int CMysqlImpl::sql_start_result_metadata(void *ctx, uint num_cols, uint flags, const CHARSET_INFO * resultcs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (!impl)
                return 1;
            impl->m_sql_resultcs = resultcs;
            impl->m_sql_flags = flags;
            impl->m_vColInfo.clear();
            impl->m_bBlob = false;
            return 0;
        }

        int CMysqlImpl::sql_field_metadata(void *ctx, struct st_send_field *f, const CHARSET_INFO * charset) {
            CDBColumnInfo info;
            size_t len = strlen(f->col_name);
            info.DisplayName.assign(f->col_name, f->col_name + len);
            if (f->org_col_name && (len = strlen(f->org_col_name)))
                info.OriginalName.assign(f->org_col_name, f->org_col_name + len);
            else
                info.OriginalName = info.DisplayName;

            if (f->org_table_name && (len = strlen(f->org_table_name)))
                info.TablePath.assign(f->org_table_name, f->org_table_name + len);
            else if (f->table_name && (len = strlen(f->table_name)))
                info.TablePath.assign(f->table_name, f->table_name + len);

            if (f->db_name && (len = strlen(f->db_name)))
                info.DBPath.assign(f->db_name, f->db_name + len);

            if ((f->flags & NOT_NULL_FLAG) == NOT_NULL_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
            }
            if ((f->flags & PRI_KEY_FLAG) == PRI_KEY_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
            }
            if ((f->flags & UNIQUE_KEY_FLAG) == UNIQUE_KEY_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
            }
            if ((f->flags & AUTO_INCREMENT_FLAG) == AUTO_INCREMENT_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                info.Flags |= CDBColumnInfo::FLAG_NOT_WRITABLE;
            }
            if ((f->flags & ENUM_FLAG) == ENUM_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_IS_ENUM;
            } else if ((f->flags & SET_FLAG) == SET_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_IS_SET;
            }

            if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG) {
                info.Flags |= CDBColumnInfo::FLAG_IS_UNSIGNED;
            }

            switch (f->type) {
                case MYSQL_TYPE_BIT:
                    info.ColumnSize = f->length;
                    info.DeclaredType = L"BIT";
                    info.Flags |= CDBColumnInfo::FLAG_IS_BIT;
                    if (f->length == 1)
                        info.DataType = VT_BOOL;
                    else if (f->length <= 8)
                        info.DataType = VT_UI1;
                    else if (f->length <= 16)
                        info.DataType = VT_UI2;
                    else if (f->length <= 32)
                        info.DataType = VT_UI4;
                    else if (f->length <= 64)
                        info.DataType = VT_UI8;
                    else {
                        assert(false); //not implemented
                    }
                    break;
                case MYSQL_TYPE_LONG_BLOB:
                    info.ColumnSize = f->length;
                    if (f->charsetnr == IS_BINARY) {
                        info.DeclaredType = L"LONG_BLOB";
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary
                    } else {
                        info.DeclaredType = L"LONG_TEXT";
                        info.DataType = (VT_I1 | VT_ARRAY); //text
                    }
                    break;
                case MYSQL_TYPE_BLOB:
                    info.ColumnSize = f->length;
                    if (f->charsetnr == IS_BINARY) {
                        if (f->length == MYSQL_TINYBLOB) {
                            info.DeclaredType = L"TINY_BLOB";
                        } else if (f->length == MYSQL_MIDBLOB) {
                            info.DeclaredType = L"MEDIUM_BLOB";
                        } else if (f->length == MYSQL_BLOB) {
                            info.DeclaredType = L"BLOB";
                        } else {
                            info.DeclaredType = L"LONG_BLOB";
                        }
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary
                    } else {
                        if (f->length == MYSQL_TINYBLOB) {
                            info.DeclaredType = L"TINY_TEXT";
                        } else if (f->length == MYSQL_MIDBLOB) {
                            info.DeclaredType = L"MEDIUM_TEXT";
                        } else if (f->length == MYSQL_BLOB) {
                            info.DeclaredType = L"TEXT";
                        } else {
                            info.DeclaredType = L"LONG_TEXT";
                        }
                        info.DataType = (VT_I1 | VT_ARRAY); //text
                    }
                    break;
                case MYSQL_TYPE_MEDIUM_BLOB:
                    info.ColumnSize = f->length;
                    if (f->charsetnr == IS_BINARY) {
                        info.DeclaredType = L"MEDIUM_BLOB";
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary
                    } else {
                        info.DeclaredType = L"MEDIUM_TEXT";
                        info.DataType = (VT_I1 | VT_ARRAY); //text
                    }
                    break;
                case MYSQL_TYPE_DATE:
                    info.DeclaredType = L"DATE";
                    info.DataType = VT_DATE;
                    break;
                case MYSQL_TYPE_NULL:
                    info.DeclaredType = L"NULL";
                    info.DataType = VT_NULL;
                    break;
                case MYSQL_TYPE_NEWDATE:
                    info.DeclaredType = L"NEWDATE";
                    info.DataType = VT_DATE;
                    break;
                case MYSQL_TYPE_SET:
                    info.ColumnSize = f->length;
                    info.DeclaredType = L"SET";
                    info.DataType = (VT_I1 | VT_ARRAY); //string
                    break;
                case MYSQL_TYPE_DATETIME:
                    info.DeclaredType = L"DATETIME";
                    info.Scale = (unsigned char) f->decimals;
                    info.DataType = VT_DATE;
                    break;
                case MYSQL_TYPE_NEWDECIMAL:
                    info.DeclaredType = L"NEWDECIMAL";
                    info.DataType = VT_DECIMAL;
                    info.Scale = (unsigned char) f->decimals;
                    info.Precision = (unsigned char) (f->length - f->decimals);
                    break;
                case MYSQL_TYPE_DECIMAL:
                    info.DeclaredType = L"DECIMAL";
                    info.DataType = VT_DECIMAL;
                    info.Scale = (unsigned char) f->decimals;
                    info.Precision = (unsigned char) (f->length - f->decimals);
                    break;
                case MYSQL_TYPE_DOUBLE:
                    info.DeclaredType = L"DOUBLE";
                    info.DataType = VT_R8;
                    break;
                case MYSQL_TYPE_ENUM:
                    info.ColumnSize = f->length;
                    info.DeclaredType = L"ENUM";
                    info.DataType = (VT_I1 | VT_ARRAY); //string
                    break;
                case MYSQL_TYPE_FLOAT:
                    info.DeclaredType = L"FLOAT";
                    info.DataType = VT_R4;
                    break;
                case MYSQL_TYPE_GEOMETRY:
                    info.ColumnSize = f->length;
                    info.DeclaredType = L"GEOMETRY";
                    info.DataType = (VT_UI1 | VT_ARRAY); //binary array
                    break;
                case MYSQL_TYPE_INT24:
                    info.DeclaredType = L"INT24";
                    if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                        info.DataType = VT_UI4;
                    else
                        info.DataType = VT_I4;
                    break;
                case MYSQL_TYPE_LONG:
                    info.DeclaredType = L"INT";
                    if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                        info.DataType = VT_UI4;
                    else
                        info.DataType = VT_I4;
                    break;
                case MYSQL_TYPE_LONGLONG:
                    info.DeclaredType = L"BIGINT";
                    if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                        info.DataType = VT_UI8;
                    else
                        info.DataType = VT_I8;
                    break;
                case MYSQL_TYPE_SHORT:
                    info.DeclaredType = L"SHORT";
                    if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                        info.DataType = VT_UI2;
                    else
                        info.DataType = VT_I2;
                    break;
                case MYSQL_TYPE_STRING:
                    if ((f->flags & ENUM_FLAG) == ENUM_FLAG) {
                        info.DeclaredType = L"ENUM";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                    } else if ((f->flags & SET_FLAG) == SET_FLAG) {
                        info.DeclaredType = L"SET";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                    } else {
                        if (f->charsetnr == IS_BINARY) {
                            info.DeclaredType = L"BINARY";
                            info.DataType = (VT_UI1 | VT_ARRAY);
                        } else {
                            info.DeclaredType = L"CHAR";
                            info.DataType = (VT_I1 | VT_ARRAY); //string
                        }
                    }
                    info.ColumnSize = f->length / 3;
                    break;
                case MYSQL_TYPE_TIME:
                    info.DeclaredType = L"TIME";
                    info.Scale = (unsigned char) f->decimals;
                    info.DataType = VT_DATE;
                    break;
                case MYSQL_TYPE_TIMESTAMP:
                    info.DeclaredType = L"TIMESTAMP";
                    info.Scale = (unsigned char) f->decimals;
                    info.DataType = VT_DATE;
                    break;
                case MYSQL_TYPE_TINY:
                    info.DeclaredType = L"TINY";
                    if ((f->flags & UNSIGNED_FLAG) == UNSIGNED_FLAG)
                        info.DataType = VT_UI1;
                    else
                        info.DataType = VT_I1;
                    break;
                case MYSQL_TYPE_JSON:
                    info.ColumnSize = f->length;
                    if (!info.ColumnSize)
                        info.ColumnSize = (~0);
                    info.DeclaredType = L"JSON";
                    info.DataType = (VT_I1 | VT_ARRAY); //string
                    break;
                case MYSQL_TYPE_TINY_BLOB:
                    info.ColumnSize = f->length;
                    if (f->charsetnr == IS_BINARY) {
                        info.DeclaredType = L"TINY_BLOB";
                        info.DataType = (VT_UI1 | VT_ARRAY); //binary
                    } else {
                        info.DeclaredType = L"TINY_TEXT";
                        info.DataType = (VT_I1 | VT_ARRAY); //text
                    }
                    break;
                case MYSQL_TYPE_VAR_STRING:
                case MYSQL_TYPE_VARCHAR:
                    info.ColumnSize = f->length / 3;
                    if (f->charsetnr == IS_BINARY) {
                        info.DeclaredType = L"VARBINARY";
                        info.DataType = (VT_UI1 | VT_ARRAY);
                    } else {
                        info.DeclaredType = L"VARCHAR";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                    }
                    break;
                case MYSQL_TYPE_YEAR:
                    info.DeclaredType = L"YEAR";
                    info.DataType = VT_I2;
                    break;
                default:
                    info.ColumnSize = f->length;
                    info.DeclaredType = L"?-unknown-?";
                    if (f->charsetnr == IS_BINARY) {
                        info.DeclaredType = L"VARBINARY";
                        info.DataType = (VT_UI1 | VT_ARRAY);
                    } else {
                        info.DeclaredType = L"VARCHAR";
                        info.DataType = (VT_I1 | VT_ARRAY); //string
                    }
                    break;
            }
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_vColInfo.push_back(info);
            return 0;
        }

        int CMysqlImpl::sql_end_result_metadata(void *ctx, uint server_status, uint warn_count) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_server_status = server_status;
            CUQueue &q = impl->m_qSend;
            q.SetSize(0);
            if (!impl->m_NoSending) {
                q << impl->m_vColInfo << impl->m_indexCall;
                if ((server_status & SERVER_PS_OUT_PARAMS) == SERVER_PS_OUT_PARAMS) {
                    q << (unsigned int) impl->m_vColInfo.size();
                }
                unsigned int ret = impl->SendResult(idRowsetHeader, q.GetBuffer(), q.GetSize());
                q.SetSize(0);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return 1; //an error occured, server will abort the command
                }
            }
            return 0;
        }

        int CMysqlImpl::sql_start_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            impl->m_ColIndex = 0;
            return 0;
        }

        int CMysqlImpl::sql_end_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            if ((q.GetSize() >= DEFAULT_RECORD_BATCH_SIZE || impl->m_bBlob) && !impl->SendRows(q)) {
                return 1;
            }
            impl->m_bBlob = false;
            return 0;
        }

        void CMysqlImpl::sql_abort_row(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
        }

        ulong CMysqlImpl::sql_get_client_capabilities(void *ctx) {
            ulong power = (CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_PS_MULTI_RESULTS);
            return power;
        }

        int CMysqlImpl::sql_get_null(void *ctx) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_NULL;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_integer(void * ctx, longlong value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            q << info.DataType;
            switch (info.DataType) {
                default:
                    assert(false);
                    break;
                case VT_UI1:
                    q.Push((const unsigned char*) &value, sizeof (unsigned char));
                    break;
                case VT_UI2:
                    q << (const unsigned short&) value;
                    break;
                case VT_UI4:
                    q << (const unsigned int&) value;
                    break;
                case VT_UI8:
                    q << (const UINT64&) value;
                    break;
                case VT_I1:
                    q.Push((const char*) &value, sizeof (char));
                    break;
                case VT_I2:
                    q << (const short&) value;
                    break;
                case VT_I4:
                    q << (const int&) value;
                    break;
                case VT_I8:
                    q << (const INT64&) value;
                    break;
            }
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_longlong(void * ctx, longlong value, uint is_unsigned) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            q << info.DataType;
            switch (info.DataType) {
                default:
                    assert(false);
                    break;
                case VT_I1:
                    q.Push((const char*) &value, sizeof (char));
                    break;
                case VT_I2:
                    q << (const short&) value;
                    break;
                case VT_I4:
                    q << (const int&) value;
                    break;
                case VT_I8:
                    q << (const INT64&) value;
                    break;
                case VT_UI1:
                    q.Push((const unsigned char*) &value, sizeof (unsigned char));
                    break;
                case VT_UI2:
                    q << (const unsigned short&) value;
                    break;
                case VT_UI4:
                    q << (const unsigned int&) value;
                    break;
                case VT_UI8:
                    q << (const UINT64&) value;
                    break;
            }
            ++impl->m_ColIndex;
            return 0;
        }

        void CMysqlImpl::ToDecimal(const decimal_t &src, bool large, DECIMAL & dec) {
            char str[64] =
            {0};
            int len = sizeof (str);
            CSetGlobals::Globals.decimal2string(&src, str, &len, 0, 0, 0);
            if (large) {
                SPA::ParseDec_long(str, dec);
            } else {
                SPA::ParseDec(str, dec);
            }
        }

        int CMysqlImpl::sql_get_decimal(void * ctx, const decimal_t * value) {
            DECIMAL dec;
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            ToDecimal(*value, info.Precision > 19, dec);
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DECIMAL << dec;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_double(void * ctx, double value, uint32 decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            q << info.DataType;
            switch (info.DataType) {
                default:
                    assert(false);
                    break;
                case VT_R4:
                    q << (const float&) value;
                    break;
                case VT_R8:
                    q << value;
                    break;
            }
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_date(void * ctx, const MYSQL_TIME * value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_time(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_datetime(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_string(void * ctx, const char * const value, size_t length, const CHARSET_INFO * const valuecs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            CUQueue &q = impl->m_qSend;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            if (info.DeclaredType == L"BIT") {
                q << info.DataType;
                if (info.DataType == VT_BOOL) {
                    VARIANT_BOOL b = (*value ? VARIANT_TRUE : VARIANT_FALSE);
                    q << b;
                } else {
                    switch (info.DataType) {
                        case VT_UI1:
                            assert(length == sizeof (unsigned char));
                            q.Push((const unsigned char*) value, sizeof (unsigned char));
                            break;
                        case VT_UI2:
                        {
                            assert(length == sizeof (unsigned short));
                            UINT64 data = ConvertBitsToInt((const unsigned char *) value, sizeof (unsigned short));
                            q << (unsigned short) data;
                        }
                            break;
                        case VT_UI4:
                        {
                            assert(length == sizeof (unsigned int));
                            UINT64 data = ConvertBitsToInt((const unsigned char *) value, sizeof (unsigned int));
                            q << (unsigned int) data;
                        }
                            break;
                        case VT_I8:
                        {
                            assert(length == sizeof (UINT64));
                            UINT64 data = ConvertBitsToInt((const unsigned char *) value, sizeof (UINT64));
                            q << data;
                        }
                            break;
                        default:
                            assert(false);
                            break;
                    }
                }
            } else {
                if (info.DataType == VT_DECIMAL) {
                    q << info.DataType;
                    DECIMAL dec;
                    if (length <= 20) {
                        ParseDec(value, dec);
                    } else {
                        ParseDec_long(value, dec);
                    }
                    q << dec;
                } else if (length <= DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    q << info.DataType;
                    q << (unsigned int) length;
                    q.Push((const unsigned char*) value, (unsigned int) length);
                } else {
                    if (q.GetSize() && !impl->SendRows(q, true)) {
                        return 1;
                    }
                    bool batching = impl->IsBatching();
                    if (batching) {
                        impl->CommitBatching();
                    }
                    if (!impl->SendBlob(info.DataType, (const unsigned char *) value, (unsigned int) length)) {
                        return 1;
                    }
                    if (batching) {
                        impl->StartBatching();
                    }
                    impl->m_bBlob = true;
                }
            }
            ++impl->m_ColIndex;
            return 0;
        }

        void CMysqlImpl::sql_handle_ok(void * ctx, uint server_status, uint statement_warn_count, ulonglong affected_rows, ulonglong last_insert_id, const char * const message) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (!impl)
                return;
            CUQueue &q = impl->m_qSend;
            if ((impl->m_server_status & SERVER_PS_OUT_PARAMS) == SERVER_PS_OUT_PARAMS) {
                //tell output parameter data
                unsigned int sent = impl->SendResult(idOutputParameter, q.GetBuffer(), q.GetSize());
                if (sent == REQUEST_CANCELED || sent == SOCKET_NOT_FOUND) {
                    return;
                }
            } else if (q.GetSize()) {
                if (!impl->SendRows(q))
                    return;
            }
            impl->m_sql_errno = 0;
            impl->m_server_status = server_status;
            impl->m_statement_warn_count = statement_warn_count;
            impl->m_affected_rows += affected_rows;
            impl->m_last_insert_id = last_insert_id;
            if (!impl->m_bExecutingParameters)
                ++impl->m_oks;
            if (message)
                impl->m_err_msg = SPA::Utilities::ToWide(message);
            else
                impl->m_err_msg.clear();
        }

        void CMysqlImpl::sql_handle_error(void * ctx, uint sql_errno, const char * const err_msg, const char * const sqlstate) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (!impl)
                return;
            ++impl->m_fails;
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

        bool CMysqlImpl::OpenSession(const std::wstring &userName, const std::string & ip) {
            MYSQL_SESSION st_session = srv_session_open(nullptr, this);
            if (!st_session)
                return false;
            m_pMysql.reset(st_session, [](MYSQL_SESSION mysql) {
                if (mysql) {
                    my_bool fail = srv_session_detach(mysql);
                    //assert(!fail);
                    fail = srv_session_close(mysql);
                    assert(!fail);
                }
            });
            my_bool fail = srv_session_info_set_connection_type(st_session, VIO_TYPE_PLUGIN);
            assert(!fail);
            if (fail)
                return false;
            fail = thd_get_security_context(srv_session_info_get_thd(st_session), &m_sc);
            assert(!fail);
            if (fail)
                return false;
            std::string userA = SPA::Utilities::ToUTF8(userName.c_str(), userName.size());
            fail = security_context_lookup(m_sc, userA.c_str(), "localhost", ip.c_str(), nullptr);
            if (fail) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "looking up security context failed(user_id=%s; ip_address==%s)", userA.c_str(), ip.c_str());
                return false;
            }
            return true;
        }

        void CMysqlImpl::SetPublishDBEvent(CMysqlImpl & impl) {
#ifdef WIN32_64
            std::wstring wsql = L"CREATE FUNCTION PublishDBEvent RETURNS INTEGER SONAME 'smysql.dll'";
#else
            std::wstring wsql = L"CREATE FUNCTION PublishDBEvent RETURNS INTEGER SONAME 'smysql.so'";
#endif
            if (!impl.m_pMysql && !impl.OpenSession(L"root", "localhost"))
                return;
            impl.m_NoSending = true;
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            std::wstring errMsg;
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            //Setting streaming DB events failed(errCode=1125; errMsg=Function 'PublishDBEvent' already exists)
            if (res && res != ER_UDF_EXISTS) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Setting streaming DB events failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
            }
        }

        std::unordered_map<std::string, std::string> CMysqlImpl::ConfigStreamingDB(CMysqlImpl & impl) {
            std::unordered_map<std::string, std::string> map;
            if (!impl.m_pMysql && !impl.OpenSession(L"root", "localhost"))
                return map;
            std::wstring wsql = L"Create database if not exists sp_streaming_db character set utf8 collate utf8_general_ci;USE sp_streaming_db";
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            impl.m_NoSending = true;
            std::wstring errMsg;
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Configuring streaming DB failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return map;
            }
            wsql = L"CREATE TABLE IF NOT EXISTS config(mykey varchar(32) PRIMARY KEY NOT NULL, value text not null)";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Configuring streaming DB failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return map;
            }
            wsql = L"select mykey, value from config";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Configuring streaming DB failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return map;
            }
            SPA::UDB::CDBVariant vtKey, vtValue;
            while (impl.m_qSend.GetSize() && !res) {
                impl.m_qSend >> vtKey >> vtValue;
                std::string s0 = ToString(vtKey);
                std::string s1 = ToString(vtValue);
                std::transform(s0.begin(), s0.end(), s0.begin(), ::tolower);
                if (s0 == STREAMING_DB_CACHE_TABLES)
                    std::transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
                map[s0] = s1;
            }
			std::unordered_map<std::string, std::string> &config = CSetGlobals::Globals.DefaultConfig;
			for (auto it = config.begin(), end = config.end(); it != end; ++it) {
				auto found = map.find(it->first);
				if (found == map.end()) {
					wsql = L"insert into config values('" + Utilities::ToWide(it->first.c_str(), it->first.size()) + L"','" + Utilities::ToWide(it->second.c_str(), it->second.size()) + L"')";
					impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
					map[it->first] = it->second;
				}
			}
            return map;
        }

        bool CMysqlImpl::Authenticate(const std::wstring &userName, const wchar_t *password, const std::string & ip) {
            std::unique_ptr<CMysqlImpl> impl(new CMysqlImpl);
            if (!impl->OpenSession(userName, ip))
                return false;
            std::wstring wsql(L"select host from mysql.user where password_expired='N' and account_locked='N' and user='");
            wsql += (userName + L"' and authentication_string=password('");
            wsql += password;
            wsql += L"')";
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            impl->m_NoSending = true;
            std::wstring errMsg;
            impl->Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return false;
            }
            memset(&wsql[0], 0, wsql.size() * sizeof (wchar_t));
            SPA::UDB::CDBVariant vt0;
            impl->m_qSend.Utf8ToW(true);
            if (impl->m_qSend.GetSize() && !res) {
                impl->m_qSend >> vt0;
                std::wstring s = vt0.bstrVal;
                std::transform(s.begin(), s.end(), s.begin(), ::tolower);
                if (s == L"localhost" || s == L"127.0.0.1" || s == L"::ffff:127.0.0.1" || s == L"::1") {
                    if (ip != "localhost" && ip != "127.0.0.1" && ip != "::ffff:127.0.0.1" && ip != "::1") {
                        CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed(user_id=%s; ip_address=%s)", SPA::Utilities::ToUTF8(userName.c_str(), userName.size()).c_str(), ip.c_str());
                        return false;
                    }
                }
                return true;
            }
            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed(user_id=%s; ip_address=%s)", SPA::Utilities::ToUTF8(userName.c_str(), userName.size()).c_str(), ip.c_str());
            return false;
        }

        void CMysqlImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            unsigned int port;
            res = 0;
            ms = msMysql;
            m_EnableMessages = false;
            CleanDBObjects();
            std::string ip = GetPeerName(&port);
            std::wstring user = GetUID();
            OpenSession(user, ip);
            InitMysqlSession();
            my_bool fail = 0;
            if (strConnection.size()) {
                std::string db = SPA::Utilities::ToUTF8(strConnection.c_str(), strConnection.size());
                COM_DATA cmd;
                cmd.com_init_db.db_name = db.c_str();
                cmd.com_init_db.length = (unsigned long) db.size();
                fail = command_service_run_command(m_pMysql.get(), COM_INIT_DB, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, nullptr);
            }
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
                m_pMysql.reset();
            } else if (fail) {
                res = SPA::Mysql::ER_UNABLE_TO_SWITCH_TO_DATABASE;
                errMsg = UNABLE_TO_SWITCH_TO_DATABASE + strConnection;
                m_pMysql.reset();
            } else {
                res = 0;
                LEX_CSTRING db_name = srv_session_info_get_current_db(m_pMysql.get());
                errMsg = SPA::Utilities::ToWide(db_name.str, db_name.length);
                if ((flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) == SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) {
                    m_EnableMessages = GetPush().Subscribe(&SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
                }
            }
        }

        void CMysqlImpl::CloseDb(int &res, std::wstring & errMsg) {
            if (m_EnableMessages) {
                GetPush().Unsubscribe();
                m_EnableMessages = false;
            }
            CleanDBObjects();
            res = 0;
        }

        void CMysqlImpl::CloseStmt() {
            m_stmt.parameters = 0;
            if (m_stmt.prepared) {
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                cmd.com_stmt_close.stmt_id = m_stmt.stmt_id;
                my_bool fail = command_service_run_command(m_pMysql.get(), COM_STMT_CLOSE, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, nullptr);
                assert(!fail);
                m_stmt.prepared = false;

                //don't set stmt_it to 0 !!!
            }
        }

        void CMysqlImpl::CleanDBObjects() {
            CloseStmt();
            m_pMysql.reset();
            m_stmt.stmt_id = 0;
            m_vParam.clear();
            ResetMemories();
        }

        void CMysqlImpl::OnBaseRequestArrive(unsigned short requestId) {
            m_bExecutingParameters = false;
            switch (requestId) {
                case idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                {
                    int res;
                    std::wstring errMsg;
                    EndTrans((int) rpRollbackAlways, res, errMsg);
                }
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
                std::string sql;
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
                        break;
                }
                if (sql.size())
                    sql += ";";
                sql += "SET AUTOCOMMIT=0;START TRANSACTION";
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                cmd.com_query.query = sql.c_str();
                cmd.com_query.length = (unsigned int) sql.size();
                my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
                if (m_sql_errno) {
                    res = m_sql_errno;
                    errMsg = m_err_msg;
                } else if (fail) {
                    errMsg = SERVICE_COMMAND_ERROR;
                    res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
                } else {
                    res = 0;
                    m_fails = 0;
                    m_oks = 0;
                    m_ti = (tagTransactionIsolation) isolation;
                }
            }
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
            std::string sql(rollback ? "ROLLBACK" : "COMMIT");
            sql += ";SET AUTOCOMMIT=1";
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_query.query = sql.c_str();
            cmd.com_query.length = (unsigned int) sql.size();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
            } else if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
            } else {
                res = 0;
                m_ti = tiUnspecified;
                m_fails = 0;
                m_oks = 0;
            }

        }

        bool CMysqlImpl::SendRows(CUQueue& sb, bool transferring) {
            if (m_NoSending)
                return true;
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

        bool CMysqlImpl::SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) {
            if (m_NoSending) {
                m_qSend << data_type << bytes;
                m_qSend.Push(buffer, bytes);
                return true;
            }
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

        void CMysqlImpl::InitMysqlSession() {
            m_affected_rows = 0;
            m_vColInfo.clear();
            m_sql_errno = 0;
            m_last_insert_id = 0;
            m_err_msg.clear();
            m_qSend.SetSize(0);
        }

        void CMysqlImpl::Execute(const std::wstring& wsql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            m_indexCall = index;
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
            std::string sql = SPA::Utilities::ToUTF8(wsql.c_str(), wsql.size());
            if (m_EnableMessages && !sql.size() && CSetGlobals::Globals.cached_tables.size()) {
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
            InitMysqlSession();
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            cmd.com_query.query = sql.c_str();
            cmd.com_query.length = (unsigned int) sql.size();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
                affected = 0;
                vtId = (SPA::UINT64)m_last_insert_id;
            } else if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
                ++m_fails;
            } else {
                affected = (SPA::INT64) m_affected_rows;
                if (lastInsertId)
                    vtId = (SPA::UINT64)m_last_insert_id;
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
            CloseStmt();
            res = 0;
            m_vParam.clear();
            m_sqlPrepare = Utilities::ToUTF8(wsql.c_str(), wsql.size());
            CMysqlImpl::Trim(m_sqlPrepare);
            PreprocessPreparedStatement();
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_stmt_prepare.query = sqlUtf8;
            cmd.com_stmt_prepare.length = sb->GetSize();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_STMT_PREPARE, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            ++m_stmt.stmt_id; //always increase statement id by one
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
            } else if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
            } else {
                m_stmt.prepared = true;
                m_stmt.parameters = ComputeParameters(wsql);
                parameters = (unsigned int) m_stmt.parameters;
                if (parameters == 0) {
                    res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                    errMsg = NO_PARAMETER_SPECIFIED;
                    CloseStmt();
                } else {
                    res = 0;
                }
            }
        }

        size_t CMysqlImpl::ComputeParameters(const std::wstring & sql) {
            const wchar_t coma = '\'', slash = '\\', question = '?', at = '@';
            bool b_slash = false, balanced = true;
            size_t params = 0, len = sql.size();
            for (size_t n = 0; n < len; ++n) {
                const wchar_t &c = sql[n];
                if (c == slash) {
                    b_slash = true;
                    continue;
                }
                if (c == coma && b_slash) {
                    b_slash = false;
                    continue; //ignore a coma if there is a slash ahead
                }
                b_slash = false;
                if (c == coma) {
                    balanced = (!balanced);
                    continue;
                }
                if (balanced) {
                    params += ((c == question || c == at) ? 1 : 0);
                }
            }
            return params;
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

        void CMysqlImpl::ReserveNullBytesPlus(CUQueue& buffer, unsigned int parameters) {
            unsigned int null_bytes = (parameters + 7) / 8;
            buffer.SetSize(0);
            buffer.CleanTrack();
            buffer.SetSize(null_bytes + 1);
            unsigned char *header = (unsigned char*) buffer.GetBuffer();
            header[null_bytes] = 1; //always send types to server
        }

        void CMysqlImpl::StoreParamNull(CUQueue& buffer, unsigned int pos) {
            unsigned char *header = (unsigned char*) buffer.GetBuffer();
            header[pos / 8] |= (unsigned char) (1 << (pos & 7));
        }

        void CMysqlImpl::StoreFixedParam(CUQueue& buffer, char c) {
            buffer.Push((const unsigned char*) &c, sizeof (c));
        }

        void CMysqlImpl::StoreFixedParam(CUQueue& buffer, unsigned char c) {
            buffer.Push((const unsigned char*) &c, sizeof (c));
        }

        void CMysqlImpl::StoreParamTime(CUQueue& buffer, const MYSQL_TIME & dt) {
            const MYSQL_TIME *tm = &dt;
            uchar buff[13], *pos;
            pos = buff + 1;
            pos[0] = tm->neg ? 1 : 0;
            int4store(pos + 1, tm->day);
            pos[5] = (uchar) tm->hour;
            pos[6] = (uchar) tm->minute;
            pos[7] = (uchar) tm->second;
            int4store(pos + 8, tm->second_part);
            uint length;
            if (tm->second_part)
                length = 12;
            else if (tm->hour || tm->minute || tm->second || tm->day)
                length = 8;
            else
                length = 0;
            buff[0] = (char) length++;
            buffer.Push(buff, length);
        }

        void CMysqlImpl::StoreParamDateTime(CUQueue& buffer, const MYSQL_TIME & dt) {
            const MYSQL_TIME *tm = &dt;
            uchar buff[12], *pos;
            pos = buff + 1;
            int2store(pos, tm->year);
            pos[2] = (uchar) tm->month;
            pos[3] = (uchar) tm->day;
            pos[4] = (uchar) tm->hour;
            pos[5] = (uchar) tm->minute;
            pos[6] = (uchar) tm->second;
            int4store(pos + 7, tm->second_part);
            uint length;
            if (tm->second_part)
                length = 11;
            else if (tm->hour || tm->minute || tm->second)
                length = 7;
            else if (tm->year || tm->month || tm->day)
                length = 4;
            else
                length = 0;
            buff[0] = (char) length++;
            buffer.Push(buff, length);
        }

        uchar * CMysqlImpl::net_store_length(uchar *packet, ulonglong length) {
            if (length < (ulonglong) 251LL) {
                *packet = (uchar) length;
                return packet + 1;
            }
            /* 251 is reserved for NULL */
            if (length < (ulonglong) 65536LL) {
                *packet++ = 252;
                int2store(packet, (uint) length);
                return packet + 2;
            }
            if (length < (ulonglong) 16777216LL) {
                *packet++ = 253;
                int3store(packet, (ulong) length);
                return packet + 3;
            }
            *packet++ = 254;
            int8store(packet, length);
            return packet + 8;
        }

        void CMysqlImpl::StoreParamDecimal(CUQueue& buffer, const DECIMAL & dec) {
            std::string s = dec.Hi32 ? SPA::ToString_long(dec) : SPA::ToString(dec);
            StoreParam(buffer, (const unsigned char *) s.c_str(), (unsigned int) s.size());
        }

        void CMysqlImpl::StoreParam(CUQueue& buffer, const unsigned char *str, unsigned int length) {
            unsigned int tail = buffer.GetTailSize();
            if (tail < (length + sizeof (UINT64))) {
                buffer.ReallocBuffer(buffer.GetSize() + length + sizeof (UINT64));
            }
            uchar *begin = (uchar*) buffer.GetBuffer(buffer.GetSize());
            uchar *to = net_store_length(begin, length);
            unsigned int increase = (unsigned int) (to - begin);
            buffer.SetSize(buffer.GetSize() + increase);
            buffer.Push(str, length);
        }

        void CMysqlImpl::StoreParamDatas(CUQueue& buffer, int row) {
            for (size_t n = 0; n < m_stmt.parameters; ++n) {
                CDBVariant &data = m_vParam[row * m_stmt.parameters + n];
                unsigned short vt = data.Type();
                switch (vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        StoreParamNull(buffer, (unsigned int) n);
                        break;
                    case VT_BOOL:
                    {
                        unsigned char b = (data.boolVal ? 1 : 0);
                        StoreFixedParam(buffer, b);
                    }
                        break;
                    case VT_I1:
                        StoreFixedParam(buffer, data.cVal);
                        break;
                    case VT_UI1:
                        StoreFixedParam(buffer, data.bVal);
                        break;
                    case VT_I2:
                        StoreFixedParam(buffer, data.iVal);
                        break;
                    case VT_UI2:
                        StoreFixedParam(buffer, data.uiVal);
                        break;
                    case VT_INT:
                    case VT_I4:
                        StoreFixedParam(buffer, data.intVal);
                        break;
                    case VT_UI4:
                    case VT_UINT:
                        StoreFixedParam(buffer, data.uintVal);
                        break;
                    case VT_I8:
                        StoreFixedParam(buffer, data.llVal);
                        break;
                    case VT_UI8:
                        StoreFixedParam(buffer, data.ullVal);
                        break;
                    case VT_R4:
                        StoreFixedParam(buffer, data.fltVal);
                        break;
                    case VT_R8:
                        StoreFixedParam(buffer, data.dblVal);
                        break;
                    case VT_DATE:
                    {
                        SPA::UDateTime dt(data.ullVal);
                        MYSQL_TIME mt;
                        memset(&mt, 0, sizeof (mt));
                        unsigned int micros = 0;
                        tm ct = dt.GetCTime(&micros);
                        mt.second_part = micros;
                        mt.hour = ct.tm_hour;
                        mt.minute = ct.tm_min;
                        mt.second = ct.tm_sec;
                        mt.day = ct.tm_mday;
                        mt.month = ct.tm_mon + 1;
                        mt.year = ct.tm_year + 1900;
                        if (ct.tm_mday == 0) {
                            mt.time_type = MYSQL_TIMESTAMP_TIME;
                            StoreParamTime(buffer, mt);
                        } else if (mt.hour == 0 && mt.minute == 0 && mt.second == 0) {
                            mt.time_type = MYSQL_TIMESTAMP_DATE;
                            StoreParamDateTime(buffer, mt);
                        } else {
                            mt.time_type = MYSQL_TIMESTAMP_DATETIME;
                            StoreParamDateTime(buffer, mt);
                        }
                    }
                        break;
                    case VT_DECIMAL:
                        StoreParamDecimal(buffer, data.decVal);
                        break;
                    case VT_STR:
                    case VT_BYTES:
                    case (VT_ARRAY | VT_UI1):
                    case (VT_ARRAY | VT_I1):
                    {
                        unsigned char *str;
                        unsigned int len = data.parray->rgsabound->cElements;
                        ::SafeArrayAccessData(data.parray, (void**) &str);
                        StoreParam(buffer, str, len);
                        ::SafeArrayUnaccessData(data.parray);
                    }
                        break;
                    default:
                        assert(false);
                        break;
                }
            }
        }

        int CMysqlImpl::StoreParamTypes(CUQueue & buffer, int row, std::wstring & errMsg) {
            int res = 0;
            errMsg.clear();
            for (size_t n = 0; n < m_stmt.parameters; ++n) {
                CDBVariant &data = m_vParam[row * m_stmt.parameters + n];
                unsigned short vt = data.Type();
                switch (vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        buffer << (short) MYSQL_TYPE_NULL;
                        break;
                    case VT_I1:
                    case VT_UI1:
                    case VT_BOOL:
                        buffer << (short) MYSQL_TYPE_TINY;
                        break;
                    case VT_I2:
                    case VT_UI2:
                        buffer << (short) MYSQL_TYPE_SHORT;
                        break;
                    case VT_INT:
                    case VT_UINT:
                    case VT_I4:
                    case VT_UI4:
                        buffer << (short) MYSQL_TYPE_LONG;
                        break;
                    case VT_I8:
                    case VT_UI8:
                        buffer << (short) MYSQL_TYPE_LONGLONG;
                        break;
                    case VT_R4:
                        buffer << (short) MYSQL_TYPE_FLOAT;
                        break;
                    case VT_R8:
                        buffer << (short) MYSQL_TYPE_DOUBLE;
                        break;
                    case VT_DECIMAL:
                        buffer << (short) MYSQL_TYPE_NEWDECIMAL;
                        break;
                    case VT_DATE:
                        buffer << (short) MYSQL_TYPE_DATETIME;
                        break;
                    case VT_STR:
                    case (VT_ARRAY | VT_I1):
                        buffer << (short) MYSQL_TYPE_VAR_STRING;
                        break;
                    case VT_BYTES:
                    case (VT_ARRAY | VT_UI1):
                        buffer << (short) MYSQL_TYPE_BLOB;
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
            return res;
        }

        void CMysqlImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            affected = 0;
            m_indexCall = index;
            vtId = (UINT64) 0;
            if (!m_stmt.prepared || !m_stmt.parameters) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (m_vParam.size() == 0) {
                res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (m_vParam.size() % m_stmt.parameters) {
                res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                m_fails += (m_vParam.size() / m_stmt.parameters);
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                m_fails += (m_vParam.size() / m_stmt.parameters);
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            m_bExecutingParameters = true;
            fail_ok = 0;
            res = 0;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            CScopeUQueue sb;
            CUQueue &buffer = *sb;
            int rows = (int) (m_vParam.size() / m_stmt.parameters);
            for (int row = 0; row < rows; ++row) {
                buffer.SetSize(0);
                ReserveNullBytesPlus(buffer, (unsigned int) m_stmt.parameters);
                int ret = StoreParamTypes(buffer, row, errMsg);
                if (ret) {
                    if (!res) {
                        res = ret;
                    }
                    ++m_fails;
                    continue;
                }
                StoreParamDatas(buffer, row);
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                cmd.com_stmt_execute.stmt_id = m_stmt.stmt_id;
                cmd.com_stmt_execute.params = (unsigned char *) buffer.GetBuffer();
                cmd.com_stmt_execute.params_length = buffer.GetSize();
                my_bool fail = command_service_run_command(m_pMysql.get(), COM_STMT_EXECUTE, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
                if (m_sql_errno) {
                    ++m_fails;
                    if (!res) {
                        res = m_sql_errno;
                        errMsg = m_err_msg;
                    }
                    if (m_last_insert_id && lastInsertId)
                        vtId = (SPA::UINT64)m_last_insert_id;
                } else if (fail) {
                    if (!res) {
                        errMsg = SERVICE_COMMAND_ERROR;
                        res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
                    }
                    if (m_last_insert_id && lastInsertId)
                        vtId = (SPA::UINT64)m_last_insert_id;
                    ++m_fails;
                } else {
                    ++m_oks;
                    affected += (INT64) m_affected_rows;
                    if (lastInsertId) {
                        vtId = (UINT64) m_last_insert_id;
                    }
                }
            }
            m_bExecutingParameters = false;
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CMysqlImpl::StartBLOB(unsigned int lenExpected) {
            m_qSend.SetSize(0);
            if (lenExpected > m_qSend.GetMaxSize()) {
                m_qSend.ReallocBuffer(lenExpected);
            }
            CUQueue &q = m_UQueue;
            m_qSend.Push(q.GetBuffer(), q.GetSize());
            assert(q.GetSize() > sizeof (unsigned short) + sizeof (unsigned int));
            q.SetSize(0);
        }

        void CMysqlImpl::Chunk() {
            CUQueue &q = m_UQueue;
            if (q.GetSize()) {
                m_qSend.Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
            }
        }

        void CMysqlImpl::EndBLOB() {
            Chunk();
            m_vParam.push_back(CDBVariant());
            CDBVariant &vt = m_vParam.back();
            m_qSend >> vt;
            assert(m_qSend.GetSize() == 0);
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
            }
            assert(q.GetSize() == 0);
        }

    } //namespace ServerSide
} //namespace SPA