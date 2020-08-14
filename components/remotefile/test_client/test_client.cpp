#include "stdafx.h"

using namespace SPA;
using namespace SPA::ClientSide;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    std::cout << "Remote SocketPro file streaming server:" << std::endl;
    std::getline(std::cin, cc.Host);
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    typedef CSocketPool<CStreamingFile, CClientSocket> CMyPool;
#ifndef NDEBUG
    CMyPool spRf(true, 7200000); //for better debugging
#else
    CMyPool spRf;
#endif
    if (!spRf.StartSocketPool(cc, 1)) {
        std::cout << "Can not connect to remote server with error code: " << spRf.GetSockets()[0]->GetErrorCode() << std::endl;
        return -1;
    }
    auto rf = spRf.Seek();

    //test both downloading and uploading files in file stream (it is different from byte stream)
    std::wstring RemoteFile = L"jvm.lib";
    std::wstring LocalFile(L"spfile1.test");
    try{
        //downloading test
        std::future<ErrInfo> fd0 = rf->download(LocalFile.c_str(), RemoteFile.c_str(), [](CStreamingFile* file, SPA::UINT64 downloaded) {
            std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
        });

        LocalFile = L"spfile2.test";
        RemoteFile = L"libboost_wave-vc100-mt-sgd-1_60.lib";
        auto fd1 = rf->download(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile3.test";
        RemoteFile = L"libboost_coroutine-vc100-mt-s-1_60.lib";
        auto fd2 = rf->download(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile4.test";
        RemoteFile = L"libboost_serialization-vc100-mt-s-1_60.lib";
        auto fd3 = rf->download(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile5.test";
        RemoteFile = L"libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
        auto fd4 = rf->download(LocalFile.c_str(), RemoteFile.c_str());

        //uploading test
        LocalFile = L"spfile1.test";
        RemoteFile = L"jvm_copy.lib";
        std::future<ErrInfo> fu0 = rf->upload(LocalFile.c_str(), RemoteFile.c_str(), [](CStreamingFile* file, SPA::UINT64 uploaded) {
            std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
        });

        LocalFile = L"spfile2.test";
        RemoteFile = L"libboost_wave-vc100-mt-sgd-1_60_copy.lib";
        auto fu1 = rf->upload(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile3.test";
        RemoteFile = L"libboost_coroutine-vc100-mt-s-1_60_copy.lib";
        auto fu2 = rf->upload(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile4.test";
        RemoteFile = L"libboost_serialization-vc100-mt-s-1_60_copy.lib";
        auto fu3 = rf->upload(LocalFile.c_str(), RemoteFile.c_str());

        LocalFile = L"spfile5.test";
        //uploading test
        RemoteFile = L"libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
        auto fu4 = rf->upload(LocalFile.c_str(), RemoteFile.c_str());

        std::wcout << fd0.get().ToString() << std::endl;
        std::wcout << fd1.get().ToString() << std::endl;
        std::wcout << fd2.get().ToString() << std::endl;
        std::wcout << fd3.get().ToString() << std::endl;
        std::wcout << fd4.get().ToString() << std::endl;
        std::wcout << fu0.get().ToString() << std::endl;
        std::wcout << fu1.get().ToString() << std::endl;
        std::wcout << fu2.get().ToString() << std::endl;
        std::wcout << fu3.get().ToString() << std::endl;
        std::wcout << fu4.get().ToString() << std::endl;
    }

    catch(CServerError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(CSocketError & ex) {
        std::wcout << ex.ToString() << std::endl;
    }

    catch(std::exception & ex) {
        std::cout << "Some unexpected error: " << ex.what() << std::endl;
    }

    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
