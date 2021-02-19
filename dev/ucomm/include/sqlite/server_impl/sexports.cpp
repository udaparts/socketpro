#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "sqliteimpl.h"

using namespace rapidjson;
using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.4");

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
    if (doc.HasMember(MONITORED_TABLES) && doc[MONITORED_TABLES].IsString()) {
        std::string s = doc[MONITORED_TABLES].GetString();
        CDBString ws = Utilities::ToUTF16(s);
        Trim(ws);
        CSqliteImpl::SetCachedTables(ws.c_str());
    } else {
        CSqliteImpl::SetCachedTables(u"");
    }

    if (doc.HasMember(GLOBAL_CONNECTION_STRING) && doc[GLOBAL_CONNECTION_STRING].IsString()) {
        std::string s = doc[GLOBAL_CONNECTION_STRING].GetString();
        CDBString ws = Utilities::ToUTF16(s);
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
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key(SQLITE_UTF8_ENCODING);
    writer.Bool(true);
    writer.Key(DISABLE_SQLITE_EX_ERROR);
    writer.Bool(((nParam & ServerSide::Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE) == ServerSide::Sqlite::DO_NOT_USE_EXTENDED_ERROR_CODE));
    writer.Key(ENABLE_SQLITE_UPDATE_HOOK);
    writer.Bool(true);
    writer.Key(USE_SQLITE_SHARED_CACHE_MODE);
    writer.Bool(((nParam & ServerSide::Sqlite::USE_SHARED_CACHE_MODE) == ServerSide::Sqlite::USE_SHARED_CACHE_MODE));
    writer.Key(GLOBAL_CONNECTION_STRING);
    std::string str = Utilities::ToUTF8(CSqliteImpl::GetDBGlobalConnectionString());
    writer.String(str.c_str(), (SizeType) str.size());
    writer.Key(MONITORED_TABLES);
    str = CSqliteImpl::GetCachedTables();
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
