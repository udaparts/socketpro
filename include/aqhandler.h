
#ifndef _UDAPARTS_ASYNC_QUEUE_CLIENT_IMPL_H_
#define _UDAPARTS_ASYNC_QUEUE_CLIENT_IMPL_H_

#include "queue/uasyncqueue.h"
#include "aclientw.h"

namespace SPA {
    namespace ClientSide {

        /**
         * A client side class for easy accessing remote persistent message queues by use of SocketPro communication framework
         */
        class CAsyncQueue : public CAsyncServiceHandler {
        public:

            CAsyncQueue(CClientSocket *pClientSocket, unsigned int sid = SPA::Queue::sidQueue) : CAsyncServiceHandler(sid, pClientSocket), m_nBatchSize(0), MessageQueued(m_mq) {
                m_mq.SetCS(&m_csQ);
            }

            //callback definitions
            typedef std::function<void(CAsyncQueue *aq, int errCode) > DQueueTrans;
            typedef std::function<void(CAsyncQueue *aq, std::vector<std::string>& v) > DGetKeys;
            typedef std::function<void(CAsyncQueue *aq, UINT64 messageCount, UINT64 fileSize) > DFlush;
            typedef std::function<void(CAsyncQueue *aq, UINT64 indexMessage) > DEnqueue;
            typedef std::function<void(CAsyncQueue *aq, int errCode) > DClose;
            typedef std::function<void(CAsyncQueue *aq, UINT64 messageCount, UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) > DDequeue;

        public:

            /**
             * Dequeue batch size in bytes
             */
            unsigned int GetDequeueBatchSize() {
                CSpinAutoLock al(m_csQ);
                return (m_nBatchSize & 0xffffff);
            }

            /**
             * Check if remote queue server is able to automatically notify a client when a message is enqueued at server side
             */
            bool GetEnqueueNotified() {
                CSpinAutoLock al(m_csQ);
                return ((m_nBatchSize >> 24) == 0);
            }

            /**
             * Get last dequeue callback
             */
            DDequeue GetLastDequeueCallback() {
                CSpinAutoLock al(m_csQ);
                return m_dDequeue;
            }

            /**
             * set last dequeue callback
             */
            void SetLastDequeueCallback(DDequeue deq = nullptr) {
                CSpinAutoLock al(m_csQ);
                m_dDequeue = deq;
            }

            /**
             * Query queue keys opened at server side
             * @param gk A callback for tracking an array of key names
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool GetKeys(const DGetKeys & gk, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                if (gk) {
                    rh = [gk](CAsyncResult & ar) {
                        unsigned int size;
                        std::vector<std::string> v;
                        ar >> size;
                        for (unsigned int n = 0; n < size; ++n) {
                            std::string s;
                            ar >> s;
                            v.push_back(s);
                        }
                        gk((CAsyncQueue*) ar.AsyncServiceHandler, v);
                    };
                }
                return SendRequest(Queue::idGetKeys, rh, discarded, se);
            }

            /**
             * Start enqueuing message with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param key An ASCII string for identifying a queue at server side
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool StartQueueTrans(const char *key, const DQueueTrans& qt = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                IClientQueue &cq = GetSocket()->GetClientQueue();
                if (cq.IsAvailable()) {
                    bool ok = cq.StartJob();
                    assert(ok);
                }
                DResultHandler rh;
                if (qt) {
                    rh = [qt](CAsyncResult & ar) {
                        int errCode;
                        ar >> errCode;
                        qt((CAsyncQueue*) ar.AsyncServiceHandler, errCode);
                    };
                }
                return SendRequest(Queue::idStartTrans, key, rh, discarded, se);
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param rollback true for rollback, and false for committing
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool EndQueueTrans(bool rollback = false, const DQueueTrans& qt = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                if (qt) {
                    rh = [qt](CAsyncResult & ar) {
                        int errCode;
                        ar >> errCode;
                        qt((CAsyncQueue*) ar.AsyncServiceHandler, errCode);
                    };
                }
                bool ok = SendRequest(Queue::idEndTrans, rollback, rh, discarded, se);
                IClientQueue &cq = GetSocket()->GetClientQueue();
                if (cq.IsAvailable()) {
                    bool ok;
                    if (rollback)
                        ok = cq.AbortJob();
                    else
                        ok = cq.EndJob();
                    assert(ok);
                }
                return ok;
            }

            /**
             * Try to close or delete a persistent queue opened at server side
             * @param key An ASCII string for identifying a queue at server side
             * @param c A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool CloseQueue(const char *key, const DClose& c = nullptr, bool permanent = false, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                if (c) {
                    rh = [c](CAsyncResult & ar) {
                        int errCode;
                        ar >> errCode;
                        c((CAsyncQueue*) ar.AsyncServiceHandler, errCode);
                    };
                }
                return SendRequest(Queue::idClose, key, permanent, rh, discarded, se);
            }

            /**
             * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string for identifying a queue at server side
             * @param f A callback for tracking returning message count and queue file size in bytes
             * @param option One of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool FlushQueue(const char *key, const DFlush& f, tagOptimistic option = oMemoryCached, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                if (f) {
                    rh = [f](CAsyncResult & ar) {
                        UINT64 messageCount, fileSize;
                        ar >> messageCount >> fileSize;
                        f((CAsyncQueue*) ar.AsyncServiceHandler, messageCount, fileSize);
                    };
                }
                return SendRequest(Queue::idFlush, key, (int) option, rh, discarded, se);
            }

            struct QueueInfo {

                QueueInfo(UINT64 message_count = 0, UINT64 file_size = 0) : messages(message_count), fSize(file_size) {
                }

                /**
                 * The messages remaining in server message queue file
                 */
                UINT64 messages;

