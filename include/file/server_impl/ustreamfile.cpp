
#include "sfileimpl.h"

std::shared_ptr<SPA::ServerSide::CSFileService> g_pFile;
std::wstring g_pathRoot;

void WINAPI SetRootDirectory(const wchar_t *pathRoot) {
    if (pathRoot && ::wcslen(pathRoot))
        g_pathRoot = pathRoot;
#ifdef WIN32_64
    if (g_pathRoot.back() != L'\\')
        g_pathRoot += L'\\';
#else
    if (g_pathRoot.back() != L'/')
        g_pathRoot += L'/';
#endif
}

bool WINAPI InitServerLibrary(int param) {
    SPA::CScopeUQueue sb;
#ifdef WIN32_64
    ::GetCurrentDirectoryW(sb->GetMaxSize() / sizeof (wchar_t), (wchar_t*) sb->GetBuffer());
    g_pathRoot = (const wchar_t*) sb->GetBuffer();
    if (g_pathRoot.back() != L'\\')
        g_pathRoot += L'\\';
#else
    char *s = getcwd((char*) sb->GetBuffer(), sb->GetMaxSize());
    g_pathRoot = SPA::Utilities::ToWide((const char*) sb->GetBuffer());
    if (g_pathRoot.back() != L'/')
        g_pathRoot += L'/';
#endif
    g_pFile.reset(new SPA::ServerSide::CSFileService(SPA::SFile::sidFile, SPA::taNone));
    return true;
}

void WINAPI UninitServerLibrary() {
    g_pFile.reset();
}

unsigned short WINAPI GetNumOfServices() {
    return 1; //The library exposes 1 service only
}

unsigned int WINAPI GetAServiceID(unsigned short index) {
    if (index == 0)
        return SPA::SFile::sidFile;
    return 0;
}

CSvsContext WINAPI GetOneSvsContext(unsigned int serviceId) {
    CSvsContext sc;
    if (g_pFile && serviceId == SPA::SFile::sidFile)
        sc = g_pFile->GetSvsContext();
    else
        memset(&sc, 0, sizeof (sc));
    return sc;
}

unsigned short WINAPI GetNumOfSlowRequests(unsigned int serviceId) {
    return 4; //The service only has four slow requests
}

unsigned short WINAPI GetOneSlowRequestID(unsigned int serviceId, unsigned short index) {
    //The following four requests are slow ones
    switch (index) {
        case 0:
            return SPA::SFile::idDownload;
        case 1:
            return SPA::SFile::idUpload;
        case 2:
            return SPA::SFile::idUploadCompleted;
        case 3:
            return SPA::SFile::idUploading;
        default:
            break;
    }
    return 0;
}
