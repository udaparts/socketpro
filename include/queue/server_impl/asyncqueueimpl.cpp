#include "asyncqueueimpl.h"

namespace SPA
{
    namespace ServerSide{

        std::atomic<unsigned int> CAsyncQueueImpl::m_nBatchSize = 0;
        std::atomic<unsigned char> CAsyncQueueImpl::m_bNoAuto = 0;
        CSpinLock CAsyncQueueImpl::m_cs;
        std::unordered_map<std::string, std::shared_ptr < CAsyncQueueImpl::CMyQueue >> CAsyncQueueImpl::m_mapKeyQueue;
        unsigned int CAsyncQueueImpl::m_clients = 0;

        CAsyncQueueImpl::CAsyncQueueImpl() : m_count(0), m_bufferBatch(nullptr) {
        }

        void CAsyncQueueImpl::OnSwitchFrom(unsigned int nOldServiceId) {
            m_cs.lock();
            ++m_clients;
            m_cs.unlock();
            MakeRequest(Queue::idBatchSizeNotified, nullptr, 0);
        }

        void CAsyncQueueImpl::OnReleaseSource(bool bClosing, unsigned int info) {
            if (m_bufferBatch || m_qTrans) {
                SPA::CScopeUQueue::Unlock(m_bufferBatch);
                m_bufferBatch = nullptr;
                m_count = 0;
                m_qTrans.reset();
            }

            //reduce memory when pool contains more than 256 mega bytes
            if (CScopeUQueue::GetMemoryConsumed() >= 0x10000000) {
                CScopeUQueue::DestroyUQueuePool();
            }

            CSpinAutoLock al(m_cs);
            --m_clients;
            if (!m_clients) {
                m_mapKeyQueue.clear();
            }
        }

        void CAsyncQueueImpl::OnBaseRequestArrive(unsigned short requestId) {
            switch (requestId) {
                case (unsigned short) tagBaseRequestID::idCancel:
#ifndef NDEBUG
                    std::cout << "Cancel called" << std::endl;
#endif
                    SPA::CScopeUQueue::Unlock(m_bufferBatch);
                    m_bufferBatch = nullptr;
                    m_count = 0;
                    m_qTrans.reset();
                    break;
                default:
                    break;
            }
        }

        void CAsyncQueueImpl::OnFastRequestArrive(unsigned short reqId, unsigned int len) {
            switch (reqId) {
                case Queue::idGetKeys:
                {
                    SPA::CScopeUQueue sb;
                    {
                        CSpinAutoLock al(m_cs);
                        unsigned int size = (unsigned int) m_mapKeyQueue.size();
                        sb << size;
                        for (auto it = m_mapKeyQueue.begin(), end = m_mapKeyQueue.end(); it != end; ++it) {
                            sb << it->first;
                        }
                    }
                    SendResult(reqId, sb->GetBuffer(), sb->GetSize());
                }
                    return;
                default:
                    break;
            }
            BEGIN_SWITCH(reqId)
            M_I0_R1(Queue::idBatchSizeNotified, NotifyBatchSize, unsigned int)
            M_I1_R1(Queue::idStartTrans, StartTrans, std::string, int)
            END_SWITCH
        }

        int CAsyncQueueImpl::OnSlowRequestArrive(unsigned short reqId, unsigned int len) {
            BEGIN_SWITCH(reqId)
            M_I2_R3(Queue::idDequeue, Dequeue, std::string, unsigned int, UINT64, UINT64, UINT64)
            M_I2_R1(Queue::idEnqueue, Enqueue, std::string, unsigned short, UINT64)
            M_I2_R2(Queue::idFlush, Flush, std::string, int, UINT64, UINT64)
            M_I2_R1(Queue::idClose, Close, std::string, bool, int)
            M_I1_R1(Queue::idEndTrans, EndTrans, bool, int)
            M_I2_R1(Queue::idEnqueueBatch, EnqueueBatch, std::string, unsigned int, UINT64)
            END_SWITCH
            return 0;
        }

        void CAsyncQueueImpl::NotifyBatchSize(unsigned int &batchSize) {
            batchSize = ((unsigned int) m_bNoAuto << 24) + m_nBatchSize;
        }

