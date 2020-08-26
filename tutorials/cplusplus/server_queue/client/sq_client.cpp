#include "stdafx.h"
#include "../test_queue.h"

#define TEST_QUEUE_KEY  "queue_name_0"

typedef CSocketPool<CAsyncQueue, CClientSocket> CMyPool;
void TestEnqueue(CMyPool::PHandler &sq);
std::future<CAsyncQueue::DeqInfo> TestDequeue(CMyPool::PHandler &sq);

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    CMyPool spSq;
    if (!spSq.StartSocketPool(cc, 1)) {
        std::cout << "Failed to connect to remote host for enqueuing" << std::endl;
        std::cout << "Press a key to complete dequeuing messages from server ......" << std::endl;
        ::getchar();
        return 1;
    }
    auto sq = spSq.Seek();
    try{
        TestEnqueue(sq);
        std::wcout << TestDequeue(sq).get().ToString() << std::endl;
    }

    catch(CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(std::exception & ex) {
        std::wcout << "Some unexpected error: " << ex.what() << std::endl;
    }
    std::cout << "Press a key to complete dequeuing messages from server ......" << std::endl;
    ::getchar();
    return 0;
}

void TestEnqueue(CMyPool::PHandler &sq) {
    std::cout << "Going to enqueue 1024 messages ......" << std::endl;
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
            sq->raise(L"Enqueue", Queue::idEnqueue);
        }
    }
}

std::future<CAsyncQueue::DeqInfo> TestDequeue(CMyPool::PHandler &sq) {
    //prepare callback for parsing messages dequeued from server side
    sq->ResultReturned = [](CAsyncServiceHandler *sender, unsigned short idReq, CUQueue & q) -> bool {
        bool processed = false;
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

                std::wcout << L", name=" << name << L", str=" << str << L", index=" << index << std::endl;
            }
                processed = true;
                break;
            default:
                break;
        }
        return processed;
    };
    std::shared_ptr<std::promise<CAsyncQueue::DeqInfo> > prom(new std::promise<CAsyncQueue::DeqInfo>);
    auto aborted = CAsyncQueue::get_aborted(prom, L"Dequeue", Queue::idDequeue);
    auto se = CAsyncQueue::get_se(prom);
    //prepare a callback for processing returned result of dequeue request
    CAsyncQueue::DDequeue d = [prom, aborted, se](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
        if (bytes) {
            std::cout << "Total message count=" << messageCount
                    << ", queue file size=" << fileSize
                    << ", messages dequeued=" << messages
                    << ", message bytes dequeued=" << bytes
                    << std::endl;
        }
        if (messageCount > 0) {
            //there are more messages left at server queue, we re-send a request to dequeue
            aq->Dequeue(TEST_QUEUE_KEY, aq->GetLastDequeueCallback(), 0, aborted, se);
        } else {
            try{
                prom->set_value(CAsyncQueue::DeqInfo(messageCount, fileSize, messages, bytes));
            }

            catch(std::future_error&) {
                //ignore it
            }
        }
    };
    std::cout << "Going to dequeue message ......" << std::endl;
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    if (!(sq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se) && sq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se))) {
        sq->raise(L"Dequeue", Queue::idDequeue);
    }
    return prom->get_future();
}
