
#include "sqliteimpl.h"
#include <algorithm>
#ifndef WIN32_64
#include <sched.h>
#endif
#ifndef NDEBUG
#include <iostream>
#endif
namespace SPA
{
    namespace ServerSide{

        const wchar_t * CSqliteImpl::NO_DB_OPENED_YET = L"No sqlite database opened yet";
        const wchar_t * CSqliteImpl::BAD_END_TRANSTACTION_PLAN = L"Bad end transaction plan";
        const wchar_t * CSqliteImpl::NO_PARAMETER_SPECIFIED = L"No parameter specified";
        const wchar_t * CSqliteImpl::BAD_PARAMETER_DATA_ARRAY_SIZE = L"Bad parameter data array length";
        const wchar_t * CSqliteImpl::BAD_PARAMETER_COLUMN_SIZE = L"Bad parameter column size";
        const wchar_t * CSqliteImpl::DATA_TYPE_NOT_SUPPORTED = L"Data type not supported";
        const wchar_t * CSqliteImpl::NO_DB_FILE_SPECIFIED = L"No sqlite database file specified";
        std::wstring CSqliteImpl::m_strGlobalConnection;

        unsigned int CSqliteImpl::m_nParam = 0;

        CUCriticalSection CSqliteImpl::m_csPeer;
        std::vector<CSqliteImpl*> CSqliteImpl::m_vSqlitePeer;
        CSqliteImpl::CSqliteUpdateMap CSqliteImpl::m_mapUpdate;

        CSqliteImpl::CSqliteImpl() : m_oks(0), m_fails(0), m_ti(tiUnspecified), m_global(true), m_parameters(0) {
#ifdef WIN32_64
            //use high precision datetime
            m_UQueue.TimeEx(true);
#endif
        }

        void CSqliteImpl::SetDBGlobalConnectionString(const wchar_t * dbConnection) {
            m_csPeer.lock();
            if (dbConnection) {
                m_strGlobalConnection = dbConnection;
            } else {
                m_strGlobalConnection.clear();
            }
            m_csPeer.unlock();
        }

        void CSqliteImpl::SetInitialParam(unsigned int param) {
            m_nParam = param;
        }

        CSqliteImpl::~CSqliteImpl() {
            Clean();
        }

        void CSqliteImpl::Clean() {
            {
                CAutoLock al(m_csPeer);
                {
                    auto it = m_mapUpdate.find(this);
                    if (it != m_mapUpdate.end()) {
                        m_mapUpdate.erase(it);
                    }
                }
                {
                    auto it = std::find(m_vSqlitePeer.begin(), m_vSqlitePeer.end(), this);
                    if (it != m_vSqlitePeer.end()) {
                        m_vSqlitePeer.erase(it);
                    }
                }
            }
            m_vParam.clear();
            m_vPreparedStatements.clear();
            m_pSqlite.reset();
            ResetMemories();
        }

        void CSqliteImpl::OnBaseRequestArrive(unsigned short requestId) {
            switch (requestId) {
                case idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                    if (m_pSqlite) {
                        sqlite3_interrupt(m_pSqlite.get());
                        m_ti = tiUnspecified;
                    }
                    break;
                default:
                    break;
            }
        }

        void CSqliteImpl::ReleaseArray() {
            for (auto it = m_vArray.begin(), end = m_vArray.end(); it != end; ++it) {
                SAFEARRAY *arr = *it;
                ::SafeArrayUnaccessData(arr);
            }
            m_vArray.clear();
        }

        void CSqliteImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            Clean();
            m_global = true;
        }

        void CSqliteImpl::ResetMemories() {
            m_Blob.SetSize(0);
            if (m_Blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                m_Blob.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
            }
        }

