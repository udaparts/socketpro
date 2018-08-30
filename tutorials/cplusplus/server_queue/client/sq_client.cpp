
#include "stdafx.h"
#include "../test_queue.h"

#define TEST_QUEUE_KEY  "queue_name_0"

typedef CSocketPool<CAsyncQueue, CClientSocket> CMyPool;
bool TestEnqueue(CMyPool::PHandler &sq);
void TestDequeue(CMyPool::PHandler &sq);

int main(int argc, char* argv[]) {
	CConnectionContext cc;
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    CMyPool spSq;
    if (!spSq.StartSocketPool(cc, 1, 1)) {
        std::cout << "Failed to connect to remote host for enqueuing" << std::endl;
        std::cout << "Press a key to complete dequeuing messages from server ......" << std::endl;
        ::getchar();
        return 1;
    }
    auto sq = spSq.Seek();

    bool ok = TestEnqueue(sq);
    TestDequeue(sq);

	std::cout << "Press a key to complete dequeuing messages from server ......" << std::endl;
	::getchar();
	return 0;
}

bool TestEnqueue(CMyPool::PHandler &sq) {
    bool ok = true;
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
        ok = sq->Enqueue(TEST_QUEUE_KEY, idMessage, L"SampleName", str, n);
        if (!ok)
            break;
    }
    return ok;
}

void TestDequeue(CMyPool::PHandler &sq) {
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

    //prepare a callback for processing returned result of dequeue request
    CAsyncQueue::DDequeue d = [](CAsyncQueue *aq, SPA::UINT64 messageCount, SPA::UINT64 fileSize, unsigned int messages, unsigned int bytes) {
        std::cout << "Total message count=" << messageCount
                << ", queue file size=" << fileSize
                << ", messages dequeued=" << messages
                << ", message bytes dequeued=" << bytes
                << std::endl;
        if (messageCount > 0) {
            //there are more messages left at server queue, we re-send a request to dequeue
            aq->Dequeue(TEST_QUEUE_KEY, aq->GetLastDequeueCallback());
        }
    };

    std::cout << "Going to dequeue message ......" << std::endl;
    bool ok = sq->Dequeue(TEST_QUEUE_KEY, d);

    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    ok = sq->Dequeue(TEST_QUEUE_KEY, d);
}
