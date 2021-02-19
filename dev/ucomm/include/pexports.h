#ifndef _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_
#define _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_

#include "definebase.h"

#define SP_PLUGIN_AUTH_OK                   ((int)1) //authentication permitted, and DB handle opened and cached
#define SP_PLUGIN_AUTH_PROCESSED            ((int)0) //authentication not implemented, but DB handle opened and cached
#define SP_PLUGIN_AUTH_FAILED               ((int)-1) //authentication failed
#define SP_PLUGIN_AUTH_INTERNAL_ERROR       ((int)-2) //authentication failed due to an intenal error 
#define SP_PLUGIN_AUTH_NOT_IMPLEMENTED      ((int)-3) //authentication not implemented at all

#ifdef __cplusplus
extern "C" {
#endif
    /**
     * Returns a plugin version string like "1.0.0.1 for this plugin"
     * @return a plugin version string like "2.1.0.7"
     */
    const char* const U_MODULE_OPENED WINAPI GetSPluginVersion();

    /**
     * Set zero, one or more pairs of settings (key/value) by a json string
     * @param jsonUtf8Options A utf8 JSON string containing zero, one or more pairs of settings (key/value). Specifically, all supported settings will be set to default states when setting an empty JSON object string
     * @return true if it is successful; Otherwise, false if a non-json string is passed in
     * @remark It will silently ignore all pairs of unknown or unsupported settings (key/value)
     */
    bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char *jsonUtf8Options);

    /**
     * Get a json string object having zero, one or more pair of settings (key/value)
     * @param jsonUtf8 A buffer to receive output json string
     * @param buffer_size The buffer size
     * @return The number of characters received
     * @remark It is noted that calling the method will still return a json string and show all supported settings even though the method SetSPluginGlobalOptions has not been called yet
     */
    unsigned int U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char *jsonUtf8, unsigned int buffer_size);

    /**
     * Provide authentication at server side by use of this plugin.
     * @param hSocket SocketPro server socket handle
     * @param userId a user identification string from client
     * @param password a password string from client
     * @param nSvsId a service identification number
     * @param options an optional or required connection string without user id or password
     * @return 1: authentication permitted and opened handle cached, 0: authentication denied, -1: authentication not implemented, -2: authentication failed due to an internal error,
     *         -3: authentication not implemented, but opened handle cached and processed in some way
     */
    int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *options);

#ifdef __cplusplus
}
#endif

typedef const char* const (WINAPI *PGetSPluginVersion)();
typedef bool (WINAPI *PSetSPluginGlobalOptions)(const char *jsonUtf8Options);
typedef unsigned int (WINAPI *PGetSPluginGlobalOptions)(char *jsonUtf8, unsigned int buffer_size);
typedef int (WINAPI *PDoSPluginAuthentication)(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t* options);

#define GLOBAL_CONNECTION_STRING    "global_connection_string"

#endif
