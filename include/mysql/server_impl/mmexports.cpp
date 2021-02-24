#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "mysqlimpl.h"

using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.8");

const char* const U_MODULE_OPENED WINAPI GetSPluginVersion() {
    return g_version.c_str();
}

bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char* jsonOptions) {
    if (!jsonOptions) return false;
    std::unique_ptr<JSON::JValue> jv(JSON::Parse(jsonOptions));
    if (!jv) {
        return false;
    }
    JSON::JValue* v = jv->Child(GLOBAL_CONNECTION_STRING);
    if (v && v->GetType() == JSON::enumType::String) {
        std::string& s = v->AsString();
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
    JSON::JObject obj;
    obj[GLOBAL_CONNECTION_STRING] = Utilities::ToUTF8(CMysqlImpl::GetDBGlobalConnectionString());
    JSON::JValue jv(std::move(obj));
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
    if (!CMysqlImpl::IsMysqlInitialized()) {
        return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    }
    return CMysqlImpl::DoSQLAuthentication(hSocket, userId, password, nSvsId, dbConnection) ? SP_PLUGIN_AUTH_OK : SP_PLUGIN_AUTH_FAILED;
}
