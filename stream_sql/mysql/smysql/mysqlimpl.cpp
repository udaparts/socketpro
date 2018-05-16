
#include "mysqlimpl.h"
#include <algorithm>
#ifndef NDEBUG
#include <iostream>
#endif
#include "streamingserver.h"
#include "mysqld_error.h"
#include "../../../include/server_functions.h"
#include "crypt_genhash_impl.h"

namespace SPA
{
    namespace ServerSide{

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
        m_qSend(*m_sb), m_stmt(0, false), m_bExecutingParameters(false), m_NoSending(false), m_sql_errno(0),
        m_sc(nullptr), m_sql_resultcs(nullptr), m_ColIndex(0), m_sql_flags(0), m_affected_rows(0),
        m_last_insert_id(0), m_server_status(0), m_statement_warn_count(0), m_indexCall(0),
        m_bBlob(false), m_cmd(COM_SLEEP), m_NoRowset(false) {
            m_qSend.ToUtf8(true); //convert UNICODE into UTF8 automatically
            m_UQueue.ToUtf8(true); //convert UNICODE into UTF8 automatically
        }

        CMysqlImpl::~CMysqlImpl() {
            CleanDBObjects();
        }

        unsigned int CMysqlImpl::GetParameters() const {
            return (unsigned int) m_stmt.parameters;
        }

        void CALLBACK CMysqlImpl::OnThreadEvent(SPA::ServerSide::tagThreadEvent te) {
            if (te == SPA::ServerSide::teStarted) {
                int fail = srv_session_init_thread(CSetGlobals::Globals.Plugin);
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
            if (reqId == idExecuteParameters)
                m_vParam.clear();
            return 0;
        }

        int CMysqlImpl::sql_start_result_metadata(void *ctx, uint num_cols, uint flags, const CHARSET_INFO * resultcs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
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
                info.Flags |= CDBColumnInfo::FLAG_UNIQUE;
                info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
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
            if (impl->m_cmd == COM_STMT_PREPARE) {
                if (info.DisplayName == L"?")
                    impl->m_stmt.parameters += 1;
            }
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
                } else if (impl->m_NoRowset) {
                    q.SetSize(0);
                    return 0;
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
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
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
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_NULL;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_integer(void * ctx, longlong value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_cmd == COM_STMT_PREPARE && impl->m_ColIndex == 0) {
                impl->m_stmt.stmt_id = (unsigned long) value;
            } else if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
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
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
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
            { 0};
            int len = sizeof (str);
            decimal2string(&src, str, &len, 0, 0, 0);
            if (large) {
                SPA::ParseDec_long(str, dec);
            } else {
                SPA::ParseDec(str, dec);
            }
        }

        int CMysqlImpl::sql_get_decimal(void * ctx, const decimal_t * value) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
            DECIMAL dec;
            const CDBColumnInfo &info = impl->m_vColInfo[impl->m_ColIndex];
            ToDecimal(*value, info.Precision > 19, dec);
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DECIMAL << dec;
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_double(void * ctx, double value, uint32 decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
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
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_time(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_datetime(void * ctx, const MYSQL_TIME * value, uint decimals) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
            CUQueue &q = impl->m_qSend;
            q << (VARTYPE) VT_DATE << ToUDateTime(*value);
            ++impl->m_ColIndex;
            return 0;
        }

        int CMysqlImpl::sql_get_string(void * ctx, const char * const value, size_t length, const CHARSET_INFO * const valuecs) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
            if (impl->m_NoRowset && !(impl->m_server_status & SERVER_PS_OUT_PARAMS))
                return 0;
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
            if ((server_status & SERVER_PS_OUT_PARAMS) == SERVER_PS_OUT_PARAMS) {
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
                    int fail = srv_session_detach(mysql);
                    //assert(!fail);
                    fail = srv_session_close(mysql);
                    assert(!fail);
                }
            });
            int fail = srv_session_info_set_connection_type(st_session, VIO_TYPE_PLUGIN);
            assert(!fail);
            if (fail)
                return false;
            fail = thd_get_security_context(srv_session_info_get_thd(st_session), &m_sc);
            assert(!fail);
            if (fail)
                return false;
            std::string userA = SPA::Utilities::ToUTF8(userName.c_str(), userName.size());
            std::string host = "localhost";
            if (ip != host)
                host = "%";
            fail = security_context_lookup(m_sc, userA.c_str(), host.c_str(), ip.c_str(), nullptr);
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
            std::wstring wsql = L"CREATE FUNCTION PublishDBEvent RETURNS INTEGER SONAME 'libsmysql.so'";
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

