
#ifndef _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_
#define _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_

#include "udatabase.h"
#include "aclientw.h"

#ifdef NODE_JS_ADAPTER_PROJECT
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
                m_Blob.Utf8ToW(true);
#ifdef NODE_JS_ADAPTER_PROJECT
                ::memset(&m_typeDB, 0, sizeof (m_typeDB));
                m_typeDB.data = this;
                int fail = uv_async_init(uv_default_loop(), &m_typeDB, req_cb);
                assert(!fail);
                m_bProc = false;
#endif
            }

        public:

            CAsyncDBHandler(CClientSocket *cs)
            : CAsyncServiceHandler(serviceId, cs),
            m_affected(-1), m_dbErrCode(0), m_lastReqId(0),
            m_indexRowset(0), m_indexProc(0), m_ms(msUnknown), m_flags(0),
            m_parameters(0), m_outputs(0), m_bCallReturn(false), m_queueOk(false), m_nParamPos(0) {
                m_Blob.Utf8ToW(true);
#ifdef NODE_JS_ADAPTER_PROJECT
                ::memset(&m_typeDB, 0, sizeof (m_typeDB));
                m_typeDB.data = this;
                int fail = uv_async_init(uv_default_loop(), &m_typeDB, req_cb);
                assert(!fail);
                m_bProc = false;
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
            typedef std::function<void(CAsyncDBHandler &dbHandler) > DRowsetHeader;
            typedef std::function<void(CAsyncDBHandler &dbHandler, CDBVariantArray &vData) > DRows;

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

#ifdef NODE_JS_ADAPTER_PROJECT

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
             * @param batchHeader a callback for tracking returning batch start error messages
             * @param vPInfo a given array of parameter informations which may be empty to some of database management systems
             * @param plan a value for computing how included transactions should be rollback
             * @param discarded a callback for tracking socket closed or request canceled event
             * @param delimiter a case-sensitive delimiter string used for separating the batch SQL statements into individual SQL statements at server side for processing
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool ExecuteBatch(tagTransactionIsolation isolation, const wchar_t *sql, CDBVariantArray &vParam = CDBVariantArray(),
                    DExecuteResult handler = nullptr, DRows row = nullptr, DRowsetHeader rh = nullptr, DRowsetHeader batchHeader = nullptr,
                    const CParameterInfoArray& vPInfo = CParameterInfoArray(), tagRollbackPlan plan = rpDefault, DDiscarded discarded = nullptr,
                    const wchar_t *delimiter = L";", bool meta = true, bool lastInsertId = true) {
                bool rowset = (rh || row) ? true : false;
                if (!rowset) {
                    meta = false;
                }
                CScopeUQueue sb;
                sb << sql << delimiter << (int) isolation << (int) plan << rowset << meta << lastInsertId;

                UINT64 callIndex;
                bool queueOk = false;

                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
                CAutoLock alOne(m_csOneSending);
                if (vParam.size())
                    queueOk = GetAttachedClientSocket()->GetClientQueue().StartJob();
                {
                    if (!SendParametersData(vParam)) {
                        Clean();
                        return false;
                    }
                    callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock 
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset) {
                        m_mapRowset[callIndex] = CRowsetHandler(rh, row);
                    }
                    m_mapParameterCall[callIndex] = &vParam;
                    m_mapHandler[callIndex] = batchHeader;
                    sb << m_strConnection << m_flags;
                }
                sb << callIndex << vPInfo;
                ResultHandler arh = [callIndex, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecuteBatch, callIndex);
                };
                if (!SendRequest(idExecuteBatch, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    CAutoLock al(m_csDB);
                    m_mapParameterCall.erase(callIndex);
                    if (rowset) {
                        m_mapRowset.erase(callIndex);
                    }
                    m_mapHandler.erase(callIndex);
                    return false;
                }
                if (queueOk)
                    GetAttachedClientSocket()->GetClientQueue().EndJob();
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
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(CDBVariantArray &vParam, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(),
                    bool meta = true, bool lastInsertId = true, DDiscarded discarded = nullptr) {
                bool rowset = (rh || row) ? true : false;
                if (!rowset) {
                    meta = false;
                }
                CScopeUQueue sb;
                sb << rowset << meta << lastInsertId;

                UINT64 callIndex;
                bool queueOk = false;
                //make sure all parameter data sending and ExecuteParameters sending as one combination sending
                //to avoid possible request sending overlapping within multiple threading environment
                CAutoLock alOne(m_csOneSending);
                {
                    if (vParam.size()) {
                        queueOk = GetAttachedClientSocket()->GetClientQueue().StartJob();
                        if (!SendParametersData(vParam)) {
                            Clean();
                            return false;
                        }
                    }
                    callIndex = GetCallIndex();
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset) {
                        m_mapRowset[callIndex] = CRowsetHandler(rh, row);
                    }
                    m_mapParameterCall[callIndex] = &vParam;
                }
                sb << callIndex;
                ResultHandler arh = [callIndex, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecuteParameters, callIndex);
                };
                if (!SendRequest(idExecuteParameters, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    CAutoLock al(m_csDB);
                    m_mapParameterCall.erase(callIndex);
                    if (rowset) {
                        m_mapRowset.erase(callIndex);
                    }
                    return false;
                }
                if (queueOk)
                    GetAttachedClientSocket()->GetClientQueue().EndJob();
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
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(const wchar_t* sql, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(), bool meta = true, bool lastInsertId = true, DDiscarded discarded = nullptr) {
                bool rowset = (rh || row) ? true : false;
                if (!rowset) {
                    meta = false;
                }
                CScopeUQueue sb;
                CAutoLock alOne(m_csOneSending);
                UINT64 index = GetCallIndex();
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (rowset) {
                        m_mapRowset[index] = CRowsetHandler(rh, row);
                    }
                }
                sb << sql << rowset << meta << lastInsertId << index;
                ResultHandler arh = [index, handler, this](CAsyncResult & ar) {
                    this->Process(handler, ar, idExecute, index);
                };
                if (!SendRequest(idExecute, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    CAutoLock al(m_csDB);
                    m_mapRowset.erase(index);
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
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0, DDiscarded discarded = nullptr) {
                std::wstring s;
                CScopeUQueue sb;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    m_flags = flags;
                    if (strConnection) {
                        s = m_strConnection;
                        m_strConnection = strConnection;
                    }
                }
                sb << strConnection << flags;
                ResultHandler arh = [handler](CAsyncResult & ar) {
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
                if (SendRequest(UDB::idOpen, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    return true;
                }
                CAutoLock al(m_csDB);
                if (strConnection) {
                    m_strConnection = s;
                }
                return false;
            }

            /**
             * Send a parameterized SQL statement for preparing with a given array of parameter informations asynchronously
             * @param sql a parameterized SQL statement
             * @param handler a callback for SQL preparing result
             * @param vParameterInfo a given array of parameter informations
             * @param discarded a callback for tracking socket closed or request canceled event
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Prepare(const wchar_t *sql, DResult handler = nullptr, const CParameterInfoArray& vParameterInfo = CParameterInfoArray(), DDiscarded discarded = nullptr) {
                CScopeUQueue sb;
                ResultHandler arh = [handler](CAsyncResult & ar) {
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
                return SendRequest(idPrepare, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr);
            }

            /**
             * Notify connected remote server to close database connection string asynchronously
             * @param handler a callback for closing result, which should be OK always as long as there is network or queue available
             * @param discarded a callback for tracking socket closed or request canceled event
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Close(DResult handler = nullptr, DDiscarded discarded = nullptr) {
                ResultHandler arh = [handler](CAsyncResult & ar) {
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
                return SendRequest(idClose, (const unsigned char*) nullptr, (unsigned int) 0, arh, discarded, nullptr);
            }

            /**
             * Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param isolation a value for isolation
             * @param handler a callback for tracking its response result
             * @param discarded a callback for tracking socket closed or request canceled event
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool BeginTrans(tagTransactionIsolation isolation = tiReadCommited, DResult handler = nullptr, DDiscarded discarded = nullptr) {
                unsigned int flags;
                std::wstring connection;
                CScopeUQueue sb;
                ResultHandler arh = [handler](CAsyncResult & ar) {
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
                CAutoLock alOne(m_csOneSending);

                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    connection = m_strConnection;
                    flags = m_flags;
                }

                sb << (int) isolation << connection << flags;
                //associate begin transaction with underlying client persistent message queue
                m_queueOk = GetAttachedClientSocket()->GetClientQueue().StartJob();
                return SendRequest(idBeginTrans, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr);
            }

            /**
             * End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param plan a value for computing how included transactions should be rollback
             * @param handler a callback for tracking its response result
             * @param discarded a callback for tracking socket closed or request canceled event
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool EndTrans(tagRollbackPlan plan = rpDefault, DResult handler = nullptr, DDiscarded discarded = nullptr) {
                CScopeUQueue sb;
                sb << (int) plan;
                ResultHandler arh = [handler](CAsyncResult & ar) {
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
                CAutoLock alOne(m_csOneSending);

                if (SendRequest(idEndTrans, sb->GetBuffer(), sb->GetSize(), arh, discarded, nullptr)) {
                    if (m_queueOk) {
                        //associate end transaction with underlying client persistent message queue
                        GetAttachedClientSocket()->GetClientQueue().EndJob();
                        m_queueOk = false;
                    }
                    return true;
                }
                return false;
            }

        protected:

            virtual void OnAllProcessed() {
                CAutoLock al1(m_csDB);
                m_vData.clear();
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
                    for (auto it = m_mapParameterCall.begin(), end = m_mapParameterCall.end(); it != end; ++it) {
                        dbTo.m_mapParameterCall[it->first] = it->second;
                    }
                    m_mapParameterCall.clear();
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
                            cb(*this);
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
                        m_vData.clear();
                        {
                            unsigned int outputs = 0;
                            CAutoLock al(m_csDB);
                            mc >> m_vColInfo >> m_indexRowset;
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
                            header(*this);
                        }
                    }
                        break;
                    case idCallReturn:
                    {
                        CDBVariant vt;
                        mc >> vt;
                        CAutoLock al(m_csDB);
#ifdef NODE_JS_ADAPTER_PROJECT
                        m_vtRet = vt;
#endif
                        auto it = m_mapParameterCall.find(m_indexRowset);
                        if (it != m_mapParameterCall.end()) {
                            //crash? make sure that vParam is valid after calling the method Execute
                            CDBVariantArray &vParam = *(it->second);
                            size_t pos = m_parameters * m_indexProc + (m_nParamPos >> 16);
                            vParam[pos] = vt;
                        }
                        m_bCallReturn = true;
                    }
                        break;
                    case idBeginRows:
                        m_Blob.SetSize(0);
                        m_vData.clear();
                        if (mc.GetSize()) {
                            CAutoLock al(m_csDB);
                            mc >> m_indexRowset;
                        }
                        break;
                    case idTransferring:
                        if (mc.GetSize()) {
                            m_csDB.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csDB.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
                            while (mc.GetSize()) {
                                m_vData.push_back(CDBVariant());
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
                        }
                        break;
                    case idOutputParameter:
                    case idEndRows:
                        if (mc.GetSize() || m_vData.size()) {
                            m_csDB.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
                            m_csDB.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
                            CDBVariant vtOne;
                            while (mc.GetSize()) {
                                m_vData.push_back(vtOne);
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
                            DRows row;
                            if (reqId == idOutputParameter) {
                                {
                                    CAutoLock al(m_csDB);
#ifdef NODE_JS_ADAPTER_PROJECT
                                    m_bProc = true;
                                    auto it0 = m_mapRowset.find(m_indexRowset);
                                    if (it0 != m_mapRowset.end()) {
                                        row = it0->second.second;
                                    }
#endif
                                    if (m_lastReqId == idSqlBatchHeader) {
                                        if (!m_indexProc) {
                                            m_outputs += ((unsigned int) m_vData.size() + (unsigned int) m_bCallReturn);
                                        }
                                    } else {
                                        if (!m_outputs) {
                                            m_outputs = ((unsigned int) m_vData.size() + (unsigned int) m_bCallReturn);
                                        }
                                    }
#ifndef NODE_JS_ADAPTER_PROJECT
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
#ifdef NODE_JS_ADAPTER_PROJECT
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
                        m_vData.clear();
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
                            m_vData.push_back(CDBVariant());
                            CDBVariant &vt = m_vData.back();
                            m_Blob >> vt;
                            assert(m_Blob.GetSize() == 0);
                        }
                        break;
                    default:
                        CAsyncServiceHandler::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:

            void Process(DExecuteResult handler, CAsyncResult & ar, unsigned short reqId, UINT64 index) {
                INT64 affected;
                UINT64 fail_ok;
                int res;
                std::wstring errMsg;
                CDBVariant vtId;
                CAsyncDBHandler<serviceId> *ash = (CAsyncDBHandler<serviceId>*)ar.AsyncServiceHandler;
                ar >> affected >> res >> errMsg >> vtId >> fail_ok;
                {
                    SPA::CAutoLock al(m_csDB);
                    m_lastReqId = reqId;
                    m_affected = affected;
                    m_dbErrCode = res;
                    m_dbErrMsg = errMsg;
                    auto it = m_mapRowset.find(index);
                    if (it != m_mapRowset.end()) {
                        m_mapRowset.erase(it);
                    }
                    auto pit = m_mapParameterCall.find(index);
                    if (pit != m_mapParameterCall.end()) {
                        m_mapParameterCall.erase(pit);
                    }
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
                m_mapParameterCall.clear();
                m_mapHandler.clear();
                m_vColInfo.clear();
                m_lastReqId = 0;
                m_Blob.SetSize(0);
                if (m_Blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    m_Blob.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
                }
                m_vData.clear();
            }

        protected:
            CUCriticalSection m_csDB;
            CDBColumnInfoArray m_vColInfo;
            std::unordered_map<UINT64, CRowsetHandler> m_mapRowset;
            INT64 m_affected;
            int m_dbErrCode;
            std::wstring m_dbErrMsg;
            unsigned short m_lastReqId;
            UINT64 m_indexRowset;

        private:
            std::wstring m_strConnection;
            std::unordered_map<UINT64, CDBVariantArray*> m_mapParameterCall;
            unsigned int m_indexProc;
            CUQueue m_Blob;
            CDBVariantArray m_vData;
            tagManagementSystem m_ms;
            unsigned int m_flags;
            unsigned int m_parameters;
            unsigned int m_outputs;
            bool m_bCallReturn;
            CUCriticalSection m_csOneSending;
            bool m_queueOk;
            std::unordered_map<UINT64, DRowsetHeader> m_mapHandler;
            unsigned int m_nParamPos;

#ifdef NODE_JS_ADAPTER_PROJECT
            CDBVariant m_vtRet;
            bool m_bProc;

        public:

            UINT64 BeginTrans(Isolate* isolate, int args, Local<Value> *argv, tagTransactionIsolation isolation) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result;
                DDiscarded dd;
                if (args > 0) {
                    result = GetResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    dd = Get(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                return BeginTrans(isolation, result, dd) ? index : INVALID_NUMBER;
            }

            UINT64 Close(Isolate* isolate, int args, Local<Value> *argv) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result;
                DDiscarded dd;
                if (args > 0) {
                    result = GetResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    dd = Get(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                return Close(result, dd) ? index : INVALID_NUMBER;
            }

            UINT64 EndTrans(Isolate* isolate, int args, Local<Value> *argv, tagRollbackPlan plan) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result;
                DDiscarded dd;
                if (args > 0) {
                    result = GetResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    bool bad;
                    dd = Get(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                return EndTrans(plan, result, dd) ? index : INVALID_NUMBER;
            }

            UINT64 Execute(Isolate* isolate, int args, Local<Value> *argv, CDBVariantArray &vParam) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result;
                DDiscarded dd;
                DRows r;
                DRowsetHeader rh;
                if (args > 0) {
                    result = GetExecuteResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    r = GetRows(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                if (args > 2) {
                    rh = GetRowsetHeader(isolate, argv[2], bad);
                    if (bad) return 0;
                }
                if (args > 3) {
                    dd = Get(isolate, argv[3], bad);
                    if (bad) return 0;
                }
                return Execute(vParam, result, r, rh, true, true, dd) ? index : INVALID_NUMBER;
            }

            UINT64 Execute(Isolate* isolate, int args, Local<Value> *argv, const wchar_t *sql) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result;
                DDiscarded dd;
                DRows r;
                DRowsetHeader rh;
                if (args > 0) {
                    result = GetExecuteResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    r = GetRows(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                if (args > 2) {
                    rh = GetRowsetHeader(isolate, argv[2], bad);
                    if (bad) return 0;
                }
                if (args > 3) {
                    dd = Get(isolate, argv[3], bad);
                    if (bad) return 0;
                }
                return Execute(sql, result, r, rh, true, true, dd) ? index : INVALID_NUMBER;
            }

            UINT64 ExecuteBatch(Isolate* isolate, int args, Local<Value> *argv, tagTransactionIsolation isolation, const wchar_t *sql, CDBVariantArray &vParam, tagRollbackPlan plan, const wchar_t *delimiter, const CParameterInfoArray& vPInfo) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DExecuteResult result;
                DDiscarded dd;
                DRows r;
                DRowsetHeader rh;
                DRowsetHeader bh;
                if (args > 0) {
                    result = GetExecuteResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    r = GetRows(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                if (args > 2) {
                    rh = GetRowsetHeader(isolate, argv[2], bad);
                    if (bad) return 0;
                }
                if (args > 3) {
                    bh = GetBatchHeader(isolate, argv[3], bad);
                    if (bad) return 0;
                }
                if (args > 4) {
                    dd = Get(isolate, argv[4], bad);
                    if (bad) return 0;
                }
                return ExecuteBatch(isolation, sql, vParam, result, r, rh, bh, vPInfo, plan, dd, delimiter) ? index : INVALID_NUMBER;
            }

            UINT64 Open(Isolate* isolate, int args, Local<Value> *argv, const wchar_t* strConnection, unsigned int flags) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result;
                DDiscarded dd;
                if (args > 0) {
                    result = GetResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    dd = Get(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                return Open(strConnection, result, flags, dd) ? index : INVALID_NUMBER;
            }

            UINT64 Prepare(Isolate* isolate, int args, Local<Value> *argv, const wchar_t *sql, const CParameterInfoArray& vParameterInfo) {
                bool bad;
                SPA::UINT64 index = GetCallIndex();
                DResult result;
                DDiscarded dd;
                if (args > 0) {
                    result = GetResult(isolate, argv[0], bad);
                    if (bad) return 0;
                }
                if (args > 1) {
                    dd = Get(isolate, argv[1], bad);
                    if (bad) return 0;
                }
                return Prepare(sql, result, vParameterInfo, dd) ? index : INVALID_NUMBER;
            }

        protected:

            enum tagDBEvent {
                eResult = 0,
                eExecuteResult,
                eRowsetHeader,
                eRows,
                eBatchHeader,
                eDiscarded
            };

            struct DBCb {
                tagDBEvent Type;
                PUQueue Buffer;
                std::shared_ptr<CNJFunc> Func;
                std::shared_ptr<CDBVariantArray> VData;
            };

            std::deque<DBCb> m_deqDBCb; //protected by m_csDB;
            uv_async_t m_typeDB; //DB request events

            DRowsetHeader GetBatchHeader(Isolate* isolate, Local<Value> header, bool &bad) {
                bad = false;
                DRowsetHeader rh;
                if (header->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(header));
                    rh = [func](CAsyncDBHandler & db) {
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
                } else if (!header->IsNullOrUndefined()) {
                    NJA::ThrowException(isolate, "A callback expected for batch header");
                    bad = true;
                }
                return rh;
            }

            DRowsetHeader GetRowsetHeader(Isolate* isolate, Local<Value> header, bool &bad) {
                bad = false;
                DRowsetHeader rh;
                if (header->IsFunction()) {
                    std::shared_ptr<CNJFunc> func(new CNJFunc);
                    func->Reset(isolate, Local<Function>::Cast(header));
                    rh = [func](CAsyncDBHandler & db) {
                        DBCb cb;
                        cb.Type = eRowsetHeader;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        PAsyncDBHandler ash = &db;
                        *cb.Buffer << ash << db.GetColumnInfo();
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!header->IsNullOrUndefined()) {
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
                    rows = [func](CAsyncDBHandler &db, CDBVariantArray & vData) {
                        DBCb cb;
                        cb.Type = eRows;
                        cb.Func = func;
                        cb.Buffer = CScopeUQueue::Lock();
                        cb.VData.reset(new CDBVariantArray);
                        vData.swap(*cb.VData);
                        PAsyncDBHandler ash = &db;
                        bool proc = db.IsProc();
                        if (proc && db.GetCallReturn())
                            cb.VData->insert(cb.VData->begin(), db.GetRetValue());
                        *cb.Buffer << ash << proc;
                        CAutoLock al(ash->m_csDB);
                        ash->m_deqDBCb.push_back(cb);
                        int fail = uv_async_send(&ash->m_typeDB);
                        assert(!fail);
                    };
                } else if (!r->IsNullOrUndefined()) {
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
                } else if (!er->IsNullOrUndefined()) {
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
                } else if (!res->IsNullOrUndefined()) {
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
                } else if (!abort->IsNullOrUndefined()) {
                    NJA::ThrowException(isolate, "A callback expected for tracking socket closed or canceled events");
                    bad = true;
                }
                return dd;
            }

        private:
            static void req_cb(uv_async_t* handle);
#endif
        };
    } //namespace ClientSide
} //namespace SPA
#endif
