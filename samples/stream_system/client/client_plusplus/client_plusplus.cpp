
#include "stdafx.h"
#include "webasynchandler.h"

typedef SPA::CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef SPA::ClientSide::CSocketPool<CWebAsyncHandler> CMyPool;

int main(int argc, char* argv[]) {
    std::wstring filter;
    CConnectionContext cc;
    std::cout << "Remote middle tier host: " << std::endl;
    std::getline(std::cin, cc.Host);
    std::cout << "Sakila.payment filter: " << std::endl;
#ifdef WIN32_64
    std::getline(std::wcin, filter);
#else
    std::string temp;
    std::getline(std::cin, temp);
    filter = SPA::Utilities::ToWide(temp.c_str(), temp.size());
#endif
    //cc.Host = "localhost";
    cc.Port = 20911;
    cc.UserId = L"SomeUserId";
    cc.Password = L"A_Password_For_SomeUserId";
    cc.EncrytionMethod = SPA::tagEncryptionMethod::TLSv1;
#ifndef NDEBUG
    CWebMasterPool master(L"", 3000000); //for your easy debug
#else
    CWebMasterPool master(L"");
#endif

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

    bool ok = master.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote middle tier server, and press any key to close the application ......" << std::endl;
        ::getchar();
        return 1;
    }
    //No need to use the methods Lock and Unlock for removing request stream overlap
    //as the following calls do not care for request stream overlap on one session between front and mid-tier
    auto handler = master.Seek();
    SPA::CDataSet &cache = master.Cache; //accessing real-time update cache

    ok = handler->GetMasterSlaveConnectedSessions([](unsigned int master_connection, unsigned int slave_connection) {
        std::cout << "master connection: " << master_connection << ", slave connection: " << slave_connection << std::endl;
    });

    ok = handler->QueryPaymentMaxMinAvgs(filter.c_str(), [](const CMaxMinAvg &mma, int res, const std::wstring & errMsg) {
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
    ok = handler->UploadEmployees(vData, [prom](int res, const std::wstring &errMsg, CInt64Array & vId) {
        if (res) {
            std::cout << "UploadEmployees Error code = " << res << ", error message = ";
            std::wcout << errMsg.c_str() << std::endl;
        } else {
            for (auto it = vId.cbegin(), end = vId.cend(); it != end; ++it) {
                std::cout << "Last id: " << *it << std::endl;
            }
        }
        prom->set_value();
    }, [prom](CAsyncServiceHandler *ash, bool canceled) {
        std::cout << "Socket closed" << std::endl;
        prom->set_value();
    });
    if (ok) {
        //Use wait_for instead of handler->WaitAll() for better completion event as a session may be shared by multiple threads
#ifndef NDEBUG
        auto status = prom->get_future().wait_for(std::chrono::seconds(500));
#else
        auto status = prom->get_future().wait_for(std::chrono::seconds(5));
#endif
        if (status == std::future_status::timeout) {
            std::cout << "The above requests are not completed in 500 seconds" << std::endl;
        }
    } else
        std::cout << "Socket already closed before sending request" << std::endl;

    std::cout << "Press a key to test requests parallel processing and fault tolerance at server side ......" << std::endl;
    ::getchar();
    CMaxMinAvg sum_mma;
    ::memset(&sum_mma, 0, sizeof (sum_mma));
    auto start = std::chrono::system_clock::now();
    for (unsigned int n = 0; n < 10000; ++n) {
        ok = handler->QueryPaymentMaxMinAvgs(filter.c_str(), [&sum_mma](const CMaxMinAvg & mma, int res, const std::wstring & errMsg) {
            if (res) {
                std::cout << "Error code: " << res << ", error message: ";
                std::wcout << errMsg.c_str() << std::endl;
            } else {
                sum_mma.Avg += mma.Avg;
                sum_mma.Max += mma.Max;
                sum_mma.Min += mma.Min;
            }
        });
        if (!ok) {
            std::cout << "Socket closed" << std::endl;
            break;
        }
    }
    if (ok)
        ok = handler->WaitAll();
    std::chrono::duration<double> diff = (std::chrono::system_clock::now() - start);
    std::cout << "Time required: " << diff.count() << " seconds" << std::endl;
    std::cout << "QueryPaymentMaxMinAvgs sum_max: " << sum_mma.Max << ", sum_min: " << sum_mma.Min << ", sum_avg: " << sum_mma.Avg << std::endl;
    std::cout << "Press a key to test requests server parallel processing, fault tolerance and sequence returning ......" << std::endl;
    ::getchar();
    SPA::INT64 prev_rental_id = 0;
    CWebAsyncHandler::DRentalDateTimes rdt = [&prev_rental_id](const CRentalDateTimes &dates, int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "GetRentalDateTimes error code: " << res << ", error message: ";
            std::wcout << errMsg.c_str() << std::endl;
            prev_rental_id = 0;
        } else if (dates.Rental == 0 && dates.Return == 0 && dates.LastUpdate == 0) {
            std::cout << "GetRentalDateTimes rental_id = " << dates.rental_id << " not available" << std::endl;
            prev_rental_id = 0;
        } else {
            SPA::UDateTime rental_date(dates.Rental), return_date(dates.Return), laste_update(dates.LastUpdate);
            if (0 == prev_rental_id || dates.rental_id == prev_rental_id + 1) {
                std::cout << "GetRentalDateTimes rental_id = " << dates.rental_id << " and dates (" << rental_date.ToDBString() << ", " << return_date.ToDBString() << ", " << laste_update.ToDBString() << ")" << std::endl;
            } else
                std::cout << "****** GetRentalDateTimes returned out of order ******" << std::endl;
            prev_rental_id = dates.rental_id;
        }
    };
    //all requests should be returned in sequence (max rental_id = 16049)
    for (unsigned int n = 0; n < 1000; ++n) {
        ok = handler->GetRentalDateTimes(n + 1, rdt);
        if (!ok) {
            std::cout << "Socket closed" << std::endl;
        }
    }
    ok = handler->WaitAll();
    if (!ok) {
        std::cout << "Socket closed" << std::endl;
    }
    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
