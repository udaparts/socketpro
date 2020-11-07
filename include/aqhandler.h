
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
            typedef DQueueTrans DClose;
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
             * @param discarded A callback for tracking socket closed or request canceled event
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
             * @param key An ASCII string to identify a queue at server side
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             * @param discarded A callback for tracking socket closed or request canceled event
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
                return SendRequest(Queue::idStartTrans, rh, discarded, se, key);
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param rollback true for rollback, and false for committing
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             * @param discarded A callback for tracking socket closed or request canceled event
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
                bool ok = SendRequest(Queue::idEndTrans, rh, discarded, se, rollback);
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
             * @param key An ASCII string to identify a queue at server side
             * @param c A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @param discarded A callback for tracking socket closed or request canceled event
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
                return SendRequest(Queue::idClose, rh, discarded, se, key, permanent);
            }

            /**
             * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes.
             * Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string to identify a queue at server side
             * @param f A callback for tracking returning message count and queue file size in bytes
             * @param option One of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @param discarded A callback for tracking socket closed or request canceled event
             * @param se A callback for tracking an exception from server side
             * @return true for sending the request successfully, and false for failure
             */
            virtual bool FlushQueue(const char *key, const DFlush& f, tagOptimistic option = tagOptimistic::oMemoryCached, const DDiscarded& discarded = nullptr, const DServerException& se = nullptr) {
                DResultHandler rh;
                if (f) {
                    rh = [f](CAsyncResult & ar) {
                        UINT64 messageCount, fileSize;
                        ar >> messageCount >> fileSize;
                        f((CAsyncQueue*) ar.AsyncServiceHandler, messageCount, fileSize);
                    };
                }
                return SendRequest(Queue::idFlush, rh, discarded, se, key, (int) option);
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
             * @param key An ASCII string to identify a queue at server side
             * @param d A callback for tracking remaining message count within a server queue file, queue file size in bytes, messages and bytes dequeued within this batch
             * @param timeout A server side time-out number in milliseconds
             * @param discarded A callback for tracking socket closed or request canceled event
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
                return SendRequest(Queue::idDequeue, rh, discarded, se, key, timeout);
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
#ifdef HAVE_COROUTINE

            struct QWaiter : public CWaiter<int> {

                QWaiter(CAsyncQueue* aq, const char* key)
                : CWaiter<int>(Queue::idStartTrans) {
                    if (!aq->StartQueueTrans(key, get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idStartTrans);
                    }
                }

                QWaiter(CAsyncQueue* aq, bool rollback)
                : CWaiter<int>(Queue::idEndTrans) {
                    if (!aq->EndQueueTrans(rollback, get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idEndTrans);
                    }
                }

                QWaiter(CAsyncQueue* aq, const char* key, bool permanent)
                : CWaiter<int>(Queue::idClose) {
                    if (!aq->CloseQueue(key, get_rh(), permanent, get_aborted(), get_se())) {
                        aq->raise(Queue::idClose);
                    }
                }

            private:

                DClose get_rh() {
                    auto& wc = m_wc;
                    return [wc](CAsyncQueue * aq, int errCode) {
                        wc->m_r = errCode;
                        wc->resume();
                    };
                }
            };

            /**
             * Start enqueuing message with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param key An ASCII string to identify a queue at server side
             * @return A waiter for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             */
            QWaiter wait_startQueueTrans(const char* key) {
                return QWaiter(this, key);
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param rollback true for rollback, and false for committing
             * @return A waiter for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             */
            QWaiter wait_endQueueTrans(bool rollback = false) {
                return QWaiter(this, rollback);
            }

            /**
             * Try to close or delete a persistent queue opened at server side
             * @param key An ASCII string to identify a queue at server side
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             */
            QWaiter wait_closeQueue(const char* key, bool permanent = false) {
                return QWaiter(this, key, permanent);
            }

            struct InfoWaiter : public CWaiter<QueueInfo> {

                InfoWaiter(CAsyncQueue* aq, const char* key, tagOptimistic option)
                : CWaiter<QueueInfo>(Queue::idFlush) {
                    auto& wc = m_wc;
                    if (!aq->FlushQueue(key, [wc](CAsyncQueue * aq, UINT64 messages, UINT64 fileSize) {
                            wc->m_r.messages = messages;
                            wc->m_r.fSize = fileSize;
                            wc->resume();
                        }, option, get_aborted(), get_se())) {
                        aq->raise(Queue::idFlush);
                    }
                }
            };

            /**
             * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes.
             * Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string to identify a queue at server side
             * @param option One of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @return A waiter for for returning message count and queue file size in bytes
             */
            InfoWaiter wait_flushQueue(const char* key, tagOptimistic option = tagOptimistic::oMemoryCached) {
                return InfoWaiter(this, key, option);
            }

            struct DeqWaiter : public CWaiter<DeqInfo> {

                DeqWaiter(CAsyncQueue* aq, const char* key, unsigned int timeout)
                : CWaiter<DeqInfo>(Queue::idDequeue) {
                    auto& wc = m_wc;
                    if (!aq->Dequeue(key, [wc](CAsyncQueue * aq, UINT64 messages, UINT64 fileSize, unsigned int msgsDequeued, unsigned int bytes) {
                            wc->m_r.messages = messages;
                            wc->m_r.fSize = fileSize;
                            wc->m_r.DeMessages = msgsDequeued;
                            wc->m_r.DeBytes = bytes;
                            wc->resume();
                        }, timeout, get_aborted(), get_se())) {
                        aq->raise(Queue::idDequeue);
                    }
                }
            };

            /**
             * Dequeue messages from a persistent message queue file at server side in batch
             * @param key An ASCII string to identify a queue at server side
             * @param timeout A server side time-out number in milliseconds
             * @return A waiter for remaining message count within a server queue file, queue file size in bytes, messages and bytes dequeued within this batch
             */
            DeqWaiter wait_dequeue(const char* key, unsigned int timeout = 0) {
                return DeqWaiter(this, key, timeout);
            }

            struct KeysWaiter : public CWaiter<std::vector < std::string>>
            {

                KeysWaiter(CAsyncQueue * aq) : CWaiter<std::vector < std::string >> (Queue::idGetKeys) {
                    auto& wc = m_wc;
                    if (!aq->GetKeys([wc](CAsyncQueue * aq, std::vector<std::string>& v) {
                            wc->m_r.swap(v);
                            wc->resume();
                        }, get_aborted(), get_se())) {
                        aq->raise(Queue::idGetKeys);
                    }
                }
            };

            /**
             * Query queue keys opened at server side
             * @return A waiter for for an array of key names
             */
            KeysWaiter wait_getKeys() {
                return KeysWaiter(this);
            }

            struct EnqWaiter : public CWaiter<UINT64> {

                EnqWaiter(CAsyncQueue* aq, const char* key, const unsigned char* buffer, unsigned int size)
                : CWaiter<UINT64>(Queue::idEnqueueBatch) {
                    if (!aq->EnqueueBatch(key, buffer, size, get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idEnqueueBatch);
                    }
                }

                EnqWaiter(CAsyncQueue* aq, const char* key, CUQueue& q)
                : CWaiter<UINT64>(Queue::idEnqueueBatch) {
                    if (!aq->EnqueueBatch(key, q.GetBuffer(), q.GetSize(), get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idEnqueueBatch);
                    }
                    q.SetSize(0);
                }

                EnqWaiter(CAsyncQueue* aq, const char* key, unsigned short idMsg, const unsigned char* buffer, unsigned int size)
                : CWaiter<UINT64>(Queue::idEnqueue) {
                    if (!aq->Enqueue(key, idMsg, buffer, size, get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idEnqueueBatch);
                    }
                }

                EnqWaiter(CAsyncQueue* aq, const char* key, unsigned short idMsg)
                : CWaiter<UINT64>(Queue::idEnqueue) {
                    if (!aq->Enqueue(key, idMsg, (const unsigned char*) nullptr, (unsigned int) 0, get_rh(), get_aborted(), get_se())) {
                        aq->raise(Queue::idEnqueueBatch);
                    }
                }

            private:

                DEnqueue get_rh() {
                    auto& wc = m_wc;
                    return [wc](CAsyncQueue * aq, UINT64 index) {
                        wc->m_r = index;
                        wc->resume();
                    };
                }
            };

            /**
             * Enqueue a batch of messages in one single shot
             * @param key An ASCII string to identify a queue at server side
             * @param buffer A pointer to messages
             * @param size Buffer size in bytes
             * @return A waiter for the last message index at a server queue file
             */
            EnqWaiter wait_enqueueBatch(const char* key, const unsigned char* buffer, unsigned int size) {
                return EnqWaiter(this, key, buffer, size);
            }

            /**
             * Enqueue a batch of messages in one single shot
             * @param key An ASCII string to identify a queue at server side
             * @param q An instance of CUQueue containing a batch of messages
             * @return A waiter for the last message index at a server queue file
             * @remarks Calling the method will automatically set q size to zero if no exception happens
             */
            EnqWaiter wait_enqueueBatch(const char* key, CUQueue& q) {
                return EnqWaiter(this, key, q);
            }
#endif

            /**
             * Start enqueuing message with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param key An ASCII string to identify a queue at server side
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             */
            std::future<int> startQueueTrans(const char *key) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                if (!StartQueueTrans(key, get_ec(prom), get_aborted(prom, Queue::idStartTrans), get_se(prom))) {
                    raise(Queue::idStartTrans);
                }
                return prom->get_future();
            }

            /**
             * Query queue keys opened at server side
             * @return A future for for an array of key names
             */
            std::future<std::vector<std::string>> getKeys() {
                std::shared_ptr<std::promise<std::vector < std::string>> > prom(new std::promise<std::vector < std::string>>);
                if (!GetKeys([prom](CAsyncQueue* aq, std::vector<std::string>& v) {
                        prom->set_value(std::move(v));
                    }, get_aborted(prom, Queue::idGetKeys), get_se(prom))) {
                    raise(Queue::idGetKeys);
                }
                return prom->get_future();
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes 
             * @param rollback true for rollback, and false for committing
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             */
            std::future<int> endQueueTrans(bool rollback = false) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                if (!EndQueueTrans(rollback, get_ec(prom), get_aborted(prom, Queue::idEndTrans), get_se(prom))) {
                    raise(Queue::idEndTrans);
                }
                return prom->get_future();
            }

            /**
             * Try to close or delete a persistent queue opened at server side
             * @param key An ASCII string to identify a queue at server side
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @return A future for returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             */
            std::future<int> closeQueue(const char *key, bool permanent = false) {
                std::shared_ptr<std::promise<int> > prom(new std::promise<int>);
                if (!CloseQueue(key, get_ec(prom), permanent, get_aborted(prom, Queue::idClose), get_se(prom))) {
                    raise(Queue::idClose);
                }
                return prom->get_future();
            }

            /**
             * Flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes.
             * Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string to identify a queue at server side
             * @param option One of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @return A future for for returning message count and queue file size in bytes
             */
            std::future<QueueInfo> flushQueue(const char *key, tagOptimistic option = tagOptimistic::oMemoryCached) {
                std::shared_ptr<std::promise<QueueInfo> > prom(new std::promise<QueueInfo>);
                if (!FlushQueue(key, [prom](CAsyncQueue* aq, UINT64 messages, UINT64 fileSize) {
                        prom->set_value(QueueInfo(messages, fileSize));
                    }, option, get_aborted(prom, Queue::idFlush), get_se(prom))) {
                    raise(Queue::idFlush);
                }
                return prom->get_future();
            }

            /**
             * Dequeue messages from a persistent message queue file at server side in batch
             * @param key An ASCII string to identify a queue at server side
             * @param timeout A server side time-out number in milliseconds
             * @return A future for remaining message count within a server queue file, queue file size in bytes, messages and bytes dequeued within this batch
             */
            std::future<DeqInfo> dequeue(const char *key, unsigned int timeout = 0) {
                std::shared_ptr<std::promise<DeqInfo> > prom(new std::promise<DeqInfo>);
                if (!Dequeue(key, [prom](CAsyncQueue* aq, UINT64 messages, UINT64 fileSize, unsigned int msgsDequeued, unsigned int bytes) {
                        prom->set_value(DeqInfo(messages, fileSize, msgsDequeued, bytes));
                    }, timeout, get_aborted(prom, Queue::idDequeue), get_se(prom))) {
                    raise(Queue::idDequeue);
                }
                return prom->get_future();
            }

            /**
             * Enqueue a batch of messages in one single shot
             * @param key An ASCII string to identify a queue at server side
             * @param buffer A pointer to messages
             * @param size Buffer size in bytes
             * @return A future for the last message index at a server queue file
             */
            std::future<UINT64> enqueueBatch(const char* key, const unsigned char* buffer, unsigned int size) {
                std::shared_ptr<std::promise<UINT64> > prom(new std::promise<UINT64>);
                if (!EnqueueBatch(key, buffer, size, [prom](CAsyncQueue* aq, UINT64 index) {
                        prom->set_value(index);
                    }, get_aborted(prom, Queue::idEnqueueBatch), get_se(prom))) {
                    raise(Queue::idEnqueueBatch);
                }
                return prom->get_future();
            }

            /**
             * Enqueue a batch of messages in one single shot
             * @param key An ASCII string to identify a queue at server side
             * @param q An instance of CUQueue containing a batch of messages
             * @return A future for the last message index at a server queue file
             * @remarks Calling the method will automatically set q size to zero if no exception happens
             */
            std::future<UINT64> enqueueBatch(const char* key, CUQueue& q) {
                std::future<UINT64> f = enqueueBatch(key, q.GetBuffer(), q.GetSize());
                q.SetSize(0);
                return f;
            }

        private:
            static DClose get_ec(const std::shared_ptr<std::promise<int> >& prom) {
                return [prom](CAsyncQueue* aq, int errCode) {
                    prom->set_value(errCode);
                };
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
                return NULL_RH;
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

            template<typename ...Ts>
            static void BatchMessage(unsigned short idMessage, CUQueue& q, const Ts& ...t) {
                SPA::CScopeUQueue sb;
                sb->Save(t ...);
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
                return SendRequest(Queue::idEnqueue, GetRH(e), discarded, se, key, idMessage);
            }

            template<typename ...Ts>
            bool Enqueue(const char* key, unsigned short idMessage, const DEnqueue& e, const DDiscarded& discarded, const DServerException& se, const Ts& ...t) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Save(t ...);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, se);
            }

            template<typename ...Ts>
            bool Enqueue(const char* key, unsigned short idMessage, const DEnqueue& e, const DDiscarded& discarded, const Ts& ...t) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Save(t ...);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), discarded, NULL_SE);
            }

            template<typename ...Ts>
            bool Enqueue(const char* key, unsigned short idMessage, const DEnqueue& e, const Ts& ...t) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Save(t ...);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e), NULL_ABORTED, NULL_SE);
            }

            template<typename ...Ts>
            bool Enqueue(const char* key, unsigned short idMessage, const Ts& ...t) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Save(t ...);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), NULL_RH, NULL_ABORTED, NULL_SE);
            }

        protected:

            virtual void OnBaseRequestprocessed(unsigned short reqId) {
                switch (reqId) {
                    case (unsigned short)tagBaseRequestID::idMessageQueued:
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