                /**
                 * server message queue file in bytes
                 */
                UINT64 fSize;

                virtual std::wstring ToString() {
                    std::wstring s = L"messages: " + std::to_wstring(messages);
                    s += L", fsize: " + std::to_wstring(fSize);
                    return s;
                }
            };

            /**
             * Dequeue messages from a persistent message queue file at server side in batch
             * @param key An ASCII string for identifying a queue at server side
             * @param d A callback for tracking remaining message count within a server queue file, queue file size in bytes, messages and bytes dequeued within this batch
             * @param timeout A server side time-out number in milliseconds
             * @param discarded A callback for tracking socket closed or request cancelled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool Dequeue(const char *key, const DDequeue& d, unsigned int timeout = 0, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                {
                    CSpinAutoLock al(m_csQ);
                    m_keyDequeue = key ? key : "";
                    if (d) {
                        rh = [d](CAsyncResult & ar) {
                            UINT64 messageCount, fileSize, ret;
                            ar >> messageCount >> fileSize >> ret;
                            unsigned int messages = (unsigned int) ret;
                            unsigned int bytes = (unsigned int) (ret >> 32);
                            d((CAsyncQueue*) ar.AsyncServiceHandler, messageCount, fileSize, messages, bytes);
                        };
                        m_dDequeue = d;
                    } else {
                        m_dDequeue = nullptr;
                    }
                }
                return SendRequest(Queue::idDequeue, key, timeout, rh, discarded, se);
            }

            struct DeqInfo : public QueueInfo {

                DeqInfo(UINT64 messages = 0, UINT64 fSize = 0, unsigned int msgs = 0, unsigned int bytes = 0)
                : QueueInfo(messages, fSize), DeMessages(msgs), DeBytes(bytes) {
                }

                /**
                 * messages dequeued from server by this request Dequeue
                 */
                unsigned int DeMessages;

                /**
                 * bytes dequeued from server by this request Dequeue
                 */
                unsigned int DeBytes;

                std::wstring ToString() {
                    std::wstring s = QueueInfo::ToString();
                    s += L", msgsDequeued: " + std::to_wstring(DeMessages);
                    s += L", bytes: " + std::to_wstring(DeBytes);
                    return s;
                }
            };
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
#else
#ifdef HAVE_FUTURE