        void CAsyncQueueImpl::OnRequestArrive(unsigned short requestId, unsigned int len) {
            switch (requestId) {
                case Queue::idEnqueue:
                    if (len < MAIN_THREAD_BYTES) {
                        //if message size is small, we directly enqueue messages within main thread to avoid expensive thread context switch
                        std::string key;
                        unsigned short idMessage;
                        m_UQueue >> key >> idMessage;
                        SPA::UINT64 index;
                        Enqueue(key, idMessage, index);
                        DropCurrentSlowRequest();
                        SendResult(requestId, index);
                    }
                    break;
                case Queue::idFlush:
                {
                    unsigned int len = *((unsigned int*) m_UQueue.GetBuffer());
                    int option = *((int*) m_UQueue.GetBuffer(sizeof (unsigned int) +len));
                    if (option == (int) tagOptimistic::oMemoryCached) {
                        //if option is oMemoryCached, we directly flush within main thread to avoid expensive thread context switch
                        std::string key;
                        m_UQueue >> key >> option;
                        UINT64 messageCount, fileSize;
                        Flush(key, option, messageCount, fileSize);
                        DropCurrentSlowRequest();
                        SendResult(requestId, messageCount, fileSize);
                    }
                }
                    break;
                case Queue::idEndTrans:
                {
                    bool *b = (bool*) m_UQueue.GetBuffer();
                    if (*b || !m_bufferBatch || m_bufferBatch->GetSize() < MAIN_THREAD_BYTES) {
                        bool rollback;
                        int errCode;
                        m_UQueue >> rollback;
                        EndTrans(rollback, errCode);
                        DropCurrentSlowRequest();
                        SendResult(requestId, errCode);
                    }
                }
                    break;
                case Queue::idEnqueueBatch:
                    if (len < MAIN_THREAD_BYTES) {
                        //if message size is small, we directly enqueue messages within main thread to avoid expensive thread context switch
                        std::string key;
                        unsigned int count;
                        UINT64 res;
                        m_UQueue >> key >> count;
                        EnqueueBatch(key, count, res);
                        DropCurrentSlowRequest();
                        SendResult(requestId, res);
                    }
                    break;
                default:
                    break;
            }
        }

        void CAsyncQueueImpl::StartTrans(std::string &key, int &errCode) {
            if (m_bufferBatch || m_qTrans) {
                errCode = Queue::QUEUE_TRANS_ALREADY_STARTED;
                return;
            }
            Pretreat(key);
            {
                CSpinAutoLock al(m_cs);
                auto it = m_mapKeyQueue.find(key);
                if (it == m_mapKeyQueue.end()) {
                    m_qTrans.reset(new CMyQueue(key.c_str()));
                    if (m_qTrans && m_qTrans->GetHandle()) {
                        m_mapKeyQueue[key] = m_qTrans;
                    } else {
                        m_qTrans.reset();
                        errCode = Queue::QUEUE_TRANS_STARTING_FAILED;
                        return;
                    }
                } else {
                    m_qTrans = it->second;
                }
            }
            errCode = Queue::QUEUE_OK;
            m_count = 0;
            m_bufferBatch = CScopeUQueue::Lock();
        }

        void CAsyncQueueImpl::EndTrans(bool rollback, int &errCode) {
            if (!m_bufferBatch || !m_qTrans) {
                errCode = Queue::QUEUE_TRANS_NOT_STARTED_YET;
                return;
            }
            if (rollback) {
                errCode = Queue::QUEUE_OK;
            } else {
                m_qTrans->BatchEnqueue(m_count, *m_bufferBatch, errCode);
            }
            CScopeUQueue::Unlock(m_bufferBatch);
            m_count = 0;
            m_qTrans.reset();
            m_bufferBatch = nullptr;
        }

