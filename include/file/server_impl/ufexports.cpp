#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "sfileimpl.h"

#ifndef WIN32_64
#include <sys/stat.h>
#endif

using namespace rapidjson;
using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.1");

const char* const U_MODULE_OPENED WINAPI GetSPluginVersion() {
    return g_version.c_str();
}

bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char* jsonOptions) {
    if (!jsonOptions) return false;
    Document doc;
    doc.SetObject();
    ParseResult ok = doc.Parse(jsonOptions, ::strlen(jsonOptions));
    if (!ok) {
        return false;
    }
    if (doc.HasMember(UFILE_ROOT_DIRECTORY) && doc[UFILE_ROOT_DIRECTORY].IsString()) {
        std::string s = doc[UFILE_ROOT_DIRECTORY].GetString();
        Trim(s);
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
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key(UFILE_ROOT_DIRECTORY);
    std::string str = Utilities::ToUTF8(CSFileImpl::GetRootDirectory());
    writer.String(str.c_str(), (SizeType) str.size());
    writer.EndObject();
    std::string s = buffer.GetString();
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
    return -1;
}
