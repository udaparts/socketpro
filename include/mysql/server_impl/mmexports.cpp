#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "mysqlimpl.h"

using namespace rapidjson;
using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.2");

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
    if (doc.HasMember(GLOBAL_CONNECTION_STRING) && doc[GLOBAL_CONNECTION_STRING].IsString()) {
        std::string s = doc[GLOBAL_CONNECTION_STRING].GetString();
        std::wstring ws = Utilities::ToWide(s);
        Trim(ws);
        CMysqlImpl::SetDBGlobalConnectionString(ws.c_str(), false);
    } else {
        CMysqlImpl::SetDBGlobalConnectionString(L"", false);
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
    writer.Key(GLOBAL_CONNECTION_STRING);
    std::string str = Utilities::ToUTF8(CMysqlImpl::GetDBGlobalConnectionString());
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
    if (!CMysqlImpl::IsMysqlInitialized()) {
        return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    }
    return CMysqlImpl::DoSQLAuthentication(hSocket, userId, password, nSvsId, dbConnection) ? SP_PLUGIN_AUTH_OK : SP_PLUGIN_AUTH_FAILED;
}
