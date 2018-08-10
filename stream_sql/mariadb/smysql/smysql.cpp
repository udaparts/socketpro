
#include "streamingserver.h"

#if 1
//mariadb plugin declaration

maria_declare_plugin(async_sql) {
    MYSQL_DAEMON_PLUGIN,
            &CSetGlobals::Globals.async_sql_plugin,
            "UDAParts_SQL_Streaming",
            "UDAParts",
            "Async SQL Real-time Streaming Processing",
            PLUGIN_LICENSE_PROPRIETARY,
            async_sql_plugin_init, /* Plugin Init */
            async_sql_plugin_deinit, /* Plugin Deinit */
            0x0100 /* 1.0 */,
            nullptr, /* status variables */
            nullptr, /* system variables */
            "1.0", /* string version */
            MariaDB_PLUGIN_MATURITY_STABLE /* maturity */
}
maria_declare_plugin_end;

#else
//mysql plugin declaration

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
            nullptr, /* status variables */
            nullptr, /* system variables */
            nullptr, /* reserved */
            0 /* flags */
}
mysql_declare_plugin_end;

#endif
