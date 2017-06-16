
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
        m_Blob(*m_sb), m_qSend(*m_sqSend), m_stmt(0, false), m_bCall(false), m_NoSending(false), m_sql_errno(0),
        m_sc(nullptr), m_sql_resultcs(nullptr), m_ColIndex(0),
        m_sql_flags(0), m_affected_rows(0), m_last_insert_id(0),
        m_server_status(0), m_statement_warn_count(0), m_indexCall(0), m_bBlob(false) {
            m_Blob.ToUtf8(true);
#ifdef WIN32_64
            m_UQueue.TimeEx(true); //use high-precision datetime
#endif
            m_UQueue.ToUtf8(true);
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
            CUQueue &q = impl->m_qSend;
            q.SetSize(0);
            if (!impl->m_NoSending) {
                q << impl->m_vColInfo << impl->m_indexCall;
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
            ulong power = (CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_LONG_FLAG);
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
            q << info.DataType;
            if (info.DeclaredType == L"BIT") {
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
                if (length <= DEFAULT_BIG_FIELD_CHUNK_SIZE) {
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
            CUQueue &q = impl->m_qSend;
            if (q.GetSize()) {
                if (!impl->SendRows(q))
                    return;
            }
            impl->m_sql_errno = 0;
            impl->m_server_status = server_status;
            impl->m_statement_warn_count = statement_warn_count;
            impl->m_affected_rows += affected_rows;
            impl->m_last_insert_id = last_insert_id;
            ++impl->m_oks;
            if (message)
                impl->m_err_msg = SPA::Utilities::ToWide(message);
            else
                impl->m_err_msg.clear();
        }

        void CMysqlImpl::sql_handle_error(void * ctx, uint sql_errno, const char * const err_msg, const char * const sqlstate) {
            CMysqlImpl *impl = (CMysqlImpl *) ctx;
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

        void CMysqlImpl::srv_session_error_cb(void *ctx, unsigned int sql_errno, const char *err_msg) {
            CMysqlImpl *p = (CMysqlImpl *) ctx;
            p->m_sql_errno = (int) sql_errno;
            p->m_err_msg = SPA::Utilities::ToWide(err_msg);
        }

        unsigned int CMysqlImpl::GetMySqlServerVersion() {
            CMysqlImpl impl;
            std::wstring db, errMsg, wsql(L"show variables where variable_name = 'version'");
            int res, ms;
            INT64 affected;
            SPA::UDB::CDBVariant vtId;
            UINT64 fail_ok;
            impl.Open(db, 0, res, errMsg, ms);
            if (res)
                return MYSQL_VERSION_ID;
            impl.m_NoSending = true;
            impl.Execute(wsql, true, true, false, 0, affected, res, errMsg, vtId, fail_ok);
            SPA::UDB::CDBVariant vt0, vt1;
            impl.m_qSend.Utf8ToW(true);
            if (impl.m_qSend.GetSize()) {
                impl.m_qSend >> vt0 >> vt1;
                std::string s = SPA::Utilities::ToUTF8(vt1.bstrVal);
                const char *end;
                unsigned int major = SPA::atoui(s.c_str(), end);
                unsigned int minor = SPA::atoui(++end, end);
                unsigned int build = SPA::atoui(++end, end);
                return (major * 10000 + minor * 100 + build);
            }
            return MYSQL_VERSION_ID;
        }

        void CMysqlImpl::Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            res = 0;
            ms = msMysql;
            m_EnableMessages = false;
            CleanDBObjects();
            MYSQL_SESSION st_session = srv_session_open(srv_session_error_cb, this);
            m_pMysql.reset(st_session, [this](MYSQL_SESSION mysql) {
                if (mysql) {
                    srv_session_close(mysql);
                }
            });
            unsigned int port;
            std::string ip = GetPeerName(&port);
            std::wstring user = GetUID();
            std::string userA = SPA::Utilities::ToUTF8(user.c_str(), user.size());
            if (!userA.size()) {
                userA = "root";
                ip = "127.0.0.1";
            }
            my_bool fail = thd_get_security_context(srv_session_info_get_thd(st_session), &m_sc);
            fail = security_context_lookup(m_sc, userA.c_str(), "localhost", ip.c_str(), nullptr);
            InitMysqlSession();
            m_NoSending = true;
            if (strConnection.size()) {
                std::string db = SPA::Utilities::ToUTF8(strConnection.c_str(), strConnection.size());
                COM_DATA cmd;
                cmd.com_init_db.db_name = db.c_str();
                cmd.com_init_db.length = (unsigned long) db.size();
                fail = command_service_run_command(st_session, COM_INIT_DB, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            }
            if (fail) {
                res = SPA::Mysql::ER_UNABLE_TO_SWITCH_TO_DATABASE;
                errMsg = UNABLE_TO_SWITCH_TO_DATABASE + strConnection;
                m_pMysql.reset();
            } else if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
                m_pMysql.reset();
            } else {
                res = 0;
                LEX_CSTRING db_name = srv_session_info_get_current_db(st_session);
                errMsg = SPA::Utilities::ToWide(db_name.str, db_name.length);
                if ((flags & SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) == SPA::UDB::ENABLE_TABLE_UPDATE_MESSAGES) {
                    m_EnableMessages = GetPush().Subscribe(&SPA::UDB::STREAMING_SQL_CHAT_GROUP_ID, 1);
                }
            }
            m_NoSending = false;
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
                m_NoSending = true;
                command_service_run_command(m_pMysql.get(), COM_STMT_CLOSE, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
                m_NoSending = false;
                m_stmt.prepared = false;

                //don't set stmt_it to 0 !!!
            }
        }

        void CMysqlImpl::CleanDBObjects() {
            CloseStmt();
            m_pMysql.reset();
            m_stmt.prepared = false;
            m_stmt.stmt_id = 0;
            m_vParam.clear();
            ResetMemories();
        }

        void CMysqlImpl::OnBaseRequestArrive(unsigned short requestId) {
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
                sql += "SET AUTOCOMMIT=0";
                m_NoSending = true;
                COM_DATA cmd;
                ::memset(&cmd, 0, sizeof (cmd));
                InitMysqlSession();
                cmd.com_query.query = sql.c_str();
                cmd.com_query.length = (unsigned int) sql.size();
                my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
                m_NoSending = false;
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
            std::string sql(rollback ? "rollback" : "commit");
            m_NoSending = true;
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_query.query = sql.c_str();
            cmd.com_query.length = (unsigned int) sql.size();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            m_NoSending = false;
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
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_query.query = sqlUtf8;
            cmd.com_query.length = sb->GetSize();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_QUERY, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
                ++m_fails;
            } else if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
                affected = 0;
                vtId = m_last_insert_id;
            } else {
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
            CloseStmt();
            res = 0;
            m_vParam.clear();
            m_sqlPrepare = Utilities::ToUTF8(wsql.c_str(), wsql.size());
            CMysqlImpl::Trim(m_sqlPrepare);
            PreprocessPreparedStatement();
            m_NoSending = true;
            CScopeUQueue sb;
            Utilities::ToUTF8(wsql.c_str(), wsql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            COM_DATA cmd;
            ::memset(&cmd, 0, sizeof (cmd));
            InitMysqlSession();
            cmd.com_stmt_prepare.query = sqlUtf8;
            cmd.com_stmt_prepare.length = sb->GetSize();
            my_bool fail = command_service_run_command(m_pMysql.get(), COM_STMT_PREPARE, &cmd, CSetGlobals::Globals.utf8_general_ci, &m_sql_cbs, CS_BINARY_REPRESENTATION, this);
            m_NoSending = false;
            ++m_stmt.stmt_id; //always increase statement id by one
            if (m_sql_errno) {
                res = m_sql_errno;
                errMsg = m_err_msg;
            } else if (fail) {
                errMsg = SERVICE_COMMAND_ERROR;
                res = SPA::Mysql::ER_SERVICE_COMMAND_ERROR;
            } else {
                res = 0;
                m_stmt.prepared = true;
                m_stmt.parameters = ComputeParameters(wsql);
                parameters = (unsigned int) m_stmt.parameters;
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

        void CMysqlImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            affected = 0;
            m_indexCall = index;
            vtId = (UINT64) 0;
            if (!m_pMysql) {
                res = SPA::Mysql::ER_NO_DB_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (!m_stmt.prepared) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (!m_stmt.parameters) {
                res = SPA::Mysql::ER_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            } else if ((m_vParam.size() % m_stmt.parameters) || (m_vParam.size() == 0)) {
                res = SPA::Mysql::ER_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            fail_ok = 0;
            res = 0;
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            m_NoSending = false;
            InitMysqlSession();

            affected = (INT64) m_affected_rows;
            if (lastInsertId) {
                vtId = (UINT64) m_last_insert_id;
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

    } //namespace ServerSide
} //namespace SPA