#ifndef _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_
#define _SOCKETPRO_PLUGIN_EXTRA_FUNCS_H_

#include "definebase.h"

#ifdef __cplusplus
extern "C" {
#endif
	const char* U_MODULE_OPENED WINAPI GetSPluginVersion();
	void U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char *jsonOptions);
	void U_MODULE_OPENED WINAPI GetSPluginGlobalOptions(char *json, unsigned int buffer_size);
	
	/**
	 * Provide authentication at server side by use of this plugin.
	 * @param hSocket SocketPro server socket handle
	 * @param userId a user identification string from client
	 * @param password a password string from client
	 * @param nSvsId a service identification number
	 * @return 1 -- authentication permitted, 0 -- authentication denied, -1 -- authentication not implemented
	 */
	int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId);

#ifdef __cplusplus
}
#endif

typedef const char* (WINAPI *PGetSPluginVersion)();
typedef void (WINAPI *PSetSPluginGlobalOptions)(const char *jsonOptions);
typedef void (WINAPI *PGetSPluginGlobalOptions)(const char *json, unsigned int buffer_size);
typedef int (WINAPI *PDoSPluginAuthentication)(SPA::UINT64 hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId);

#endif