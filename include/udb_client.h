
#ifndef _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_
#define _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_

#include "udatabase.h"
#include "aclientw.h"

#ifdef PHP_ADAPTER_PROJECT
#define NO_OUTPUT_BINDING
#elif defined NODE_JS_ADAPTER_PROJECT
#define NO_OUTPUT_BINDING
namespace NJA {
    Local<Array> ToMeta(Isolate* isolate, const SPA::UDB::CDBColumnInfoArray &v);
}
#endif

namespace SPA {
    namespace ClientSide {
        using namespace UDB;

        template<unsigned int serviceId>
        class CAsyncDBHandler : public CAsyncServiceHandler {
            static const unsigned int ONE_MEGA_BYTES = 0x100000;
            static const unsigned int BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0;

        protected:
            //you may use this constructor for extending the class

            CAsyncDBHandler(unsigned int sid, CClientSocket *cs)
            : CAsyncServiceHandler(sid, cs),
            m_affected(-1), m_dbErrCode(0), m_lastReqId(0),
            m_indexRowset(0), m_indexProc(0), m_ms(msUnknown), m_flags(0),
            m_parameters(0), m_outputs(0), m_bCallReturn(false), m_queueOk(false), m_nParamPos(0) {
#ifdef NODE_JS_ADAPTER_PROJECT
                ::memset(&m_typeDB, 0, sizeof (m_typeDB));
                m_typeDB.data = this;
                int fail = uv_async_init(uv_default_loop(), &m_typeDB, req_cb);
                assert(!fail);
#endif

#ifdef NO_OUTPUT_BINDING
                m_bProc = false;
#else
                m_Blob.Utf8ToW(true);
#endif
            }

        public:

            CAsyncDBHandler(CClientSocket *cs)
            : CAsyncServiceHandler(serviceId, cs),
            m_affected(-1), m_dbErrCode(0), m_lastReqId(0),
            m_indexRowset(0), m_indexProc(0), m_ms(msUnknown), m_flags(0),
            m_parameters(0), m_outputs(0), m_bCallReturn(false), m_queueOk(false), m_nParamPos(0) {
#ifdef NODE_JS_ADAPTER_PROJECT
                ::memset(&m_typeDB, 0, sizeof (m_typeDB));
                m_typeDB.data = this;
                int fail = uv_async_init(uv_default_loop(), &m_typeDB, req_cb);
                assert(!fail);
#endif

#ifdef NO_OUTPUT_BINDING
                m_bProc = false;
#else
                m_Blob.Utf8ToW(true);
#endif
            }

            virtual ~CAsyncDBHandler() {
                CleanCallbacks();
#ifdef NODE_JS_ADAPTER_PROJECT
                uv_close((uv_handle_t*) & m_typeDB, nullptr);
#endif
            }

            typedef CAsyncDBHandler* PAsyncDBHandler;

            static const unsigned int SQLStreamServiceId = serviceId;

            typedef std::function<void(CAsyncDBHandler &dbHandler, int res, const std::wstring &errMsg) > DResult;
            typedef std::function<void(CAsyncDBHandler &dbHandler, int res, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, CDBVariant &vtId) > DExecuteResult;

#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
            typedef std::function<void(CAsyncDBHandler &dbHandler, const unsigned char *start, unsigned int bytes) > DRowsetHeader;
            typedef std::function<void(CAsyncDBHandler &dbHandler, CUQueue &vData) > DRows;
#else
            typedef std::function<void(CAsyncDBHandler &dbHandler) > DRowsetHeader;
            typedef std::function<void(CAsyncDBHandler &dbHandler, CDBVariantArray &vData) > DRows;
#endif

        protected:
            typedef std::pair<DRowsetHeader, DRows> CRowsetHandler;

        public:

            inline int GetLastDBErrorCode() {
                CAutoLock al(m_csDB);
                return m_dbErrCode;
            }

            inline std::wstring GetLastDBErrorMessage() {
                CAutoLock al(m_csDB);
                return m_dbErrMsg;
            }

            inline std::wstring GetConnection() {
                CAutoLock al(m_csDB);
                return m_strConnection;
            }

            inline INT64 GetLastAffected() {
                CAutoLock al(m_csDB);
                return m_affected;
            }

            inline const CDBColumnInfoArray& GetColumnInfo() {
                CAutoLock al(m_csDB);
                return m_vColInfo;
            }

            inline bool IsOpened() {
                CAutoLock al(m_csDB);
                return (m_strConnection.size() > 0 && m_lastReqId > 0);
            }

            inline tagManagementSystem GetDBManagementSystem() {
                CAutoLock al(m_csDB);
                return m_ms;
            }

            inline unsigned int GetParameters() {
                CAutoLock al(m_csDB);
                return m_parameters;
            }

            inline unsigned int GetOutputs() {
                CAutoLock al(m_csDB);
                return m_outputs;
            }

            inline bool GetCallReturn() {
                CAutoLock al(m_csDB);
                return m_bCallReturn;
            }

#ifdef NO_OUTPUT_BINDING

            inline const CDBVariant& GetRetValue() {
                CAutoLock al(m_csDB);
                assert(m_bCallReturn);
                return m_vtRet;
            }

            inline bool IsProc() {
                CAutoLock al(m_csDB);
                return m_bProc;
            }
#endif

            /**
             * Check if the object will automatically convert utf8 string into Unicode string when loading a ASCII string by VARIANT.
             * @return true if the object will do automatic converting, and false if the object will not
             */
            inline bool Utf8ToW() {
                CAutoLock al(m_csDB);
                return m_Blob.Utf8ToW();
            }

            /**
             * Enable or disable the object to automatically convert utf8 string into Unicode string when loading a ASCII string by VARIANT.
             * @param bUtf8ToW true for enabling, and false for disabling
             */
            inline void Utf8ToW(bool bUtf8ToW) {
                CAutoLock al(m_csDB);
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                m_vData.Utf8ToW(bUtf8ToW);
#endif
                m_Blob.Utf8ToW(bUtf8ToW);
            }

            virtual unsigned int CleanCallbacks() {
                {
                    CAutoLock al(m_csDB);
                    Clean();
                }
                return CAsyncServiceHandler::CleanCallbacks();
            }

        private:

            bool Send(CScopeUQueue& sb, bool &firstRow) {
                if (sb->GetSize()) {
                    if (firstRow) {
                        firstRow = false;
                        if (!SendRequest(idBeginRows, sb->GetBuffer(), sb->GetSize(), NULL_RH)) {
                            return false;
                        }
                    } else {
                        if (!SendRequest(idTransferring, sb->GetBuffer(), sb->GetSize(), NULL_RH)) {
                            return false;
                        }
                    }
                    sb->SetSize(0);
                } else if (firstRow) {
                    firstRow = false;
                    if (!SendRequest(idBeginRows, NULL_RH)) {
                        return false;
                    }
                }
                return true;
            }

