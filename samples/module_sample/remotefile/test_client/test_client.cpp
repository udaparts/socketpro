
#include "stdafx.h"
#include "../../../../include/streamingfile.h"

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
    CMyPool spRf(true, 7200000);
#else
    CMyPool spRf;
#endif
    bool ok = spRf.StartSocketPool(cc, 1, 1);
    if (!ok) {
        std::cout << "Can not connect to remote server" << std::endl;
        return -1;
    }
    auto rf = spRf.Seek();
    std::cout << "Input a remote file to download ......" << std::endl;
    std::wstring RemoteFile;
    std::getline(std::wcin, RemoteFile);
    std::string LocalFile("spfile.test");
    /*
        ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [LocalFile, RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
            if (res) {
                std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
            } else {
                std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
            }
        }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
            std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
        });

        RemoteFile = L"libboost_graph-vc100-mt-sgd-1_60_copy.lib";
        LocalFile = "spfile2.test";
        ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [LocalFile, RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
            if (res) {
                std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
            } else {
                std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
            }
        }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
            std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
        });

        RemoteFile = L"libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
        LocalFile = "spfile3.test";
        ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [LocalFile, RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
            if (res) {
                std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
            } else {
                std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
            }
        }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
            std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
        });
     */

    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [LocalFile, RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });
    RemoteFile += L".copy";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [LocalFile, RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });
    ok = rf->WaitAll();
    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}

