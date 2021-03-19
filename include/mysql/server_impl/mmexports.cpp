#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "mysqlimpl.h"

using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.11");

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
    JSON::JValue<char>* v = jv->Child(GLOBAL_CONNECTION_STRING);
    if (v && v->GetType() == JSON::enumType::String) {
        std::string& s = v->AsString();
        std::wstring ws = Utilities::ToWide(s);
        Trim(ws);
        CMysqlImpl::SetDBGlobalConnectionString(ws.c_str(), false);
    } else {
        CMysqlImpl::SetDBGlobalConnectionString(L"", false);
    }
    v = jv->Child(MANUAL_BATCHING);
    if (v && v->GetType() == JSON::enumType::Uint64) {
        int mb = (int) v->AsUint64();
        CMysqlImpl::m_mb = (SPA::ServerSide::tagMaualBatching) mb;
    }
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    JSON::JObject<char> obj;
    obj[GLOBAL_CONNECTION_STRING] = Utilities::ToUTF8(CMysqlImpl::GetDBGlobalConnectionString());
    obj[STREAMING_DB_MYSQL_CLIENT_LIB] = CMysqlImpl::GetClientLibName();
    obj[PLUGIN_SERVICE_ID] = SPA::Mysql::sidMysql;
    SPA::ServerSide::tagMaualBatching mb = CMysqlImpl::m_mb;
    obj[MANUAL_BATCHING] = (int) mb;
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
    if (!CMysqlImpl::IsMysqlInitialized()) {
        return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    }
    return CMysqlImpl::DoSQLAuthentication(hSocket, userId, password, nSvsId, dbConnection) ? SP_PLUGIN_AUTH_OK : SP_PLUGIN_AUTH_FAILED;
}