            bool SendBlob(CScopeUQueue& sb) {
                if (sb->GetSize()) {
                    bool start = true;
                    while (sb->GetSize()) {
                        unsigned int send = (sb->GetSize() >= DEFAULT_BIG_FIELD_CHUNK_SIZE) ? DEFAULT_BIG_FIELD_CHUNK_SIZE : sb->GetSize();
                        if (start) {
                            if (!SendRequest(idStartBLOB, sb->GetBuffer(), send, NULL_RH)) {
                                return false;
                            }
                            start = false;
                        } else {
                            if (!SendRequest(idChunk, sb->GetBuffer(), send, NULL_RH)) {
                                return false;
                            }
                        }
                        sb->Pop(send);
                        if (sb->GetSize() < DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                            break;
                        }
                    }

                    if (!SendRequest(idEndBLOB, sb->GetBuffer(), sb->GetSize(), NULL_RH)) {
                        return false;
                    }
                    sb->SetSize(0);
                }
                return true;
            }

            bool SendParametersData(const CDBVariantArray &vParam) {
                size_t size = vParam.size();
                if (size) {
                    bool firstRow = true;
                    CScopeUQueue sb;
                    for (size_t n = 0; n < size; ++n) {
                        const CDBVariant &vt = vParam[n];
                        unsigned short dt = vt.Type();
                        if (dt == VT_BSTR /*UNICODE string*/) {
                            unsigned int len = SysStringLen(vt.bstrVal);
                            if (len < DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                sb << vt;
                            } else {
                                if (!Send(sb, firstRow)) {
                                    return false;
                                }
                                assert(sb->GetSize() == 0);
                                len = 0;
                                sb << len << vt;
                                len = sb->GetSize() - sizeof (len);
                                sb->Replace(0, sizeof (len), (const unsigned char*) &len, sizeof (len));
                                if (!SendBlob(sb)) {
                                    return false;
                                }
                            }
                        } else if (VT_ARRAY == (dt & VT_ARRAY)) {
                            if (dt == (VT_ARRAY | VT_UI1) /*binary array*/ || dt == (VT_ARRAY | VT_I1)/*ASCII string*/) {
                                unsigned int len = (unsigned int) vt.parray->rgsabound->cElements;
                                if (len < 2 * DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                                    sb << vt;
                                } else {
                                    if (!Send(sb, firstRow)) {
                                        return false;
                                    }
                                    assert(sb->GetSize() == 0);
                                    len = 0;
                                    sb << len << vt;
                                    len = sb->GetSize() - sizeof (len);
                                    sb->Replace(0, sizeof (len), (const unsigned char*) &len, sizeof (len));
                                    if (!SendBlob(sb)) {
                                        return false;
                                    }
                                }
                            } else {
                                assert(false); //not supported!!!
                            }
                        } else {
                            sb << vt;
                        }
                        if (sb->GetSize() >= DEFAULT_RECORD_BATCH_SIZE) {
                            if (!Send(sb, firstRow)) {
                                return false;
                            }
                        }
                    }
                    if (!Send(sb, firstRow)) {
                        return false;
                    }
                }
                return true;
            }

        public:

