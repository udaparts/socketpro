
#include "stdafx.h"
#include "webasynchandler.h"

typedef SPA::CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef SPA::ClientSide::CSocketPool<CWebAsyncHandler> CMyPool;

int main(int argc, char* argv[]) {
    std::wstring filter;
    CConnectionContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    std::cout << "Sakila.payment filter: " << std::endl;
    std::getline(std::wcin, filter);
    //cc.Host = "localhost";
    cc.Port = 20911;
    cc.UserId = L"SomeUserId";
    cc.Password = L"A_Password_For_SomeUserId";
    cc.EncrytionMethod = SPA::tagEncryptionMethod::TLSv1;

    CWebMasterPool master(L"");

    //CA file is located at the directory ../socketpro/bin
    CClientSocket::SSL::SetVerifyLocation("ca.cert.pem");

    auto cb = [](CMyPool *sender, CClientSocket * cs)->bool {
        int errCode;
        SPA::IUcert *cert = cs->GetUCert();
        //std::cout << cert->SessionInfo << std::endl;
        const char* res = cert->Verify(&errCode);
        //do ssl server certificate authentication here
        return (errCode == 0); //true -- user id and password will be sent to server
    };
    master.DoSslServerAuthentication = cb;

    bool ok = master.StartSocketPool(cc, 4, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote middle tier server, and press any key to close the application ......" << std::endl;
        ::getchar();
        return 1;
    }
    //No need to use the methods Lock and Unlock for removing request stream overlap
    //as the following calls do not care for request stream overlap on one session between front and mid-tier
    auto handler = master.Seek();
    SPA::CDataSet &cache = master.Cache; //accessing real-time update cache

    SPA::UINT64 call_index = handler->GetMasterSlaveConnectedSessions([](SPA::UINT64 index, unsigned int master_connection, unsigned int slave_connection) {
        std::cout << "master connection: " << master_connection << ", slave connection: " << slave_connection << std::endl;
    });
    call_index = handler->QueryPaymentMaxMinAvgs(filter.c_str(), [](SPA::UINT64 index, const CMaxMinAvg &mma, int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "QueryPaymentMaxMinAvgs error code: " << res << ", error message: ";
            std::wcout << errMsg.c_str() << std::endl;
        } else {
            std::cout << "QueryPaymentMaxMinAvgs max: " << mma.Max << ", min: " << mma.Min << ", avg: " << mma.Avg << std::endl;
        }
    });
    SYSTEMTIME st;
    CDBVariantArray vData;

    vData.push_back(1); //Google company id
    vData.push_back(L"Ted Cruz");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);

    vData.push_back(1); //Google company id
    vData.push_back("Donald Trump");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);

    vData.push_back(2); //Microsoft company id
    vData.push_back("Hillary Clinton");
#ifdef WIN32_64
    ::GetLocalTime(&st);
#else
    ::gettimeofday(&st, nullptr);
#endif
    vData.push_back(st);
    std::shared_ptr<std::promise<void> > prom(new std::promise<void>, [](std::promise<void> *p) {
        delete p;
    });
    call_index = handler->UploadEmployees(vData, [prom](SPA::UINT64 index, int res, const std::wstring &errMsg, CInt64Array & vId) {
        if (res) {
            std::cout << "UploadEmployees Error code = " << res << ", error message = ";
            std::wcout << errMsg.c_str() << std::endl;
        } else {
            for (auto it = vId.cbegin(), end = vId.cend(); it != end; ++it) {
                std::cout << "Last id: " << *it << std::endl;
            }
        }
        prom->set_value();
    }, [prom](SPA::UINT64 index) {
        std::cout << "Socket closed or request cancelled" << std::endl;
        prom->set_value();
    });
    if (call_index) {
        //Use wait_for instead of handler->WaitAll() for better completion event as a session may be shared by multiple threads
        auto status = prom->get_future().wait_for(std::chrono::seconds(5));
        if (status == std::future_status::timeout) {
            std::cout << "The above requests are not completed in 5 seconds" << std::endl;
        }
    } else
        std::cout << "Socket already closed before sending request" << std::endl;

    std::cout << "Press a key to test random returning ......" << std::endl;
    ::getchar();
    CMaxMinAvg sum_mma;
    ::memset(&sum_mma, 0, sizeof (sum_mma));
    auto start = std::chrono::system_clock::now();
    for (unsigned int n = 0; n < 10000; ++n) {
        call_index = handler->QueryPaymentMaxMinAvgs(filter.c_str(), [&sum_mma](SPA::UINT64 index, const CMaxMinAvg & mma, int res, const std::wstring & errMsg) {
            if (res) {
                std::cout << "QueryPaymentMaxMinAvgs call index: " << index << ", error code: " << res << ", error message: ";
                std::wcout << errMsg.c_str() << std::endl;
            } else {
                sum_mma.Avg += mma.Avg;
                sum_mma.Max += mma.Max;
                sum_mma.Min += mma.Min;
                //std::cout << "QueryPaymentMaxMinAvgs call index: " << index << std::endl;
            }
        });
    }
    auto v = master.GetAsyncHandlers();
    for (auto it = v.begin(), end = v.end(); it != end; ++it) {
        ok = (*it)->WaitAll();
    }
    std::chrono::duration<double> diff = (std::chrono::system_clock::now() - start);
    std::cout << "Time required: " << diff.count() << " seconds" << std::endl;
    std::cout << "QueryPaymentMaxMinAvgs sum_max: " << sum_mma.Max << ", sum_min: " << sum_mma.Min << ", sum_avg: " << sum_mma.Avg << std::endl;

    std::cout << "Press a key to test sequence returning ......" << std::endl;
    ::getchar();
    CWebAsyncHandler::DRentalDateTimes rdt = [](SPA::UINT64 index, const CRentalDateTimes &dates, int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "GetRentalDateTimes call index: " << index << ", error code: " << res << ", error message: ";
            std::wcout << errMsg.c_str() << std::endl;
        } else if (dates.Rental == 0 && dates.Return == 0 && dates.LastUpdate == 0) {
            std::cout << "GetRentalDateTimes call index: " << index << " rental_id=" << dates.rental_id << " not available" << std::endl;
        } else {
            SPA::UDateTime rental_date(dates.Rental), return_date(dates.Return), laste_update(dates.LastUpdate);
            std::cout << "GetRentalDateTimes call index: " << index << " rental_id=" << dates.rental_id << " and dates (" << rental_date.ToDBString() << ", " << return_date.ToDBString() << ", " << laste_update.ToDBString() << ")" << std::endl;
        }
    };
    //all requests should be returned in sequence
    for (unsigned int n = 0; n < 1000; ++n) {
        call_index = handler->GetRentalDateTimes(n + 1, rdt);
    }
    ok = handler->WaitAll();

    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
