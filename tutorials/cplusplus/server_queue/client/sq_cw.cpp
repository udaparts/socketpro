#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
static_assert(false, "No co_await support");
#endif
#include <iostream>
#include <thread>
#include "../../../../include/aqhandler.h"

using namespace std;
using namespace SPA;
using namespace SPA::ClientSide;

#include "../test_queue.h"
#define TEST_QUEUE_KEY  "queue_name_0"
typedef CSocketPool<CAsyncQueue> CMyPool;

void TestEnqueue(CMyPool::PHandler& sq);

struct Awaiter : public CWaiter<CAsyncQueue::DeqInfo> {

    Awaiter(CAsyncQueue* aq)
        : CWaiter<CAsyncQueue::DeqInfo>(Queue::idDequeue) {
        auto aborted = get_aborted();
        auto se = get_se();
        auto& wc = m_wc;
        //prepare a callback for processing returned result of dequeue request
        CAsyncQueue::DDequeue d = [wc, aborted, se](CAsyncQueue* aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
            if (bytes) {
                cout << "Total messages: " << messageCount
                    << ", file size: " << fileSize
                    << ", messages dequeued: " << messages
                    << ", bytes dequeued: " << bytes
                    << "\n";
            }
            if (messageCount > 0) {
                //there are more messages left at server queue, we re-send a request to dequeue
                aq->Dequeue(TEST_QUEUE_KEY, aq->GetLastDequeueCallback(), 0, aborted, se);
            }
            else if (!wc->await_ready()) {
                wc->m_r.messages = messageCount;
                wc->m_r.fSize = fileSize;
                wc->m_r.DeMessages = messages;
                wc->m_r.DeBytes = bytes;
                wc->resume();
            }
        };
        cout << "Going to dequeue message ......\n";
        //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
        if (!(aq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se) && aq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se))) {
            aq->raise(Queue::idDequeue);
        }
    }
};

Awaiter TestDequeue(CMyPool::PHandler& sq) {
    //prepare callback for parsing messages dequeued from server side
    sq->ResultReturned = [](CAsyncServiceHandler* sender, unsigned short idReq, CUQueue& q) -> bool {
        bool processed = false;
        switch (idReq) {
        case idMessage0:
        case idMessage1:
        case idMessage2:
            cout << "message id=" << idReq;
            {
                wstring name, str;
                int index;

                //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
                q >> name >> str >> index;

                wcout << L", name=" << name << L", str=" << str << L", index=" << index << "\n";
            }
            processed = true;
            break;
        default:
            break;
        }
        return processed;
    };
    return Awaiter(sq.get());
}

CAwTask MyTest(CMyPool::PHandler& sq) {
    try {
        wcout << (co_await TestDequeue(sq)).ToString() << "\n";
        auto wfq = sq->wait_flushQueue(TEST_QUEUE_KEY);
        wcout << (co_await wfq).ToString() << "\n";
        auto v = co_await sq->wait_getKeys();
        int index = 0;
        cout << "[";
        for (auto& s : v) {
            if (index) {
                cout << ",\n";
            }
            cout << s;
            ++index;
        }
        cout << "]\n";
    }
    catch (CServerError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (CSocketError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (exception& ex) {
        wcout << "Unexpected error: " << ex.what() << "\n";
    }
}

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -lstdc++ -pthread
//GCC 11 or clang 14 -- -std=c++20 -ldl -lstdc++ -pthread
int main(int argc, char* argv[]) {

    CMyPool spSq;
    CConnectionContext cc("windesk", 20901, L"MyUserId", L"MyPassword");
    //spSq.SetQueueName("qbackup");
    if (!spSq.StartSocketPool(cc, 1)) {
        wcout << "No connection to remote helloworld server\n";
    }
    else {
        auto sq = spSq.Seek();
        TestEnqueue(sq);
        MyTest(sq);
    }
    wcout << L"Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}

void TestEnqueue(CMyPool::PHandler& sq) {
    cout << "Going to enqueue 1024 messages ......\n";
    for (int n = 0; n < 1024; ++n) {
        wstring str = to_wstring(n) + L" Object test";
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
        //this_thread::sleep_for(chrono::milliseconds(30));
        //enqueue two unicode strings and one int
        if (!sq->Enqueue(TEST_QUEUE_KEY, idMessage, L"SampleName", str, n)) {
            sq->raise(Queue::idEnqueue);
        }
    }
}
