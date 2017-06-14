#pragma once

#include <memory>
#include "../../../include/aserverw.h"
#include "../../../include/udatabase.h"
#include "../../../include/mysql/umysql.h"

#include "include/my_config.h"
#include "include/my_global.h"
#include "include/mysql_time.h"
#include "include/mysql/plugin.h"


namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class CMysqlImpl : public CClientPeer {
            //no copy constructor
            CMysqlImpl(const CMysqlImpl &impl);
            //no assignment operator
            CMysqlImpl& operator=(const CMysqlImpl &impl);

        public:
            CMysqlImpl();
            unsigned int GetParameters() const;
            bool IsStoredProcedure() const;
            const std::string& GetProcedureName() const;
			static unsigned int GetMySqlServerVersion();

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
            bool SendRows(CUQueue& sb, bool transferring = false);
            bool SendBlob(unsigned short data_type, const unsigned char *buffer, unsigned int bytes);

        private:
            void ExecuteSqlWithoutRowset(int &res, std::wstring &errMsg, INT64 &affected);
            void ExecuteSqlWithRowset(bool meta, UINT64 index, int &res, std::wstring &errMsg, INT64 &affected);
            int Bind(CUQueue &qBufferSize, int row, std::wstring &errMsg);
            void PreprocessPreparedStatement();
            void CleanDBObjects();
            void ResetMemories();
            static UINT64 ConvertBitsToInt(const unsigned char *s, unsigned int bytes);
            static void ConvertToUTF8OrDouble(CDBVariant &vt);
            static void CALLBACK OnThreadEventEmbedded(SPA::ServerSide::tagThreadEvent te);
            static void CALLBACK OnThreadEvent(SPA::ServerSide::tagThreadEvent te);
            static UINT64 ToUDateTime(const MYSQL_TIME &td);
            static void Trim(std::string &s);
            static void srv_session_error_cb(void *ctx, unsigned int sql_errno, const char *err_msg);
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
            static void ToDecimal(const decimal_t &src, bool large, DECIMAL &dec);

        protected:
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

        private:
            CScopeUQueue m_sqSend;
            SPA::CScopeUQueue m_sb;
            std::vector<SAFEARRAY *> m_vArray;
            CUQueue &m_Blob;
            SPA::CUQueue &m_qSend;

            //MySql connection handle
            std::shared_ptr<Srv_session> m_pMysql;

            //parameterized statement
            //std::shared_ptr<MYSQL_STMT> m_pPrepare;
            size_t m_parameters;
            bool m_bCall;
            std::string m_sqlPrepare;
            std::string m_procName;

            CDBColumnInfoArray m_vColInfo;
			bool m_NoSending;

            int m_sql_errno;
            std::wstring m_err_msg;
            MYSQL_SECURITY_CONTEXT m_sc;

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

            static st_command_service_cbs m_sql_cbs;

            static const int IS_BINARY = 63;
            static const int MYSQL_TINYBLOB = 0xff;
            static const int MYSQL_BLOB = 0xffff;
            static const int MYSQL_MIDBLOB = 0xffffff;
            static my_bool B_IS_NULL;

            static const wchar_t* NO_DB_OPENED_YET;
            static const wchar_t* BAD_END_TRANSTACTION_PLAN;
            static const wchar_t* NO_PARAMETER_SPECIFIED;
            static const wchar_t* BAD_PARAMETER_COLUMN_SIZE;
            static const wchar_t* BAD_PARAMETER_DATA_ARRAY_SIZE;
            static const wchar_t* DATA_TYPE_NOT_SUPPORTED;
            static const wchar_t* NO_DB_NAME_SPECIFIED;
            static const wchar_t* MYSQL_LIBRARY_NOT_INITIALIZED;
            static const wchar_t* BAD_MANUAL_TRANSACTION_STATE;
            static const wchar_t* UNABLE_TO_SWITCH_TO_DATABASE;
        };

        typedef CSocketProService<CMysqlImpl> CMysqlService;

    } //namespace ServerSide
} //namespace SPA
