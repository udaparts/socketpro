#if __has_include(<coroutine>)
#include <coroutine>
#elif __has_include(<experimental/coroutine>)
#include <experimental/coroutine>
#else
static_assert(false, "No co_await support");
#endif
#include <iostream>
#include "../../../../include/streamingfile.h"

using namespace std;
using namespace SPA;
using namespace SPA::ClientSide;

typedef CSocketPool<CStreamingFile> CMyPool;

CAwTask MyTest(CMyPool::PHandler& file, wstring& remoteFile) {
    try {
        wstring localFile(L"spfile.test");
        auto aw0 = file->wait_download(localFile.c_str(), remoteFile.c_str(), [](CStreamingFile* file, SPA::UINT64 downloaded) {
            wcout << "Downloading rate: " << (downloaded * 100) / file->GetFileSize() << "%\n";
        });
        remoteFile += L".copy";
        auto aw1 = file->wait_upload(localFile.c_str(), remoteFile.c_str(), [](CStreamingFile* file, SPA::UINT64 uploaded) {
            wcout << "Uploading rate: " << (uploaded * 100) / file->GetFileSize() << "%\n";
        });
        vector<CStreamingFile::FileWaiter> v = { aw0, aw1 };
        for (auto& w : v) {
            wcout << (co_await w).ToString() << "\n";
        }
    }
    catch (CServerError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (CSocketError& ex) {
        wcout << ex.ToString() << "\n";
    }
    catch (exception& ex) {
        wcout << "Unexpected error: " << ex.what() << "\n";
    }
}

//compile options
//Visual C++ 2017 & 2019 16.8.0 before -- /await
//Visual C++ 2019 16.8.0 preview 3.1 or later -- /std:c++latest
//GCC 10.0.1 or later -- -std=c++20 -fcoroutines -ldl -lstdc++ -pthread
//GCC 11 or clang 14 -- -std=c++20 -ldl -lstdc++ -pthread
int main(int argc, char* argv[]) {
    CMyPool spFile;
    wstring remoteFile = L"earthcity.jpg";
    //wcout << "Input a remote file to download ......\n";
    //getline(wcin, remoteFile); //not working with co_await
    CConnectionContext cc("windesk", 20901, L"MyUserId", L"MyPassword");
    spFile.StartSocketPool(cc, 1);
    auto file = spFile.Seek();
    if (!file) {
        wcout << "No connection to remote server\n";
    }
    else {
        MyTest(file, remoteFile);
    }
    wcout << L"Press a key to kill the demo ......\n";
    ::getchar();
    return 0;
}
