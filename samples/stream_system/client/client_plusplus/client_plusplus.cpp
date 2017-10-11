
#include "stdafx.h"
#include "webasynchandler.h"

typedef SPA::CMasterPool<false, CWebAsyncHandler> CWebMasterPool;
typedef CWebMasterPool::CSlavePool CSlavePool;

typedef SPA::ClientSide::CSocketPool<CWebAsyncHandler> CMyPool;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    std::cout << "Remote host: " << std::endl;
    std::getline(std::cin, cc.Host);
    //cc.Host = "localhost";
    cc.Port = 20901;
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
    bool ok = master.StartSocketPool(cc, 2, 1);
    if (!ok) {
        std::cout << "Failed in connecting to remote middle tier server, and press any key to close the application ......" << std::endl;
        ::getchar();
        return 1;
    }
    auto handler = master.Lock();
    SPA::CDataSet &cache = CWebMasterPool::Cache;
    ok = handler->GetMasterSlaveConnectedSessions([](unsigned int master_connection, unsigned int slave_connection) {
        std::cout << "master connection: " << master_connection << ", slave connection: " << slave_connection << std::endl;
    });
    ok = handler->QueryPaymentMaxMinAvgs(L"", [](const CMaxMinAvg &mma, int res, const std::wstring & errMsg) {
        if (res) {
            std::cout << "QueryPaymentMaxMinAvgs error code: " << res << ", error message: ";
            std::wcout << errMsg.c_str() << std::endl;
        } else {
            std::cout << "QueryPaymentMaxMinAvgs max: " << mma.Max << ", min: " << mma.Min << ", avg: " << mma.Avg << std::endl;
        }
    });
    ok = handler->WaitAll();
    master.Unlock(handler);
    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
