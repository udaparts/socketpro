
#ifndef _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_
#define _UDAPARTS_ASYNC_DATABASE_CLIENT_HANDLER_H_

#include "udatabase.h"
#include "aclientw.h"

namespace SPA {
    namespace ClientSide {
        using namespace UDB;

        template<unsigned int serviceId>
        class CAsyncDBHandler : public CAsyncServiceHandler {
            static const unsigned int ONE_MEGA_BYTES = 0x100000;

        public:

            CAsyncDBHandler(CClientSocket *cs = nullptr)
            : CAsyncServiceHandler(serviceId, cs),
            m_affected(-1), m_dbErrCode(0), m_lastReqId(0),
            m_nCall(0), m_indexRowset(0), m_indexProc(0), m_ms(msUnknown), m_flags(0),
            m_parameters(0), m_outputs(0) {
                m_Blob.Utf8ToW(true);
            }

            virtual ~CAsyncDBHandler() {
                CleanCallbacks();
            }

            typedef std::function<void(CAsyncDBHandler &dbHandler, int res, const std::wstring &errMsg) > DResult;
            typedef std::function<void(CAsyncDBHandler &dbHandler, int res, const std::wstring &errMsg, INT64 affected, UINT64 fail_ok, CDBVariant &vtId) > DExecuteResult;
            typedef std::function<void(CAsyncDBHandler &dbHandler) > DRowsetHeader;
            typedef std::function<void(CAsyncDBHandler &dbHandler, CDBVariantArray &vData) > DRows;
            typedef std::function<void(CAsyncDBHandler &dbHandler, tagUpdateEvent eventType, const wchar_t *instance, const wchar_t *dbPath, const wchar_t *tablePath, CDBVariant& rowId) > DUpdateEvent;

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

#ifdef WIN32_64

            /**
             * Check if the object will use high-precision time when saving or loading a ASCII string by VARIANT.
             * @return true if the object will use high-precision time, and false if the object will not
             */
            inline bool TimeEx() {
                CAutoLock al(m_csDB);
                return m_Blob.TimeEx();
            }

            /**
             * Enable or disable the object to use high-precision time when saving or loading a ASCII string by VARIANT.
             * @param timeEx true for enabling, and false for disabling
             */
            inline void TimeEx(bool timeEx) {
                CAutoLock al(m_csDB);
                m_Blob.TimeEx(timeEx);
            }
#endif

            /**
             * Set a callback for receiving table record add, delete and update as well as others events from remote server if this feature is enabled at server side
             * @param dbEvent
             */
            inline void SetDBEvent(DUpdateEvent dbEvent) {
                CAutoLock al(m_csDB);
                m_dbEvent = dbEvent;
            }

            inline DUpdateEvent GetDBEvent() {
                CAutoLock al(m_csDB);
                return m_dbEvent;
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
#ifdef WIN32_64
                    sb->TimeEx(true); //force sending high precision datetime
#endif
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
             * Process one or more sets of prepared statements with an array of parameter data asynchronously
             * @param vParam an array of parameter data which will be bounded to previously prepared parameters
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations. Note that there will be NO row set data or its column informations returned if NO such a callback is set
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number 
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(CDBVariantArray &vParam, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(), bool meta = true, bool lastInsertId = true) {
                UINT64 callIndex;
                bool rowset = rh ? true : false;
                if (!rowset) {
                    meta = false;
                }
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    if (!SendParametersData(vParam)) {
                        Clean();
                        return false;
                    }
                    ++m_nCall;
                    callIndex = m_nCall;
                    if (rowset) {
                        m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                    }
                    m_mapParameterCall[m_nCall] = &vParam;
                }
                if (!SendRequest(idExecuteParameters, rowset, meta, lastInsertId, callIndex, [callIndex, handler, this](CAsyncResult & ar) {
                        INT64 affected;
                        UINT64 fail_ok;
                                int res;
                                std::wstring errMsg;
                                CDBVariant vtId;
                                ar >> affected >> res >> errMsg >> vtId >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = idExecute;
                                this->m_affected = affected;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(callIndex);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        this->m_indexProc = 0;
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        auto pit = this->m_mapParameterCall.find(callIndex);
                        if (pit != this->m_mapParameterCall.end()) {
                            this->m_mapParameterCall.erase(pit);
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, affected, fail_ok, vtId);
                        }
                    })) {
                CAutoLock al(m_csDB);
                m_mapParameterCall.erase(callIndex);
                if (rowset) {
                    m_mapRowset.erase(callIndex);
                }
                return false;
            }
                return true;
            }