            /**
             * Execute a batch of SQL statements on one single call
             * @param isolation a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified
             * @param sql a SQL statement having a batch of individual SQL statements
             * @param vParam an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations
             * @param delimiter a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing
             * @param batchHeader a callback for tracking batch beginning event
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param plan a value for computing how included transactions should be rollback
             * @param vPInfo a given array of parameter informations which may be empty to some of database management systems
             * @param lastInsertId a boolean for last insert record identification number
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool ExecuteBatch(tagTransactionIsolation isolation, const wchar_t *sql, CDBVariantArray &vParam = CDBVariantArray(),
                    const DExecuteResult& handler = nullptr, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, const wchar_t* delimiter = L";",
                    const DRowsetHeader& batchHeader = nullptr, const DDiscarded& discarded = nullptr, bool meta = true, tagRollbackPlan plan = rpDefault,
                    const CParameterInfoArray& vPInfo = CParameterInfoArray(), bool lastInsertId = true, const DServerException& se = nullptr) {
                bool rowset = (row) ? true : false;
                meta = (meta && rh);
                CScopeUQueue sb;
                sb << sql << delimiter << (int) isolation << (int) plan << rowset << meta << lastInsertId;

                UINT64 callIndex;
                bool queueOk = false;

                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                if (vParam.size())
                    queueOk = GetSocket()->GetClientQueue().StartJob();
                {
                    if (!SendParametersData(vParam)) {
                        Clean();
                        return false;
                    }
                    callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset[callIndex] = CRowsetHandler(rh, row);
                    }
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall[callIndex] = &vParam;
#endif
                    m_mapHandler[callIndex] = batchHeader;
                    sb << m_strConnection << m_flags;
                }
                sb << callIndex << vPInfo;
                DResultHandler arh = [callIndex, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecuteBatch, callIndex);
                };
                if (!SendRequest(idExecuteBatch, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    CAutoLock al(m_csDB);
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall.erase(callIndex);
#endif
                    if (rowset || meta) {
                        m_mapRowset.erase(callIndex);
                    }
                    m_mapHandler.erase(callIndex);
                    return false;
                }
                if (queueOk)
                    GetSocket()->GetClientQueue().EndJob();
                return true;
            }

            /**
             * Process one or more sets of prepared statements with an array of parameter data asynchronously
             * @param vParam an array of parameter data which will be bounded to previously prepared parameters
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(CDBVariantArray &vParam, const DExecuteResult& handler = nullptr, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr,
                    bool meta = true, bool lastInsertId = true, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                bool rowset = (row) ? true : false;
                meta = (meta && rh);
                CScopeUQueue sb;
                sb << rowset << meta << lastInsertId;

                UINT64 callIndex;
                bool queueOk = false;
                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                {
                    if (vParam.size()) {
                        queueOk = GetSocket()->GetClientQueue().StartJob();
                        if (!SendParametersData(vParam)) {
                            Clean();
                            return false;
                        }
                    }
                    callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset[callIndex] = CRowsetHandler(rh, row);
                    }
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall[callIndex] = &vParam;
#endif
                }
                sb << callIndex;
                DResultHandler arh = [callIndex, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecuteParameters, callIndex);
                };
                if (!SendRequest(idExecuteParameters, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    CAutoLock al(m_csDB);
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall.erase(callIndex);
#endif
                    if (rowset || meta) {
                        m_mapRowset.erase(callIndex);
                    }
                    return false;
                }
                if (queueOk)
                    GetSocket()->GetClientQueue().EndJob();
                return true;
            }

            /**
             * Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously.
             * @param sql a complex SQL statement which may be combined with multiple basic SQL statements
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(const wchar_t* sql, const DExecuteResult& handler = nullptr, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, bool meta = true, bool lastInsertId = true, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                bool rowset = (row) ? true : false;
                meta = (meta && rh);
                CScopeUQueue sb;
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                UINT64 index = GetCallIndex();
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset[index] = CRowsetHandler(rh, row);
                    }
                }
                sb << sql << rowset << meta << lastInsertId << index;
                DResultHandler arh = [index, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecute, index);
                };
                if (!SendRequest(idExecute, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset.erase(index);
                    }
                    return false;
                }
                return true;
            }

#ifdef NATIVE_UTF16_SUPPORTED

            /**
             * Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
             * @param sql a parameterized SQL statement
             * @param handler a callback for SQL preparing result
             * @param vParameterInfo a given array of parameter informations
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Prepare(const char16_t *sql, const DResult& handler = nullptr, const CParameterInfoArray& vParameterInfo = CParameterInfoArray(), const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res;
                    std::wstring errMsg;
                    unsigned int parameters;
                    ar >> res >> errMsg >> parameters;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_bCallReturn = false;
                    ash->m_lastReqId = idPrepare;
                    ash->m_dbErrCode = res;
                    ash->m_dbErrMsg = errMsg;
                    ash->m_parameters = (parameters & 0xffff);
                    ash->m_outputs = (parameters >> 16);
                    ash->m_indexProc = 0;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };
                sb << sql << vParameterInfo;
                return SendRequest(idPrepare, sb->GetBuffer(), sb->GetSize(), arh, discarded, se);
            }

            /**
             * Execute a batch of SQL statements on one single call
             * @param isolation a value for manual transaction isolation. Specifically, there is no manual transaction around the batch SQL statements if it is tiUnspecified
             * @param sql a SQL statement having a batch of individual SQL statements
             * @param vParam an array of parameter data which will be bounded to previously prepared parameters. The array size can be 0 if the given batch SQL statement doesn't having any prepared statement
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations
             * @param delimiter a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing
             * @param batchHeader a callback for tracking batch beginning event
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param plan a value for computing how included transactions should be rollback
             * @param vPInfo a given array of parameter informations which may be empty to some of database management systems
             * @param lastInsertId a boolean for last insert record identification number
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool ExecuteBatch(tagTransactionIsolation isolation, const char16_t* sql, CDBVariantArray& vParam = CDBVariantArray(),
                    const DExecuteResult& handler = nullptr, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, const char16_t* delimiter = u";",
                    const DRowsetHeader& batchHeader = nullptr, const DDiscarded& discarded = nullptr, bool meta = true, tagRollbackPlan plan = rpDefault,
                    const CParameterInfoArray& vPInfo = CParameterInfoArray(), bool lastInsertId = true, const DServerException& se = nullptr) {
                bool rowset = (row) ? true : false;
                meta = (meta && rh);
                CScopeUQueue sb;
                sb << sql << delimiter << (int) isolation << (int) plan << rowset << meta << lastInsertId;

                UINT64 callIndex;
                bool queueOk = false;

                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                if (vParam.size())
                    queueOk = GetSocket()->GetClientQueue().StartJob();
                {
                    if (!SendParametersData(vParam)) {
                        Clean();
                        return false;
                    }
                    callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset[callIndex] = CRowsetHandler(rh, row);
                    }
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall[callIndex] = &vParam;
#endif
                    m_mapHandler[callIndex] = batchHeader;
                    sb << m_strConnection << m_flags;
                }
                sb << callIndex << vPInfo;
                DResultHandler arh = [callIndex, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecuteBatch, callIndex);
                };
                if (!SendRequest(idExecuteBatch, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    CAutoLock al(m_csDB);
#ifndef NO_OUTPUT_BINDING
                    m_mapParameterCall.erase(callIndex);
#endif
                    if (rowset || meta) {
                        m_mapRowset.erase(callIndex);
                    }
                    m_mapHandler.erase(callIndex);
                    return false;
                }
                if (queueOk)
                    GetSocket()->GetClientQueue().EndJob();
                return true;
            }

            /**
             * Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously.
             * @param sql a complex SQL statement which may be combined with multiple basic SQL statements
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(const char16_t* sql, const DExecuteResult& handler = nullptr, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, bool meta = true, bool lastInsertId = true, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                bool rowset = (row) ? true : false;
                meta = (meta && rh);
                CScopeUQueue sb;
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                UINT64 index = GetCallIndex();
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset[index] = CRowsetHandler(rh, row);
                    }
                }
                sb << sql << rowset << meta << lastInsertId << index;
                DResultHandler arh = [index, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecute, index);
                };
                if (!SendRequest(idExecute, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    CAutoLock al(m_csDB);
                    if (rowset || meta) {
                        m_mapRowset.erase(index);
                    }
                    return false;
                }
                return true;
            }

            /**
             * Open a database connection at server side asynchronously
             * @param strConnection a database connection string. The database connection string can be an empty string if its server side supports global database connection string
             * @param handler a callback for database connecting result
             * @param flags a set of flags transferred to server to indicate how to build database connection
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Open(const char16_t* strConnection, const DResult& handler, unsigned int flags = 0, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                std::wstring s;
                CScopeUQueue sb;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    m_flags = flags;
                    if (strConnection) {
                        s.swap(m_strConnection);
#ifdef WIN32_64
                        m_strConnection = (const wchar_t*)strConnection;
#else
                        m_strConnection = SPA::Utilities::ToWide(strConnection);
#endif
                    }
                }
                sb << strConnection << flags;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res, ms;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> ms;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_dbErrCode = res;
                    ash->m_lastReqId = idOpen;
                    if (res == 0) {
                        ash->m_strConnection = std::move(errMsg);
                        errMsg.clear();
                    } else {
                        ash->m_strConnection.clear();
                    }
                    ash->m_dbErrMsg = errMsg;
                    ash->m_ms = (tagManagementSystem) ms;
                    ash->m_parameters = 0;
                    ash->m_outputs = 0;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };
                if (SendRequest(UDB::idOpen, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    return true;
                }
                CAutoLock al(m_csDB);
                if (strConnection) {
                    m_strConnection.swap(s);
                }
                return false;
            }
#endif

            /**
             * Open a database connection at server side asynchronously
             * @param strConnection a database connection string. The database connection string can be an empty string if its server side supports global database connection string
             * @param handler a callback for database connecting result
             * @param flags a set of flags transferred to server to indicate how to build database connection
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Open(const wchar_t* strConnection, const DResult& handler, unsigned int flags = 0, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                std::wstring s;
                CScopeUQueue sb;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    m_flags = flags;
                    if (strConnection) {
                        s.swap(m_strConnection);
                        m_strConnection = strConnection;
                    }
                }
                sb << strConnection << flags;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res, ms;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> ms;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_dbErrCode = res;
                    ash->m_lastReqId = idOpen;
                    if (res == 0) {
                        ash->m_strConnection = std::move(errMsg);
                        errMsg.clear();
                    } else {
                        ash->m_strConnection.clear();
                    }
                    ash->m_dbErrMsg = errMsg;
                    ash->m_ms = (tagManagementSystem) ms;
                    ash->m_parameters = 0;
                    ash->m_outputs = 0;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };
                if (SendRequest(UDB::idOpen, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    return true;
                }
                CAutoLock al(m_csDB);
                if (strConnection) {
                    m_strConnection.swap(s);
                }
                return false;
            }

            /**
             * Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
             * @param sql a parameterized SQL statement
             * @param handler a callback for SQL preparing result
             * @param vParameterInfo a given array of parameter informations
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Prepare(const wchar_t *sql, const DResult& handler = nullptr, const CParameterInfoArray& vParameterInfo = CParameterInfoArray(), const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res;
                    std::wstring errMsg;
                    unsigned int parameters;
                    ar >> res >> errMsg >> parameters;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_bCallReturn = false;
                    ash->m_lastReqId = idPrepare;
                    ash->m_dbErrCode = res;
                    ash->m_dbErrMsg = errMsg;
                    ash->m_parameters = (parameters & 0xffff);
                    ash->m_outputs = (parameters >> 16);
                    ash->m_indexProc = 0;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };
                sb << sql << vParameterInfo;
                return SendRequest(idPrepare, sb->GetBuffer(), sb->GetSize(), arh, discarded, se);
            }

            /**
             * Notify connected remote server to close database connection string asynchronously
             * @param handler a callback for closing result, which should be OK always as long as there is network or queue available
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Close(DResult handler = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_lastReqId = idClose;
                    ash->m_strConnection.clear();
                    ash->m_dbErrCode = res;
                    ash->m_dbErrMsg = errMsg;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };
                return SendRequest(idClose, (const unsigned char*) nullptr, (unsigned int) 0, arh, discarded, se);
            }

            /**
             * Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param isolation a value for isolation
             * @param handler a callback for tracking its response result
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool BeginTrans(tagTransactionIsolation isolation = tiReadCommited, const DResult& handler = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                unsigned int flags;
                std::wstring connection;
                CScopeUQueue sb;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res, ms;
                    std::wstring errMsg;
                    ar >> res >> errMsg >> ms;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    if (res == 0) {
                        ash->m_strConnection = errMsg;
                        errMsg.clear();
                    }
                    ash->m_lastReqId = idBeginTrans;
                    ash->m_dbErrCode = res;
                    ash->m_dbErrMsg = errMsg;
                    ash->m_ms = (tagManagementSystem) ms;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };

                //make sure BeginTrans sending and underlying client persistent message queue as one combination sending
                //to avoid possible request sending/client message writing overlapping within multiple threading environment
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    connection = m_strConnection;
                    flags = m_flags;
                }

                sb << (int) isolation << connection << flags;
                //associate begin transaction with underlying client persistent message queue
                m_queueOk = GetSocket()->GetClientQueue().StartJob();
                return SendRequest(idBeginTrans, sb->GetBuffer(), sb->GetSize(), arh, discarded, se);
            }

            /**
             * End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param plan a value for computing how included transactions should be rollback
             * @param handler a callback for tracking its response result
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param se a callback for tracking an exception from server
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool EndTrans(tagRollbackPlan plan = rpDefault, const DResult& handler = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << (int) plan;
                DResultHandler arh = [handler](CAsyncResult & ar) {
                    int res;
                    std::wstring errMsg;
                    ar >> res >> errMsg;
                    CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                    ash->m_csDB.lock();
                    ash->m_lastReqId = idEndTrans;
                    ash->m_dbErrCode = res;
                    ash->m_dbErrMsg = errMsg;
                    ash->m_csDB.unlock();
                    if (handler) {
                        handler(*ash, res, errMsg);
                    }
                };

                //make sure EndTrans sending and underlying client persistent message queue as one combination sending
                //to avoid possible request sending/client message writing overlapping within multiple threading environment
#ifndef NODE_JS_ADAPTER_PROJECT
                SPA::CAutoLock alOne(m_csOneSending);
#endif
                if (SendRequest(idEndTrans, sb->GetBuffer(), sb->GetSize(), arh, discarded, se)) {
                    if (m_queueOk) {
                        //associate end transaction with underlying client persistent message queue
                        GetSocket()->GetClientQueue().EndJob();
                        m_queueOk = false;
                    }
                    return true;
                }
                return false;
            }

            struct SQLExeInfo : public ErrInfo {
                INT64 affected;
                unsigned int oks;
                unsigned int fails;
                CDBVariant lastId;

                SQLExeInfo(int res = 0, const wchar_t* errMsg = nullptr, INT64 aff = 0, unsigned int suc_ones = 0, unsigned int bad_ones = 0, const CDBVariant& id = CDBVariant())
                : ErrInfo(res, errMsg), affected(aff), oks(suc_ones), fails(bad_ones), lastId(id) {
                }

                std::wstring ToString() {
                    std::wstring s = ErrInfo::ToString();
                    s += (L", affected: " + std::to_wstring(affected));
                    s += (L", oks: " + std::to_wstring(oks));
                    s += (L", fails: " + std::to_wstring(fails));
                    s += ((lastId.vt <= VT_NULL) ? L", lastId: null" : (L", lastId: " + std::to_wstring(lastId.ullVal)).c_str());
                    return s;
                }
            };
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE

            virtual std::future<ErrInfo> beginTrans(tagTransactionIsolation isolation = tiReadCommited) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!BeginTrans(isolation, d, get_aborted(prom, L"BeginTrans", idBeginTrans), get_se(prom))) {
                    raise(L"BeginTrans", idBeginTrans);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> endTrans(tagRollbackPlan plan = rpDefault) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!EndTrans(plan, d, get_aborted(prom, L"EndTrans", idEndTrans), get_se(prom))) {
                    raise(L"EndTrans", idEndTrans);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> close() {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!Close(d, get_aborted(prom, L"Close", idClose), get_se(prom))) {
                    raise(L"Close", idClose);
                }
                return prom->get_future();
            }

            virtual std::future<SQLExeInfo> execute(CDBVariantArray& vParam, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, bool meta = true, bool lastInsertId = true) {
                std::shared_ptr<std::promise<SQLExeInfo> > prom(new std::promise<SQLExeInfo>);
                DExecuteResult handler = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring& errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                    unsigned int oks = (unsigned int) (fail_ok & 0xffffffff);
                    unsigned int fails = (unsigned int) (fail_ok >> 32);
                    prom->set_value(SQLExeInfo(res, errMsg.c_str(), affected, oks, fails, vtId));
                };
                if (!Execute(vParam, handler, row, rh, meta, lastInsertId, get_aborted(prom, L"ExecuteParameters", idExecuteParameters), get_se(prom))) {
                    raise(L"ExecuteParameters", idExecuteParameters);
                }
                return prom->get_future();
            }

            virtual std::future<SQLExeInfo> execute(const wchar_t *sql, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, bool meta = true, bool lastInsertId = true) {
                std::shared_ptr<std::promise<SQLExeInfo> > prom(new std::promise<SQLExeInfo>);
                DExecuteResult handler = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring& errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                    unsigned int oks = (unsigned int) (fail_ok & 0xffffffff);
                    unsigned int fails = (unsigned int) (fail_ok >> 32);
                    prom->set_value(SQLExeInfo(res, errMsg.c_str(), affected, oks, fails, vtId));
                };
                if (!Execute(sql, handler, row, rh, meta, lastInsertId, get_aborted(prom, L"ExecuteSQL", idExecute), get_se(prom))) {
                    raise(L"ExecuteSQL", idExecute);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> prepare(const wchar_t* sql, const CParameterInfoArray& vParameterInfo = CParameterInfoArray()) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!Prepare(sql, d, vParameterInfo, get_aborted(prom, L"Prepare", idPrepare), get_se(prom))) {
                    raise(L"Prepare", idPrepare);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> open(const wchar_t* sql, unsigned int flags = 0) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!Open(sql, d, flags, get_aborted(prom, L"Open", idOpen), get_se(prom))) {
                    raise(L"Open", idOpen);
                }
                return prom->get_future();
            }

            virtual std::future<SQLExeInfo> executeBatch(tagTransactionIsolation isolation, const wchar_t* sql, CDBVariantArray& vParam = CDBVariantArray(),
                    const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, const wchar_t* delimiter = L";", const DRowsetHeader& batchHeader = nullptr,
                    bool meta = true, tagRollbackPlan plan = rpDefault, const CParameterInfoArray& vPInfo = CParameterInfoArray(), bool lastInsertId = true) {
                std::shared_ptr<std::promise<SQLExeInfo> > prom(new std::promise<SQLExeInfo>);
                DExecuteResult handler = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring& errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                    unsigned int oks = (unsigned int) (fail_ok & 0xffffffff);
                    unsigned int fails = (unsigned int) (fail_ok >> 32);
                    prom->set_value(SQLExeInfo(res, errMsg.c_str(), affected, oks, fails, vtId));
                };
                if (!ExecuteBatch(isolation, sql, vParam, handler, row, rh, delimiter, batchHeader, get_aborted(prom, L"ExecuteBatch", idExecuteBatch), meta, plan, vPInfo, lastInsertId, get_se(prom))) {
                    raise(L"ExecuteBatch", idExecuteBatch);
                }
                return prom->get_future();
            }

#ifdef NATIVE_UTF16_SUPPORTED

            virtual std::future<SQLExeInfo> execute(const char16_t* sql, const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, bool meta = true, bool lastInsertId = true) {
                std::shared_ptr<std::promise<SQLExeInfo> > prom(new std::promise<SQLExeInfo>);
                DExecuteResult handler = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring& errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                    unsigned int oks = (unsigned int) (fail_ok & 0xffffffff);
                    unsigned int fails = (unsigned int) (fail_ok >> 32);
                    prom->set_value(SQLExeInfo(res, errMsg.c_str(), affected, oks, fails, vtId));
                };
                if (!Execute(sql, handler, row, rh, meta, lastInsertId, get_aborted(prom, L"ExecuteSQL", idExecute), get_se(prom))) {
                    raise(L"ExecuteSQL", idExecute);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> prepare(const char16_t* sql, const CParameterInfoArray& vParameterInfo = CParameterInfoArray()) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!Prepare(sql, d, vParameterInfo, get_aborted(prom, L"Prepare", idPrepare), get_se(prom))) {
                    raise(L"Prepare", idPrepare);
                }
                return prom->get_future();
            }

            virtual std::future<ErrInfo> open(const char16_t* sql, unsigned int flags = 0) {
                std::shared_ptr<std::promise<ErrInfo> > prom(new std::promise<ErrInfo>);
                DResult d = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring & errMsg) {
                    prom->set_value(ErrInfo(res, errMsg.c_str()));
                };
                if (!Open(sql, d, flags, get_aborted(prom, L"Open", idOpen), get_se(prom))) {
                    raise(L"Open", idOpen);
                }
                return prom->get_future();
            }

            virtual std::future<SQLExeInfo> executeBatch(tagTransactionIsolation isolation, const char16_t* sql, CDBVariantArray& vParam = CDBVariantArray(),
                    const DRows& row = nullptr, const DRowsetHeader& rh = nullptr, const char16_t* delimiter = u";", const DRowsetHeader& batchHeader = nullptr,
                    bool meta = true, tagRollbackPlan plan = rpDefault, const CParameterInfoArray& vPInfo = CParameterInfoArray(), bool lastInsertId = true) {
                std::shared_ptr<std::promise<SQLExeInfo> > prom(new std::promise<SQLExeInfo>);
                DExecuteResult handler = [prom](CAsyncDBHandler& dbHandler, int res, const std::wstring& errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                    unsigned int oks = (unsigned int) (fail_ok & 0xffffffff);
                    unsigned int fails = (unsigned int) (fail_ok >> 32);
                    prom->set_value(SQLExeInfo(res, errMsg.c_str(), affected, oks, fails, vtId));
                };
                if (!ExecuteBatch(isolation, sql, vParam, handler, row, rh, delimiter, batchHeader, get_aborted(prom, L"ExecuteBatch", idExecuteBatch), meta, plan, vPInfo, lastInsertId, get_se(prom))) {
                    raise(L"ExecuteBatch", idExecuteBatch);
                }
                return prom->get_future();
            }
#endif
#endif
#endif
        protected:

            virtual void OnAllProcessed() {
                CAutoLock al1(m_csDB);
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                m_vData.SetSize(0);
#else
                m_vData.clear();
#endif
                m_Blob.SetSize(0);
                if (m_Blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    m_Blob.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
                }
            }

            virtual void OnMergeTo(CAsyncServiceHandler & to) {
                CAsyncDBHandler &dbTo = (CAsyncDBHandler&) to;
                CAutoLock al0(dbTo.m_csDB);
                {
                    CAutoLock al1(m_csDB);
                    for (auto it = m_mapRowset.begin(), end = m_mapRowset.end(); it != end; ++it) {
                        dbTo.m_mapRowset[it->first] = it->second;
                    }
                    m_mapRowset.clear();
#ifndef NO_OUTPUT_BINDING
                    for (auto it = m_mapParameterCall.begin(), end = m_mapParameterCall.end(); it != end; ++it) {
                        dbTo.m_mapParameterCall[it->first] = it->second;
                    }
                    m_mapParameterCall.clear();
#endif
                    for (auto it = m_mapHandler.begin(), end = m_mapHandler.end(); it != end; ++it) {
                        dbTo.m_mapHandler[it->first] = it->second;
                    }
                    m_mapHandler.clear();
                }
            }

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case idParameterPosition:
                        mc >> m_nParamPos;
                        m_csDB.lock();
                        m_bCallReturn = false;
                        m_indexProc = 0;
                        m_csDB.unlock();
                        break;
                    case idSqlBatchHeader:
                    {
                        UINT64 callIndex;
                        int res, ms;
                        unsigned int params;
                        DRowsetHeader cb = nullptr;
                        std::wstring errMsg;
                        mc >> res >> errMsg >> ms >> params >> callIndex;
                        {
                            CAutoLock al(m_csDB);
                            m_vColInfo.clear();
                            m_indexProc = 0;
                            m_lastReqId = idSqlBatchHeader;
                            m_parameters = (params & 0xffff);
                            m_outputs = 0;
                            if (!res) {
                                m_strConnection = errMsg;
                                errMsg.clear();
                            }
                            m_dbErrCode = res;
                            m_dbErrMsg = errMsg;
                            m_ms = (tagManagementSystem) ms;
                            auto it = m_mapHandler.find(callIndex);
                            if (it != m_mapHandler.end()) {
                                cb = it->second;
                            }
                        }
                        if (cb) {
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            cb(*this, nullptr, 0);
#else
                            cb(*this);
#endif
                        }
                    }
                        break;
                    case idRowsetHeader:
                    {
                        DRowsetHeader header;
                        m_Blob.SetSize(0);
                        if (m_Blob.GetMaxSize() > ONE_MEGA_BYTES) {
                            m_Blob.ReallocBuffer(ONE_MEGA_BYTES);
                        }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                        const unsigned char *start;
                        unsigned int bytes;
                        m_vData.SetSize(0);
#else
                        m_vData.clear();
#endif
                        {
                            unsigned int outputs = 0;
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            start = mc.GetBuffer();
#endif
                            CAutoLock al(m_csDB);
                            mc >> m_vColInfo;
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            bytes = mc.GetHeadPosition();
#endif
                            mc >> m_indexRowset;
                            if (mc.GetSize()) {
                                mc >> outputs;
                            }
                            if (!outputs && m_vColInfo.size()) {
                                auto it = m_mapRowset.find(m_indexRowset);
                                if (it != m_mapRowset.end()) {
                                    header = it->second.first;
                                }
                            }
                        }
                        if (header) {
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            header(*this, start, bytes);
#else
                            header(*this);
#endif
                        }
                    }
                        break;
                    case idCallReturn:
                    {
                        CDBVariant vt;
                        mc >> vt;
                        CAutoLock al(m_csDB);
#ifdef NO_OUTPUT_BINDING
                        m_vtRet = vt;
#else
                        auto it = m_mapParameterCall.find(m_indexRowset);
                        if (it != m_mapParameterCall.end()) {
                            //crash? make sure that vParam is valid after calling the method Execute
                            CDBVariantArray &vParam = *(it->second);
                            size_t pos = m_parameters * m_indexProc + (m_nParamPos >> 16);
                            vParam[pos] = vt;
                        }
#endif
                        m_bCallReturn = true;
                    }
                        break;
                    case idBeginRows:
                        m_Blob.SetSize(0);
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                        m_vData.SetSize(0);
#else
                        m_vData.clear();
#endif
                        if (mc.GetSize()) {
                            CAutoLock al(m_csDB);
                            mc >> m_indexRowset;
                        }
                        break;
                    case idTransferring:
                        if (mc.GetSize()) {
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            m_vData.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
#else
                            m_csDB.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csDB.unlock();
                            if (Utf8ToW) {
                                mc.Utf8ToW(true);
                            }
                            while (mc.GetSize()) {
                                m_vData.push_back(CDBVariant());
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            if (Utf8ToW) {
                                mc.Utf8ToW(false);
                            }
                            assert(mc.GetSize() == 0);
#endif
                        }
                        break;
                    case idOutputParameter:
                    case idEndRows:
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                        if (mc.GetSize() || m_vData.GetSize())
#else
                        if (mc.GetSize() || m_vData.size())
#endif
                        {
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            m_vData.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
#else
                            m_csDB.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csDB.unlock();
                            if (Utf8ToW) {
                                mc.Utf8ToW(true);
                            }
                            CDBVariant vtOne;
                            while (mc.GetSize()) {
                                m_vData.push_back(vtOne);
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW) {
                                mc.Utf8ToW(false);
                            }
#endif
                            DRows row;
                            if (reqId == idOutputParameter) {
                                {
                                    CAutoLock al(m_csDB);
#ifdef NO_OUTPUT_BINDING
                                    m_bProc = true;
                                    auto it0 = m_mapRowset.find(m_indexRowset);
                                    if (it0 != m_mapRowset.end()) {
                                        row = it0->second.second;
                                    }
#endif

#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                                    if (m_lastReqId == idSqlBatchHeader) {
                                        if (!m_indexProc) {
                                            unsigned int size = 0, orig_len = m_vData.GetSize();
                                            m_vData.SetHeadPosition();
                                            CDBVariant vt;
                                            while (m_vData.GetSize()) {
                                                m_vData >> vt;
                                                ++size;
                                                vt.Clear();
                                            }
                                            m_vData.SetSize(orig_len);
                                            m_outputs += (size + (unsigned int) m_bCallReturn);
                                        }
                                    } else {
                                        if (!m_outputs) {
                                            unsigned int size = 0, orig_len = m_vData.GetSize();
                                            m_vData.SetHeadPosition();
                                            CDBVariant vt;
                                            while (m_vData.GetSize()) {
                                                m_vData >> vt;
                                                ++size;
                                                vt.Clear();
                                            }
                                            m_vData.SetSize(orig_len);
                                            m_outputs += (size + (unsigned int) m_bCallReturn);
                                        }
                                    }
#else
                                    if (m_lastReqId == idSqlBatchHeader) {
                                        if (!m_indexProc) {
                                            m_outputs += ((unsigned int) m_vData.size() + (unsigned int) m_bCallReturn);
                                        }
                                    } else {
                                        if (!m_outputs) {
                                            m_outputs = ((unsigned int) m_vData.size() + (unsigned int) m_bCallReturn);
                                        }
                                    }
#endif

#ifndef NO_OUTPUT_BINDING
                                    auto it = m_mapParameterCall.find(m_indexRowset);
                                    if (it != m_mapParameterCall.cend()) {
                                        //crash? make sure that vParam is valid after calling the method Execute
                                        size_t pos;
                                        CDBVariantArray &vParam = *(it->second);
                                        if (m_lastReqId == idSqlBatchHeader)
                                            pos = m_parameters * m_indexProc + (m_nParamPos & 0xffff) + (m_nParamPos >> 16) - (unsigned int) m_vData.size();
                                        else
                                            pos = m_parameters * m_indexProc + m_parameters - (unsigned int) m_vData.size();
                                        for (auto start = m_vData.begin(), end = m_vData.end(); start != end; ++start, ++pos) {
                                            vParam[pos] = *start;
                                        }
                                    }
#endif
                                    ++m_indexProc;
                                }
#ifdef NO_OUTPUT_BINDING
                                if (row) {
                                    row(*this, m_vData);
                                }
                                m_csDB.lock();
                                m_bProc = false;
                                m_csDB.unlock();
#endif
                            } else {
                                {
                                    CAutoLock al(m_csDB);
                                    auto it = m_mapRowset.find(m_indexRowset);
                                    if (it != m_mapRowset.end()) {
                                        row = it->second.second;
                                    }
                                }
                                if (row) {
                                    row(*this, m_vData);
                                }
                            }
                        }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                        m_vData.SetSize(0);
#else
                        m_vData.clear();
#endif
                        break;
                    case idStartBLOB:
                        if (mc.GetSize()) {
                            m_Blob.SetSize(0);
                            unsigned int len;
                            mc >> len;
                            if (len != UQUEUE_END_POSTION && len > m_Blob.GetMaxSize()) {
                                m_Blob.ReallocBuffer(len);
                            }
                            m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
                        }
                        break;
                    case idChunk:
                        if (mc.GetSize()) {
                            m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
                        }
                        break;
                    case idEndBLOB:
                        if (mc.GetSize() || m_Blob.GetSize()) {
                            m_Blob.Push(mc.GetBuffer(), mc.GetSize());
                            mc.SetSize(0);
                            unsigned int *len = (unsigned int*) m_Blob.GetBuffer(sizeof (VARTYPE));
                            if (*len >= BLOB_LENGTH_NOT_AVAILABLE) {
                                //legth should be reset if BLOB length not available from server side at beginning
                                *len = (m_Blob.GetSize() - sizeof (VARTYPE) - sizeof (unsigned int));
                            }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                            m_vData.Push(m_Blob.GetBuffer(), m_Blob.GetSize());
                            m_Blob.SetSize(0);
#else
                            m_vData.push_back(CDBVariant());
                            CDBVariant &vt = m_vData.back();
                            m_Blob >> vt;
#endif
                            assert(m_Blob.GetSize() == 0);
                        }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:

            void Process(const DExecuteResult& handler, CAsyncResult & ar, unsigned short reqId, UINT64 index) {
                INT64 affected;
                UINT64 fail_ok;
                int res;
                std::wstring errMsg;
                CDBVariant vtId;
                CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                ar >> affected >> res >> errMsg >> vtId >> fail_ok;
                {
                    CAutoLock al(m_csDB);
                    m_lastReqId = reqId;
                    m_affected = affected;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    auto it = m_mapRowset.find(index);
                    if (it != m_mapRowset.end()) {
                        m_mapRowset.erase(it);
                    }
#ifndef NO_OUTPUT_BINDING
                    auto pit = m_mapParameterCall.find(index);
                    if (pit != m_mapParameterCall.end()) {
                        m_mapParameterCall.erase(pit);
                    }
#endif
                    auto ph = m_mapHandler.find(index);
                    if (ph != m_mapHandler.end()) {
                        ash->m_mapHandler.erase(ph);
                    }
                }
                if (handler) {
                    handler(*ash, res, errMsg, affected, fail_ok, vtId);
                }
            }

            void Clean() {
                m_mapRowset.clear();
#ifndef NO_OUTPUT_BINDING
                m_mapParameterCall.clear();
#endif
                m_mapHandler.clear();
                m_vColInfo.clear();
                m_lastReqId = 0;
                m_Blob.SetSize(0);
                if (m_Blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    m_Blob.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
                }
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                m_vData.SetSize(0);
#else
                m_vData.clear();
#endif
            }

        protected:
            typedef SPA::CSpinAutoLock CAutoLock;
            CSpinLock m_csDB;
            CDBColumnInfoArray m_vColInfo;
            std::unordered_map<UINT64, CRowsetHandler> m_mapRowset;
            INT64 m_affected;
            int m_dbErrCode;
            std::wstring m_dbErrMsg;
            unsigned short m_lastReqId;
            UINT64 m_indexRowset;

        private:
            std::wstring m_strConnection;
#ifndef NO_OUTPUT_BINDING
            std::unordered_map<UINT64, CDBVariantArray*> m_mapParameterCall;
#endif
            unsigned int m_indexProc;
            CUQueue m_Blob;
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
            CUQueue m_vData;
#else
            CDBVariantArray m_vData;
#endif
            tagManagementSystem m_ms;
            unsigned int m_flags;
            unsigned int m_parameters;
            unsigned int m_outputs;
            bool m_bCallReturn;
#ifndef NODE_JS_ADAPTER_PROJECT
            CUCriticalSection m_csOneSending;
#endif
            bool m_queueOk;
            std::unordered_map<UINT64, DRowsetHeader> m_mapHandler;
            unsigned int m_nParamPos;

#ifdef NO_OUTPUT_BINDING
            CDBVariant m_vtRet;
            bool m_bProc;
#endif

#ifdef NODE_JS_ADAPTER_PROJECT
        public:

            UINT64 BeginTrans(Isolate* isolate, int args, Local<Value> *argv, tagTransactionIsolation isolation) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result = GetResult(isolate, argv[0], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[1], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[2], bad);
                if (bad) return 0;
                return BeginTrans(isolation, result, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 Close(Isolate* isolate, int args, Local<Value> *argv) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result = GetResult(isolate, argv[0], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[1], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[2], bad);
                if (bad) return 0;
                return Close(result, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 EndTrans(Isolate* isolate, int args, Local<Value> *argv, tagRollbackPlan plan) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result = GetResult(isolate, argv[0], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[1], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[2], bad);
                if (bad) return 0;
                return EndTrans(plan, result, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 Execute(Isolate* isolate, int args, Local<Value> *argv, CDBVariantArray &vParam) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result = GetExecuteResult(isolate, argv[0], bad);
                if (bad) return 0;
                DRows r = GetRows(isolate, argv[1], bad);
                if (bad) return 0;
                DRowsetHeader rh = GetRowsetHeader(isolate, argv[2], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[3], bad);
                if (bad) return 0;
                bool meta = GetMeta(isolate, argv[4], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[5], bad);
                if (bad) return 0;
                return Execute(vParam, result, r, rh, meta, true, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 Execute(Isolate* isolate, int args, Local<Value> *argv, const UTF16 *sql) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result = GetExecuteResult(isolate, argv[0], bad);
                if (bad) return 0;
                DRows r = GetRows(isolate, argv[1], bad);
                if (bad) return 0;
                DRowsetHeader rh = GetRowsetHeader(isolate, argv[2], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[3], bad);
                if (bad) return 0;
                bool meta = GetMeta(isolate, argv[4], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[5], bad);
                if (bad) return 0;
                return Execute(sql, result, r, rh, meta, true, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 ExecuteBatch(Isolate* isolate, int args, Local<Value> *argv, tagTransactionIsolation isolation, const UTF16 *sql, CDBVariantArray &vParam, tagRollbackPlan plan, const UTF16 *delimiter, const CParameterInfoArray& vPInfo) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result = GetExecuteResult(isolate, argv[0], bad);
                if (bad) return 0;
                DRows r = GetRows(isolate, argv[1], bad);
                if (bad) return 0;
                DRowsetHeader rh = GetRowsetHeader(isolate, argv[2], bad);
                if (bad) return 0;
                DRowsetHeader bh = GetBatchHeader(isolate, argv[3], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[4], bad);
                if (bad) return 0;
                bool meta = GetMeta(isolate, argv[5], bad);
                if (bad) return 0;
                bool lastInsertId = GetMeta(isolate, argv[6], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[7], bad);
                if (bad) return 0;
                return ExecuteBatch(isolation, sql, vParam, result, r, rh, delimiter, bh, dd, meta, plan, vPInfo, lastInsertId, se) ? index : INVALID_NUMBER;
            }

            UINT64 Open(Isolate* isolate, int args, Local<Value> *argv, const UTF16* strConnection, unsigned int flags) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result = GetResult(isolate, argv[0], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[1], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[2], bad);
                if (bad) return 0;
                return Open(strConnection, result, flags, dd, se) ? index : INVALID_NUMBER;
            }

            UINT64 Prepare(Isolate* isolate, int args, Local<Value> *argv, const UTF16 *sql, const CParameterInfoArray& vParameterInfo) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result = GetResult(isolate, argv[0], bad);
                if (bad) return 0;
                DDiscarded dd = Get(isolate, argv[1], bad);
                if (bad) return 0;
                DServerException se = GetSE(isolate, argv[2], bad);
                if (bad) return 0;
                return Prepare(sql, result, vParameterInfo, dd, se) ? index : INVALID_NUMBER;
            }

        protected:

            enum tagDBEvent {
                eResult = 0,
                eExecuteResult,
                eRowsetHeader,
                eRows,
                eBatchHeader,
                eDiscarded,
                eException
            };

            struct DBCb {
                tagDBEvent Type;
                PUQueue Buffer;
                std::shared_ptr<CNJFunc> Func;
                std::shared_ptr<CUQueue> VData;
            };

            std::deque<DBCb> m_deqDBCb; //protected by m_csDB;
            uv_async_t m_typeDB; //DB request events

            DRowsetHeader GetBatchHeader(Isolate* isolate, Local<Value> header, bool &bad) {
                bad = false;
                DRowsetHeader rh;
                if (header->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(header));
                    Backup(func);
                    rh = [func](CAsyncDBHandler & db, const unsigned char *start, unsigned int bytes) {
                        assert(!bytes);
                        DBCb cb;
                        cb.Type = eBatchHeader;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(header)) {
                    NJA::ThrowException(isolate, "A callback expected for batch header");
                    bad = true;
                }
                return rh;
            }

            bool GetMeta(Isolate* isolate, Local<Value> m, bool &bad) {
                bad = false;
                if (m->IsBoolean() || m->IsUint32()) {
#ifdef BOOL_ISOLATE
                    return m->BooleanValue(isolate);
#else
                    return m->BooleanValue(isolate->GetCurrentContext()).ToChecked();
#endif
                } else if (!NJA::IsNullOrUndefined(m)) {
                    NJA::ThrowException(isolate, "A boolean value expected");
                    bad = true;
                }
                return false;
            }

            DRowsetHeader GetRowsetHeader(Isolate* isolate, Local<Value> header, bool &bad) {
                bad = false;
                DRowsetHeader rh;
                if (header->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(header));
                    Backup(func);
                    rh = [func](CAsyncDBHandler & db, const unsigned char *start, unsigned int bytes) {
                        DBCb cb;
                        cb.Type = eRowsetHeader;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash;
                        cb.Buffer->Push(start, bytes);
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(header)) {
                    NJA::ThrowException(isolate, "A callback expected for record meta");
                    bad = true;
                }
                return rh;
            }

            DRows GetRows(Isolate* isolate, Local<Value> r, bool &bad) {
                bad = false;
                DRows rows;
                if (r->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(r));
                    Backup(func);
                    rows = [func](CAsyncDBHandler &db, CUQueue & vData) {
                        DBCb cb;
                        cb.Type = eRows;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        CUQueue *p = CScopeUQueue::Lock();
                        bool proc = db.IsProc();
                        if (proc && db.GetCallReturn()) {
                            *p << db.GetRetValue();
                            p->Push(vData.GetBuffer(), vData.GetSize());
                        } else {
                            p->Swap(vData);
                        }
                        cb.VData.reset(p, [](CUQueue * p) {
                            CScopeUQueue::Unlock(p);
                        });
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash << proc << (int) db.GetColumnInfo().size();
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(r)) {
                    NJA::ThrowException(isolate, "A callback expected for row data");
                    bad = true;
                }
                return rows;
            }

            DExecuteResult GetExecuteResult(Isolate* isolate, Local<Value> er, bool &bad) {
                bad = false;
                DExecuteResult result;
                if (er->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(er));
                    Backup(func);
                    result = [func](CAsyncDBHandler &db, int errCode, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, CDBVariant & vtId) {
                        DBCb cb;
                        cb.Type = eExecuteResult;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash << errCode << errMsg << affected << fail_ok << vtId;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(er)) {
                    NJA::ThrowException(isolate, "A callback expected for Execute end result");
                    bad = true;
                }
                return result;
            }

            DResult GetResult(Isolate* isolate, Local<Value> res, bool &bad) {
                bad = false;
                DResult result;
                if (res->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(res));
                    Backup(func);
                    result = [func](CAsyncDBHandler &db, int errCode, const std::wstring & errMsg) {
                        DBCb cb;
                        cb.Type = eResult;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash << errCode << errMsg;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(res)) {
                    NJA::ThrowException(isolate, "A callback expected for end result");
                    bad = true;
                }
                return result;
            }

            DDiscarded Get(Isolate* isolate, Local<Value> abort, bool &bad) {
                bad = false;
                DDiscarded dd;
                if (abort->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(abort));
                    Backup(func);
                    dd = [func](CAsyncServiceHandler *db, bool canceled) {
                        DBCb cb;
                        cb.Type = eDiscarded;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = (PAsyncDBHandler) db;
                        *cb.Buffer << ash << canceled;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(abort)) {
                    NJA::ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
                    bad = true;
                }
                return dd;
            }

            DServerException GetSE(Isolate* isolate, Local<Value> se, bool& bad) {
                bad = false;
                SPA::ClientSide::CAsyncServiceHandler::DServerException dSe;
                if (se->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(se));
                    Backup(func);
                    dSe = [func](CAsyncServiceHandler* db, unsigned short requestId, const wchar_t* errMessage, const char* errWhere, unsigned int errCode) {
                        DBCb cb;
                        cb.Type = eException;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = (PAsyncDBHandler) db;
                        *cb.Buffer << ash << requestId << errMessage << errWhere << errCode;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!NJA::IsNullOrUndefined(se)) {
                    NJA::ThrowException(isolate, "A callback expected for tracking exception from server");
                    bad = true;
                }
                return dSe;
            }

        private:
            static void req_cb(uv_async_t* handle);
#endif
        };
    } //namespace ClientSide
} //namespace SPA
#endif
