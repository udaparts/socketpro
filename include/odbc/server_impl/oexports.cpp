#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "odbcimpl.h"

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
    if (doc.HasMember(GLOBAL_CONNECTION_STRING) && doc[GLOBAL_CONNECTION_STRING].IsString()) {
        std::string s = doc[GLOBAL_CONNECTION_STRING].GetString();
        std::wstring ws = Utilities::ToWide(s);
        Trim(ws);
        COdbcImpl::SetGlobalConnectionString(ws.c_str());
    } else {
        COdbcImpl::SetGlobalConnectionString(L"");
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
    COdbcImpl::m_csPeer.lock();
    std::string str = Utilities::ToUTF8(COdbcImpl::m_strGlobalConnection);
    COdbcImpl::m_csPeer.unlock();
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

int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t* userId, const wchar_t* password, unsigned int nSvsId, const wchar_t* dsn) {
    SQLHDBC hdbc = nullptr;
    if (!COdbcImpl::g_hEnv) {
        return -2;
    }
    SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_DBC, COdbcImpl::g_hEnv, &hdbc);
    if (!SQL_SUCCEEDED(retcode)) {
        return -2;
    }
    std::wstring conn(dsn ? dsn : L"");
    if (userId && ::wcslen(userId)) {
        if (conn.size()) conn.push_back(L';');
        conn += L"UID=";
        conn += userId;
    }
    if (conn.size()) conn.push_back(L';');
    conn += L"PWD=";
    conn += (password ? password : L"");

    CScopeUQueue sb;
    SQLSMALLINT cbConnStrOut = 0;
    std::string strConn = Utilities::ToUTF8(conn);
    retcode = SQLDriverConnect(hdbc, nullptr, (SQLCHAR*) strConn.c_str(), (SQLSMALLINT) strConn.size(), (SQLCHAR*) sb->GetBuffer(), (SQLSMALLINT) sb->GetMaxSize(), &cbConnStrOut, SQL_DRIVER_NOPROMPT);
    sb->CleanTrack(); //clean password
    if (!SQL_SUCCEEDED(retcode)) {
        SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        return 0;
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
    return 1;
}
