#include "../../pexports.h"
#include "../../jsonvalue.h"
#include "asyncqueueimpl.h"

using namespace SPA;
using namespace SPA::ServerSide;

std::string g_version("1.0.0.8");

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
    JSON::JValue<char>* v = jv->Child(DEQUEUE_BATCH_SIZE);
    if (v && v->GetType() == JSON::enumType::Uint64) {
        unsigned int bs = (unsigned int) v->AsUint64();
        bs &= 0xffffff;
        if (bs < MIN_DEQUEUE_BATCH_SIZE) bs = MIN_DEQUEUE_BATCH_SIZE;
        CAsyncQueueImpl::m_nBatchSize = bs;
    }
    v = jv->Child(DISABLE_AUTO_NOTIFICATION);
    if (v && v->GetType() == JSON::enumType::Uint64) {
        unsigned int disable_auto_notification = (unsigned int) v->AsUint64();
        disable_auto_notification &= 0xff;
        CAsyncQueueImpl::m_bNoAuto = disable_auto_notification;
    }
    return true;
}

unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char* json, unsigned int buffer_size) {
    if (!json || !buffer_size) {
        return 0;
    }
    JSON::JObject<char> obj;
    obj[DEQUEUE_BATCH_SIZE] = CAsyncQueueImpl::m_nBatchSize;
    obj[DISABLE_AUTO_NOTIFICATION] = CAsyncQueueImpl::m_bNoAuto;
    obj[PLUGIN_SERVICE_ID] = SPA::Queue::sidQueue;
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
    return SP_PLUGIN_AUTH_NOT_IMPLEMENTED;
}
