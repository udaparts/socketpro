
#pragma once

#ifndef SQL_NOUNICODEMAP
#define SQL_NOUNICODEMAP
#endif

#include "../uodbc_server.h"
#include "../../udatabase.h"
#include "../../aserverw.h"
#include<unordered_map>

namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class COdbcImpl : public CClientPeer {
            //no copy constructor
            COdbcImpl(const COdbcImpl &impl);
            //no assignment operator
            COdbcImpl& operator=(const COdbcImpl &impl);

            struct ODBC_CONNECTION_STRING {

                ODBC_CONNECTION_STRING() : timeout(0), port(0), async(false) {
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

                void Parse(const wchar_t *s);
                static void Trim(std::wstring &s);
            };
        public:
            COdbcImpl();

        public:
            static bool SetODBCEnv(int param);
            static void FreeODBCEnv();
            static void SetGlobalConnectionString(const wchar_t *str);

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
            virtual void Prepare(const std::wstring& sql, CParameterInfoArray& params, int &res, std::wstring &errMsg, unsigned int &parameters);
            virtual void ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, std::wstring &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void DoSQLColumnPrivileges(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLColumns(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLForeignKeys(const std::wstring& pkCatalogName, const std::wstring& pkSchemaName, const std::wstring& pkTableName, const std::wstring& fkCatalogName, const std::wstring& fkSchemaName, const std::wstring& fkTableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLPrimaryKeys(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLProcedureColumns(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& procName, const std::wstring& columnName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLProcedures(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& procName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLSpecialColumns(SQLSMALLINT identifierType, const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, SQLSMALLINT scope, SQLSMALLINT nullable, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLStatistics(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, SQLUSMALLINT unique, SQLUSMALLINT reserved, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLTablePrivileges(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);
            virtual void DoSQLTables(const std::wstring& catalogName, const std::wstring& schemaName, const std::wstring& tableName, const std::wstring& tableType, UINT64 index, int &res, std::wstring &errMsg, UINT64 &fail_ok);

        private:
            void ReleaseArray();
            void StartBLOB(unsigned int lenExpected);
            void Chunk();
            void EndBLOB();
            void BeginRows();
            void EndRows();
            void Transferring();
            bool SendRows(CUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);
            bool SendBlob(SQLHSTMT hstmt, SQLUSMALLINT index, VARTYPE vt, CUQueue &qTemp, CUQueue &q, bool &blob);
            bool SendUText(SQLHSTMT hstmt, SQLUSMALLINT index, CUQueue &qTemp, CUQueue &q, bool &blob);

        private:
            void CleanDBObjects();
            CDBColumnInfoArray GetColInfo(SQLHSTMT hstmt, SQLSMALLINT columns, bool meta);
            bool PushRecords(SQLHSTMT hstmt, const CDBColumnInfoArray &vColInfo, int &res, std::wstring &errMsg);
            bool PushInfo(SQLHDBC hdbc);
            void PreprocessPreparedStatement();
            bool CheckInputParameterDataTypes();
            bool SetInputParamInfo();
            bool BindParameters(unsigned int r, SQLLEN *pLenInd);
            static void ConvertDecimalAString(CDBVariant &vt);
            static void SetUShortInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetUIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetStringInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetUInt64Info(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void SetIntInfo(SQLHDBC hdbc, SQLUSMALLINT infoType, std::unordered_map<SQLUSMALLINT, CComVariant> &mapInfo);
            static void GetErrMsg(SQLSMALLINT HandleType, SQLHANDLE Handle, std::wstring &errMsg);
            static unsigned short ToSystemTime(const TIMESTAMP_STRUCT &d, SYSTEMTIME &st);
            static void ToSystemTime(const TIME_STRUCT &d, SYSTEMTIME &st);
            static void ToSystemTime(const DATE_STRUCT &d, SYSTEMTIME &st);

        protected:
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

        private:
            CScopeUQueue m_sb;
            std::vector<SAFEARRAY *> m_vArray;
            bool m_global;
            CUQueue &m_Blob;

            //ODBC connection handle
            std::shared_ptr<void> m_pOdbc;

            //parameterized statement
            std::shared_ptr<void> m_pPrepare;

            //excuting statement
            std::shared_ptr<void> m_pExcuting;

            SQLSMALLINT m_parameters;
            bool m_bCall;
            std::wstring m_sqlPrepare;
            std::wstring m_procName;
            CParameterInfoArray m_vPInfo;
            bool m_bReturn;
            SQLSMALLINT m_outputs;

            static const wchar_t* NO_DB_OPENED_YET;
            static const wchar_t* BAD_END_TRANSTACTION_PLAN;
            static const wchar_t* NO_PARAMETER_SPECIFIED;
            static const wchar_t* BAD_PARAMETER_COLUMN_SIZE;
            static const wchar_t* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const wchar_t* DATA_TYPE_NOT_SUPPORTED;
            static const wchar_t* NO_DB_NAME_SPECIFIED;
            static const wchar_t* ODBC_ENVIRONMENT_NOT_INITIALIZED;
            static const wchar_t* BAD_MANUAL_TRANSACTION_STATE;
            static const wchar_t* BAD_INPUT_PARAMETER_DATA_TYPE;
            static const wchar_t* BAD_PARAMETER_DIRECTION_TYPE;

            static SQLHENV g_hEnv;

            static const wchar_t* ODBC_GLOBAL_CONNECTION_STRING;

            static CUCriticalSection m_csPeer;
            static std::wstring m_strGlobalConnection; //ODBC source, protected by m_csPeer
        };

        typedef CSocketProService<COdbcImpl> COdbcService;
    } //namespace ServerSide
} //namespace SPA