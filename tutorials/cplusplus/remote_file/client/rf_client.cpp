#include "stdafx.h"

using namespace SPA;
using namespace SPA::ClientSide;

int main(int argc, char* argv[]) {
    CConnectionContext cc;
    cc.Host = "localhost";
    cc.Port = 20901;
    cc.UserId = L"MyUserId";
    cc.Password = L"MyPassword";

    typedef CSocketPool<CStreamingFile, CClientSocket> CMyPool;
#ifndef NDEBUG
    CMyPool spRf(true, 7200000); //large timeout for better debugging
#else
    CMyPool spRf;
#endif
    if (!spRf.StartSocketPool(cc, 1)) {
        std::cout << "Can not connect to remote server" << std::endl;
        return -1;
    }
    auto rf = spRf.Seek();
    std::cout << "Input a remote file to download ......" << std::endl;
    std::wstring RemoteFile;
    std::getline(std::wcin, RemoteFile);
    std::wstring LocalFile(L"spfile.test");
    //test both downloading and uploading files in file stream (it is different from byte stream)
    try{
        //downloading test
        std::future<ErrInfo> fd = rf->download(LocalFile.c_str(), RemoteFile.c_str(), [](CStreamingFile *file, SPA::UINT64 downloaded) {
            std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
        });
        //uploading test
        RemoteFile += L".copy";
        std::future<ErrInfo> fu = rf->upload(LocalFile.c_str(), RemoteFile.c_str(), [](CStreamingFile *file, SPA::UINT64 uploaded) {
            std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
        });
        std::wcout << fd.get().ToString() << std::endl;
        std::wcout << fu.get().ToString() << std::endl;
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
