
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
    //test both downloading and uploading files in file stream (it is different from byte stream)
    std::wstring RemoteFile = L"jvm.lib";
    std::wstring LocalFile(L"spfile1.test");
    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        //downloading progress
        std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile2.test";
    RemoteFile = L"libboost_wave-vc100-mt-sgd-1_60.lib";
    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        //std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile3.test";
    RemoteFile = L"libboost_coroutine-vc100-mt-s-1_60.lib";
    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        //std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile4.test";
    RemoteFile = L"libboost_serialization-vc100-mt-s-1_60.lib";
    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        //std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile5.test";
    RemoteFile = L"libboost_math_tr1f-vc100-mt-sgd-1_60.lib";
    //downloading test
    ok = rf->Download(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else
            std::wcout << L"Downloading " << RemoteFile << L" completed" << std::endl;
    }, [](CStreamingFile *file, SPA::UINT64 downloaded) {
        //std::cout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%" << std::endl;
    });
    ok = rf->WaitAll();

    //uploading test
    LocalFile = L"spfile1.test";
    RemoteFile = L"jvm_copy.lib";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        //uploading progress
        std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile2.test";
    RemoteFile = L"libboost_wave-vc100-mt-sgd-1_60_copy.lib";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        //std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile3.test";
    RemoteFile = L"libboost_coroutine-vc100-mt-s-1_60_copy.lib";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        //std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile4.test";
    RemoteFile = L"libboost_serialization-vc100-mt-s-1_60_copy.lib";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        //std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });

    LocalFile = L"spfile5.test";
    //uploading test
    RemoteFile = L"libboost_math_tr1f-vc100-mt-sgd-1_60_copy.lib";
    ok = rf->Upload(LocalFile.c_str(), RemoteFile.c_str(), [RemoteFile](CStreamingFile *file, int res, const std::wstring & errMsg) {
        if (res) {
            std::wcout << L"Error code: " << res << L", error message: " << errMsg << std::endl;
        } else {
            std::wcout << L"Uploading " << RemoteFile << L" completed" << std::endl;
        }
    }, [](CStreamingFile *file, SPA::UINT64 uploaded) {
        //std::cout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%" << std::endl;
    });
    ok = rf->WaitAll();
    std::cout << "Press a key to shutdown the demo application ......" << std::endl;
    ::getchar();
    return 0;
}
