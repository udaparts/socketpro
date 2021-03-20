#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "sfileimpl.h"

#ifndef WIN32_64
#include <sys/stat.h>
#endif

using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.6");

#ifdef WIN32_64

const char* const U_MODULE_OPENED WINAPI GetSPluginVersion() {
    return g_version.c_str();
}
#else

SPA::INT64 U_MODULE_OPENED WINAPI GetSPluginVersion() {
    return (SPA::INT64)g_version.c_str();
}
#endif

bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char* jsonOptions) {
    if (!jsonOptions) return false;
    std::unique_ptr<JSON::JValue<char>> jv(JSON::Parse(jsonOptions));
    if (!jv) {
        return false;
    }
    JSON::JValue<char>* v = jv->Child(UFILE_ROOT_DIRECTORY);
    if (v && v->GetType() == JSON::enumType::String) {
        std::string s = v->AsString();
        Trim(JSON::Unescape(s));
#ifdef WIN32_64
        std::wstring ws = Utilities::ToWide(s);
        DWORD dwAttrs = GetFileAttributesW(ws.c_str());
        if (dwAttrs == INVALID_FILE_ATTRIBUTES) return false;
        if ((dwAttrs & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) return false;
#else
        struct stat sb;
        if (stat(s.c_str(), &sb) == -1) return false;
        if ((sb.st_mode & S_IFMT) != S_IFDIR) return false;
        std::wstring ws = Utilities::ToWide(s);
#endif
        CSFileImpl::SetRootDirectory(ws.c_str());
    }
    v = jv->Child(MANUAL_BATCHING);
    if (v && v->GetType() == JSON::enumType::Uint64) {
        int mb = (int) v->AsUint64();
        CSFileImpl::m_mb = (tagMaualBatching) mb;
    }
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    JSON::JObject<char> obj;
    tagMaualBatching mb = CSFileImpl::m_mb;
    obj[MANUAL_BATCHING] = (int) mb;
    obj[UFILE_ROOT_DIRECTORY] = Utilities::ToUTF8(CSFileImpl::GetRootDirectory());
    obj[PLUGIN_SERVICE_ID] = SPA::SFile::sidFile;
    JSON::JValue<char> jv(std::move(obj));
    std::string s = jv.Stringify(false);
    size_t len = s.size();
    if (len > buffer_size - 1) {
        len = buffer_size - 1;
    }
    if (len) {
        memcpy(json, s.c_str(), len);
    }
    json[len] = 0;
    return (unsigned int) len;
}

int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t* userId, const wchar_t* password, unsigned int nSvsId, const wchar_t* dbConnection) {
    return SP_PLUGIN_AUTH_NOT_IMPLEMENTED;
}
