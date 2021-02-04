
#include "sfileimpl.h"
#include<memory>

std::shared_ptr<SPA::ServerSide::CSFileService> g_pFile;

bool WINAPI InitServerLibrary(int param) {
    SPA::CScopeUQueue sb;
    sb->CleanTrack();
#ifdef WIN32_64
    ::GetCurrentDirectoryW(sb->GetMaxSize() / sizeof (wchar_t), (wchar_t*) sb->GetBuffer());
#else
    char* s = get_current_dir_name();
    SPA::Utilities::ToWide((const char*)s, ::strlen((const char*)s, *sb);
#endif
    SPA::ServerSide::CSFileImpl::SetRootDirectory((const wchar_t *)sb->GetBuffer());
    g_pFile.reset(new SPA::ServerSide::CSFileService(SPA::SFile::sidFile, SPA::tagThreadApartment::taNone));
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
    return 5; //The service only has four slow requests
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
        case 4:
            return SPA::SFile::idUploadBackup;
        default:
            break;
    }
    return 0;
}