        void CAsyncQueueImpl::Close(std::string &key, bool permanent, int &errCode) {
            Pretreat(key);
            if (m_qTrans || m_bufferBatch) {
                CScopeUQueue::Unlock(m_bufferBatch);
                m_count = 0;
                m_qTrans.reset();
                m_bufferBatch = nullptr;
            }
            CSpinAutoLock al(m_cs);
            auto it = m_mapKeyQueue.find(key);
            if (it == m_mapKeyQueue.end()) {
                errCode = Queue::QUEUE_OK;
                return;
            }
            if (it->second.use_count() == 1) {
                it->second->Close(permanent, errCode);
                if (errCode == Queue::QUEUE_OK) {
                    m_mapKeyQueue.erase(it);
                }
            } else {
                errCode = Queue::QUEUE_OTHER_WORKING_WITH_SAME_QUEUE;
            }
        }

        void CAsyncQueueImpl::Flush(std::string &key, int option, UINT64 &messageCount, UINT64 & fileSize) {
            std::shared_ptr<CMyQueue> p = Find(key);
            if (!p) {
                messageCount = INVALID_NUMBER;
                fileSize = INVALID_NUMBER;
                return;
            }
            p->Flush((tagOptimistic) option, messageCount, fileSize);
        }

        void CAsyncQueueImpl::EnqueueBatch(std::string &key, unsigned int count, UINT64 & res) {
            res = -1;
            std::shared_ptr<CMyQueue> p = m_qTrans;
            if (!p) {
                p = Find(key);
            }
            if (!p) {
                p.reset(new CMyQueue(key.c_str()));
                if (p && p->GetHandle()) {
                    CSpinAutoLock al(m_cs);
                    m_mapKeyQueue[key] = p;
                } else {
                    return;
                }
            }
            if (m_bufferBatch) {
                m_bufferBatch->Push(m_UQueue.GetBuffer(), m_UQueue.GetSize());
                m_count += count;
                res = count;
            } else {
                p->Enqueue(count, m_UQueue.GetBuffer(), res);
            }
            m_UQueue.SetSize(0);
        }

        void CAsyncQueueImpl::Enqueue(std::string &key, unsigned short idmessage, SPA::UINT64 & index) {
            std::shared_ptr<CMyQueue> p = m_qTrans;
            if (!p) {
                p = Find(key);
            }
            if (!p) {
                p.reset(new CMyQueue(key.c_str()));
                if (p && p->GetHandle()) {
                    CSpinAutoLock al(m_cs);
                    m_mapKeyQueue[key] = p;
                } else {
                    index = INVALID_NUMBER; // == -1
                    return;
                }
            }
            if (m_bufferBatch) {
                assert(m_qTrans);
                *m_bufferBatch << idmessage << m_UQueue.GetSize();
                m_bufferBatch->Push(m_UQueue.GetBuffer(), m_UQueue.GetSize());
                ++m_count;
            } else {
                p->Enqueue(m_UQueue, idmessage, index);
            }
            m_UQueue.SetSize(0);
        }

        void CAsyncQueueImpl::Dequeue(std::string &key, unsigned int timeout, SPA::UINT64 &messageCount, SPA::UINT64 &fileSize, SPA::UINT64 & ret) {
            std::shared_ptr<CMyQueue> p = Find(key);
            if (!p) {
                p.reset(new CMyQueue(key.c_str()));
                if (!p || !p->GetHandle()) {
                    messageCount = INVALID_NUMBER;
                    fileSize = INVALID_NUMBER;
                    ret = INVALID_NUMBER;
                    return;
                }
                CSpinAutoLock al(m_cs);
                m_mapKeyQueue[key] = p;
            }
            if (m_qTrans == p)
                timeout = 0;
            p->Dequeue(GetSocketHandle(), timeout, messageCount, fileSize, ret);
        }

        void CAsyncQueueImpl::Pretreat(std::string & key) {
#ifdef WIN32_64
            ToLower(key);
            while (key.back() == '\\') {
#else
            if (key.back() == '/') {
#endif
                key.pop_back();
            }
        }

        std::shared_ptr<CAsyncQueueImpl::CMyQueue> CAsyncQueueImpl::Find(std::string & key) {
            Pretreat(key);
            CSpinAutoLock al(m_cs);
            auto it = m_mapKeyQueue.find(key);
            if (it == m_mapKeyQueue.end()) {
                return nullptr;
            }
            return it->second;
        }
    } //SPA
} //ServerSide
