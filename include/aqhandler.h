
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

            CAsyncQueue(CClientSocket *pClientSocket, unsigned int sid = SPA::Queue::sidQueue) : CAsyncServiceHandler(sid, pClientSocket), m_nBatchSize(0) {
            }

            //callback definitions
            typedef std::function<void(int errCode) > DQueueTrans;
            typedef std::function<void(std::vector<std::string>& v) > DGetKeys;
            typedef std::function<void(UINT64 messageCount, UINT64 fileSize) > DFlush;
            typedef std::function<void(UINT64 indexMessage) > DEnqueue;
            typedef std::function<void(int errCode) > DClose;
            typedef std::function<void(UINT64 messageCount, UINT64 fileSize, unsigned int messagesDequeuedInBatch, unsigned int bytesDequeuedInBatch) > DDequeue;
            typedef std::function<void() > DMessageQueued;

        public:

            /**
             * Dequeue batch size in bytes
             */
            unsigned int GetDequeueBatchSize() const {
                return (m_nBatchSize & 0xffffff);
            }

            /**
             * Check if remote queue server is able to automatically notify a client when a message is enqueued at server side
             */
            bool GetEnqueueNotified() const {
                return ((m_nBatchSize >> 24) == 0);
            }

            /**
             * Last dequeue callback
             */
            DDequeue GetLastDequeueCallback() {
                CAutoLock al(m_csQ);
                return m_dDequeue;
            }

            /**
             * Query queue keys opened at server side
             * @param gk A callback for tracking a list of key names
             * @return true for sending the request successfully, and false for failure
             */
            bool GetKeys(const DGetKeys & gk) {
                return SendRequest(Queue::idGetKeys, [gk, this](CAsyncResult & ar) {
                    if (gk) {
                        unsigned int size;
                        std::vector<std::string> v;
                        ar >> size;
                        for (unsigned int n = 0; n < size; ++n) {
                            std::string s;
                            ar >> s;
                            v.push_back(s);
                        }
                        gk(v);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                });
            }

            /**
             * Start enqueuing message with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param key An ASCII string for identifying a queue at server side
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_ALREADY_STARTED, and so on
             * @return true for sending the request successfully, and false for failure
             */
            bool StartQueueTrans(const char *key, const DQueueTrans &qt) {
                return SendRequest(Queue::idStartTrans, key, [qt](CAsyncResult & ar) {
                    if (qt) {
                        int errCode;
                        ar >> errCode;
                        qt(errCode);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                });
            }

            /**
             * End enqueuing messages with transaction style. Currently, total size of queued messages must be less than 4 G bytes
             * @param rollback true for rollback, and false for committing
             * @param qt A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_TRANS_NOT_STARTED_YET, and so on
             * @return true for sending the request successfully, and false for failure
             */
            bool EndQueueTrans(bool rollback = false, const DQueueTrans &qt = DQueueTrans()) {
                return SendRequest(Queue::idEndTrans, rollback, [qt](CAsyncResult & ar) {
                    if (qt) {
                        int errCode;
                        ar >> errCode;
                        qt(errCode);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                });
            }

            /**
             * Try to close or delete a persistent queue opened at server side
             * @param key An ASCII string for identifying a queue at server side
             * @param c A callback for tracking returning error code, which can be one of QUEUE_OK, QUEUE_DEQUEUING, and so on
             * @param permanent true for deleting a queue file, and false for closing a queue file
             * @return true for sending the request successfully, and false for failure
             */
            bool CloseQueue(const char *key, const DClose &c = DClose(), bool permanent = false) {
                return SendRequest(Queue::idClose, key, permanent, [c](CAsyncResult & ar) {
                    if (c) {
                        int errCode;
                        ar >> errCode;
                        c(errCode);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                });
            }

            /**
             * May flush memory data into either operation system memory or hard disk, and return message count and queue file size in bytes. Note the method only returns message count and queue file size in bytes if the option is oMemoryCached
             * @param key An ASCII string for identifying a queue at server side
             * @param f A callback for tracking returning message count and queue file size in bytes
             * @param option one of options, oMemoryCached, oSystemMemoryCached and oDiskCommitted
             * @return true for sending the request successfully, and false for failure
             */
            bool FlushQueue(const char *key, const DFlush & f, tagOptimistic option = oMemoryCached) {
                return SendRequest(Queue::idFlush, key, (int) option, [f, this](CAsyncResult & ar) {
                    if (f) {
                        UINT64 messageCount, fileSize;
                        ar >> messageCount >> fileSize;
                        f(messageCount, fileSize);
                    } else {
                        ar.UQueue.SetSize(0);
                    }
                });
            }

            /**
             * Dequeue messages from a persistent message queue file at server side in batch
             * @param key An ASCII string for identifying a queue at server side
             * @param d A callback for tracking data like remaining message count within a server queue file, queue file size in bytes, message dequeued within this batch and bytes dequeued within this batch
             * @param timeout A time-out number in milliseconds
             * @return true for sending the request successfully, and false for failure
             */
            bool Dequeue(const char *key, const DDequeue & d, unsigned int timeout = 0) {
                ResultHandler rh;
                m_csQ.lock();
                m_keyDequeue = key ? key : "";
                if (d) {
                    rh = [d, this](CAsyncResult & ar) {
                        UINT64 messageCount, fileSize, ret;
                        ar >> messageCount >> fileSize >> ret;
                        unsigned int messages = (unsigned int) ret;
                        unsigned int bytes = (unsigned int) (ret >> 32);
                        d(messageCount, fileSize, messages, bytes);
                    };
                    m_dDequeue = d;
                } else {
                    m_dDequeue = DDequeue();
                }
                m_csQ.unlock();
                return SendRequest(Queue::idDequeue, key, timeout, rh);
            }

        private:

            inline ResultHandler GetRH(const DEnqueue & e) {
                ResultHandler rh;
                if (e) {
                    rh = [e](CAsyncResult & ar) {
                        UINT64 index;
                        ar >> index;
                        e(index);
                    };
                }
                return rh;
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

            bool EnqueueBatch(const char *key, CUQueue &q, const DEnqueue & e = DEnqueue()) {
                if (q.GetSize() < 2 * sizeof (unsigned int)) {
                    //bad operation!
                    assert(false);
                    return false;
                }
                CScopeUQueue sb;
                sb << key;
                sb->Push(q.GetBuffer(), q.GetSize());
                q.SetSize(0);
                return SendRequest(Queue::idEnqueueBatch, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            bool Enqueue(const char *key, unsigned short idMessage, const unsigned char *buffer, unsigned int size, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage;
                sb->Push(buffer, size);
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            bool Enqueue(const char *key, unsigned short idMessage, const DEnqueue & e = DEnqueue()) {
                return SendRequest(Queue::idEnqueue, key, idMessage, GetRH(e));
            }

            template<typename T0>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
            }

            template<typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
            bool Enqueue(const char *key, unsigned short idMessage, const T0 &t0, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6, const T7 &t7, const T8 &t8, const T9 &t9, const DEnqueue & e = DEnqueue()) {
                CScopeUQueue sb;
                sb << key << idMessage << t0 << t1 << t2 << t3 << t4 << t5 << t6 << t7 << t8 << t9;
                return SendRequest(Queue::idEnqueue, sb->GetBuffer(), sb->GetSize(), GetRH(e));
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
                        if (MessageQueued) {
                            MessageQueued();
                        }
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
                        mc >> m_nBatchSize;
                        break;
                    default:
                        break;
                }
            }

        public:
            /**
             * An event for tracking message queued notification from server side
             */
            DMessageQueued MessageQueued;

        private:
            unsigned int m_nBatchSize;
            SPA::CUCriticalSection m_csQ;
            std::string m_keyDequeue; //protected by m_csQ
            DDequeue m_dDequeue; //protected by m_csQ
        };

    } //namespace ClientSide
} //namespace SPA

#endif