#include "stdafx.h"
#include "HW.h"
#include "../../uqueue_demo/mystruct.h"

int main(int argc, char* argv[]) {
    typedef CSocketPool<HelloWorld, CClientSocket> CMyPool;
    CMyPool spHw;
    CConnectionContext cc("localhost", 20901, L"MyUserId", L"MyPassword");

    //spHw.SetQueueName("helloworld"); //optionally start a queue for auto failure recovery
    if (!spHw.StartSocketPool(cc, 1)) {
        std::wcout << "Failed in connecting to remote helloworld server" << std::endl;
        return -1;
    }
    auto hw = spHw.Seek();
    CMyStruct ms0, ms;
    SetMyStruct(ms0);
    try {
        //process requests one by one synchronously
        std::wcout << hw->async<std::wstring>(idSayHelloHelloWorld, L"John", L"Dole").get() << std::endl;
        hw->async0(idSleepHelloWorld, (unsigned int) 4000).get();
        ms = hw->async<CMyStruct>(idEchoHelloWorld, ms0).get();
        assert(ms == ms0);

        //asynchronously process multiple requests with in-line batching for best network efficiency
        auto f0 = hw->async<std::wstring>(idSayHelloHelloWorld, L"Jack", L"Smith");
        auto f1 = hw->async<CMyStruct>(idEchoHelloWorld, ms0);
        auto f2 = hw->async0(idSleepHelloWorld, (unsigned int) 15000);
        auto f3 = hw->async<std::wstring>(idSayHelloHelloWorld, L"Donald", L"Trump");
        auto f4 = hw->async<std::wstring>(idSayHelloHelloWorld, L"Hilary", L"Trump");

        //waiting ......
        std::wcout << f0.get() << std::endl;
        std::wcout << "Echo equal: " << (ms == f1.get()) << std::endl;
        std::wcout << "Sleep returns " << f2.get()->GetSize() << " byte because server side returns nothing" << std::endl;
        std::wcout << f3.get() << std::endl;
        std::wcout << f4.get() << std::endl;
    } catch (CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    } catch (CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    } catch (std::exception & ex) {
        std::wcout << "Some unexpected error: " << ex.what() << std::endl;
    }

    if (!hw->SendRequest(idSayHelloHelloWorld, L"SocketPro", L"UDAParts", [](CAsyncResult & ar) {
            std::wstring s;
            ar >> s;
            std::wcout << s << std::endl;
        }, [](CAsyncServiceHandler *ash, bool canceled) {
            if (canceled) {
                std::cout << "Request SendRequest canceled" << std::endl;
            } else {
                CClientSocket *cs = ash->GetAttachedClientSocket();
                int ec = cs->GetErrorCode();
                if (ec) {
                    std::string em = cs->GetErrorMsg();
                    std::cout << "ec: " << ec << ", em: " << em << std::endl;
                } else {
                    std::wcout << "ec: " << HelloWorld::SESSION_CLOSED_AFTER << ", em: Session closed after sending the request SendRequest" << std::endl;
                }
            }
        }, [](CAsyncServiceHandler *ash, unsigned short reqId, const wchar_t *errMsg, const char* errWhere, unsigned int errCode) {
            std::wcout << L"Server exception error message: " << errMsg;
            std::wcout << ", location: " << errWhere << std::endl;
        })) {
        std::wcout << "ec: " << HelloWorld::SESSION_CLOSED_BEFORE << ", em: Session already closed before sending the request SendRequest" << std::endl;
    }

    std::wcout << L"Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
