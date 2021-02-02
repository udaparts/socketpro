#pragma once

#include <memory>
#include "mysql_funcs.h"
#ifdef MM_DB_SERVER_PLUGIN
#include "../umysql.h"
#else
#include "../umysql_server.h"
#endif
#include "../../udatabase.h"
#include "../../aserverw.h"
#include <unordered_map>

namespace SPA {
    namespace ServerSide {
        using namespace UDB;
        using namespace Mysql;

        class U_MODULE_HIDDEN CMysqlImpl : public CClientPeer {
            //no copy constructor
            CMysqlImpl(const CMysqlImpl &impl);
            //no assignment operator
            CMysqlImpl& operator=(const CMysqlImpl &impl);

            struct MYSQL_BIND_RESULT_FIELD {
            private:
                static const unsigned int DEFAULT_BUFFER_SIZE = 1024;

            private:
                static_assert(sizeof (MYSQL_TIME) <= DEFAULT_BUFFER_SIZE, "Bad default buffer size");
                typedef CScopeUQueueEx<DEFAULT_BUFFER_SIZE, DEFAULT_MEMORY_BUFFER_BLOCK_SIZE> CScopeUQueue;
                CScopeUQueue m_sb;
                MYSQL_BIND_RESULT_FIELD(const MYSQL_BIND_RESULT_FIELD &field);
                MYSQL_BIND_RESULT_FIELD& operator=(const MYSQL_BIND_RESULT_FIELD &field);

            public:

                MYSQL_BIND_RESULT_FIELD() : is_null(0), buffer_length(m_sb->GetMaxSize()), length(0) {
                }

                my_bool is_null;
                unsigned long buffer_length;
                unsigned long length;

                inline unsigned char* GetBuffer() {
                    return (unsigned char*) m_sb->GetBuffer();
                }

                void ResetMaxBuffer(unsigned int newSize) {
                    m_sb->ReallocBuffer(newSize);
                    buffer_length = m_sb->GetMaxSize();
                }

                static void ShrinkMemoryPool() {
                    CScopeUQueue::ResetSize();
                }
            };

            struct MYSQL_CONNECTION_STRING {

                MYSQL_CONNECTION_STRING() : timeout(10), port(3306) {
                }
                unsigned int timeout; //timeout | connect-timeout seconds
                std::string database; //database
                std::string host; //host | server
                std::string password; //pwd | password
                unsigned int port; //default to 3306
                std::string ssl_ca; //file_name
                std::string ssl_capath; //dir_name
                std::string ssl_cert; //file_name
                std::string ssl_cipher; //cipher_list
                std::string ssl_key; //file_name
                std::string user; //user | uid
                std::string socket;

                bool IsSSL() const {
                    return (ssl_ca.size() || ssl_capath.size() || ssl_cert.size() || ssl_cipher.size() || ssl_key.size());
                }
                void Parse(const char *s);

            private:
                void Init();
                MYSQL_CONNECTION_STRING(const MYSQL_CONNECTION_STRING &conn);
                MYSQL_CONNECTION_STRING& operator=(const MYSQL_CONNECTION_STRING &conn);
            };

        public:
            CMysqlImpl();
            virtual void Open(const CDBString &strConnection, unsigned int flags, int &res, CDBString &errMsg, int &ms);
            static void CALLBACK OnThreadEvent(tagThreadEvent te);
            static void SetDBGlobalConnectionString(const wchar_t *dbConnection, bool remote);
            static void UnloadMysql();
            static bool InitMySql();
            static bool DoSQLAuthentication(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *dbConnection);
            std::shared_ptr<MYSQL> GetDBConnHandle();
            CDBString GetDefaultDBName();

#ifdef MM_DB_SERVER_PLUGIN
            static std::string ToString(const CDBVariant &vtUTF8);
            static bool SetPublishDBEvent(CMysqlImpl &impl);
            static bool CreateTriggers(CMysqlImpl &impl, const std::vector<std::string> &vecTables);

        private:

