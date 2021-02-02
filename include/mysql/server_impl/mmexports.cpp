#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "mysqlimpl.h"

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
    CMysqlImpl::m_csPeer.lock();
    std::string str = Utilities::ToUTF8(CMysqlImpl::m_strGlobalConnection);
    CMysqlImpl::m_csPeer.unlock();
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
    if (!CMysqlImpl::m_bInitMysql) {
        return -2;
    }
    CMysqlImpl impl;
    std::wstring db(dbConnection ? dbConnection : L"");
    if (!db.size()) {
        db = L"host=localhost;port=3306;timeout=30";
    }
    if (userId && ::wcslen(userId)) {
        db += L";uid=";
        db += userId;
    }
    if (password && ::wcslen(password)) {
        db += L";pwd=";
        db += password;
    }
    int res = 0, ms = 0;
    CDBString errMsg;
    CDBString conn = Utilities::ToUTF16(db);
    impl.Open(conn, 0, res, errMsg, ms);
    if (res) {
        return 0;
    }
    if (nSvsId == SPA::Mysql::sidMysql) {
        CMysqlImpl::MyStruct ms;
        ms.Handle = impl.GetDBConnHandle();
        ms.DefaultDB = impl.GetDefaultDBName();
        CAutoLock al(CMysqlImpl::m_csPeer);
        CMysqlImpl::m_mapConnection[hSocket] = ms;
    }
    return 1;
}