            /**
             * Query queue keys opened at server side
             * @param key An ASCII string for identifying a queue at server side
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             */
            virtual std::future<int> startQueueTrans(const char *key) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                DQueueTrans qt = [prom](CAsyncQueue *aq, int errCode) {
                    prom->set_value(errCode);
                };
                if (!StartQueueTrans(key, qt, get_aborted(prom, L"StartQueueTrans", Queue::idStartTrans), get_se(prom))) {
                    raise(L"StartQueueTrans", Queue::idStartTrans);
                }
                return prom->get_future();
            }

            /**
             * Query queue keys opened at server side
             * @return A future for for an array of key names
             */
            virtual std::future<std::vector<std::string>> getKeys() {
                std::shared_ptr<std::promise<std::vector < std::string>> > prom(new std::promise<std::vector < std::string>>);
                DGetKeys gk = [prom](CAsyncQueue *aq, std::vector<std::string> &v) {
                    prom->set_value(std::move(v));
                };
                if (!GetKeys(gk, get_aborted(prom, L"GetKeys", Queue::idGetKeys), get_se(prom))) {
                    raise(L"GetKeys", Queue::idGetKeys);
                }
                return prom->get_future();
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes 
             * @param rollback true for rollback, and false for committing
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             */
            virtual std::future<int> endQueueTrans(bool rollback = false) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                DQueueTrans qt = [prom](CAsyncQueue *aq, int errCode) {
                    prom->set_value(errCode);
                };
                if (!EndQueueTrans(rollback, qt, get_aborted(prom, L"EndQueueTrans", Queue::idEndTrans), get_se(prom))) {
                    raise(L"EndQueueTrans", Queue::idEndTrans);
                }
                return prom->get_future();
            }

            /**
             * Try to close or delete a persistent queue opened at server side
             * @param key An ASCII string for identifying a queue at server side
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             */
            virtual std::future<int> closeQueue(const char *key, bool permanent = false) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                DClose c = [prom](CAsyncQueue *aq, int errCode) {
                    prom->set_value(errCode);
                };
                if (!CloseQueue(key, c, permanent, get_aborted(prom, L"CloseQueue", Queue::idClose), get_se(prom))) {
                    raise(L"CloseQueue", Queue::idClose);
                }
                return prom->get_future();
            }

            /**
             * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string for identifying a queue at server side
             * @param option One of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @return A future for for returning message count and queue file size in bytes
             */
            virtual std::future<QueueInfo> flushQueue(const char *key, tagOptimistic option = oMemoryCached) {
                std::shared_ptr<std::promise<QueueInfo> > prom(new std::promise<QueueInfo>);
                DFlush f = [prom](CAsyncQueue *aq, UINT64 messages, UINT64 fileSize) {
                    prom->set_value(QueueInfo(messages, fileSize));
                };
                if (!FlushQueue(key, f, option, get_aborted(prom, L"FlushQueue", Queue::idFlush), get_se(prom))) {
                    raise(L"FlushQueue", Queue::idFlush);
                }
                return prom->get_future();
            }

            /**
             * Dequeue messages from a persistent message queue file at server side in batch
             * @param key An ASCII string for identifying a queue at server side
             * @param timeout A server side time-out number in milliseconds
             * @return A future for remaining message count within a server queue file, queue file size in bytes, messages and bytes dequeued within this batch
             */
            virtual std::future<DeqInfo> dequeue(const char *key, unsigned int timeout = 0) {
                std::shared_ptr<std::promise<DeqInfo> > prom(new std::promise<DeqInfo>);
                DDequeue d = [prom](CAsyncQueue *aq, UINT64 messages, UINT64 fileSize, unsigned int msgsDequeued, unsigned int bytes) {
                    prom->set_value(DeqInfo(messages, fileSize, msgsDequeued, bytes));
                };
                if (!Dequeue(key, d, timeout, get_aborted(prom, L"Dequeue", Queue::idDequeue), get_se(prom))) {
                    raise(L"Dequeue", Queue::idDequeue);
                }
                return prom->get_future();
            }