            struct PriKey {
                std::string ColumnName;
                bool Pri;
            };
            typedef std::vector<PriKey> CPriKeyArray;
            bool RemoveUnusedTriggers(const std::vector<std::string> &vecTables);
            bool CreateTriggers(const std::string &schema, const std::string &table);
            CDBString GetCreateTriggerSQL(const UTF16 *db, const UTF16 *table, const CPriKeyArray &vPriKey, SPA::UDB::tagUpdateEvent eventType);
#endif
        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        private:
            virtual void CloseDb(int &res, CDBString &errMsg);
            virtual void BeginTrans(int isolation, const CDBString &dbConn, unsigned int flags, int &res, CDBString &errMsg, int &ms);
            virtual void EndTrans(int plan, int &res, CDBString &errMsg);
            virtual void Execute(const CDBString& sql, bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void Prepare(const CDBString& sql, CParameterInfoArray& params, int &res, CDBString &errMsg, unsigned int &parameters);
            virtual void ExecuteParameters(bool rowset, bool meta, bool lastInsertId, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);
            virtual void ExecuteBatch(const CDBString& sql, const CDBString& delimiter, int isolation, int plan, bool rowset, bool meta, bool lastInsertId, const CDBString &dbConn, unsigned int flags, UINT64 index, INT64 &affected, int &res, CDBString &errMsg, CDBVariant &vtId, UINT64 &fail_ok);

        private:
            void StartBLOB(unsigned int lenExpected);
            void Chunk();
            void EndBLOB();
            void BeginRows();
            void EndRows();
            void Transferring();
            bool SendRows(CScopeUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);

        private:
            void ExecuteSqlWithoutRowset(int &res, CDBString &errMsg, INT64 &affected);
            void ExecuteSqlWithRowset(bool rowset, bool meta, UINT64 index, int &res, CDBString &errMsg, INT64 &affected);
            CDBColumnInfoArray GetColInfo(MYSQL_RES *result, unsigned int cols, bool meta);
            bool PushRecords(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, CDBString &errMsg);
            int Bind(CUQueue &qBufferSize, int row, CDBString &errMsg);
            std::shared_ptr<MYSQL_BIND> PrepareBindResultBuffer(MYSQL_RES *result, const CDBColumnInfoArray &vColInfo, int &res, CDBString &errMsg, std::shared_ptr<MYSQL_BIND_RESULT_FIELD> &field);
            bool PushRecords(UINT64 index, MYSQL_BIND *binds, MYSQL_BIND_RESULT_FIELD *fields, const CDBColumnInfoArray &vColInfo, bool rowset, bool output, int &res, CDBString &errMsg);
            void PreprocessPreparedStatement();
            void CleanDBObjects();
            void ResetMemories();
            void SetVParam(CDBVariantArray& vAll, size_t parameters, size_t pos, size_t ps);

            //mysql specific functions
            static UINT64 ConvertBitsToInt(const unsigned char *s, unsigned int bytes);
            static void ConvertToUTF8OrDouble(CDBVariant &vt);
            static UINT64 ToUDateTime(const MYSQL_TIME &td);
            static std::vector<CDBString> Split(const CDBString &sql, const CDBString &delimiter);
            static size_t ComputeParameters(const CDBString &sql);

        private:
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

            SPA::CScopeUQueue m_sb;
            bool m_global;
            CUQueue &m_Blob;

            //MySql connection handle
            std::shared_ptr<MYSQL> m_pMysql;

            //parameterized statement
            std::shared_ptr<MYSQL_STMT> m_pPrepare;
            size_t m_parameters;
            bool m_bCall;
            std::string m_sqlPrepare;
            std::string m_procName;
            bool m_bManual;
            bool m_EnableMessages;
            CUQueue *m_pNoSending;
            CDBString m_dbNameOpened;

            static const int IS_BINARY = 63;
            static const int MYSQL_TINYBLOB = 0xff;
            static const int MYSQL_BLOB = 0xffff;
            static const int MYSQL_MIDBLOB = 0xffffff;
            static my_bool B_IS_NULL;

            static const UTF16* NO_DB_OPENED_YET;
            static const UTF16* BAD_END_TRANSTACTION_PLAN;
            static const UTF16* NO_PARAMETER_SPECIFIED;
            static const UTF16* BAD_PARAMETER_COLUMN_SIZE;
            static const UTF16* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const UTF16* DATA_TYPE_NOT_SUPPORTED;
            static const UTF16* NO_DB_NAME_SPECIFIED;
            static const UTF16* MYSQL_LIBRARY_NOT_INITIALIZED;
            static const UTF16* BAD_MANUAL_TRANSACTION_STATE;

            static const UTF16* MYSQL_GLOBAL_CONNECTION_STRING;
            static CMysqlLoader m_remMysql;

        public:
            struct MyStruct {
                std::shared_ptr<MYSQL> Handle;
                CDBString DefaultDB;
            };

            static CUCriticalSection m_csPeer;
            static bool m_bInitMysql; //protected by m_csPeer
            static CDBString m_strGlobalConnection; //remote mysql server, protected by m_csPeer
            typedef std::unordered_map<USocket_Server_Handle, MyStruct> CMyMap;
            static CMyMap m_mapConnection; //protected by m_csPeer
        };

        typedef CSocketProService<CMysqlImpl> CMysqlService;

    } //namespace ServerSide
} //namespace SPA
