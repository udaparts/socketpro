#pragma once

#include <memory>
#include <unordered_map>
#include "../../../include/aserverw.h"
#include "../../../include/udatabase.h"
#include "../../../include/mysql/umysql.h"

#ifdef WIN32_64
typedef int socklen_t;
#endif

#include "my_config.h"
#ifdef HAVE_PSI_SOCKET_INTERFACE
#undef HAVE_PSI_SOCKET_INTERFACE
#endif
#include "mysql/plugin.h"

namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class U_MODULE_HIDDEN CMysqlImpl : public CClientPeer {
            //no copy constructor
            CMysqlImpl(const CMysqlImpl &impl);
            //no assignment operator
            CMysqlImpl& operator=(const CMysqlImpl &impl);

            struct Stmt {

                Stmt() : stmt_id(0), parameters(0) {
                }
                unsigned long stmt_id;
                size_t parameters;
                std::shared_ptr<PS_PARAM> m_pParam;
            };

            struct PriKey {
                CDBString ColumnName;
                bool Pri;
            };
            typedef std::vector<PriKey> CPriKeyArray;

        public:
            CMysqlImpl();
            ~CMysqlImpl();
            unsigned int GetParameters() const;
            static bool Authenticate(const std::wstring &userName, const wchar_t *password, const std::string &ip, unsigned int svsId = SPA::Mysql::sidMysql);
            static void CALLBACK OnThreadEvent(SPA::ServerSide::tagThreadEvent te);
            static void CreateTriggers(CMysqlImpl &impl, const std::vector<std::string> &vecTables);
            static void SetPublishDBEvent(CMysqlImpl &impl);

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        protected:
            virtual void Open(const CDBString &strConnection, unsigned int flags, int &res, CDBString &errMsg, int &ms);
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
            bool SendRows(CUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);

        private:
            void CleanDBObjects();
            void ResetMemories();
            void InitMysqlSession();
            void CloseStmt();
            int SetParams(int row, CDBString & errMsg);
            bool OpenSession(const CDBString &userName, const std::string &ip);
            void RemoveUnusedTriggers(const std::vector<std::string> &vecTables);
            void CreateTriggers(const std::string &schema, const std::string &table);
            void SetVParam(CDBVariantArray& vAll, size_t parameters, size_t pos, size_t ps);
            static std::vector<CDBString> Split(const CDBString &sql, const CDBString &delimiter);
            static size_t ComputeParameters(const CDBString &sql);
            static CDBString GetCreateTriggerSQL(const UTF16 *db, const UTF16 *table, const CPriKeyArray &vPriKey, SPA::UDB::tagUpdateEvent eventType);
            static std::string ToString(const CDBVariant &vtUTF8);
            static UINT64 ConvertBitsToInt(const unsigned char *s, unsigned int bytes);
            static UINT64 ToUDateTime(const MYSQL_TIME &td);
            static int sql_start_result_metadata(void *ctx, uint num_cols, uint flags, const CHARSET_INFO *resultcs);
            static int sql_field_metadata(void *ctx, struct st_send_field *field, const CHARSET_INFO *charset);
            static int sql_end_result_metadata(void *ctx, uint server_status, uint warn_count);
            static int sql_start_row(void *ctx);
            static int sql_end_row(void *ctx);
            static void sql_abort_row(void *ctx);
            static ulong sql_get_client_capabilities(void *ctx);
            static int sql_get_null(void *ctx);
            static int sql_get_integer(void * ctx, longlong value);
            static int sql_get_longlong(void * ctx, longlong value, uint is_unsigned);
            static int sql_get_decimal(void * ctx, const decimal_t * value);
            static int sql_get_double(void * ctx, double value, uint32 decimals);
            static int sql_get_date(void * ctx, const MYSQL_TIME * value);
            static int sql_get_time(void * ctx, const MYSQL_TIME * value, uint decimals);
            static int sql_get_datetime(void * ctx, const MYSQL_TIME * value, uint decimals);
            static int sql_get_string(void * ctx, const char * const value, size_t length, const CHARSET_INFO * const valuecs);
            static void sql_handle_ok(void * ctx, uint server_status, uint statement_warn_count, ulonglong affected_rows, ulonglong last_insert_id, const char * const message);
            static void sql_handle_error(void * ctx, uint sql_errno, const char * const err_msg, const char * const sqlstate);
            static void sql_shutdown(void *ctx, int shutdown_server);
            static void ToDecimal(unsigned char precision, const decimal_t &src, DECIMAL &dec);
            static bool DoAuthentication(const wchar_t *password, const std::string &hash);
            static int decimal2string(const decimal_t *from, char *to, int *to_len, int fixed_precision, int fixed_decimals, char filler);
        private:
            bool m_EnableMessages;
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            bool m_bManual;
            CDBVariantArray m_vParam;
            SPA::CScopeUQueue m_sb;
            CUQueue &m_qSend;

            //MySql connection handle
            std::shared_ptr<Srv_session> m_pMysql;

            //parameterized statement
            Stmt m_stmt;

            CDBColumnInfoArray m_vColInfo;
            bool m_NoSending;

            int m_sql_errno;
            CDBString m_err_msg;

            const CHARSET_INFO *m_sql_resultcs;
            unsigned int m_ColIndex;
            unsigned int m_sql_flags;
            ulonglong m_affected_rows;
            ulonglong m_last_insert_id;
            uint m_server_status;
            uint m_statement_warn_count;
            std::string m_sqlstate;
            UINT64 m_indexCall;
            bool m_bBlob;
            enum_server_command m_cmd;
            bool m_NoRowset;
            bool m_meta;
            static st_command_service_cbs m_sql_cbs;

            static const int IS_BINARY = 63;
            static const int MYSQL_TINYBLOB = 0xff;
            static const int MYSQL_BLOB = 0xffff;
            static const int MYSQL_MIDBLOB = 0xffffff;

            static const UTF16* NO_DB_OPENED_YET;
            static const UTF16* BAD_END_TRANSTACTION_PLAN;
            static const UTF16* NO_PARAMETER_SPECIFIED;
            static const UTF16* BAD_PARAMETER_COLUMN_SIZE;
            static const UTF16* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const UTF16* DATA_TYPE_NOT_SUPPORTED;
            static const UTF16* NO_DB_NAME_SPECIFIED;
            static const UTF16* MYSQL_LIBRARY_NOT_INITIALIZED;
            static const UTF16* BAD_MANUAL_TRANSACTION_STATE;
            static const UTF16* UNABLE_TO_SWITCH_TO_DATABASE;
            static const UTF16* SERVICE_COMMAND_ERROR;
        };

        typedef CSocketProService<CMysqlImpl> CMysqlService;

    } //namespace ServerSide
} //namespace SPA
