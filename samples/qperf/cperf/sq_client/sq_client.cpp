
#include "stdafx.h"
#ifndef WIN32_64
#include <chrono>
using std::chrono::system_clock;
typedef std::chrono::milliseconds ms;
#endif

#define TEST_QUEUE_KEY "qperf"
static const unsigned short idMessage = idReservedTwo + 128;

typedef CSocketPool<CAsyncQueue, CClientSocket> CMyPool;

void EnqueueToServer(CMyPool::PHandler sq, const std::string &msg, unsigned int cycles);
void EnqueueToServerBatch(CMyPool::PHandler sq, const std::string &msg, unsigned int cycles, unsigned int batchSize = 1024 * 16);
void DequeueFromServer(CMyPool::PHandler sq);

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cc.Port = 20901;
    cc.UserId = L"root";
    cc.Password = L"Smash123";
    std::cout << "Tell me the remote server address: " << std::endl;
    std::getline(std::cin, cc.Host);
    CMyPool spSq;
    if (!spSq.StartSocketPool(cc, 1, 1)) {
        std::cout << "Failed to connect to remote host for enqueuing" << std::endl;
        std::cout << "Press a key to shutdown the application ......" << std::endl;
        ::getchar();
        return 1;
    }
    int wan = 0;
    std::cout << "Wide Area Network (yes -- 1 or no -- 0)? " << std::endl;
    std::cin >> wan;
    auto sq = spSq.Seek();

    std::string s4("Sock");
    EnqueueToServer(sq, s4, wan ? 10000000 : 200000000);
    DequeueFromServer(sq);

    //batching tiny messages improves throughput
    EnqueueToServerBatch(sq, s4, wan ? 10000000 : 200000000, 1024 * 8);
    DequeueFromServer(sq);

    std::string s32("SocketPro is a world-leading pac");
    EnqueueToServer(sq, s32, wan ? 10000000 : 200000000);
    DequeueFromServer(sq);

    //batching small messages improves throughput
    EnqueueToServerBatch(sq, s32, wan ? 10000000 : 200000000, 1024 * 8);
    DequeueFromServer(sq);

    std::string s("SocketPro is a world-leading package of secured communication software components written with request batching, asynchrony and parallel computation in mind. It offers superior performance and scalabi");
    EnqueueToServer(sq, s, wan ? 2500000 : 50000000);
    DequeueFromServer(sq);

    //batching middle messages improves throughput
    EnqueueToServerBatch(sq, s, wan ? 2500000 : 50000000, 1024 * 8);
    DequeueFromServer(sq);

    std::string s1024(s);
    for (int n = 0; n < 5; ++n) {
        s1024 += s;
    }
    s1024 = s1024.substr(0, 1024);
    EnqueueToServer(sq, s1024, wan ? 500000 : 10000000);
    DequeueFromServer(sq);

    std::string s10240;
    for (int n = 0; n < 10; ++n) {
        s10240 += s1024;
    }
    EnqueueToServer(sq, s10240, wan ? 50000 : 1000000);
    DequeueFromServer(sq);

    std::cout << "Press a key to shutdown the application ......" << std::endl;
    std::getchar();
    std::getchar();
    return 0;
}

void EnqueueToServerBatch(CMyPool::PHandler sq, const std::string &msg, unsigned int cycles, unsigned int batchSize) {
    unsigned int n;
    SPA::CScopeUQueue sb;
#ifdef WIN32_64
    DWORD start = ::GetTickCount();
#else
    system_clock::time_point start = system_clock::now();
#endif
    std::cout << "Going to enqueue " << cycles << " messages ......" << std::endl;
    for (n = 0; n < cycles; ++n) {
        CAsyncQueue::BatchMessage(idMessage, (const unsigned char*) msg.c_str(), (unsigned int) msg.size(), *sb);
        if (sb->GetSize() > batchSize) {
            sq->EnqueueBatch(TEST_QUEUE_KEY, *sb);
        }
    }
    if (sb->GetSize()) {
        sq->EnqueueBatch(TEST_QUEUE_KEY, *sb);
    }
    sq->WaitAll();
#ifdef WIN32_64
    DWORD stop = ::GetTickCount();
    std::cout << cycles << " messages sent to server and enqueued within " << stop - start << " ms" << std::endl;
#else
    system_clock::time_point stop = system_clock::now();
    ms d = std::chrono::duration_cast<ms>(stop - start);
    std::cout << cycles << " messages sent to server and enqueued within " << d.count() << " ms" << std::endl;
#endif
}

void EnqueueToServer(CMyPool::PHandler sq, const std::string &msg, unsigned int cycles) {
    unsigned int n;
#ifdef WIN32_64
    DWORD start = ::GetTickCount();
#else
    system_clock::time_point start = system_clock::now();
#endif
    std::cout << "Going to enqueue " << cycles << " messages ......" << std::endl;
    for (n = 0; n < cycles; ++n) {
        sq->Enqueue(TEST_QUEUE_KEY, idMessage, (const unsigned char*) msg.c_str(), (unsigned int) msg.size());
    }
    sq->WaitAll();
#ifdef WIN32_64
    DWORD stop = ::GetTickCount();
    std::cout << cycles << " messages sent to server and enqueued within " << stop - start << " ms" << std::endl;
#else
    system_clock::time_point stop = system_clock::now();
    ms d = std::chrono::duration_cast<ms>(stop - start);
    std::cout << cycles << " messages sent to server and enqueued within " << d.count() << " ms" << std::endl;
#endif
}

void DequeueFromServer(CMyPool::PHandler sq) {
#ifdef WIN32_64
    DWORD start = ::GetTickCount();
#else
    system_clock::time_point start = system_clock::now();
#endif
    unsigned int messages_dequeued = 0;

    //prepare a callback for processing returned result of dequeue request
    CAsyncQueue::DDequeue d = [](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
        if (messageCount > 0) {
            //there are more messages left at server queue, we re-send a request to dequeue
            aq->Dequeue(TEST_QUEUE_KEY, aq->GetLastDequeueCallback());
        } else {
            //set dequeue callback to null and stop dequeuing
            aq->SetLastDequeueCallback(nullptr);
        }
    };
    sq->ResultReturned = [&messages_dequeued](CAsyncServiceHandler *sender, unsigned short reqId, CUQueue & q) -> bool {
        bool processed = false;
        switch (reqId) {
            case idMessage:
            {
                ++messages_dequeued;
                q.SetNull();
                std::string s = (const char*) q.GetBuffer();
            }
                processed = true;
                break;
            default:
                break;
        }
        return processed;
    };

    std::cout << "Going to dequeue message ......" << std::endl;
    bool ok = sq->Dequeue(TEST_QUEUE_KEY, d);

    //optionally, add one or two extra to improve processing concurrency at both client and server sides for better performance and through-output
    ok = sq->Dequeue(TEST_QUEUE_KEY, d);
    ok = sq->Dequeue(TEST_QUEUE_KEY, d);

    sq->WaitAll();
#ifdef WIN32_64
    DWORD stop = ::GetTickCount();
    std::cout << messages_dequeued << " messages dequeued from server within " << stop - start << " ms" << std::endl;
#else
    system_clock::time_point stop = system_clock::now();
    ms diff = std::chrono::duration_cast<ms>(stop - start);
    std::cout << messages_dequeued << " messages dequeued from server within " << diff.count() << " ms" << std::endl;
#endif
}