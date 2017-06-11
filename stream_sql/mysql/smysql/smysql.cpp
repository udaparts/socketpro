
#include "streamingserver.h"

CStreamingServer *g_pStreamingServer = nullptr;
CSetGlobals CSetGlobals::Globals;

static int async_sql_plugin_init(void *p) {
    my_plugin_log_message(&p, MY_INFORMATION_LEVEL, "Installation");
    /*
    g_pStreamingServer = new CStreamingServer(CSetGlobals::Globals.m_nParam);
    if (CSetGlobals::Globals.TLSv) {

    }
    if (!g_pStreamingServer->Run(CSetGlobals::Globals.Port)) {
            my_plugin_log_message(&p, MY_INFORMATION_LEVEL, "Installation failed as SQL streaming service is not started successfully");
            return 1;
    }
     */
    return 0;
}

static int async_sql_plugin_deinit(void *p) {
    if (g_pStreamingServer) {
        g_pStreamingServer->StopSocketProServer();
        g_pStreamingServer->Clean();
        delete g_pStreamingServer;
        g_pStreamingServer = nullptr;
    }
    my_plugin_log_message(&p, MY_INFORMATION_LEVEL, "Uninstallation");
    return 0;
}

mysql_declare_plugin(async_sql) {
    MYSQL_DAEMON_PLUGIN,
            &CSetGlobals::Globals.async_sql_plugin,
            "UDAParts_SQL_Streaming",
            "UDAParts",
            "Async SQL Real-time Streaming Processing",
            PLUGIN_LICENSE_PROPRIETARY,
            async_sql_plugin_init, /* Plugin Init */
            async_sql_plugin_deinit, /* Plugin Deinit */
            0x0100 /* 1.0 */,
            NULL, /* status variables */
            NULL, /* system variables */
            NULL, /* config options */
            0, /* flags */
}
mysql_declare_plugin_end;
