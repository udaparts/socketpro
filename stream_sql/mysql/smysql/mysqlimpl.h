#pragma once

#include <memory>
#include "../../../include/aserverw.h"
#include "../../../include/udatabase.h"
#include "../../../include/mysql/umysql.h"

#include "include/my_config.h"
#include "include/my_global.h"
//#include "include/mysql.h"
#include "include/mysql_time.h"
#include "include/mysql/plugin_auth.h"


namespace SPA {
    namespace ServerSide {
        using namespace UDB;

        class CMysqlImpl : public CClientPeer {
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

        public:
            CMysqlImpl();
            unsigned int GetParameters() const;
            bool IsGloballyConnected() const;
            bool IsStoredProcedure() const;
            const std::string& GetProcedureName() const;

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
            static const wchar_t *fieldtype2str(enum_field_types type);

        protected:
            UINT64 m_oks;
            UINT64 m_fails;
            tagTransactionIsolation m_ti;
            CDBVariantArray m_vParam;

        private:
            SPA::CScopeUQueue m_sb;
            std::vector<SAFEARRAY *> m_vArray;
            bool m_global;
            CUQueue &m_Blob;

            //MySql connection handle
            std::shared_ptr<Srv_session> m_pMysql;

            //parameterized statement
            //std::shared_ptr<MYSQL_STMT> m_pPrepare;
            size_t m_parameters;
            bool m_bCall;
            std::string m_sqlPrepare;
            std::string m_procName;

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
        };

        typedef CSocketProService<CMysqlImpl> CMysqlService;

    } //namespace ServerSide
} //namespace SPA
