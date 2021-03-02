#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "sqliteimpl.h"

using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.5");

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
    JSON::JValue<char>* v = jv->Child(MONITORED_TABLES);
    if (v && v->GetType() == JSON::enumType::String) {
        CDBString ws = Utilities::ToUTF16(v->AsString());
        Trim(ws);
        CSqliteImpl::SetCachedTables(ws.c_str());
    } else {
        CSqliteImpl::SetCachedTables(u"");
    }
    v = jv->Child(GLOBAL_CONNECTION_STRING);
    if (v && v->GetType() == JSON::enumType::String) {
        CDBString ws = Utilities::ToUTF16(v->AsString());
        Trim(ws);
        CSqliteImpl::SetDBGlobalConnectionString(ws.c_str());
    } else {
        CSqliteImpl::SetDBGlobalConnectionString(u"");
    }
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    unsigned int nParam = CSqliteImpl::GetInitialParam();
    JSON::JObject<char> obj;
    obj[SQLITE_CODE_VERSION] = sqlite3_version;
    obj[SQLITE_UTF8_ENCODING] = true;
    obj[DISABLE_SQLITE_EX_ERROR] = ((nParam & ServerSide::Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == ServerSide::Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE);
    obj[ENABLE_SQLITE_UPDATE_HOOK] = true;
    obj[USE_SQLITE_SHARED_CACHE_MODE] = ((nParam & ServerSide::Sqlite::USE_SHARED_CACHE_MODE) == ServerSide::Sqlite::USE_SHARED_CACHE_MODE);
    obj[GLOBAL_CONNECTION_STRING] = Utilities::ToUTF8(CSqliteImpl::GetDBGlobalConnectionString());
    obj[MONITORED_TABLES] = CSqliteImpl::GetCachedTables();
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

int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t* userId, const wchar_t* password, unsigned int nSvsId, const wchar_t* connection_options) {
    if (nSvsId != SPA::Sqlite::sidSqlite) return SP_PLUGIN_AUTH_NOT_IMPLEMENTED;
    CDBString conn = Utilities::ToUTF16(connection_options);
    if (!conn.size()) conn = CSqliteImpl::GetDBGlobalConnectionString();
    CSqliteImpl sqlite;
    int errCode = 0, ms = 0;
    CDBString errMsg;
    sqlite.Open(conn, 0, errCode, errMsg, ms);
    if (errCode) return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    CSqliteImpl::CacheHandle(hSocket, conn, sqlite.GetDBHandle());
    return SP_PLUGIN_AUTH_PROCESSED;
}
