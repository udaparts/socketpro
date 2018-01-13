
#ifndef _UDAPARTS_ASYNC_ODBC_HANDLER_H_
#define _UDAPARTS_ASYNC_ODBC_HANDLER_H_

#include "odbc/uodbc.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Odbc::sidOdbc> COdbcBase;

        class COdbc : public COdbcBase {
            //no copy constructor supported
            COdbc(const COdbc& ao);

            //no assignment operator supported
            COdbc& operator=(const COdbc& ao);

        public:

            COdbc(CClientSocket *cs = nullptr) : COdbcBase(cs) {

            }

        public:

            virtual bool ColumnPrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded);
            }

            virtual bool Columns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded);
            }

            virtual bool ProcedureColumns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, const wchar_t *ColumnName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, discarded);
            }

            virtual bool PrimaryKeys(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, discarded);
            }

            virtual bool ForeignKeys(const wchar_t *PKCatalogName, const wchar_t *PKSchemaName, const wchar_t *PKTableName, const wchar_t *FKCatalogName, const wchar_t *FKSchemaName, const wchar_t *FKTableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh, discarded);
            }

            virtual bool Procedures(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, discarded);
            }

            virtual bool SpecialColumns(short identifierType, const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, short scope, short nullable, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh, discarded);
            }

            virtual bool Statistics(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, unsigned short unique, unsigned short reserved, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << CatalogName << SchemaName << TableName << unique << reserved << index;
                ResultHandler arh = [handler, this](CAsyncResult & ar) {
                    UINT64 fail_ok;
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> fail_ok;
                    this->m_csDB.lock();
                    this->m_lastReqId = SPA::Odbc::idSQLStatistics;
                    this->m_affected = 0;
                    this->m_dbErrCode = res;
                    this->m_dbErrMsg = errMsg;
                    auto it = this->m_mapRowset.find(this->m_indexRowset);
                    if (it != this->m_mapRowset.end()) {
                        this->m_mapRowset.erase(it);
                    }
                    this->m_csDB.unlock();
                    if (handler) {
                        CDBVariant vtNull;
                        handler(*this, res, errMsg, 0, fail_ok, vtNull);
                    }
                };
                if (!SendRequest(SPA::Odbc::idSQLStatistics, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            virtual bool TablePrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, discarded);
            }

            virtual bool Tables(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *TableType, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, discarded);
            }

            CComVariant GetInfo(unsigned short infoType) {
                CComVariant infoValue;
                m_csDB.lock();
                if (m_mapInfo.find(infoType) != m_mapInfo.end()) {
                    infoValue = m_mapInfo[infoType];
                }
                m_csDB.unlock();
                return infoValue;
            }

        protected:

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case SPA::Odbc::idSQLGetInfo:
                    {
                        CAutoLock al(m_csDB);
                        m_mapInfo.clear();
                        while (mc.GetSize()) {
                            unsigned short infoType;
                            CComVariant infoValue;
                            mc >> infoType >> infoValue;
                            m_mapInfo[infoType] = infoValue;
                        }
                    }
                        break;
                    default:
                        COdbcBase::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:

            bool DoMeta(unsigned short id, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << index;
                ResultHandler arh = [id, handler, this](CAsyncResult & ar) {
                    UINT64 fail_ok;
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> fail_ok;
                    this->m_csDB.lock();
                    this->m_lastReqId = id;
                    this->m_affected = 0;
                    this->m_dbErrCode = res;
                    this->m_dbErrMsg = errMsg;
                    auto it = this->m_mapRowset.find(this->m_indexRowset);
                    if (it != this->m_mapRowset.end()) {
                        this->m_mapRowset.erase(it);
                    }
                    this->m_csDB.unlock();
                    if (handler) {
                        CDBVariant vtNull;
                        handler(*this, res, errMsg, 0, fail_ok, vtNull);
                    }
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            bool DoMeta(unsigned short id, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << s3 << index;
                ResultHandler arh = [id, handler, this](CAsyncResult & ar) {
                    UINT64 fail_ok;
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> fail_ok;
                    this->m_csDB.lock();
                    this->m_lastReqId = id;
                    this->m_affected = 0;
                    this->m_dbErrCode = res;
                    this->m_dbErrMsg = errMsg;
                    auto it = this->m_mapRowset.find(this->m_indexRowset);
                    if (it != this->m_mapRowset.end()) {
                        this->m_mapRowset.erase(it);
                    }
                    this->m_csDB.unlock();
                    if (handler) {
                        CDBVariant vtNull;
                        handler(*this, res, errMsg, 0, fail_ok, vtNull);
                    }
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            template<typename T0, typename T1, typename T2>
            bool DoMeta(unsigned short id, const T0 &t0, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, const T1 &t1, const T2 &t2, DExecuteResult handler, DRows row, DRowsetHeader rh, DDiscarded discarded) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << t0 << s0 << s1 << s2 << t1 << t2 << index;
                ResultHandler arh = [id, handler, this](CAsyncResult & ar) {
                    UINT64 fail_ok;
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> fail_ok;
                    this->m_csDB.lock();
                    this->m_lastReqId = id;
                    this->m_affected = 0;
                    this->m_dbErrCode = res;
                    this->m_dbErrMsg = errMsg;
                    auto it = this->m_mapRowset.find(this->m_indexRowset);
                    if (it != this->m_mapRowset.end()) {
                        this->m_mapRowset.erase(it);
                    }
                    this->m_csDB.unlock();
                    if (handler) {
                        CDBVariant vtNull;
                        handler(*this, res, errMsg, 0, fail_ok, vtNull);
                    }
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

        private:
            std::unordered_map<unsigned short, CComVariant> m_mapInfo;
        };
    } //namespace ClientSide
} //namespace SPA

#endif