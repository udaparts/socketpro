

#ifndef _UDAPARTS_PERSISTENT_QUEUE_IMPL_H_
#define _UDAPARTS_PERSISTENT_QUEUE_IMPL_H_

#include "../uasyncqueue_server.h"
#include "../../aserverw.h"
#include <unordered_map>
#include "../../scloader.h"
#include<memory>

namespace SPA {
    namespace ServerSide {

        class CAsyncQueueImpl : public CClientPeer {
        public:
            //The following two static members are set within the method bool WINAPI InitServerLibrary(int param)
            static unsigned int m_nBatchSize;
            static unsigned char m_bNoAuto;

            CAsyncQueueImpl();

        private:
            static const unsigned int m_ttl = 120 * 3600; //120 hours
            static const unsigned int MAIN_THREAD_BYTES = 4 * 1024; //4 k bytes

            class CMyQueue {
            public:

                CMyQueue(const char *key) : m_q(CSocketProServer::QueueManager::StartQueue(key, m_ttl, true).GetHandle()) {

                }

                inline unsigned int GetHandle() {
                    if (!m_q.IsAvailable())
                        return 0;
                    return m_q.GetHandle();
                }

                void Flush(tagOptimistic option, UINT64 &messageCount, UINT64 &fileSize) {
                    if (option != oMemoryCached) {
                        m_q.SetOptimistic((tagOptimistic) option);
                        m_q.SetOptimistic(oMemoryCached);
                    }
                    messageCount = m_q.GetMessageCount();
                    fileSize = m_q.GetQueueSize();
                }

                void Enqueue(const CUQueue &buffer, unsigned short idmessage, SPA::UINT64 &index) {
                    index = m_q.Enqueue(idmessage, buffer.GetBuffer(), buffer.GetSize());
                }

                void Enqueue(unsigned int count, const unsigned char *msgStruct, SPA::UINT64 &index) {
                    index = m_q.BatchEnqueue(count, msgStruct);
                }

                void BatchEnqueue(unsigned int count, CUQueue &buffer, int &errCode) {
                    if (!count) {
                        errCode = Queue::QUEUE_OK;
                        return;
                    }
                    assert(buffer.GetSize());
                    unsigned short idmessage;
                    unsigned int bytes;
                    UINT64 index;
                    if (count == 1) {
                        buffer >> idmessage >> bytes;
                        index = m_q.Enqueue(idmessage, buffer.GetBuffer(), bytes);
                        if (INVALID_NUMBER == index) {
                            errCode = Queue::QUEUE_ENQUEUING_FAILED;
                        } else {
                            errCode = Queue::QUEUE_OK;
                        }
                        return;
                    }
                    if (!m_q.StartJob()) {
                        errCode = Queue::QUEUE_TRANS_COMMITTING_FAILED;
                        return;
                    }

                    index = m_q.BatchEnqueue(count, buffer.GetBuffer());
                    if (INVALID_NUMBER == index) {
                        errCode = Queue::QUEUE_ENQUEUING_FAILED;
                        m_q.AbortJob();
                        return;
                    }

                    if (!m_q.EndJob()) {
                        Queue::QUEUE_TRANS_COMMITTING_FAILED;
                    } else {
                        errCode = Queue::QUEUE_OK;
                    }
                }

                void Dequeue(USocket_Server_Handle socket, unsigned int timeout, SPA::UINT64 &messageCount, SPA::UINT64 &fileSize, SPA::UINT64 &ret) {
                    if (!m_q.IsAvailable()) {
                        messageCount = INVALID_NUMBER;
                        fileSize = INVALID_NUMBER;
                        ret = INVALID_NUMBER;
                        return;
                    }
                    ret = ServerCoreLoader.Dequeue2(m_q.GetHandle(), socket, CAsyncQueueImpl::m_nBatchSize, !CAsyncQueueImpl::m_bNoAuto, timeout);
                    messageCount = m_q.GetMessageCount() - m_q.GetMessagesInDequeuing();
                    fileSize = m_q.GetQueueSize();
                }

                void Close(bool permanent, int &errCode) {
                    if (m_q.GetMessagesInDequeuing() != 0) {
                        errCode = Queue::QUEUE_DEQUEUING;
                        return;
                    }
                    m_q.StopQueue(permanent);
                    errCode = Queue::QUEUE_OK;
                }

            private:
                CMyQueue(const CMyQueue &mq);
                CMyQueue& operator=(const CMyQueue &mq);

            private:
                CServerQueue m_q;
            };

        protected:
            virtual void OnFastRequestArrive(unsigned short reqId, unsigned int len);
            virtual int OnSlowRequestArrive(unsigned short reqId, unsigned int len);
            virtual void OnSwitchFrom(unsigned int nOldServiceId);
            virtual void OnReleaseSource(bool bClosing, unsigned int info);
            virtual void OnRequestArrive(unsigned short requestId, unsigned int len);
            virtual void OnBaseRequestArrive(unsigned short requestId);

        private:
            //no copy constructor
            CAsyncQueueImpl(const CAsyncQueueImpl &aq);
            //no assignment operator
            CAsyncQueueImpl& operator=(const CAsyncQueueImpl &aq);

            void NotifyBatchSize(unsigned int &batchSize);
            void Flush(std::string &key, int option, UINT64 &messageCount, UINT64 &fileSize);
            void Enqueue(std::string &key, unsigned short idmessage, UINT64 &index);
            void Dequeue(std::string &key, unsigned int timeout, UINT64 &messageCount, UINT64 &fileSize, UINT64 &ret);
            void EnqueueBatch(std::string &key, unsigned int count, UINT64 &res);

            void StartTrans(std::string &key, int &errCode);
            void EndTrans(bool rollback, int &errCode);
            void Close(std::string &key, bool permanent, int &errCode);

            static std::shared_ptr<CMyQueue> Find(std::string &key);
            static void Pretreat(std::string &key);

        private:
            unsigned int m_count;
            CUQueue *m_bufferBatch;
            std::shared_ptr<CMyQueue> m_qTrans;
            static CUCriticalSection m_cs;
            static std::unordered_map<std::string, std::shared_ptr<CMyQueue>> m_mapKeyQueue; //protected by m_cs
            static unsigned int m_clients; //protected by m_cs
        };
    } //SPA
} //ServerSide

#endif //_UDAPARTS_PERSISTENT_QUEUE_IMPL_H_