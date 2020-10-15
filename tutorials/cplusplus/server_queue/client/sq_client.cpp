#include "stdafx.h"
#include "../test_queue.h"

#define TEST_QUEUE_KEY  "queue_name_0"
using namespace std;
typedef CSocketPool<CAsyncQueue> CMyPool;
void TestEnqueue(CMyPool::PHandler &sq);
future<CAsyncQueue::DeqInfo> TestDequeue(CMyPool::PHandler &sq);

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    cout << "Remote host: " << endl;
    getline(cin, cc.Host);
    CMyPool spSq;
    //spSq.SetQueueName("qname");
    if (!spSq.StartSocketPool(cc, 1)) {
        cout << "Failed to connect to remote host for enqueuing\n";
        cout << "Press a key to kill the demo ......\n";
        ::getchar();
        return 1;
    }
    auto sq = spSq.Seek();
    try{
        TestEnqueue(sq);
        wcout << TestDequeue(sq).get().ToString() << endl;
    }
    catch(CServerError & ex) {
        wcout << ex.ToString() << endl;
    }
    catch(CSocketError & ex) {
        wcout << ex.ToString() << endl;
    }
    catch(exception & ex) {
        wcout << "Unexpected error: " << ex.what() << endl;
    }
    cout << "Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}

void TestEnqueue(CMyPool::PHandler &sq) {
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
        //this_thread::sleep_for(chrono::milliseconds(100));
        //enqueue two unicode strings and one int
        if (!sq->Enqueue(TEST_QUEUE_KEY, idMessage, L"SampleName", str, n)) {
            sq->raise(Queue::idEnqueue);
        }
    }
}

future<CAsyncQueue::DeqInfo> TestDequeue(CMyPool::PHandler &sq) {
    //prepare callback for parsing messages dequeued from server side
    sq->ResultReturned = [](CAsyncServiceHandler *sender, unsigned short idReq, CUQueue & q) -> bool {
        bool processed = false;
        switch (idReq) {
            case idMessage0:
            case idMessage1:
            case idMessage2:
                cout << "message id=" << idReq;
            {
                wstring name, str; int index;

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
    shared_ptr<promise<CAsyncQueue::DeqInfo> > prom(new promise<CAsyncQueue::DeqInfo>);
    auto aborted = CAsyncQueue::get_aborted(prom, Queue::idDequeue);
    auto se = CAsyncQueue::get_se(prom);
    //prepare a callback for processing returned result of dequeue request
    CAsyncQueue::DDequeue d = [prom, aborted, se](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
        if (bytes) {
            cout << "Total message count=" << messageCount
                    << ", queue file size=" << fileSize
                    << ", messages dequeued=" << messages
                    << ", message bytes dequeued=" << bytes
                    << "\n";
        }
        if (messageCount > 0) {
            //there are more messages left at server queue, we re-send a request to dequeue
            aq->Dequeue(TEST_QUEUE_KEY, aq->GetLastDequeueCallback(), 0, aborted, se);
        } else {
            try{
                prom->set_value(CAsyncQueue::DeqInfo(messageCount, fileSize, messages, bytes));
            }

            catch(future_error&) {
                //ignore it
            }
        }
    };
    cout << "Going to dequeue message ......\n";
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    if (!(sq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se) && sq->Dequeue(TEST_QUEUE_KEY, d, 0, aborted, se))) {
        sq->raise(Queue::idDequeue);
    }
    return prom->get_future();
}