#endif
#endif
        private:

            inline static DResultHandler GetRH(const DEnqueue & e) {
                if (e) {
#if defined(PHP_ADAPTER_PROJECT) || defined(NODE_JS_ADAPTER_PROJECT)
                    return [e](CAsyncResult & ar) {
#else
                    return [&e](CAsyncResult & ar) {
#endif
                        UINT64 index;
                        ar >> index;
                        e((CAsyncQueue*) ar.AsyncServiceHandler, index);
                    };
                }
                return nullptr;
            }

        public:

            static void BatchMessage(unsigned short idMessage, const unsigned char *buffer, unsigned int size, CUQueue &q) {
#ifndef NDEBUG
                if (!buffer) {
                    assert(size == 0);
                }
#endif
                if (q.GetSize() == 0) {
                    unsigned int count = 1;
                    q << count;
                } else {
                    unsigned int *count = (unsigned int*) q.GetBuffer();
                    *count += 1;
                }
                q << idMessage << size;
                q.Push(buffer, size);
            }

            static void BatchMessage(unsigned short idMessage, CUQueue &q) {
                BatchMessage(idMessage, (const unsigned char*) nullptr, 0, q);
            }

            template<typename T0>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            static void BatchMessage(unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, CUQueue &q) {
                SPA::CScopeUQueue sb;
                sb << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                BatchMessage(idMessage, sb->GetBuffer(), sb->GetSize(), q);
            }

            bool EnqueueBatch(const char *key, CUQueue &q, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                if (EnqueueBatch(key, q.GetBuffer(), q.GetSize(), e, discarded, se)) {
                    q.SetSize(0);
                    return true;
                }
                return false;
            }

            virtual bool EnqueueBatch(const char *key, const unsigned char *buffer, unsigned int size, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                if (!buffer || size < 2 * sizeof (unsigned int) + sizeof (unsigned short)) {
                    //bad operation because no message batched yet!
                    assert(false);
                    return false;
                }
                CScopeUQueue sb;
                sb << key;
                sb->Push(buffer, size);
                return SendRequest(Queue::idEnqueueBatch, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            bool Enqueue(const char *key, unsigned short idMessage, const unsigned char *buffer, unsigned int size, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Push(buffer, size);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            bool Enqueue(const char *key, unsigned short idMessage, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                return SendRequest(Queue::idEnqueue, key, idMessage, GetRH(e), discarded, se);
            }

            template<typename T0>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const DEnqueue& e = nullptr, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

        protected:

            virtual void OnBaseRequestprocessed(unsigned short reqId) {
                switch (reqId) {
                    case SPA::idMessageQueued:
                    {
                        m_csQ.lock();
                        auto key = m_keyDequeue;
                        auto d = m_dDequeue;
                        m_csQ.unlock();
                        if (d) {
                            //we automatically send a request to dequeue messages after a notification message arrives that a new message is enqueued at server side
                            Dequeue(key.c_str(), d, 0);
                        }
                    }
                        m_mq.Invoke(this);
                        break;
                    default:
                        break;
                }
            }

            virtual void OnResultReturned(unsigned short reqId, CUQueue &mc) {
                switch (reqId) {
                    case Queue::idClose:
                    case Queue::idEnqueue:
                        mc.SetSize(0);
                        break;
                    case Queue::idBatchSizeNotified:
                        m_csQ.lock();
                        mc >> m_nBatchSize;
                        m_csQ.unlock();
                        break;
                    default:
                        break;
                }
            }

        private:
            CSpinLock m_csQ;
            unsigned int m_nBatchSize; //protected by m_csQ
            std::string m_keyDequeue; //protected by m_csQ
            DDequeue m_dDequeue; //protected by m_csQ

        public:
#ifndef SAFE_RESULT_RETURN_EVENT
            typedef std::function<void(CAsyncQueue *aq) > DMessageQueued;
#else
            typedef void(*DMessageQueued)(CAsyncQueue *aq);
#endif
        private:
            IUDelImpl<DMessageQueued> m_mq;

        public:
            /**
             * An event for tracking message queued notification from server side
             */
            IUDel<DMessageQueued> &MessageQueued;
        };

        typedef CSocketPool<CAsyncQueue> CAsyncQueuePool;
    } //namespace ClientSide
} //namespace SPA

#endif