#ifndef _UDAPARTS_ASYNC_ODBC_HANDLER_H_
#define _UDAPARTS_ASYNC_ODBC_HANDLER_H_

#include "odbc/uodbc.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Odbc::sidOdbc> COdbcBase;

        class COdbc : public COdbcBase {
            COdbc(const COdbc& ao) = delete;
            COdbc& operator=(const COdbc& ao) = delete;

        public:

            COdbc(CClientSocket *cs) : COdbcBase(cs) {

            }

        public:

#ifdef NATIVE_UTF16_SUPPORTED

            virtual bool ColumnPrivileges(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, const char16_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool Columns(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, const char16_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool ProcedureColumns(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *ProcName, const char16_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool PrimaryKeys(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, discarded, se);
            }

            virtual bool ForeignKeys(const char16_t *PKCatalogName, const char16_t *PKSchemaName, const char16_t *PKTableName, const char16_t *FKCatalogName, const char16_t *FKSchemaName, const char16_t *FKTableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh, discarded, se);
            }

            virtual bool Procedures(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *ProcName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, discarded, se);
            }

            virtual bool SpecialColumns(short identifierType, const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, short scope, short nullable, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh, discarded, se);
            }

            virtual bool Statistics(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, unsigned short unique, unsigned short reserved, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << CatalogName << SchemaName << TableName << unique << reserved << index;
                DResultHandler arh = [handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, SPA::Odbc::idSQLStatistics, index);
                };
                if (!SendRequest(SPA::Odbc::idSQLStatistics, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            virtual bool TablePrivileges(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, discarded, se);
            }

            virtual bool Tables(const char16_t *CatalogName, const char16_t *SchemaName, const char16_t *TableName, const char16_t *TableType, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, discarded, se);
            }
#endif

            virtual bool ColumnPrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumnPrivileges, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool Columns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLColumns, CatalogName, SchemaName, TableName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool ProcedureColumns(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, const wchar_t *ColumnName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedureColumns, CatalogName, SchemaName, ProcName, ColumnName, handler, row, rh, discarded, se);
            }

            virtual bool PrimaryKeys(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLPrimaryKeys, CatalogName, SchemaName, TableName, handler, row, rh, discarded, se);
            }

            virtual bool ForeignKeys(const wchar_t *PKCatalogName, const wchar_t *PKSchemaName, const wchar_t *PKTableName, const wchar_t *FKCatalogName, const wchar_t *FKSchemaName, const wchar_t *FKTableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLForeignKeys, PKCatalogName, PKSchemaName, PKTableName, FKCatalogName, FKSchemaName, FKTableName, handler, row, rh, discarded, se);
            }

            virtual bool Procedures(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *ProcName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLProcedures, CatalogName, SchemaName, ProcName, handler, row, rh, discarded, se);
            }

            virtual bool SpecialColumns(short identifierType, const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, short scope, short nullable, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException& se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLSpecialColumns, identifierType, CatalogName, SchemaName, TableName, scope, nullable, handler, row, rh, discarded, se);
            }

            virtual bool Statistics(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, unsigned short unique, unsigned short reserved, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << CatalogName << SchemaName << TableName << unique << reserved << index;
                DResultHandler arh = [handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, SPA::Odbc::idSQLStatistics, index);
                };
                if (!SendRequest(SPA::Odbc::idSQLStatistics, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            virtual bool TablePrivileges(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTablePrivileges, CatalogName, SchemaName, TableName, handler, row, rh, discarded, se);
            }

            virtual bool Tables(const wchar_t *CatalogName, const wchar_t *SchemaName, const wchar_t *TableName, const wchar_t *TableType, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded = nullptr, const DServerException &se = nullptr) {
                return DoMeta(SPA::Odbc::idSQLTables, CatalogName, SchemaName, TableName, TableType, handler, row, rh, discarded, se);
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

            void ProcessODBC(const DExecuteResult &handler, CAsyncResult & ar, unsigned short reqId, UINT64 index) {
                UINT64 fail_ok;
                int res;
                std::wstring errMsg;
                ar >> res >> errMsg >> fail_ok;
                COdbc *ash = (COdbc *) ar.AsyncServiceHandler;
                ash->m_csDB.lock();
                ash->m_lastReqId = reqId;
                ash->m_affected = 0;
                ash->m_dbErrCode = res;
                ash->m_dbErrMsg = errMsg;
                auto it = ash->m_mapRowset.find(index);
                if (it != ash->m_mapRowset.end()) {
                    ash->m_mapRowset.erase(it);
                }
                ash->m_csDB.unlock();
                if (handler) {
                    CDBVariant vtNull;
                    handler(*ash, res, errMsg, 0, fail_ok, vtNull);
                }
            }

            bool DoMeta(unsigned short id, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException& se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            bool DoMeta(unsigned short id, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, const wchar_t *s3, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException& se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << s3 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            template<typename T0, typename T1, typename T2>
            bool DoMeta(unsigned short id, const T0 &t0, const wchar_t *s0, const wchar_t *s1, const wchar_t *s2, const T1 &t1, const T2 &t2, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException& se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << t0 << s0 << s1 << s2 << t1 << t2 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }


#ifdef NATIVE_UTF16_SUPPORTED

            bool DoMeta(unsigned short id, const char16_t *s0, const char16_t *s1, const char16_t *s2, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException& se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            bool DoMeta(unsigned short id, const char16_t *s0, const char16_t *s1, const char16_t *s2, const char16_t *s3, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException& se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << s0 << s1 << s2 << s3 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }

            template<typename T0, typename T1, typename T2>
            bool DoMeta(unsigned short id, const T0 &t0, const char16_t *s0, const char16_t *s1, const char16_t *s2, const T1 &t1, const T2 &t2, const DExecuteResult &handler, const DRows &row, const DRowsetHeader &rh, const DDiscarded &discarded, const DServerException &se) {
                UINT64 index = GetCallIndex();
                CScopeUQueue sb;
                //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                m_csDB.lock();
                m_mapRowset[index] = CRowsetHandler(rh, row);
                m_csDB.unlock();
                sb << t0 << s0 << s1 << s2 << t1 << t2 << index;
                DResultHandler arh = [id, handler, this, index](CAsyncResult & ar) {
                    this->ProcessODBC(handler, ar, id, index);
                };
                if (!SendRequest(id, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    m_csDB.lock();
                    m_mapRowset.erase(index);
                    m_csDB.unlock();
                    return false;
                }
                return true;
            }
#endif
        private:
            std::unordered_map<unsigned short, CComVariant> m_mapInfo;
        };
        typedef CSocketPool<COdbc> COdbcPool;
    } //namespace ClientSide
} //namespace SPA

#endif