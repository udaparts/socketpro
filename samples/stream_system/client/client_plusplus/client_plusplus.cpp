#include "stdafx.h"
#include "webasynchandler.h"
#include <deque>

typedef CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef CSocketPool<CWebAsyncHandler> CMyPool;
typedef deque<future<CScopeUQueue>> CQFuture;

int main(int argc, char* argv[]) {
    wstring filter;
    CConnectionContext cc;
    cout << "Remote middle tier host: \n";
    getline(cin, cc.Host);
    cout << "Sakila.payment filter: \n";
    string temp;
    //for example: payment_id between 1 and 49
    getline(cin, temp);
    filter = Utilities::ToWide(temp.c_str(), temp.size());
    cc.Port = 20911;
    cc.UserId = L"SomeUserId";
    cc.Password = L"A_Password_For_SomeUserId";
    cc.EncrytionMethod = SPA::tagEncryptionMethod::TLSv1;
#ifndef NDEBUG
    CWebMasterPool master(L"", 3000000); //for your easy debug
#else
    CWebMasterPool master(L"");
#endif

#ifndef WIN32_64
    //CA file is located at the directory ../socketpro/bin
    CClientSocket::SSL::SetVerifyLocation("ca.cert.pem");
#endif

    master.DoSslServerAuthentication = [](CMyPool *sender, CClientSocket * cs)->bool {
        int errCode;
        IUcert *cert = cs->GetUCert();
        //cout << cert->SessionInfo << endl;
        const char* res = cert->Verify(&errCode);
        //do ssl server certificate authentication here
        return (errCode == 0); //true -- user id and password will be sent to server
    };
    //master.SetQueueName("mcqueue");
    if (!master.StartSocketPool(cc, 1)) {
        cout << "No connection to remote middle tier server, and press a key to kill the demo ......\n";
        ::getchar();
        return 1;
    }
    auto handler = master.Seek();
    //accessing real-time update cache
    CDataSet &cache = master.Cache;

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

    try{
        auto fms = handler->sendRequest(idGetMasterSlaveConnectedSessions);
        auto fmma = handler->sendRequest(idQueryMaxMinAvgs, filter);
        auto fue = handler->sendRequest(idUploadEmployees, vData);

        unsigned int master_connection, slave_connection;
        int res;
        wstring errMsg;
        CMaxMinAvg mma;
        fms.get() >> master_connection >> slave_connection;
        cout << "master connections: " << master_connection << ", slave connections: " << slave_connection << endl;
        fmma.get() >> res >> errMsg >> mma;
        cout << "QueryPaymentMaxMinAvgs\n";
        if (res) {
            cout << "\terror code: " << res << ", error message: ";
            wcout << errMsg.c_str() << endl;
        } else {
            cout << "\tmax: " << mma.Max << ", min: " << mma.Min << ", avg: " << mma.Avg << endl;
        }
        auto status = fue.wait_for(chrono::seconds(5));
        if (status == future_status::timeout) {
            cout << "The request UploadEmployees not completed in 5 seconds\n";
        } else {
            int res;
            wstring errMsg;
            CInt64Array vId;
            fue.get() >> res >> errMsg >> vId;
            cout << "UploadEmployees\n";
            if (res) {
                cout << "\tError code: " << res << ", message: ";
                wcout << errMsg << endl;
            } else {
                for (auto id : vId) {
                    cout << "\tLast id: " << id << endl;
                }
            }
        }
        cout << "Press a key to test server parallel processing ......\n";
        ::getchar();
        CQFuture qF;
        CMaxMinAvg s_mma;
        ::memset(&s_mma, 0, sizeof (s_mma));
        cout << "QueryPaymentMaxMinAvgs\n";
        auto start = chrono::system_clock::now();
        for (unsigned int n = 0; n < 10000; ++n) {
            qF.push_back(handler->sendRequest(idQueryMaxMinAvgs, filter));
        }
        size_t count = qF.size();
        while (qF.size()) {
            qF.front().get() >> res >> errMsg >> mma;
            if (res) {
                cout << "\tError code: " << res << ", message: ";
                wcout << errMsg << endl;
            } else {
                s_mma.Avg += mma.Avg;
                s_mma.Max += mma.Max;
                s_mma.Min += mma.Min;
            }
            qF.pop_front();
        }
        chrono::duration<double> diff = (chrono::system_clock::now() - start);
        cout << "\tTime required: " << diff.count() << " seconds for " << count << " requests\n";
        cout << "\tsum_max: " << s_mma.Max << ", sum_min: " << s_mma.Min << ", sum_avg: " << s_mma.Avg << endl;
        cout << "Press a key to test server parallel processing and sequence returning ......\n";
        ::getchar();
        for (SPA::INT64 n = 0; n < 16000; ++n) {
            qF.push_back(handler->sendRequest(idGetRentalDateTimes, n + 1));
        }
        cout << "GetRentalDateTimes:\n";
        SPA::INT64 prev_rental_id = 0;
        while (qF.size()) {
            CRentalDateTimes dates;
            qF.front().get() >> dates >> res >> errMsg;
            if (res) {
                cout << "\tError code: " << res << ", message: ";
                wcout << errMsg << endl;
            } else if (dates.Rental == 0 && dates.Return == 0 && dates.LastUpdate == 0) {
                cout << "\trental_id: " << dates.rental_id << " not available\n";
            } else {
                UDateTime rental_date(dates.Rental), return_date(dates.Return), laste_update(dates.LastUpdate);
                if (0 == prev_rental_id || dates.rental_id == prev_rental_id + 1) {
                    //cout << "\trental_id: " << dates.rental_id << " and dates (" << rental_date.ToDBString() << ", " << return_date.ToDBString() << ", " << laste_update.ToDBString() << ")\n";
                } else
                    cout << "\t****** returned out of order ******\n";
            }
            prev_rental_id = dates.rental_id;
            qF.pop_front();
        }
    }

    catch(CSocketError & ex) {
        wcout << ex.ToString() << endl;
    }

    cout << "Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}
