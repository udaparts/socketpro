#include "stdafx.h"
#include "../test_queue.h"

#define TEST_QUEUE_KEY  "queue_name_0"

typedef CSocketPool<CAsyncQueue> CMyPool;

void TestEnqueue(CMyPool::PHandler &sq);
void TestDequeue(CMyPool::PHandler &sq);

int main(int argc, char* argv[]) {

    CConnectionContext cc;
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    CMyPool spSq;
    if (!spSq.StartSocketPool(cc, 1)) {
        std::cout << "Failed to remote host for enqueuing\n";
        std::cout << "Press a key to kill the demo ......\n";
        ::getchar();
        return 1;
    }
    auto sq = spSq.Seek();

    try{
        //Optionally, you can enqueue messages with transaction style by calling the methods StartQueueTrans and EndQueueTrans in pair
        auto fsqt = sq->startQueueTrans(TEST_QUEUE_KEY);
        TestEnqueue(sq);

        //test message batching
        {
            SPA::CScopeUQueue sb;
            CAsyncQueue::BatchMessage(idMessage3, *sb, L"Hello", L"World");
            CAsyncQueue::BatchMessage(idMessage4, *sb, true, 234.456, L"MyTestWhatever");
            if (!sq->EnqueueBatch(TEST_QUEUE_KEY, *sb)) {
                sq->raise(Queue::idEnqueueBatch);
            }
        }
        auto feqt = sq->endQueueTrans(false);
        TestDequeue(sq);
        sq->WaitAll();

        //test GetKeys
        std::future<std::vector < std::string>> fk = sq->getKeys();

        //get a queue key two parameters, message count and queue file size by default option oMemoryCached
        std::future<CAsyncQueue::QueueInfo> fq = sq->flushQueue(TEST_QUEUE_KEY);

        //test CloseQueue
        std::future<int> fc = sq->closeQueue(TEST_QUEUE_KEY);

        std::cout << "StartQueueTrans/res: " << fsqt.get() << std::endl;
        std::cout << "EndQueueTrans/res: " << feqt.get() << std::endl;

        int index = 0;
        const auto& v = fk.get();
        std::cout << "[";
        for (auto& s : v) {
            if (index) {
                std::cout << "," << std::endl;
            }
            std::cout << s;
            ++index;
        }
        std::cout << "]" << std::endl;

        std::wcout << fq.get().ToString() << std::endl;
        std::cout << "CloseQueue/res: " << fc.get() << "\n";
    }

    catch(CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(std::exception & ex) {
        std::cout << "Unexpected error: " << ex.what() << "\n";
    }
    std::cout << "Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}

void TestEnqueue(CMyPool::PHandler &sq) {
    std::cout << "Going to enqueue 1024 messages ......\n";
    for (int n = 0; n < 1024; ++n) {
        std::wstring str = std::to_wstring((SPA::UINT64)n) + L" Object test";
        unsigned short idMessage;
        switch (n % 3) {
            case 0:
                idMessage = idMessage0;
                break;
            case 1:
                idMessage = idMessage1;
                break;
            default:
                idMessage = idMessage2;
                break;
        }
        //enqueue two unicode strings and one int
        if (!sq->Enqueue(TEST_QUEUE_KEY, idMessage, L"SampleName", str, n)) {
            sq->raise(Queue::idEnqueue);
        }
    }
}

void TestDequeue(CMyPool::PHandler &sq) {
    //prepare callback for parsing messages dequeued from server side
    sq->ResultReturned = [](CAsyncServiceHandler *sender, unsigned short idReq, CUQueue & q) -> bool {
        bool processed = true;
        switch (idReq) {
            case idMessage0:
            case idMessage1:
            case idMessage2:
                std::cout << "message id=" << idReq;
            {
                std::wstring name, str;
                int index;

                //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
                q >> name >> str >> index;

                std::wcout << L", name=" << name << L", str=" << str << L", index=" << index << "\n";
            }
                processed = true;
                break;
            case idMessage3:
            {
                std::wstring s0, s1;
                q >> s0 >> s1;
                std::wcout << s0 << L" " << s1 << std::endl;
            }
                break;
            case idMessage4:
            {
                bool b;
                double d;
                std::wstring s;
                q >> b >> d >> s;
                std::cout << "bool= " << b << ", d= " << d << ", s= ";
                std::wcout << s << std::endl;
            }
                break;
            default:
                processed = false;
                break;
        }
        return processed;
    };

    //prepare a callback for processing returned result of dequeue request
    CAsyncQueue::DDequeue d = [sq](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
        if (bytes) {
            std::cout << "Total message count=" << messageCount
                    << ", queue file size=" << fileSize
                    << ", messages dequeued=" << messages
                    << ", message bytes dequeued=" << bytes
                    << "\n";
        }
        if (messageCount > 0) {
            //there are more messages left at server queue, we re-send a request to dequeue
            sq->Dequeue(TEST_QUEUE_KEY, sq->GetLastDequeueCallback());
        }
    };

    std::cout << "Going to dequeue message ......\n";
    //add an extra Dequeue call for better performance
    if (!(sq->Dequeue(TEST_QUEUE_KEY, d) && sq->Dequeue(TEST_QUEUE_KEY, d))) {
        sq->raise(Queue::idDequeue);
    }
}
