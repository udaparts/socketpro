#pragma once

#ifndef SQL_NOUNICODEMAP
#define SQL_NOUNICODEMAP
#endif

#include "../../udatabase.h"
#include "../../aserverw.h"
#include<unordered_map>
#include <memory>

#ifdef SP_DB2_PLUGIN
#include "../../db2/udb2_server.h"
#undef UNICODE
#include <sqlcli1.h>
#else
#include "../uodbc_server.h"
#include <sqlext.h>
#endif

namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class COdbcImpl : public CClientPeer {
            //no copy constructor
            COdbcImpl(const COdbcImpl &impl) = delete;
            //no assignment operator
            COdbcImpl& operator=(const COdbcImpl &impl) = delete;

            static const unsigned int DEFAULT_UNICODE_CHAR_SIZE = 4 * 1024;
            static const unsigned int DEFAULT_OUTPUT_BUFFER_SIZE = 8 * 1024; //bytes

            //8 mega bytes of binary or UTF8 string or 4 mega unicode string for inputout or output stored procedure
            static const unsigned int MAX_OUTPUT_BLOB_BUFFER_SIZE = 8 * 1024 * 1024;
            static const unsigned char MAX_DECIMAL_PRECISION = 29;
            static const unsigned int DECIMAL_STRING_BUFFER_SIZE = 32;
            static const unsigned int DATETIME_STRING_BUFFER_SIZE = 32;
            static const unsigned char MAX_TIME_DIGITS = 7;
            static const unsigned int MAX_ORACLE_VARCHAR2 = 32 * 1024;

            struct CBindInfo {
                VARTYPE DataType;
                unsigned int Offset;
                unsigned int BufferSize;
            };

            struct ExecuteContext {
                CDBString sql;
                bool rowset = false;
                bool meta = false;
                UINT64 index = INVALID_NUMBER;
            };

        public:

            struct ODBC_CONNECTION_STRING {

                ODBC_CONNECTION_STRING() : timeout(0), port(0), async(false), QueryBatching(false) {
                }
                std::wstring database; //database -- SQL_ATTR_CURRENT_CATALOG
                std::wstring dsn; //dsn
                std::wstring host; //host | server
                std::wstring user; //user | uid
                std::wstring password; //pwd | password
                std::wstring filedsn; //FILEDSN
                std::wstring driver; //DRIVER
                std::wstring savefile; //SAVEFILE
                std::wstring remaining;
                std::wstring connection_string;
                unsigned int timeout; //timeout | connect-timeout in seconds -- SQL_ATTR_CONNECTION_TIMEOUT
                unsigned int port; //????
                bool async; //async | asynchronous -- SQL_ATTR_ASYNC_ENABLE
                bool QueryBatching;
                void Parse(const wchar_t* s);
            };

            COdbcImpl();

        public:
            static bool SetODBCEnv(int param);
            static void FreeODBCEnv();
            static void SetGlobalConnectionString(const wchar_t *str);
            static std::atomic<unsigned int> m_mb;
            static void GetErrMsg(SQLSMALLINT HandleType, SQLHANDLE Handle, CDBString& errMsg);

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        private:
            virtual void Open(const CDBString &strConnection, unsigned int flags, int &res, CDBString &errMsg, int &ms);
            virtual void CloseDb(int &res, CDBString &errMsg);
            virtual void BeginTrans(int isolation, const CDBString &dbConn, unsigned int flags, int &res, CDBString &errMsg, int &ms);
            virtual void EndTrans(int plan, int &res, CDBString &errMsg);
            virtual void Execute(CDBString& sql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void Prepare(const CDBString& sql, CParameterInfoArray& params, int &res, CDBString &errMsg, unsigned int &parameters);
            virtual void ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void ExecuteBatch(const CDBString& sql, const CDBString& delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, const CDBString &dbConn, unsigned int flags, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void DoSQLColumnPrivileges(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLColumns(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLForeignKeys(const CDBString& pkCatalogName, const CDBString& pkSchemaName, const CDBString& pkTableName, const CDBString& fkCatalogName, const CDBString& fkSchemaName, const CDBString& fkTableName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLPrimaryKeys(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLProcedureColumns(const CDBString& catalogName, const CDBString& schemaName, const CDBString& procName, const CDBString& columnName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLProcedures(const CDBString& catalogName, const CDBString& schemaName, const CDBString& procName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLSpecialColumns(SQLSMALLINT identifierType, const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, SQLSMALLINT scope, SQLSMALLINT nullable, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLStatistics(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, SQLUSMALLINT unique, SQLUSMALLINT reserved, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLTablePrivileges(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);
            virtual void DoSQLTables(const CDBString& catalogName, const CDBString& schemaName, const CDBString& tableName, const CDBString& tableType, UINT64 index, int &res, CDBString &errMsg, UINT64 &fail_ok);

        private:
            void StartBLOB(unsigned int lenExpected);
            void Chunk();
            void EndBLOB();
            void BeginRows();
            void EndRows();
            void Transferring();
            bool SendRows(CUQueue& sb, bool transferring = false);
            bool SendBlob(SQLHSTMT hstmt, SQLUSMALLINT index, VARTYPE vt, CUQueue &qTemp, CUQueue &q, bool &blob);
            bool SendUText(SQLHSTMT hstmt, SQLUSMALLINT index, CUQueue &qTemp, CUQueue &q, bool &blob);

        private:
            SQLHSTMT ResetStmt();
            void CleanDBObjects();
            CDBColumnInfoArray GetColInfo(SQLHSTMT hstmt, SQLSMALLINT columns, bool meta);
            bool PushRecords(SQLHSTMT hstmt, int &res, CDBString &errMsg, bool output = false);
            bool PushRecords(SQLHSTMT hstmt, const CDBColumnInfoArray& vColInfo, bool output, int& res, CDBString& errMsg);
            bool PushInfo(SQLHDBC hdbc);
            bool PreprocessPreparedStatement();
            bool SetInputParamInfo();
            bool BindParameters(SQLHSTMT hstmt, unsigned int r, SQLLEN *pLenInd);
            unsigned int ComputeOutputMaxSize();
            bool PushOutputParameters(unsigned int r, UINT64 index);
            void ResetMemories();
            void SetVParam(CDBVariantArray& vAll, size_t parameters, size_t pos, size_t ps);
            void SetCallParams(const std::vector<tagParameterDirection> &vPD, int &res, CDBString &errMsg);
#ifndef SP_DB2_PLUGIN
            void SetOracleCallParams(const std::vector<tagParameterDirection> &vPD, int &res, CDBString &errMsg);
#endif
            CDBString GenerateMsSqlForCachedTables();
            ExecuteContext PopExecContext();
            bool IsSeparator(const CDBColumnInfoArray& vCol, bool meta) const;
            CDBString MakeSQL(ExecuteContext& ec);

            static CParameterInfoArray GetVInfo(const CParameterInfoArray& vPInfo, size_t pos, size_t ps);
            static std::vector<CDBString> Split(const CDBString &sql, const CDBString &delimiter);
            static size_t ComputeParameters(const CDBString &sql);
            static void SaveSqlServerVariant(const unsigned char *buffer, unsigned int bytes, SQLSMALLINT c_type, CUQueue &q);
            static void ConvertDecimalAString(CDBVariant &vt);
            static void SetUShortInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetUIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetStringInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetUInt64Info(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static unsigned int ToCTime(const TIMESTAMP_STRUCT &d, std::tm &tm);
            static unsigned int ToCTime(const TIME_STRUCT &d, std::tm &tm);
            static unsigned int ToCTime(const DATE_STRUCT &d, std::tm &tm);
            static void ToDecimal(const SQL_NUMERIC_STRUCT &num, DECIMAL &dec);
            static std::vector<tagParameterDirection> GetCallDirections(const CDBString &sql);

        protected:
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

        private:
            CScopeUQueue m_sbRecord;
            CDBString m_dbName;
            CDBString m_dbms;
            CDBString m_userName;
            CScopeUQueue m_sb;
            bool m_global;
            CUQueue &m_Blob;
            CUQueue &m_BlobRecord;

            //ODBC connection handle
            std::shared_ptr<void> m_pOdbc;

            //executing statement
            std::shared_ptr<void> m_stmt;

            SQLSMALLINT m_parameters;
            bool m_bCall;
            CDBString m_sqlPrepare;
            CDBString m_procName;
            CDBString m_procCatalogSchema;
            CParameterInfoArray m_vPInfo;
            bool m_bReturn;
            SQLSMALLINT m_outputs;

            std::vector<CBindInfo> m_vBindInfo;
            unsigned int m_nRecordSize;
            CUQueue *m_pNoSending;

            tagManagementSystem m_msDriver;
            bool m_EnableMessages;
            SQLUSMALLINT m_bPrimaryKeys;
            SQLUSMALLINT m_bProcedureColumns;
            std::vector<tagParameterDirection> m_vPD;

            unsigned int m_maxQueriesBatched;
            bool m_bQueryBatching;
            typedef std::deque<ExecuteContext> CEexcContextArray;
            CEexcContextArray m_vEexcContext;

            static const UTF16* NO_DB_OPENED_YET;
            static const UTF16* BAD_END_TRANSTACTION_PLAN;
            static const UTF16* NO_PARAMETER_SPECIFIED;
            static const UTF16* BAD_PARAMETER_COLUMN_SIZE;
            static const UTF16* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const UTF16* DATA_TYPE_NOT_SUPPORTED;
            static const UTF16* NO_DB_NAME_SPECIFIED;
            static const UTF16* ODBC_ENVIRONMENT_NOT_INITIALIZED;
            static const UTF16* BAD_MANUAL_TRANSACTION_STATE;
            static const UTF16* BAD_INPUT_PARAMETER_DATA_TYPE;
            static const UTF16* BAD_PARAMETER_DIRECTION_TYPE;
            static const UTF16* CORRECT_PARAMETER_INFO_NOT_PROVIDED_YET;
            static const UTF16* ODBC_GLOBAL_CONNECTION_STRING;

        public:
            static SQLHENV g_hEnv;
            static CUCriticalSection m_csPeer;
            static CDBString m_strGlobalConnection; //ODBC source, protected by m_csPeer

            struct CMyStruct {
                SQLHDBC hdbc;
                bool QueryBatching;
            };
            static std::unordered_map<USocket_Server_Handle, CMyStruct> m_mapConnection; //protected by m_csPeer
        };

        typedef CSocketProService<COdbcImpl> COdbcService;
    } //namespace ServerSide
} //namespace SPA
