#pragma once

#include <memory>
#include "sqlite3.h"
#include "../usqlite_server.h"
#include "../../udatabase.h"
#include "../../aserverw.h"
#include <unordered_map>

namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class CSqliteImpl : public CClientPeer {
            //no copy constructor
            CSqliteImpl(const CSqliteImpl &impl);
            //no assignment operator
            CSqliteImpl& operator=(const CSqliteImpl &impl);

            class CSqliteUpdateContext {
                CSqliteUpdateContext(const CSqliteUpdateContext &ctx);
                CSqliteUpdateContext& operator=(const CSqliteUpdateContext &ctx);

            public:

                CSqliteUpdateContext()
                : type(ueInsert) {
                }

                CSqliteUpdateContext(tagUpdateEvent ue)
                : type(ue) {
                }

                template <typename RowID>
                CSqliteUpdateContext(tagUpdateEvent ue, const RowID &id)
                : type(ue), rowid(id) {
                }

                CSqliteUpdateContext(CSqliteUpdateContext &&ctx)
                : type(ctx.type),
                instance(std::move(ctx.instance)),
                db_name(std::move(ctx.db_name)),
                tbl_name(std::move(ctx.tbl_name)),
                rowid(std::move(ctx.rowid)) {
                }

                CSqliteUpdateContext& operator=(CSqliteUpdateContext &&ctx) {
                    if (this == &ctx) {
                        return *this;
                    }
                    type = ctx.type;
                    instance = std::move(ctx.instance);
                    db_name = std::move(ctx.db_name);
                    tbl_name = std::move(ctx.tbl_name);
                    rowid = std::move(ctx.rowid);
                    return *this;
                }

            public:
                tagUpdateEvent type; //tagUpdateEvent
                std::wstring instance;
                std::wstring db_name;
                std::wstring tbl_name;
                CDBVariant rowid;
            };

        public:
            CSqliteImpl();
            ~CSqliteImpl();
            unsigned int GetParameters() const;
            size_t GetParameterStatements() const;
            sqlite3* GetDBHandle() const;
            const std::vector<std::shared_ptr<sqlite3_stmt> >& GetPreparedStatements() const;
            bool IsGloballyConnected() const;
            static void SetDBGlobalConnectionString(const wchar_t *dbConnection);
            static void SetInitialParam(unsigned int param);

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        protected:
            virtual void Open(const std::wstring &strConnection, unsigned int flags, int &res, std::wstring &errMsg, int &ms);
            virtual void CloseDb(int &res, std::wstring &errMsg);
            virtual void BeginTrans(int isolation, const std::wstring &dbConn, unsigned int flags, int &res, std::wstring &errMsg, int &ms);
            virtual void EndTrans(int plan, int &res, std::wstring &errMsg);
            virtual void Execute(const std::wstring& sql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void Prepare(const std::wstring& sql, const CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters);
            virtual void ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void ExecuteBatch(const std::wstring& sql, const std::wstring& delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, const std::wstring &dbConn, unsigned int flags, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 &fail_ok);

        private:
            void ReleaseArray();
            void StartBLOB(unsigned int lenExpected);
            void Chunk();
            void EndBLOB();
            bool PushRecords(sqlite3_stmt *statement, const CDBColumnInfoArray &vColInfo, int &res, std::string &errMsg);
            void BeginRows();
            void EndRows();
            void Transferring();

            //common methods for all database management systems
            bool SendRows(CScopeUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);

        private:
            void ExecuteSqlWithRowset(const char* sql, bool meta, bool lastInsertId, UINT64 index, int &res, std::wstring &errMsg, CDBVariant &vtId);
            void ExecuteSqlWithoutRowset(const char* sql, bool lastInsertId, int &res, std::wstring &errMsg, CDBVariant &vtId);
            void ResetMemories();

            //sqlite specific functions
            CDBColumnInfoArray GetColInfo(bool meta, sqlite3_stmt *stmt);
            void SetOtherColumnInfoFlags(CDBColumnInfoArray &vCol);
            int Bind(int row, std::string &errMsg);
            bool Process(UINT64 index, bool rowset, bool meta, int &res, std::string &errMsg);
            void Process(int &res, std::string &errMsg);
            int ResetStatements();
            void ConvertVariantDateToString();
            int DoSafeOpen(const std::wstring &strConnection, unsigned int flags);
            void Clean();
            bool SubscribeForEvents(sqlite3 *db, const std::wstring &strConnection);

            static std::vector<std::wstring> Split(const std::wstring &sql, const std::wstring &delimiter);
            static size_t ComputeParameters(const std::wstring &sql);

            static int DoStep(sqlite3_stmt *stmt);
            static int DoFinalize(sqlite3_stmt *stmt);
            static void SetDataType(const char *str, CDBColumnInfo &info);
            static void SetLen(const std::string& str, CDBColumnInfo &info);
            static void SetPrecisionScale(const std::string& str, CDBColumnInfo &info);
            static int sqlite3_sleep(int time);
            static void SetCacheTables(const std::wstring &str);
            static void ltrim(std::string &s);
            static void rtrim(std::string &s);
            static void trim(std::string &s);
            static void ltrim_w(std::wstring &s);
            static void rtrim_w(std::wstring &s);
            static void trim_w(std::wstring &s);
            static void SetTriggers();
            static std::vector<std::pair<std::string, char> > GetKeys(sqlite3 *db, const std::string &tblName);
            static int cbGetKeys(void *p, int argc, char **argv, char **azColName);
            static void SetTriggers(sqlite3 *db, const std::string &tblName, const std::vector<std::pair<std::string, char> > &vCol);
            static bool SetUpdateTrigger(sqlite3 *db, const std::string &tblName, const std::vector<std::pair<std::string, char> > &vCol);
            static bool SetInsertTrigger(sqlite3 *db, const std::string &tblName, const std::vector<std::pair<std::string, char> > &vCol);
            static bool SetDeleteTrigger(sqlite3 *db, const std::string &tblName, const std::vector<std::pair<std::string, char> > &vCol);
            static const std::vector<std::string>* InCache(const std::string &dbFile);
            static int cbGetAllTables(void *p, int argc, char **argv, char **azColName);
            static void DropAllTriggers(sqlite3 *db, const std::vector<std::string> &vTable);
            static void DropATrigger(sqlite3 *db, const std::string &sql);
            static size_t HasKey(const std::vector<std::pair<std::string, char> > &vCol);
            static void XFunc(sqlite3_context *context, int count, sqlite3_value **pp);


        protected:
            bool m_EnableMessages;
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

        private:
            std::vector<SAFEARRAY *> m_vArray;
            bool m_global;
            size_t m_parameters;
            CUQueue m_Blob;

            static CUCriticalSection m_csPeer;
            static const wchar_t* NO_DB_OPENED_YET;
            static const wchar_t* BAD_END_TRANSTACTION_PLAN;
            static const wchar_t* NO_PARAMETER_SPECIFIED;
            static const wchar_t* BAD_PARAMETER_COLUMN_SIZE;
            static const wchar_t* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const wchar_t* DATA_TYPE_NOT_SUPPORTED;
            static const wchar_t* NO_DB_FILE_SPECIFIED;

            //sqlite handles
            std::shared_ptr<sqlite3> m_pSqlite;
            std::vector<std::shared_ptr<sqlite3_stmt> > m_vPreparedStatements;

            static unsigned int m_nParam;
            static std::wstring m_strGlobalConnection; //protected by m_csPeer
            static const int SLEEP_TIME = 1; //ms
            static std::unordered_map<std::string, std::vector<std::string>> m_mapCache;

            static std::string DIU_TRIGGER_PREFIX;
            static std::string DIU_TRIGGER_FUNC;
        };

        typedef CSocketProService<CSqliteImpl> CSqliteService;

    } //namespace ServerSide
} //namespace SPA
