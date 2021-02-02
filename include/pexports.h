#ifndef _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_
#define _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_

#include "definebase.h"

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * Returns a plugin version string like "1.0.0.1"
     * @return 1: authentication permitted, 0: authentication denied, -1: authentication not implemented, -2: authentication failed due to an internal error
     */
    const char* const U_MODULE_OPENED WINAPI GetSPluginVersion();

    /**
     * Set one or more pairs of settings (key/value) by a json string
     */
    void U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char *jsonOptions);

    /**
     * Get a json string object having zero, one or more pair of settings (key/value)
     * @param json A buffer to receive output json string
     * @param buffer_size The buffer size
     * @return The number of characters
     * @remark It is noted that calling the method will still return a json string and show all supported settings, even though the method SetSPluginGlobalOptions has not been called yet.
     */
    unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char *json, unsigned int buffer_size);

    /**
     * Provide authentication at server side by use of this plugin.
     * @param hSocket SocketPro server socket handle
     * @param userId a user identification string from client
     * @param password a password string from client
     * @param nSvsId a service identification number
     * @param options an optional string
     * @return 1: authentication permitted, 0: authentication denied, -1: authentication not implemented, -2: authentication failed due to an internal error
     */
    int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *options);

#ifdef __cplusplus
}
#endif

typedef const char* const (WINAPI *PGetSPluginVersion)();
typedef void (WINAPI *PSetSPluginGlobalOptions)(const char *jsonOptions);
typedef unsigned int (WINAPI *PGetSPluginGlobalOptions)(const char *json, unsigned int buffer_size);
typedef int (WINAPI *PDoSPluginAuthentication)(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t* options);

#define GLOBAL_CONNECTION_STRING    "global_connection_string"

#endif
