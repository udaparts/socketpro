#pragma once

#include "../uodbc_server.h"
#include "../../udatabase.h"
#include "../../aserverw.h"

namespace SPA {
    namespace ServerSide {
        using namespace UDB;
        using namespace Odbc;

        class COdbcImpl : public CClientPeer {
            //no copy constructor
            COdbcImpl(const COdbcImpl &impl);
            //no assignment operator
            COdbcImpl& operator=(const COdbcImpl &impl);

            struct ODBC_CONNECTION_STRING {

                ODBC_CONNECTION_STRING() : timeout(0), port(0), async(false) {
                }
                std::wstring database; //database -- SQL_ATTR_CURRENT_CATALOG
                std::wstring host; //host | server | dsn
                std::wstring user; //user | uid
                std::wstring password; //pwd | password
                unsigned int timeout; //timeout | connect-timeout in seconds -- SQL_ATTR_CONNECTION_TIMEOUT
                unsigned int port; //????
                bool async; //async | asynchronous -- SQL_ATTR_ASYNC_ENABLE

                void Parse(const wchar_t *s);
                static void Trim(std::wstring &s);
            };
        public:
            COdbcImpl();

        public:
            static bool SetODBCEnv();
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

        private:
            void ReleaseArray();
            void StartBLOB(unsigned int lenExpected);
            void Chunk();
            void EndBLOB();
            void BeginRows();
            void EndRows();
            void Transferring();
            bool SendRows(CScopeUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);

        private:
            void CleanDBObjects();

            static void ConvertToUTF8OrDouble(CDBVariant &vt);
            static void GetErrMsg(SQLSMALLINT HandleType, SQLHANDLE Handle, std::wstring &errMsg);


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

            SQLSMALLINT m_parameters;

            static const wchar_t* NO_DB_OPENED_YET;
            static const wchar_t* BAD_END_TRANSTACTION_PLAN;
            static const wchar_t* NO_PARAMETER_SPECIFIED;
            static const wchar_t* BAD_PARAMETER_COLUMN_SIZE;
            static const wchar_t* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const wchar_t* DATA_TYPE_NOT_SUPPORTED;
            static const wchar_t* NO_DB_NAME_SPECIFIED;
            static const wchar_t* ODBC_ENVIRONMENT_NOT_INITIALIZED;
            static const wchar_t* BAD_MANUAL_TRANSACTION_STATE;

            static SQLHENV g_hEnv;

            static const wchar_t* ODBC_GLOBAL_CONNECTION_STRING;

            static CUCriticalSection m_csPeer;
            static std::wstring m_strGlobalConnection; //ODBC source, protected by m_csPeer
        };

        typedef CSocketProService<COdbcImpl> COdbcService;
    } //namespace ServerSide
} //namespace SPA