        void CMysqlImpl::ConfigServices(CMysqlImpl & impl) {
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            impl.m_NoSending = true;
            std::wstring errMsg;
            if (!impl.m_pMysql && !impl.OpenSession(L"root", "localhost"))
                return;
            std::wstring wsql = L"USE sp_streaming_db;CREATE TABLE IF NOT EXISTS service(id INT UNSIGNED PRIMARY KEY NOT NULL,library VARCHAR(2048)NOT NULL,param INT NULL,description VARCHAR(2048)NULL)";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Creating the table service failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return;
            }
            wsql = L"CREATE TABLE IF NOT EXISTS permission(svsid INT UNSIGNED NOT NULL,user VARCHAR(32)NOT NULL,PRIMARY KEY(svsid,user),FOREIGN KEY(svsid)REFERENCES service(id)ON DELETE CASCADE ON UPDATE CASCADE)";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Creating the table permission failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return;
            }
            std::vector<CService> vService;
            wsql = L"select id,library,param,description from service";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            SPA::UDB::CDBVariant vtLib, vtParam, vtDesc;
            while (impl.m_qSend.GetSize() && !res) {
                impl.m_qSend >> vtId >> vtLib >> vtParam >> vtDesc;
                CService svs;
                svs.ServiceId = vtId.uintVal;
                svs.Library = ToString(vtLib);
                switch (vtParam.Type()) {
                    case VT_I4:
                    case VT_INT:
                    case VT_I8:
                    case VT_UI4:
                    case VT_UINT:
                    case VT_UI8:
                        svs.Param = (int) vtParam.lVal;
                        break;
                    default:
                        svs.Param = 0;
                        break;
                }
                if (vtDesc.Type() == (VT_I1 | VT_ARRAY))
                    svs.Description = ToString(vtDesc);
                vService.push_back(svs);
            }
            auto it = std::find_if(vService.begin(), vService.end(), [](const CService & svs)->bool {
                return (svs.ServiceId == SPA::Mysql::sidMysql);
            });
            if (it == vService.end()) {
                wsql = L"INSERT INTO service VALUES(" + std::to_wstring((UINT64) Mysql::sidMysql) +
#ifdef WIN32_64
                        L",'smysql.dll'" +
#else
                        L",'libsmysql.so'" +
#endif
                        L",0,'Continous SQL streaming processing service')";
                impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Inserting the table service failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                }
            }

            it = std::find_if(vService.begin(), vService.end(), [](const CService & svs)->bool {
                return (svs.ServiceId == (unsigned int) SPA::sidHTTP);
            });
            if (it == vService.end()) {
                wsql = L"INSERT INTO service VALUES(" + std::to_wstring((UINT64) SPA::sidHTTP) +
#ifdef WIN32_64
                        L",'uservercore.dll'" +
#else
                        L",'libuservercore.so'" +
#endif
                        L",0,'HTTP/Websocket processing service')";
                impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                if (res) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Inserting the table service failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                }
            }

            for (auto p = CSetGlobals::Globals.services.begin(), end = CSetGlobals::Globals.services.end(); p != end; ++p) {
                auto found = std::find_if(vService.begin(), vService.end(), [p](const CService & svs)->bool {
                    if (!p->size() || !svs.Library.size())
                        return false;
                    return (::strstr(svs.Library.c_str(), p->c_str()) != nullptr);
                });
                int param = 0;
                if (found != vService.end())
                    param = found->Param;
                HINSTANCE hModule = CSocketProServer::DllManager::AddALibrary(p->c_str(), param);
                if (!hModule) {
                    CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Not able o load server plugin %s", p->c_str());
                    continue;
                }
                PGetNumOfServices GetNumOfServices = (PGetNumOfServices)::GetProcAddress(hModule, "GetNumOfServices");
                PGetAServiceID GetAServiceID = (PGetAServiceID)::GetProcAddress(hModule, "GetAServiceID");
                unsigned short count = GetNumOfServices();
                for (unsigned short n = 0; n < count; ++n) {
                    unsigned int svsId = GetAServiceID(n);
                    it = std::find_if(vService.begin(), vService.end(), [svsId](const CService & svs)->bool {
                        return (svs.ServiceId == svsId);
                    });
                    if (it == vService.end()) {
                        wsql = L"INSERT INTO service(id,library,param,description)VALUES(" + std::to_wstring((UINT64) svsId) + L",'" +
                                SPA::Utilities::ToWide(p->c_str(), p->size()) + L"'," + std::to_wstring((INT64) param) + L",'')";
                        impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
                        if (res) {
                            CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Inserting the table service failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                        }
                    }
                }
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
            wsql = L"CREATE TABLE IF NOT EXISTS config(mykey varchar(32)PRIMARY KEY NOT NULL,value text not null)";
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Configuring streaming DB failed(errCode=%d; errMsg=%s)", res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return map;
            }
            wsql = L"select mykey,value from config";
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
                Trim(s0);
                Trim(s1);
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

        bool CMysqlImpl::Authenticate(const std::wstring &userName, const wchar_t *password, const std::string &ip, unsigned int svsId) {
            std::unique_ptr<CMysqlImpl> impl(new CMysqlImpl);
            if (!impl->OpenSession(L"root", "localhost")) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as root account not available");
                return false;
            }
            std::wstring host = L"localhost";
            if (ip != "localhost")
                host = L"%";
            std::string user = SPA::Utilities::ToUTF8(userName.c_str(), userName.size());
            std::wstring wsql(L"select authentication_string from mysql.user where password_expired='N' and account_locked='N' and user='");
            wsql += (userName + L"' and host='" + host + L"'");
            int res = 0;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            impl->m_NoSending = true;
            std::wstring errMsg;
            impl->Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res || !impl->m_qSend.GetSize()) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as user %s not found (errCode=%d; errMsg=%s)", user.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return false;
            }
            SPA::UDB::CDBVariant vtAuth;
            impl->m_qSend >> vtAuth;
            char *auth_id = nullptr;
            ::SafeArrayAccessData(vtAuth.parray, (void**) &auth_id);
            std::string hash((const char*) auth_id, vtAuth.parray->rgsabound->cElements);
            ::SafeArrayUnaccessData(vtAuth.parray);
            if (!DoAuthentication(password, hash)) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as wrong password for user %s (errCode=%d; errMsg=%s)", user.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return false;
            }
            if (svsId == SPA::Mysql::sidMysql)
                return true;
            wsql = L"SELECT user from sp_streaming_db.permission,sp_streaming_db.service where svsid=id AND svsid=" + std::to_wstring((UINT64) svsId) + L" AND user='" + userName + L"'";
            impl->Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            if (res || !impl->m_qSend.GetSize()) {
                std::string user = SPA::Utilities::ToUTF8(userName.c_str(), userName.size());
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as service %d is not set for user %s yet (errCode=%d; errMsg=%s)", svsId, user.c_str(), res, SPA::Utilities::ToUTF8(errMsg.c_str(), errMsg.size()).c_str());
                return false;
            }
            return true;
        }

        bool CMysqlImpl::DoAuthentication(const wchar_t *password, const std::string & hash) {
            if (hash.size() != 70) {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as wrong hash length (expected length=70; found length=%d)", hash.size());
                return false;
            }
            std::string pwd = SPA::Utilities::ToUTF8(password);
            std::string header = hash.substr(0, 7);
            if (header != "$A$005$") {
                CSetGlobals::Globals.LogMsg(__FILE__, __LINE__, "Authentication failed as wrong hash algorithm (expected=$A$005$; this=%s)", header.c_str());
                return false;
            }
            std::string salt = hash.substr(7, 20);
            std::string digest = hash.substr(27);
            char buffer[CRYPT_MAX_PASSWORD_SIZE + 1] =
            { 0};
            unsigned int iterations = 5000; //CACHING_SHA2_PASSWORD_ITERATIONS;
            my_crypt_genhash(buffer, CRYPT_MAX_PASSWORD_SIZE, pwd.c_str(), pwd.length(), salt.c_str(), nullptr, &iterations);
            std::string s(buffer);
            size_t pos = s.rfind('$');
            s = s.substr(pos + 1);
            return (digest == s);
        }

        void CMysqlImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            unsigned int port;
            res = 0;
            ms = msMysql;
            m_EnableMessages = false;
            CleanDBObjects();
            std::string ip = GetPeerName(&port);
            if (ip == "127.0.0.1" || ip == "::ffff:127.0.0.1" || ip == "::1")
                ip = "localhost";
            std::wstring user = GetUID();
            OpenSession(user, ip);
            InitMysqlSession();
            int fail = 0;
            if (strConnection.size()) {
                std::string db = SPA::Utilities::ToUTF8(strConnection.c_str(), strConnection.size());
                COM_DATA cmd;
                cmd.com_init_db.db_name = db.c_str();
                cmd.com_init_db.length = (unsigned long) db.size();
                m_cmd = COM_INIT_DB;
                fail = command_service_run_command(m_pMysql.get(), COM_INIT_DB, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, nullptr);
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
            if (m_stmt.m_pParam) {
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                cmd.com_stmt_close.stmt_id = m_stmt.stmt_id;
                m_cmd = COM_STMT_CLOSE;
                int fail = command_service_run_command(m_pMysql.get(), COM_STMT_CLOSE, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, nullptr);
                assert(!fail);
                m_stmt.m_pParam.reset();
            }
            m_stmt.stmt_id = (~0);
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
                m_cmd = COM_QUERY;
                int fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
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
            m_cmd = COM_QUERY;
            int fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
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
            bool isBatching = IsBatching();
            if (isBatching)
                CommitBatching();
            while (bytes > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                ret = SendResult(idChunk, buffer, DEFAULT_BIG_FIELD_CHUNK_SIZE);
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                assert(ret == DEFAULT_BIG_FIELD_CHUNK_SIZE);
                buffer += DEFAULT_BIG_FIELD_CHUNK_SIZE;
                bytes -= DEFAULT_BIG_FIELD_CHUNK_SIZE;
            }
            if (isBatching)
                StartBatching();
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
            m_cmd = COM_QUERY;
            m_NoRowset = !rowset;
            int fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
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

        void CMysqlImpl::Prepare(const std::wstring& wsql, CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters) {
            m_NoRowset = false;
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
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_stmt_prepare.query = sqlUtf8;
            cmd.com_stmt_prepare.length = sb->GetSize();
            m_cmd = COM_STMT_PREPARE;
            m_NoSending = true;
            int fail = command_service_run_command(m_pMysql.get(), COM_STMT_PREPARE, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            m_NoSending = false;
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
            } else if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
            } else {
                parameters = (unsigned int) m_stmt.parameters;
                if (parameters == 0) {
                    res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                    errMsg = NO_PARAMETER_SPECIFIED;
                    CloseStmt();
                } else {
                    res = 0;
                    m_stmt.m_pParam.reset(new PS_PARAM[parameters], [](PS_PARAM * p) {
                        delete []p;
                    });
                }
            }
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

        int CMysqlImpl::SetParams(int row, std::wstring & errMsg) {
            int res = 0;
            errMsg.clear();
            PS_PARAM *pParam = m_stmt.m_pParam.get();
            for (size_t n = 0; n < m_stmt.parameters; ++n, ++pParam) {
                pParam->null_bit = false;
                pParam->unsigned_type = false;
                size_t pos = row * m_stmt.parameters + n;
                CDBVariant &data = m_vParam[pos];
                unsigned short vt = data.Type();
                switch (vt) {
                    case VT_NULL:
                    case VT_EMPTY:
                        pParam->length = 0;
                        pParam->type = MYSQL_TYPE_NULL;
                        pParam->null_bit = true;
                        pParam->value = nullptr;
                        break;
                    case VT_I1:
                        pParam->length = sizeof (char);
                        pParam->type = MYSQL_TYPE_TINY;
                        pParam->value = (const unsigned char *) &data.cVal;
                        break;
                    case VT_UI1:
                        pParam->length = sizeof (unsigned char);
                        pParam->type = MYSQL_TYPE_TINY;
                        pParam->value = &data.bVal;
                        pParam->unsigned_type = true;
                        break;
                    case VT_BOOL:
                        pParam->length = sizeof (unsigned char);
                        pParam->type = MYSQL_TYPE_TINY;
                        data.bVal = data.boolVal ? 1 : 0;
                        pParam->value = &data.bVal;
                        break;
                    case VT_I2:
                        pParam->length = sizeof (short);
                        pParam->type = MYSQL_TYPE_SHORT;
                        pParam->value = (const unsigned char *) &data.iVal;
                        break;
                    case VT_UI2:
                        pParam->length = sizeof (unsigned short);
                        pParam->type = MYSQL_TYPE_SHORT;
                        pParam->value = (const unsigned char *) &data.uiVal;
                        pParam->unsigned_type = true;
                        break;
                    case VT_INT:
                    case VT_I4:
                        pParam->length = sizeof (int);
                        pParam->type = MYSQL_TYPE_LONG;
                        pParam->value = (const unsigned char *) &data.intVal;
                        break;
                    case VT_UINT:
                    case VT_UI4:
                        pParam->length = sizeof (unsigned int);
                        pParam->type = MYSQL_TYPE_LONG;
                        pParam->value = (const unsigned char *) &data.uintVal;
                        pParam->unsigned_type = true;
                        break;
                    case VT_I8:
                        pParam->length = sizeof (SPA::INT64);
                        pParam->type = MYSQL_TYPE_LONGLONG;
                        pParam->value = (const unsigned char *) &data.llVal;
                        break;
                    case VT_UI8:
                        pParam->length = sizeof (SPA::UINT64);
                        pParam->type = MYSQL_TYPE_LONGLONG;
                        pParam->value = (const unsigned char *) &data.ullVal;
                        pParam->unsigned_type = true;
                        break;
                    case VT_R4:
                        pParam->length = sizeof (float);
                        pParam->type = MYSQL_TYPE_FLOAT;
                        pParam->value = (const unsigned char *) &data.fltVal;
                        break;
                    case VT_R8:
                        pParam->length = sizeof (double);
                        pParam->type = MYSQL_TYPE_DOUBLE;
                        pParam->value = (const unsigned char *) &data.dblVal;
                        break;
                        /*
                        case VT_DECIMAL:
                            pParam->type = MYSQL_TYPE_NEWDECIMAL;
                            break;
                        case VT_DATE:
                            pParam->type = MYSQL_TYPE_DATETIME;
                            break;
                         */
                    case VT_STR:
                    case (VT_ARRAY | VT_I1):
                        pParam->type = MYSQL_TYPE_VAR_STRING;
                    {
                        SAFEARRAY *parray = data.parray;
                        pParam->length = parray->rgsabound->cElements;
                        ::SafeArrayAccessData(parray, (void**) &pParam->value);
                        ::SafeArrayUnaccessData(parray);
                    }
                        break;
                    case VT_BYTES:
                    case (VT_ARRAY | VT_UI1):
                        pParam->type = MYSQL_TYPE_BLOB;
                    {
                        SAFEARRAY *parray = data.parray;
                        pParam->length = parray->rgsabound->cElements;
                        ::SafeArrayAccessData(parray, (void**) &pParam->value);
                        ::SafeArrayUnaccessData(parray);
                    }
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
            if (!m_stmt.m_pParam || !m_stmt.parameters) {
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
            int rows = (int) (m_vParam.size() / m_stmt.parameters);
            for (int row = 0; row < rows; ++row) {
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                int ret = SetParams(row, errMsg);
                if (ret) {
                    if (!res)
                        res = ret;
                    continue;
                }
                cmd.com_stmt_execute.stmt_id = m_stmt.stmt_id;
                cmd.com_stmt_execute.parameters = m_stmt.m_pParam.get();
                cmd.com_stmt_execute.parameter_count = (unsigned long) m_stmt.parameters;
                cmd.com_stmt_execute.has_new_types = true;
                cmd.com_stmt_execute.open_cursor = false;
                m_NoRowset = !rowset;
                m_cmd = COM_STMT_EXECUTE;
                int fail = command_service_run_command(m_pMysql.get(), COM_STMT_EXECUTE, &cmd, &my_charset_utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
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
                switch (vt.vt) {
                    case VT_DATE:
                    {
                        char str[64] = {0};
                        SPA::UDateTime dt(vt.ullVal);
                        dt.ToDBString(str, sizeof (str)); //date time to ASCII DB string 
                        vt = (const char*) str;
                    }
                        break;
                    case VT_DECIMAL:
                        vt = SPA::ToString(vt.decVal).c_str(); //decimal to ASCII string
                        break;
                    default:
                        break;
                }
            }
            assert(q.GetSize() == 0);
        }
    } //namespace ServerSide
} //namespace SPA