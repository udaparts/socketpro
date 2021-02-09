#include "../../pexports.h"
#include "../../3rdparty/rapidjson/include/rapidjson/document.h"
#include "../../3rdparty/rapidjson/include/rapidjson/stringbuffer.h"
#include "../../3rdparty/rapidjson/include/rapidjson/writer.h"
#include "asyncqueueimpl.h"

using namespace rapidjson;
using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.2");

const char* const U_MODULE_OPENED WINAPI GetSPluginVersion() {
    return g_version.c_str();
}

bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char* jsonOptions) {
    //no key/value implemented yet
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    writer.StartObject();
    writer.Key(DEQUEUE_BATCH_SIZE);
    writer.Int((int) CAsyncQueueImpl::m_nBatchSize);
    writer.Key(DISABLE_AUTO_NOTIFICATION);
    writer.Int((int) CAsyncQueueImpl::m_bNoAuto);
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
    return SP_PLUGIN_AUTH_NOT_IMPLEMENTED;
}