            /**
             * Process a complex SQL statement which may be combined with multiple basic SQL statements asynchronously.
             * @param sql a complex SQL statement which may be combined with multiple basic SQL statements
             * @param handler a callback for tracking final result
             * @param row a callback for receiving records of data
             * @param rh a callback for tracking row set of header column informations. Note that there will be NO row set data or its column informations returned if NO such a callback is set
             * @param meta a boolean for better or more detailed column meta details such as unique, not null, primary key, and so on
             * @param lastInsertId a boolean for last insert record identification number 
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Execute(const wchar_t* sql, DExecuteResult handler = DExecuteResult(), DRows row = DRows(), DRowsetHeader rh = DRowsetHeader(), bool meta = true, bool lastInsertId = true) {
                UINT64 index;
                bool rowset = rh ? true : false;
                if (!rowset) {
                    meta = false;
                }
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    index = ++m_nCall;
                    if (rowset) {
                        m_mapRowset[m_nCall] = CRowsetHandler(rh, row);
                    }
                }
                if (!SendRequest(idExecute, sql, rowset, meta, lastInsertId, index, [handler, this](CAsyncResult & ar) {
                        INT64 affected;
                        UINT64 fail_ok;
                                int res;
                                std::wstring errMsg;
                                CDBVariant vtId;
                                ar >> affected >> res >> errMsg >> vtId >> fail_ok;
                                this->m_csDB.lock();
                                this->m_lastReqId = idExecute;
                                this->m_affected = affected;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                auto it = this->m_mapRowset.find(this->m_indexRowset);
                        if (it != this->m_mapRowset.end()) {
                            this->m_mapRowset.erase(it);
                        }
                        if (!this->m_mapRowset.size()) {
                            this->m_nCall = 0;
                        }
                        this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg, affected, fail_ok, vtId);
                        }
                    })) {
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
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Open(const wchar_t* strConnection, DResult handler, unsigned int flags = 0) {
                std::wstring s;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    m_flags = flags;
                    if (strConnection) {
                        s = m_strConnection;
                        m_strConnection = strConnection;
                    }
                }
                if (SendRequest(UDB::idOpen, strConnection, flags, [handler, this](CAsyncResult & ar) {
                        int res, ms;
                        std::wstring errMsg;
                                ar >> res >> errMsg >> ms;
                                this->m_csDB.lock();
                                this->CleanRowset();
                                this->m_dbErrCode = res;
                                this->m_lastReqId = idOpen;
                        if (res == 0) {
                            this->m_strConnection = std::move(errMsg);
                                    errMsg.clear();
                        } else {
                            this->m_strConnection.clear();
                        }
                        this->m_dbErrMsg = errMsg;
                                this->m_ms = (tagManagementSystem) ms;
                                this->m_parameters = 0;
                                this->m_outputs = 0;
                                this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg);
                        }
                    })) {
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
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Prepare(const wchar_t *sql, DResult handler = DResult(), const CParameterInfoArray& vParameterInfo = CParameterInfoArray()) {
                if (!SendRequest(idPrepare, sql, vParameterInfo, [handler, this](CAsyncResult & ar) {
                        int res;
                        std::wstring errMsg;
                                unsigned int parameters;
                                ar >> res >> errMsg >> parameters;
                                this->m_csDB.lock();
                                this->m_lastReqId = idPrepare;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                this->m_parameters = (parameters & 0xffff);
                                this->m_outputs = (parameters >> 16);
                                this->m_indexProc = 0;
                                this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg);
                        }
                    })) {
                return false;
            }
                return true;
            }

            /**
             * Notify connected remote server to close database connection string asynchronously
             * @param handler a callback for closing result, which should be OK always as long as there is network or queue available 
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool Close(DResult handler = DResult()) {
                if (!SendRequest(idClose, [handler, this](CAsyncResult & ar) {
                        int res;
                        std::wstring errMsg;
                                ar >> res >> errMsg;
                                this->m_csDB.lock();
                                this->m_lastReqId = idClose;
                                this->m_strConnection.clear();
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                this->m_parameters = 0;
                                this->CleanRowset();
                                this->m_outputs = 0;
                                this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg);
                        }
                    })) {
                return false;
            }
                return true;
            }

            /**
             * Start a manual transaction with a given isolation asynchronously. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param isolation a value for isolation
             * @param handler a callback for tracking its response result
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool BeginTrans(tagTransactionIsolation isolation = tiReadCommited, DResult handler = DResult()) {
                unsigned int flags;
                std::wstring connection;
                {
                    //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock in case a client asynchronously sends lots of requests without use of client side queue.
                    CAutoLock al(m_csDB);
                    connection = m_strConnection;
                    flags = m_flags;
                }
                GetAttachedClientSocket()->GetClientQueue().StartJob();
                return SendRequest(idBeginTrans, (int) isolation, connection, flags, [handler, this](CAsyncResult & ar) {
                    int res, ms;
                    std::wstring errMsg;
                            ar >> res >> errMsg >> ms;
                            this->m_csDB.lock();
                            this->CleanRowset();
                    if (res == 0) {
                        this->m_strConnection = errMsg;
                                errMsg.clear();
                    }
                    this->m_lastReqId = idBeginTrans;
                            this->m_dbErrCode = res;
                            this->m_dbErrMsg = errMsg;
                            this->m_ms = (tagManagementSystem) ms;
                            this->m_csDB.unlock();
                    if (handler) {
                        handler(*this, res, errMsg);
                    }
                });
            }

            /**
             * End a manual transaction with a given rollback plan. Note the transaction will be associated with SocketPro client message queue if available to avoid possible transaction lose
             * @param plan a value for computing how included transactions should be rollback
             * @param handler a callback for tracking its response result
             * @return true if request is successfully sent or queued; and false if request is NOT successfully sent or queued
             */
            virtual bool EndTrans(tagRollbackPlan plan = rpDefault, DResult handler = DResult()) {
                if (SendRequest(idEndTrans, (int) plan, [handler, this](CAsyncResult & ar) {
                        int res;
                        std::wstring errMsg;
                                ar >> res >> errMsg;
                                this->m_csDB.lock();
                                this->m_lastReqId = idEndTrans;
                                this->m_dbErrCode = res;
                                this->m_dbErrMsg = errMsg;
                                this->CleanRowset();
                                this->m_csDB.unlock();
                        if (handler) {
                            handler(*this, res, errMsg);
                        }
                    })) {
                GetAttachedClientSocket()->GetClientQueue().EndJob();
                return true;
            }
                return false;
            }

