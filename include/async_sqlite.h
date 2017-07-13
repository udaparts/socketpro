
#ifndef _UDAPARTS_ASYNC_SQLITE_HANDLER_H_
#define _UDAPARTS_ASYNC_SQLITE_HANDLER_H_

#include "sqlite/usqlite.h"
#include "udb_client.h"

namespace SPA {
    namespace ClientSide {
        typedef CAsyncDBHandler<SPA::Sqlite::sidSqlite> CSqliteBase;

        class CSqlite : public CSqliteBase {
            //no copy constructor supported
            CSqlite(const CSqlite& s);

            //no assignment operator supported
            CSqlite& operator=(const CSqlite& s);

        public:

            CSqlite(CClientSocket *cs = nullptr) : CSqliteBase(cs) {
            }

        public:
            typedef std::function<void(CSqlite &dbHandler, tagUpdateEvent eventType, const wchar_t *instance, const wchar_t *dbPath, const wchar_t *tablePath, CDBVariant& rowId) > DUpdateEvent;

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

        protected:

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
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
                        CSqliteBase::OnResultReturned(reqId, mc);
                        break;
                }
            }

        private:
            DUpdateEvent m_dbEvent;
        };
    } //namespace ClientSide
} //namespace SPA

#endif