        void CSqliteImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_oks = 0;
            m_fails = 0;
            m_ti = tiUnspecified;
            CAutoLock al(m_csPeer);
            m_vSqlitePeer.push_back(this);
        }

        void CSqliteImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
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

        int CSqliteImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
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

        void CSqliteImpl::BeginRows() {
            Transferring();
        }

        void CSqliteImpl::EndRows() {
            Transferring();
        }

        void CSqliteImpl::Transferring() {
            CUQueue &q = m_UQueue;
            while (q.GetSize()) {
#ifndef WIN32_64
                unsigned short *ps = (unsigned short*) q.GetBuffer();
                if (*ps == VT_BSTR) {
                    unsigned int *bytes = (unsigned int *) q.GetBuffer(sizeof (unsigned short));
                    *bytes >>= 1; //convert bytes into the count of unsigned short
                    *ps = (VT_ARRAY | VT_UI2);
                }
#endif
                m_vParam.push_back(CDBVariant());
                CDBVariant &vt = m_vParam.back();
                q >> vt;
            }
            assert(q.GetSize() == 0);
        }

        void CSqliteImpl::StartBLOB(unsigned int lenExpected) {
            m_Blob.SetSize(0);
            if (lenExpected > m_Blob.GetMaxSize()) {
                m_Blob.ReallocBuffer(lenExpected);
            }
            CUQueue &q = m_UQueue;
            m_Blob.Push(q.GetBuffer(), q.GetSize());
            assert(q.GetSize() > sizeof (unsigned short) + sizeof (unsigned int));
            q.SetSize(0);
#ifndef WIN32_64
            unsigned short *ps = (unsigned short*) m_Blob.GetBuffer();
            //convert VT_BSTR to (VT_UI2 | VT_ARRAY) on non-windows platforms to avoid possible string data conversions
            if (*ps == VT_BSTR) {
                unsigned int *bytes = (unsigned int*) m_Blob.GetBuffer(sizeof (unsigned short));
                *bytes >>= 1;
                *ps = (VT_UI2 | VT_ARRAY);
            }
#endif
        }

        void CSqliteImpl::Chunk() {
            CUQueue &q = m_UQueue;
            if (q.GetSize()) {
                m_Blob.Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
            }
        }

        void CSqliteImpl::EndBLOB() {
            Chunk();
            m_vParam.push_back(CDBVariant());
            CDBVariant &vt = m_vParam.back();
            m_Blob >> vt;
            assert(m_Blob.GetSize() == 0);
        }

        void CSqliteImpl::ConvertVariantDateToString() {
            for (auto it = m_vParam.begin(), end = m_vParam.end(); it != end; ++it) {
                CDBVariant &data = *it;
                VARTYPE vt = data.Type();
                if (vt == VT_DATE) {
                    char str[32] = {0};
                    UDateTime d(data.ullVal);
                    d.ToDBString(str, sizeof (str));
                    data = (const char*) str;
                } else if (vt == VT_DECIMAL) {
                    const DECIMAL &decVal = data.decVal;
                    data = SPA::ToDouble(decVal);
                } else if (vt == VT_CY) {
                    double d = (double) data.cyVal.int64;
                    d /= 10000.0;
                    data = d;
                }
            }
        }

        int CSqliteImpl::Bind(int row, std::string & errMsg) {
            int res = SQLITE_OK;
            errMsg.clear();
            int pos = 0;
            size_t cols = m_parameters;
            for (auto it = m_vPreparedStatements.begin(), end = m_vPreparedStatements.end(); it != end; ++it) {
                sqlite3_stmt *stmt = (*it).get();
                int parameters = sqlite3_bind_parameter_count(stmt);
                for (int n = 0; n < parameters; ++n) {
                    int r = SQLITE_OK;
                    const CDBVariant &data = m_vParam[row * (int) cols + pos];
                    unsigned short vt = data.Type();
                    if (vt == VT_NULL || vt == VT_EMPTY) {
                        r = sqlite3_bind_null(stmt, n + 1);
                    } else {
                        switch (vt) {
                            case (VT_ARRAY | VT_UI2): //UNICODE string
                            {
                                void *buffer;
                                unsigned int len = data.parray->rgsabound->cElements;
                                ::SafeArrayAccessData(data.parray, &buffer);
                                r = sqlite3_bind_text16(stmt, n + 1, buffer, (int) (len << 1), SQLITE_STATIC);
                                m_vArray.push_back(data.parray);
                            }
                                break;
                            case VT_BSTR: //UNICODE string
                                r = sqlite3_bind_text16(stmt, n + 1, data.bstrVal, ::SysStringLen(data.bstrVal) * sizeof (UTF16), SQLITE_STATIC);
                                break;
                            case (VT_ARRAY | VT_I1): //ASCII string
                            {
                                void *buffer;
                                unsigned int len = data.parray->rgsabound->cElements;
                                ::SafeArrayAccessData(data.parray, &buffer);
                                r = sqlite3_bind_text(stmt, n + 1, (const char*) buffer, (int) len, SQLITE_STATIC);
                                m_vArray.push_back(data.parray);
                            }
                                break;
                            case VT_BYTES:
                            case (VT_ARRAY | VT_UI1): //Binary
                            {
                                void *buffer;
                                unsigned int len = data.parray->rgsabound->cElements;
                                ::SafeArrayAccessData(data.parray, &buffer);
                                r = sqlite3_bind_blob(stmt, n + 1, (const char*) buffer, (int) len, SQLITE_STATIC);
                                m_vArray.push_back(data.parray);
                            }
                                break;
                            case VT_I1:
                                r = sqlite3_bind_int(stmt, n + 1, data.cVal);
                                break;
                            case VT_UI1:
                                r = sqlite3_bind_int(stmt, n + 1, data.bVal);
                                break;
                            case VT_UI2:
                                r = sqlite3_bind_int(stmt, n + 1, data.uiVal);
                                break;
                            case VT_I2:
                                r = sqlite3_bind_int(stmt, n + 1, data.iVal);
                                break;
                            case VT_I4:
                                r = sqlite3_bind_int(stmt, n + 1, data.lVal);
                                break;
                            case VT_INT:
                                r = sqlite3_bind_int(stmt, n + 1, data.intVal);
                                break;
                            case VT_UI4:
                                r = sqlite3_bind_int64(stmt, n + 1, data.ulVal);
                                break;
                            case VT_UINT:
                                r = sqlite3_bind_int64(stmt, n + 1, data.uintVal);
                                break;
                            case VT_I8:
                            case VT_UI8:
                                r = sqlite3_bind_int64(stmt, n + 1, data.llVal);
                                break;
                            case VT_BOOL:
                                r = sqlite3_bind_int(stmt, n + 1, data.boolVal ? 1 : 0);
                                break;
                            case VT_R4:
                                r = sqlite3_bind_double(stmt, n + 1, data.fltVal);
                                break;
                            case VT_R8:
                                r = sqlite3_bind_double(stmt, n + 1, data.dblVal);
                                break;
                            case VT_CY:
                                r = sqlite3_bind_double(stmt, n + 1, data.cyVal.int64 / 10000.00);
                                break;
                            default:
                                if (!res) {
                                    res = SPA::Sqlite::SQLITE_DATA_TYPE_NOT_SUPPORTED;
                                    errMsg = Utilities::ToUTF8(DATA_TYPE_NOT_SUPPORTED);
                                }
                                break;
                        }
                    }
                    if (r && !res) {
                        if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                            res = r;
                        } else {
                            res = sqlite3_extended_errcode(m_pSqlite.get());
                        }
                        errMsg = sqlite3_errmsg(m_pSqlite.get());
                    }
                    ++pos;
                }
            }
            assert(pos == (int) cols);
            return res;
        }

        int CSqliteImpl::sqlite3_sleep(int time) {
#ifdef WIN32_64
            Sleep(0);
#else
            sched_yield();
#endif
            return 0;
        }

        int CSqliteImpl::DoFinalize(sqlite3_stmt * stmt) {
            int res = sqlite3_finalize(stmt);
            while (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                sqlite3_sleep(SLEEP_TIME);
                res = sqlite3_finalize(stmt);
            }
            return res;
        }

        int CSqliteImpl::DoStep(sqlite3_stmt * stmt) {
            int res = sqlite3_step(stmt);
            while (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                sqlite3_sleep(SLEEP_TIME);
                res = sqlite3_step(stmt);
            }
            return res;
        }

        int CSqliteImpl::ResetStatements() {
            int res = SQLITE_OK;
            for (auto it = m_vPreparedStatements.begin(), end = m_vPreparedStatements.end(); it != end; ++it) {
                sqlite3_stmt *stmt = (*it).get();
                res = sqlite3_clear_bindings(stmt);
                assert(res == SQLITE_OK);
                do {
                    res = sqlite3_reset(stmt);
                    if (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                        sqlite3_sleep(SLEEP_TIME);
                        if (!IsOpened() || IsCanceled()) {
                            break;
                        }
                    } else {
                        break;
                    }
                } while (true);
            }
            return SQLITE_OK;
        }

        void CSqliteImpl::Process(int &res, std::string & errMsg) {
            for (auto it = m_vPreparedStatements.begin(), end = m_vPreparedStatements.end(); it != end; ++it) {
                sqlite3_stmt *stmt = (*it).get();
                int r = DoStep(stmt);
                if (r == SQLITE_DONE) {
                    r = SQLITE_OK;
                }
                if (r) {
                    ++m_fails;
                    if (!res) {
                        if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                            res = r;
                        } else {
                            res = sqlite3_extended_errcode(m_pSqlite.get());
                        }
                        errMsg = sqlite3_errmsg(m_pSqlite.get());
                    }
                } else {
                    ++m_oks;
                }
            }
        }

        bool CSqliteImpl::Process(UINT64 index, bool rowset, bool meta, int &res, std::string & errMsg) {
            CScopeUQueue sb;
            for (auto it = m_vPreparedStatements.begin(), end = m_vPreparedStatements.end(); it != end; ++it) {
                sqlite3_stmt *stmt = (*it).get();
                CDBColumnInfoArray vColInfo = GetColInfo(meta, stmt);
                sb << vColInfo << index;
                unsigned int ret = SendResult(idRowsetHeader, sb->GetBuffer(), sb->GetSize());
                if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                    return false;
                }
                int r = SQLITE_OK;
                std::string err_msg;
                if (!PushRecords(stmt, vColInfo, r, err_msg)) {
                    return false;
                }
                if (r) {
                    ++m_fails;
                    if (!res) {
                        res = r;
                        errMsg = err_msg;
                    }
                } else {
                    ++m_oks;
                }
                sb->SetSize(0);
            }
            return true;
        }

        void CSqliteImpl::ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            fail_ok = 0;
            affected = 0;
            if (!m_pSqlite) {
                res = SPA::Sqlite::SQLITE_DB_NOT_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (!m_vPreparedStatements.size() || !m_parameters || !m_vParam.size()) {
                res = SPA::Sqlite::SQLITE_NO_PARAMETER_SPECIFIED;
                errMsg = NO_PARAMETER_SPECIFIED;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            if (m_vParam.size() % m_parameters) {
                res = SPA::Sqlite::SQLITE_BAD_PARAMETER_DATA_ARRAY_SIZE;
                errMsg = BAD_PARAMETER_DATA_ARRAY_SIZE;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            int start = sqlite3_total_changes(m_pSqlite.get());
            ConvertVariantDateToString();
            std::string err_msg;
            std::string last_err_msg;
            int last_error = SQLITE_OK;
            int rows = (int) (m_vParam.size() / m_parameters);
            for (int row = 0; row < rows; ++row) {
                res = Bind(row, err_msg);
                if (res) {
                    if (!last_error) {
                        last_error = res;
                        last_err_msg = err_msg;
                    }
                }
                res = SQLITE_OK;
                if (rowset) {
                    if (!Process(index, rowset, meta, res, err_msg)) {
                        return;
                    }
                } else {
                    Process(res, err_msg);
                }
                if (res) {
                    if (!last_error) {
                        last_error = res;
                        last_err_msg = err_msg;
                    }
                }
                ResetStatements();
            }
            if (last_error) {
                res = last_error;
                errMsg = Utilities::ToWide(last_err_msg.c_str(), last_err_msg.size());
            }
            affected = sqlite3_total_changes(m_pSqlite.get()) - start;
            if (lastInsertId) {
                vtId = (INT64) sqlite3_last_insert_rowid(m_pSqlite.get());
            }
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        void CSqliteImpl::ExecuteSqlWithRowset(const char* sqlUtf8, bool meta, bool lastInsertId, UINT64 index, int &res, std::wstring &errMsg, CDBVariant & vtId) {
            unsigned int ret;
            bool header_sent = false;
            sqlite3 *db = m_pSqlite.get();
            sqlite3_stmt *statement = nullptr;
            const char *tail = nullptr;
            std::string last_err_msg;
            int last_error = SQLITE_OK;
            CScopeUQueue sbRowset;
            do {
                do {
                    res = sqlite3_prepare_v2(db, sqlUtf8, -1, &statement, &tail);
                    if ((res == SQLITE_BUSY || res == SQLITE_LOCKED) && !statement) {
                        sqlite3_sleep(SLEEP_TIME);
                        if (!IsOpened() || IsCanceled()) {
                            break;
                        }
                    } else {
                        break;
                    }
                } while (true);
                if (res) {
                    assert(!statement);
                    if (!last_error) {
                        last_err_msg = sqlite3_errmsg(db);
                        last_err_msg += " (";
                        last_err_msg += sqlUtf8;
                        last_err_msg += ")";
                        if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                            last_error = res;
                        } else {
                            last_error = sqlite3_extended_errcode(db);
                        }
                    }
                    ++m_fails;
                } else {
                    ++m_oks;
                    assert(statement);
                    std::shared_ptr<sqlite3_stmt> pStatement(statement, [this](sqlite3_stmt * s) {
                        if (s) {
                            int ret = this->DoFinalize(s);
                            assert(ret == SQLITE_OK);
                        }
                    });
                    CDBColumnInfoArray vColInfo = GetColInfo(meta, statement);
                    sbRowset << vColInfo << index;
                    ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
                    header_sent = true;
                    if (ret == REQUEST_CANCELED || ret == SOCKET_NOT_FOUND) {
                        return;
                    }
                    std::string err_msg;
                    if (!PushRecords(statement, vColInfo, res, err_msg)) {
                        return;
                    }
                    if (res) {
                        if (!last_error) {
                            last_error = res;
                            last_err_msg = err_msg;
                        }
                    }
                }
                if (sqlUtf8 == tail) {
                    break;
                } else if (!tail || !strlen(tail)) {
                    break;
                }
                sqlUtf8 = tail;
                sbRowset->SetSize(0);
            } while (true);
            if (lastInsertId) {
                vtId = (INT64) sqlite3_last_insert_rowid(db);
            }
            res = last_error;
            if (last_error) {
                errMsg = Utilities::ToWide(last_err_msg.c_str(), last_err_msg.size());
            }
            if (!header_sent) {
                sbRowset->SetSize(0);
                CDBColumnInfoArray vColInfo;
                sbRowset << vColInfo << index;
                ret = SendResult(idRowsetHeader, sbRowset->GetBuffer(), sbRowset->GetSize());
            }
        }

        void CSqliteImpl::ExecuteSqlWithoutRowset(const char* sqlUtf8, bool lastInsertId, int &res, std::wstring &errMsg, CDBVariant & vtId) {
            sqlite3 *db = m_pSqlite.get();
            sqlite3_stmt *statement = nullptr;
            const char *tail = nullptr;
            std::wstring last_err_msg;
            int last_error = SQLITE_OK;
            do {
                do {
                    res = sqlite3_prepare_v2(db, sqlUtf8, -1, &statement, &tail);
                    if ((res == SQLITE_BUSY || res == SQLITE_LOCKED) && !statement) {
                        sqlite3_sleep(SLEEP_TIME);
                        if (!IsOpened() || IsCanceled()) {
                            break;
                        }
                    } else {
                        break;
                    }
                } while (true);
                if (res) {
                    assert(!statement);
                    if (!last_error) {
                        if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                            last_error = res;
                        } else {
                            last_error = sqlite3_extended_errcode(db);
                        }
                        last_err_msg = Utilities::ToWide(sqlite3_errmsg(db));
                    }
                    ++m_fails;
                } else {
                    assert(statement);
                    int ret = DoStep(statement);
                    assert(ret == SQLITE_OK || ret == SQLITE_DONE || ret == SQLITE_ROW);
                    ret = DoFinalize(statement);
                    assert(ret == SQLITE_OK);
                    ++m_oks;
                }
                if (sqlUtf8 == tail) {
                    break;
                } else {
                    sqlUtf8 = tail;
                }
            } while (tail && strlen(tail));
            if (lastInsertId) {
                vtId = (INT64) sqlite3_last_insert_rowid(db);
            }
            if (last_error) {
                res = last_error;
                errMsg = last_err_msg;
            }
        }

        void CSqliteImpl::Execute(const std::wstring& sql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 & fail_ok) {
            ResetMemories();
            fail_ok = 0;
            affected = 0;
            if (!m_pSqlite) {
                res = SPA::Sqlite::SQLITE_DB_NOT_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                ++m_fails;
                fail_ok = 1;
                fail_ok <<= 32;
                return;
            }
            UINT64 fails = m_fails;
            UINT64 oks = m_oks;
            int start = sqlite3_total_changes(m_pSqlite.get());
            CScopeUQueue sb;
            Utilities::ToUTF8(sql.c_str(), sql.size(), *sb);
            const char *sqlUtf8 = (const char*) sb->GetBuffer();
            if (rowset) {
                ExecuteSqlWithRowset(sqlUtf8, meta, lastInsertId, index, res, errMsg, vtId);
            } else {
                ExecuteSqlWithoutRowset(sqlUtf8, lastInsertId, res, errMsg, vtId);
            }
            affected = sqlite3_total_changes(m_pSqlite.get()) - start;
            fail_ok = ((m_fails - fails) << 32);
            fail_ok += (unsigned int) (m_oks - oks);
        }

        unsigned int CSqliteImpl::GetParameters() const {
            return (unsigned int) m_parameters;
        }

        size_t CSqliteImpl::GetParameterStatements() const {
            return m_vPreparedStatements.size();
        }

        sqlite3 * CSqliteImpl::GetDBHandle() const {
            return m_pSqlite.get();
        }

        const std::vector<std::shared_ptr<sqlite3_stmt> >& CSqliteImpl::GetPreparedStatements() const {
            return m_vPreparedStatements;
        }

        bool CSqliteImpl::IsGloballyConnected() const {
            return m_global;
        }

        void CSqliteImpl::Prepare(const std::wstring& sql, const CParameterInfoArray& params, int &res, std::wstring & errMsg, unsigned int &parameters) {
            ResetMemories();
            parameters = 0;
            if (!m_pSqlite) {
                res = SPA::Sqlite::SQLITE_DB_NOT_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            int last_error = SQLITE_OK;
            std::wstring error_message;
            m_vPreparedStatements.clear();
            m_parameters = 0;
            m_vParam.clear();
            sqlite3 *db = m_pSqlite.get();
            CScopeUQueue sb;
            Utilities::ToUTF8(sql.c_str(), sql.size(), *sb);
            const char* zTail = nullptr;
            sqlite3_stmt *stmt = nullptr;
            const char *pos = nullptr;
            const char *start = (const char*) sb->GetBuffer();
            do {
                do {
                    pos = start;
                    res = sqlite3_prepare_v2(db, start, (int) strlen(start), &stmt, &zTail);
                    if ((res == SQLITE_BUSY || res == SQLITE_LOCKED) && !stmt) {
                        sqlite3_sleep(SLEEP_TIME);
                        if (!IsOpened() || IsCanceled()) {
                            break;
                        }
                    } else {
                        break;
                    }
                } while (true);
                if (res) {
                    assert(!stmt);
                    if (!last_error) {
                        if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                            last_error = res;
                        } else {
                            last_error = sqlite3_extended_errcode(db);
                        }
                        error_message = Utilities::ToWide(sqlite3_errmsg(db));
                    }
                } else {
                    assert(stmt);
                    std::shared_ptr<sqlite3_stmt> pStatement(stmt, [this](sqlite3_stmt * s) {
                        if (s) {
                            int ret = this->DoFinalize(s);
                            assert(ret == SQLITE_OK);
                        }
                    });
                    m_vPreparedStatements.push_back(pStatement);
                }
                start = zTail;
            } while (zTail != pos && zTail && strlen(zTail));
            if (last_error) {
                res = last_error;
                errMsg = error_message;
                m_vPreparedStatements.clear();
            } else {
                for (auto it = m_vPreparedStatements.begin(), end = m_vPreparedStatements.end(); it != end; ++it) {
                    sqlite3_stmt *s = (*it).get();
                    m_parameters += sqlite3_bind_parameter_count(s);
                }
                parameters = (unsigned int) m_parameters;
                if (!res && zTail && strlen(zTail)) {
                    res = SQLITE_WARNING;
                    errMsg = Utilities::ToWide(zTail);
                }
            }
        }

        void CSqliteImpl::CloseDb(int &res, std::wstring & errMsg) {
            res = 0;
            m_vPreparedStatements.clear();
            m_pSqlite.reset();
            m_vParam.clear();
            m_global = true;
            ResetMemories();
        }

        void CSqliteImpl::BeginTrans(int isolation, const std::wstring &dbConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msSqlite;
            if (!m_pSqlite) {
                std::wstring s = dbConn;
                if (!s.size() && m_global) {
                    m_csPeer.lock();
                    s = m_strGlobalConnection;
                    m_csPeer.unlock();
                }
                if (s.size()) {
                    res = DoSafeOpen(s, flags);
                }
            }
            if (!m_pSqlite) {
                if (!res) {
                    res = SPA::Sqlite::SQLITE_DB_NOT_OPENED_YET;
                    errMsg = NO_DB_OPENED_YET;
                }
                return;
            }
            char* zErrMsg = nullptr;
            sqlite3 *db = m_pSqlite.get();
            do {
                res = sqlite3_exec(db, "BEGIN", nullptr, nullptr, &zErrMsg);
                if (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                    sqlite3_sleep(SLEEP_TIME);
                    if (!IsOpened() || IsCanceled()) {
                        return;
                    }
                } else {
                    break;
                }
            } while (true);
            if (res == SQLITE_OK) {
                if (!m_global) {
                    const char *str = sqlite3_db_filename(db, nullptr);
                    errMsg = Utilities::ToWide(str);
                } else {
                    m_csPeer.lock();
                    errMsg = m_strGlobalConnection;
                    m_csPeer.unlock();
                }
                m_ti = tiSerializable;
                m_fails = 0;
                m_oks = 0;
            } else {
                if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) != Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                    res = sqlite3_extended_errcode(db);
                }
                errMsg = Utilities::ToWide(zErrMsg);
                sqlite3_free(zErrMsg);
            }
        }

        void CSqliteImpl::EndTrans(int plan, int &res, std::wstring & errMsg) {
            if (!m_pSqlite) {
                res = SPA::Sqlite::SQLITE_DB_NOT_OPENED_YET;
                errMsg = NO_DB_OPENED_YET;
                return;
            }
            if (plan < 0 || plan > rpRollbackAlways) {
                res = SPA::Sqlite::SQLITE_BAD_END_TRANSTACTION_PLAN;
                errMsg = BAD_END_TRANSTACTION_PLAN;
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
            char* zErrMsg = nullptr;
            sqlite3 *db = m_pSqlite.get();
            do {
                if (rollback) {
                    res = sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, &zErrMsg);
                } else {
                    res = sqlite3_exec(db, "COMMIT", nullptr, nullptr, &zErrMsg);
                }
                if (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                    sqlite3_sleep(SLEEP_TIME);
                } else {
                    break;
                }
            } while (true);
            if (res == SQLITE_OK) {
                m_ti = tiUnspecified;
                m_fails = 0;
                m_oks = 0;
            } else {
                errMsg = Utilities::ToWide(zErrMsg);
                if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) != Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                    res = sqlite3_extended_errcode(db);
                }
                sqlite3_free(zErrMsg);
            }
        }

        int CSqliteImpl::DoSafeOpen(const std::wstring &strConnection, unsigned int flags) {
            int res = SQLITE_OK;
            bool bUTF16 = ((Sqlite::USE_UTF16_ENCODING & m_nParam) == Sqlite::USE_UTF16_ENCODING);
            sqlite3 *db = nullptr;
            CScopeUQueue sb;
            do {
                if (bUTF16) {
#ifdef WIN32_64
                    res = sqlite3_open16(strConnection.c_str(), &db);
#else
                    Utilities::ToUTF16(strConnection.c_str(), strConnection.size(), *sb);
                    res = sqlite3_open16((const SPA::UTF16*)sb->GetBuffer(), &db);
#endif
                } else {
                    Utilities::ToUTF8(strConnection.c_str(), strConnection.size(), *sb);
                    res = sqlite3_open((const char*) sb->GetBuffer(), &db);
                }
                if (res == SQLITE_BUSY || res == SQLITE_LOCKED) {
                    sqlite3_sleep(SLEEP_TIME);
                    if (db) {
                        int ret = sqlite3_close_v2(db);
                        assert(ret == SQLITE_OK);
                        db = nullptr;
                    }
                    if (!IsOpened() || IsCanceled()) {
                        break;
                    }
                } else {
                    break;
                }
            } while (true);
            m_pSqlite.reset(db, [](sqlite3 * p) {
                if (p) {
                    int ret = sqlite3_close_v2(p);
                    assert(ret == SQLITE_OK);
                }
            });
            return res;
        }

        void CSqliteImpl::Open(const std::wstring &strConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms) {
            ms = msSqlite;
            m_vPreparedStatements.clear();
            m_pSqlite.reset();
            ResetMemories();
            std::wstring strConnection = strConn;
            if (strConnection.size()) {
                m_global = false;
            } else {
                m_csPeer.lock();
                strConnection = m_strGlobalConnection;
                m_csPeer.unlock();
                m_global = true;
            }
            if (!strConnection.size()) {
                res = SPA::Sqlite::SQLITE_NO_DB_FILE_SPECIFIED;
                errMsg = NO_DB_FILE_SPECIFIED;
                return;
            }
            res = DoSafeOpen(strConnection, flags);
            sqlite3 *db = m_pSqlite.get();
            if (res) {
                if (db) {
                    const char *str = sqlite3_errmsg(db);
                    errMsg = Utilities::ToWide(str);
                    if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) != Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                        res = sqlite3_extended_errcode(db);
                    }
                }
                return;
            } else if (!m_global) {
                const char *str = sqlite3_db_filename(db, nullptr);
                errMsg = Utilities::ToWide(str);
            } else {
                errMsg = strConnection;
            }
            if ((m_nParam & Sqlite::ENABLE_GLOBAL_SQLITE_UPDATE_HOOK) == Sqlite::ENABLE_GLOBAL_SQLITE_UPDATE_HOOK) {
                sqlite3_update_hook(db, update_callback, this);
                sqlite3_commit_hook(db, commit_hook, this);
                sqlite3_rollback_hook(db, rollback_hook, this);
            }
        }

        int CSqliteImpl::commit_hook(void *p) {
            CAutoLock al(m_csPeer);
            auto it = m_mapUpdate.find((CSqliteImpl*) p);
            if (it != m_mapUpdate.end()) {
                CScopeUQueue sb;
                auto &v = it->second;
                std::vector<CSqliteUpdateContext>::iterator pos, end = v->end();
                for (pos = v->begin(); pos != end; ++pos) {
                    sb << pos->type << pos->instance << pos->db_name << pos->db_name << pos->rowid;
                    std::vector<CSqliteImpl*>::iterator itPeer, endPeer = m_vSqlitePeer.end();
                    for (itPeer = m_vSqlitePeer.begin(); itPeer != endPeer; ++itPeer) {
                        CSqliteImpl *peer = *itPeer;
                        peer->SendResult(idDBUpdate, sb->GetBuffer(), sb->GetSize());
                    }
                    sb->SetSize(0);
                }
                m_mapUpdate.erase(it);
            }
            return 0;
        }

        void CSqliteImpl::rollback_hook(void *p) {
            CAutoLock al(m_csPeer);
            auto it = m_mapUpdate.find((CSqliteImpl*) p);
            if (it != m_mapUpdate.end()) {
                m_mapUpdate.erase(it);
            }
        }

        void CSqliteImpl::update_callback(void* udp, int type, const char* db_name, const char* tbl_name, sqlite3_int64 rowid) {
            bool autocommit = true;
            switch (type) {
                case SQLITE_DELETE:
                    type = (int) ueDelete;
                    break;
                case SQLITE_UPDATE:
                    type = (int) ueUpdate;
                    break;
                case SQLITE_INSERT:
                    type = (int) ueInsert;
                    break;
                default:
                    assert(false);
                    break;
            }
            CSqliteImpl *sender = (CSqliteImpl*) udp;
            std::wstring instance;
            if (sender) {
                sqlite3 *db = sender->m_pSqlite.get();
                assert(db != nullptr);
                autocommit = sqlite3_get_autocommit(db) ? true : false;
                if (!sender->m_global) {
                    instance = Utilities::ToWide(sqlite3_db_filename(db, db_name));
                }
            }
            if (autocommit) {
                std::wstring dbName = Utilities::ToWide(db_name);
                std::wstring tableName = Utilities::ToWide(tbl_name);
                CDBVariant vtId((INT64) rowid);
                CScopeUQueue sb;
                sb << type << instance << dbName << tableName << vtId;
                CAutoLock al(m_csPeer);
                std::vector<CSqliteImpl*>::iterator it, end = m_vSqlitePeer.end();
                for (it = m_vSqlitePeer.begin(); it != end; ++it) {
                    CSqliteImpl *peer = *it;
                    peer->SendResult(idDBUpdate, sb->GetBuffer(), sb->GetSize());
                }
            } else {
                CSqliteUpdateContext ctx((tagUpdateEvent) type, (INT64) rowid);
                ctx.instance = instance;
                ctx.db_name = Utilities::ToWide(db_name);
                ctx.tbl_name = Utilities::ToWide(tbl_name);
                CAutoLock al(m_csPeer);
                auto it = m_mapUpdate.find(sender);
                if (it != m_mapUpdate.end()) {
                    auto &p = it->second;
                    p->push_back((CSqliteUpdateContext&&)ctx);
                } else {
                    std::shared_ptr<std::vector<CSqliteUpdateContext> > p(new std::vector<CSqliteUpdateContext>);
                    p->push_back((CSqliteUpdateContext&&)ctx);
                    m_mapUpdate[sender] = p;
                }
            }
        }

        void CSqliteImpl::SetPrecisionScale(const std::string& str, CDBColumnInfo & info) {
            size_t pos = str.find('(');
            if (pos != (size_t) - 1) {
                info.Precision = (unsigned char) std::atoi(str.c_str() + pos + 1);
            }
            pos = str.find(',', pos + 1);
            if (pos != (size_t) - 1) {
                info.Scale = (unsigned char) std::atoi(str.c_str() + pos + 1);
            }
        }

        void CSqliteImpl::SetLen(const std::string& str, CDBColumnInfo & info) {
            size_t pos = str.find('(');
            if (pos != (size_t) - 1) {
                info.ColumnSize = (unsigned int) std::atoi(str.c_str() + pos + 1);
            }
        }

        void CSqliteImpl::SetDataType(const char *str, CDBColumnInfo & info) {
            if (str) {
                std::string datatype(str);
                std::transform(datatype.begin(), datatype.end(), datatype.begin(), ::toupper);
                if (datatype.find("DOUBLE") != (size_t) - 1) {
                    info.DataType = VT_R8;
                } else if (datatype.find("REAL") != (size_t) - 1) {
                    info.DataType = VT_R8;
                } else if (datatype.find("BIGINT") != (size_t) - 1) {
                    info.DataType = VT_I8;
                } else if (datatype.find("UNSIGNED") != (size_t) - 1) {
                    info.DataType = VT_UI8;
                } else if (datatype.find("BIGINT") != (size_t) - 1) {
                    info.DataType = VT_I8;
                } else if (datatype.find("INT8") != (size_t) - 1) {
                    info.DataType = VT_I8;
                } else if (datatype.find("TINYINT") != (size_t) - 1) {
                    info.DataType = VT_I1;
                } else if (datatype.find("SMALLINT") != (size_t) - 1) {
                    info.DataType = VT_I2;
                } else if (datatype.find("INT2") != (size_t) - 1) {
                    info.DataType = VT_I2;
                } else if (datatype.find("FLOAT") != (size_t) - 1) {
                    info.DataType = VT_R8;
                } else if (datatype.find("BLOB") != (size_t) - 1) {
                    info.DataType = (VT_UI1 | VT_ARRAY);
                    info.ColumnSize = -1;
                } else if (datatype.find("NTEXT") != (size_t) - 1) {
                    info.DataType = VT_BSTR;
                    info.ColumnSize = -1;
                } else if (datatype.find("TEXT") != (size_t) - 1) {
                    info.DataType = (VT_I1 | VT_ARRAY);
                    info.ColumnSize = -1;
                } else if (datatype.find("CLOB") != (size_t) - 1) {
                    info.DataType = (VT_I1 | VT_ARRAY);
                    info.ColumnSize = -1;
                } else if (datatype.find("BOOLEAN") != (size_t) - 1) {
                    info.DataType = VT_BOOL;
                } else if (datatype.find("DATETIME") != (size_t) - 1) {
                    info.DataType = VT_DATE;
                } else if (datatype.find("DATE") != (size_t) - 1) {
                    info.DataType = VT_DATE;
                } else if (datatype.find("TIME") != (size_t) - 1) {
                    info.DataType = VT_DATE;
                } else if (datatype.find("NUM") != (size_t) - 1) {
                    info.DataType = VT_R8;
                    SetPrecisionScale(datatype, info);
                } else if (datatype.find("DEC") != (size_t) - 1) {
                    info.DataType = VT_R8;
                    SetPrecisionScale(datatype, info);
                } else if (datatype.find("MEDIUMINT") != (size_t) - 1) {
                    info.DataType = VT_I4;
                } else if (datatype.find("INT") != (size_t) - 1/*INTEGER*/) {
                    info.DataType = VT_I8;
                } else if (datatype.find("NATIVE") != (size_t) - 1) {
                    info.DataType = VT_BSTR;
                    SetLen(datatype, info);
                } else if (datatype.find("NVARCHAR") != (size_t) - 1) {
                    info.DataType = VT_BSTR;
                    SetLen(datatype, info);
                } else if (datatype.find("NCHAR") != (size_t) - 1) {
                    info.DataType = VT_BSTR;
                    SetLen(datatype, info);
                } else if (datatype.find("CHAR") != (size_t) - 1) {
                    info.DataType = (VT_I1 | VT_ARRAY);
                    SetLen(datatype, info);
                } else {
                    info.DataType = VT_VARIANT;
                }
            } else {
                info.DataType = VT_VARIANT;
            }
        }

        void CSqliteImpl::SetOtherColumnInfoFlags(CDBColumnInfoArray & vCols) {
            unsigned short pk_dt = VT_EMPTY;
            int pk_col = -1;
            int pk_count = 0;
            int index = 0;
            for (auto it = vCols.begin(), end = vCols.end(); it != end; ++it) {
                if ((it->Flags & CDBColumnInfo::FLAG_PRIMARY_KEY) == CDBColumnInfo::FLAG_PRIMARY_KEY) {
                    ++pk_count;
                    pk_col = index;
                    pk_dt = it->DataType;
                }
                ++index;
            }
            if (pk_count == 1) {
                vCols[pk_col].Flags |= CDBColumnInfo::FLAG_UNIQUE;
                bool row_id = true;
                switch (pk_dt) {
                    case VT_I1:
                    case VT_UI1:
                    case VT_I2:
                    case VT_UI2:
                    case VT_I4:
                    case VT_UI4:
                    case VT_I8:
                    case VT_UI8:
                    case VT_INT:
                    case VT_UINT:
                        break;
                    default:
                        row_id = false;
                        break;
                }
                if (row_id) {
                    vCols[pk_col].Flags |= (CDBColumnInfo::FLAG_AUTOINCREMENT | CDBColumnInfo::FLAG_ROWID);
                }
            }
        }

        CDBColumnInfoArray CSqliteImpl::GetColInfo(bool meta, sqlite3_stmt * stmt) {
            std::string zDbName, zTableName, zColumnName;
            CDBColumnInfoArray vCols;
            int cols = sqlite3_column_count(stmt);
            for (int n = 0; n < cols; ++n) {
                vCols.push_back(CDBColumnInfo());
                CDBColumnInfo &info = vCols.back();
                const char *str = sqlite3_column_database_name(stmt, n);
                if (meta && str) {
                    zDbName = str;
                }
                if (str) {
                    info.DBPath = Utilities::ToWide(str);
                } else {
                    info.DBPath.clear();
                }
                str = sqlite3_column_table_name(stmt, n);
                if (meta && str) {
                    zTableName = str;
                }
                if (str) {
                    info.TablePath = Utilities::ToWide(str);
                } else {
                    info.Flags = (CDBColumnInfo::FLAG_NOT_NULL | CDBColumnInfo::FLAG_NOT_WRITABLE);
                    info.TablePath.clear();
                }

                str = sqlite3_column_name(stmt, n);
                if (str) {
                    info.DisplayName = Utilities::ToWide(str);
                } else {
                    info.DisplayName.clear();
                }

                str = sqlite3_column_origin_name(stmt, n);
                if (meta && str) {
                    zColumnName = str;
                }
                if (str) {
                    info.OriginalName = Utilities::ToWide(str);
                } else {
                    info.OriginalName.clear();
                }

                str = sqlite3_column_decltype(stmt, n);
                if (str) {
                    info.DeclaredType = Utilities::ToWide(str);
                } else {
                    info.DeclaredType.clear();
                }
                SetDataType(str, info);
                if (meta && zTableName.size()) {
                    //char const *dt = nullptr;
                    char const *colseq = nullptr;
                    int not_null = 0, pk = 0, autoinc = 0;
                    int rec = sqlite3_table_column_metadata(m_pSqlite.get(),
                            zDbName.c_str(), zTableName.c_str(), zColumnName.c_str(),
                            nullptr, &colseq, &not_null, &pk, &autoinc);
                    if (!rec) {
                        if (not_null)
                            info.Flags |= CDBColumnInfo::FLAG_NOT_NULL;
                        if (pk)
                            info.Flags |= CDBColumnInfo::FLAG_PRIMARY_KEY;
                        if (autoinc)
                            info.Flags |= CDBColumnInfo::FLAG_AUTOINCREMENT;
                        if (colseq) {
                            info.Collation = Utilities::ToWide(colseq);
                        } else {
                            info.Collation.clear();
                        }
                    }
                    zDbName.clear();
                    zTableName.clear();
                    zColumnName.clear();
                }
            }
            if (meta) {
                SetOtherColumnInfoFlags(vCols);
            }
            return vCols;
        }

        bool CSqliteImpl::SendRows(CScopeUQueue& sb, bool transferring) {
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

        bool CSqliteImpl::SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes) {
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

        bool CSqliteImpl::PushRecords(sqlite3_stmt *statement, const CDBColumnInfoArray &vColInfo, int &res, std::string & errMsg) {
            unsigned short vt;
            CScopeUQueue sb;
            int cols = (int) vColInfo.size();
            res = DoStep(statement);
            while (res == SQLITE_ROW) {
                bool blob = false;
                for (int n = 0; n < cols; ++n) {
                    const CDBColumnInfo &info = vColInfo[n];
                    int data_type = sqlite3_column_type(statement, n);
                    switch (data_type) {
                        case SQLITE_INTEGER:
                            vt = info.DataType;
                            if (vt == VT_VARIANT) {
                                vt = VT_I8;
                            }
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                            if (vt == VT_I8 || vt == VT_UI8 || vt == VT_DATE || vt == VT_VARIANT) {
                                sb << sqlite3_column_int64(statement, n);
                            } else if (vt == VT_I4) {
                                sb << sqlite3_column_int(statement, n);
                            } else if (vt == VT_I2 || vt == VT_UI2 || vt == VT_BOOL) {
                                unsigned short data = (unsigned short) sqlite3_column_int(statement, n);
                                sb << data;
                            } else if (vt == VT_I1 || vt == VT_UI1) {
                                unsigned char one_byte = (unsigned char) sqlite3_column_int(statement, n);
                                sb->Push(&one_byte, sizeof (one_byte));
                            } else {
                                assert(false); //shouldn't come here
                            }
                            break;
                        case SQLITE_FLOAT:
                            vt = VT_R8;
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                            sb << sqlite3_column_double(statement, n);
                            break;
                        case SQLITE_TEXT:
                        {
                            unsigned int bytes;
                            const unsigned char *buffer;
                            vt = info.DataType;
                            if (VT_BSTR == vt) {
                                bytes = (unsigned int) sqlite3_column_bytes16(statement, n);
                                buffer = (const unsigned char*) sqlite3_column_text16(statement, n);
                            } else {
                                vt = (VT_ARRAY | VT_I1);
                                bytes = (unsigned int) sqlite3_column_bytes(statement, n);
                                buffer = sqlite3_column_text(statement, n);
                            }
                            if (bytes > (2 * DEFAULT_BIG_FIELD_CHUNK_SIZE)) {
                                if (sb->GetSize() && !SendRows(sb, true)) {
                                    return false;
                                }
                                bool batching = IsBatching();
                                if (batching) {
                                    CommitBatching();
                                }
                                if (!SendBlob(vt, buffer, (unsigned int) bytes)) {
                                    return false;
                                }
                                if (batching) {
                                    StartBatching();
                                }
                                blob = true;
                            } else if (info.DataType == VT_DATE) {
                                const char *str = (const char*) buffer;
                                UDateTime udt(str);
                                vt = VT_DATE;
                                sb->Push((const unsigned char*) &vt, sizeof (vt));
                                sb << udt.time;
                            } else {
                                sb->Push((const unsigned char*) &vt, sizeof (vt));
                                sb << bytes;
                                if (bytes) {
                                    sb->Push(buffer, (unsigned int) bytes);
                                }
                            }
                        }
                            break;
                        case SQLITE_BLOB:
                        {
                            vt = (VT_UI1 | VT_ARRAY);
                            int bytes = sqlite3_column_bytes(statement, n);
                            if (bytes > (int) (2 * DEFAULT_BIG_FIELD_CHUNK_SIZE)) {
                                if (sb->GetSize() && !SendRows(sb, true)) {
                                    return false;
                                }
                                const unsigned char *buffer = (const unsigned char*) sqlite3_column_blob(statement, n);
                                if (!SendBlob(vt, buffer, (unsigned int) bytes)) {
                                    return false;
                                }
                                blob = true;
                            } else {
                                sb->Push((const unsigned char*) &vt, sizeof (vt));
                                sb << (unsigned int) bytes;
                                if (bytes) {
                                    sb->Push((const unsigned char*) sqlite3_column_blob(statement, n), (unsigned int) bytes);
                                }
                            }
                        }
                            break;
                        case SQLITE_NULL:
                            vt = VT_NULL;
                            sb->Push((const unsigned char*) &vt, sizeof (vt));
                            break;
                        default:
                            assert(false); //shouldn't come here
                            break;
                    }
                }
                if ((sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE || blob) && !SendRows(sb)) {
                    return false;
                }
                res = DoStep(statement);
            }
            if (res == SQLITE_DONE) {
                res = 0;
            } else if (res) {
                if ((m_nParam & Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) != Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) {
                    res = sqlite3_extended_errcode(m_pSqlite.get());
                }
                errMsg = sqlite3_errmsg(m_pSqlite.get());
            }
            if (sb->GetSize()) {
                return SendRows(sb);
            }
            return true;
        }
    } //namespace ServerSide
} //namespace SPA