        protected:

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case idRowsetHeader:
                    {
                        DRowsetHeader header;
                        m_Blob.SetSize(0);
                        if (m_Blob.GetMaxSize() > ONE_MEGA_BYTES) {
                            m_Blob.ReallocBuffer(ONE_MEGA_BYTES);
                        }
                        m_vData.clear();
                        {
                            CAutoLock al(m_csDB);
                            mc >> m_vColInfo >> m_indexRowset;
                            if (mc.GetSize()) {
                                mc >> m_outputs;
                            } else {
                                m_outputs = 0;
                            }
                            if (!m_outputs) {
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
#ifdef WIN32_64
                            bool timeEx = m_Blob.TimeEx();
#endif
                            m_csDB.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(true);
#endif
                            while (mc.GetSize()) {
                                m_vData.push_back(CDBVariant());
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(false);
#endif
                        }
                        break;
                    case idOutputParameter:
                    case idEndRows:
                        if (mc.GetSize() || m_vData.size()) {
                            m_csDB.lock();
                            bool Utf8ToW = m_Blob.Utf8ToW();
#ifdef WIN32_64
                            bool timeEx = m_Blob.TimeEx();
#endif
                            m_csDB.unlock();
                            if (Utf8ToW)
                                mc.Utf8ToW(true);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(true);
#endif
                            CDBVariant vtOne;
                            while (mc.GetSize()) {
                                m_vData.push_back(vtOne);
                                CDBVariant &vt = m_vData.back();
                                mc >> vt;
                            }
                            assert(mc.GetSize() == 0);
                            if (Utf8ToW)
                                mc.Utf8ToW(false);
#ifdef WIN32_64
                            if (timeEx)
                                mc.TimeEx(false);
#endif
                            if (reqId == idOutputParameter) {
                                {
                                    CAutoLock al(m_csDB);
                                    if (!m_outputs) {
                                        m_outputs = (unsigned int) m_vData.size();
                                    } else {
                                        assert(m_outputs == (unsigned int) m_vData.size());
                                    }
                                    auto it = m_mapParameterCall.find(m_indexRowset);
                                    if (it != m_mapParameterCall.end()) {
                                        //crash? make sure that vParam is valid after calling the method Execute
                                        CDBVariantArray &vParam = *(it->second);
                                        size_t pos = m_parameters * m_indexProc + (m_parameters - (unsigned int) m_vData.size());
                                        for (auto start = m_vData.begin(), end = m_vData.end(); start != end; ++start, ++pos) {
                                            vParam[pos] = std::move(*start);
                                        }
                                    }
                                    ++m_indexProc;
                                }
                            } else {
                                DRows row;
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
                            m_vData.push_back(CDBVariant());
                            CDBVariant &vt = m_vData.back();
                            m_Blob >> vt;
                            assert(m_Blob.GetSize() == 0);
                        }
                        break;
                    case idDBUpdate:
                        if (mc.GetSize()) {
                            int dbEventType;
                            std::wstring dbInstance, dbPath, tablePath;
                            CDBVariant idRow;
                            mc >> dbEventType >> dbInstance >> dbPath >> tablePath >> idRow;
                            m_csDB.lock();
                            DUpdateEvent dbEvent = m_dbEvent;
                            m_csDB.unlock();
                            if (dbEvent) {
                                dbEvent(*this, (tagUpdateEvent) dbEventType, dbInstance.c_str(), dbPath.c_str(), tablePath.c_str(), idRow);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        private:

            void Clean() {
                m_strConnection.clear();
                m_mapRowset.clear();
                m_vColInfo.clear();
                m_lastReqId = 0;
                m_Blob.SetSize(0);
                if (m_Blob.GetMaxSize() > DEFAULT_BIG_FIELD_CHUNK_SIZE) {
                    m_Blob.ReallocBuffer(DEFAULT_BIG_FIELD_CHUNK_SIZE);
                }
                m_vData.clear();
            }

            void CleanRowset(unsigned int size = 0) {
                if ((m_mapRowset.size() || m_vColInfo.size()) && GetAttachedClientSocket()->Sendable() && GetRequestsQueued() <= size && GetAttachedClientSocket()->GetClientQueue().GetMessageCount() <= size) {
                    m_mapRowset.clear();
                    m_vColInfo.clear();
                }
            }

        protected:
            CUCriticalSection m_csDB;
            CDBColumnInfoArray m_vColInfo;
            std::unordered_map<UINT64, CRowsetHandler> m_mapRowset;
            INT64 m_affected;
            int m_dbErrCode;
            std::wstring m_dbErrMsg;
            unsigned short m_lastReqId;
            UINT64 m_nCall;
            UINT64 m_indexRowset;

        private:
            std::wstring m_strConnection;
            std::unordered_map<UINT64, CDBVariantArray*> m_mapParameterCall;
            unsigned int m_indexProc;
            CUQueue m_Blob;
            CDBVariantArray m_vData;
            tagManagementSystem m_ms;
            DUpdateEvent m_dbEvent;
            unsigned int m_flags;
            unsigned int m_parameters;

            unsigned int m_outputs;
        };

    } //namespace ClientSide
} //namespace SPA
#endif
