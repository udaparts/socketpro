#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "odbcimpl.h"

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
    JSON::JValue<char>* v = jv->Child(GLOBAL_CONNECTION_STRING);
    if (v && v->GetType() == JSON::enumType::String) {
        std::wstring ws = Utilities::ToWide(v->AsString());
        Trim(ws);
        COdbcImpl::SetGlobalConnectionString(ws.c_str());
    } else {
        COdbcImpl::SetGlobalConnectionString(L"");
    }
    v = jv->Child(MANUAL_BATCHING);
    if (v && v->GetType() == JSON::enumType::Uint64) {
        COdbcImpl::m_mb = (unsigned int) v->AsUint64();
    }
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    JSON::JObject<char> obj;
    obj[MANUAL_BATCHING] = COdbcImpl::m_mb;
    COdbcImpl::m_csPeer.lock();
    obj[GLOBAL_CONNECTION_STRING] = Utilities::ToUTF8(COdbcImpl::m_strGlobalConnection);
    obj[PLUGIN_SERVICE_ID] = SPA::Odbc::sidOdbc;
    COdbcImpl::m_csPeer.unlock();
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

int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t* userId, const wchar_t* password, unsigned int nSvsId, const wchar_t* dsn) {
    SQLHDBC hdbc = nullptr;
    if (!COdbcImpl::g_hEnv) {
        return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    }
    SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_DBC, COdbcImpl::g_hEnv, &hdbc);
    if (!SQL_SUCCEEDED(retcode)) {
        return SP_PLUGIN_AUTH_INTERNAL_ERROR;
    }
    std::wstring conn(dsn ? dsn : L"");
    if (!conn.size()) {
        COdbcImpl::m_csPeer.lock();
        conn = Utilities::ToWide(COdbcImpl::m_strGlobalConnection);
        COdbcImpl::m_csPeer.unlock();
    }
    if (userId && ::wcslen(userId)) {
        if (conn.size()) conn.push_back(L';');
        conn += L"UID=";
        conn += userId;
    }
    if (password && ::wcslen(password)) {
        if (conn.size()) conn.push_back(L';');
        conn += L"PWD=";
        conn += (password ? password : L"");
    }
    CScopeUQueue sb;
    SQLSMALLINT cbConnStrOut = 0;
    std::string strConn = Utilities::ToUTF8(conn);
    retcode = SQLDriverConnect(hdbc, nullptr, (SQLCHAR*) strConn.c_str(), (SQLSMALLINT) strConn.size(), (SQLCHAR*) sb->GetBuffer(), (SQLSMALLINT) sb->GetMaxSize(), &cbConnStrOut, SQL_DRIVER_NOPROMPT);
    sb->CleanTrack(); //clean password
    if (!SQL_SUCCEEDED(retcode)) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        return SP_PLUGIN_AUTH_FAILED;
    }
    if (nSvsId == Odbc::sidOdbc) {
        COdbcImpl::m_csPeer.lock();
        COdbcImpl::m_mapConnection[hSocket] = hdbc; //reuse the handle for coming ODBC requests
        COdbcImpl::m_csPeer.unlock();
    } else {
        //we don't need it anymore and just close the ODBC connection handle
        retcode = SQLDisconnect(hdbc);
        retcode = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
    }
    return SP_PLUGIN_AUTH_OK;
}
