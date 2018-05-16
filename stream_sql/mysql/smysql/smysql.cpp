
#include "streamingserver.h"

mysql_declare_plugin(async_sql) {
    MYSQL_DAEMON_PLUGIN,
            &CSetGlobals::Globals.async_sql_plugin,
            "UDAParts_SQL_Streaming",
            "UDAParts",
            "Async SQL Real-time Streaming Processing",
            PLUGIN_LICENSE_PROPRIETARY,
            async_sql_plugin_init, /* Plugin Init */
            nullptr,
            async_sql_plugin_deinit, /* Plugin Deinit */
            0x0100 /* 1.0 */,
            nullptr, /* status variables */
            nullptr, /* system variables */
            nullptr, /* config options */
            0, /* flags */
}
mysql_declare_plugin_end;
