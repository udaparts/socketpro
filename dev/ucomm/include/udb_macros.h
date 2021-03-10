#ifndef _UDAPARTS_DB_PLUGIN_COMMON_MACROS_H_
#define _UDAPARTS_DB_PLUGIN_COMMON_MACROS_H_

#include "definebase.h"

#define STREAM_DB_LOG_FILE                  "streaming_db.log"
#define STREAM_DB_CONFIG_FILE	            "sp_streaming_db_config.json"

#define STREAMING_DB_PORT		    "port"
#define STREAMING_DB_MAIN_THREADS	    "main_threads"
#define STREAMING_DB_NO_IPV6		    "disable_ipv6"
#define STREAMING_DB_CACHE_TABLES	    "monitored_tables"
#define STREAMING_DB_SERVICES		    "services"
#define STREAMING_DB_WORKING_DIR            "working_dir"
#define STREAMING_DB_SERVICES_CONFIG        "services_config"
#define STREAMING_DB_VERSION                "version"

#ifdef WIN32_64
#define DEFAULT_CA_ROOT                     "root"
#define STREAMING_DB_STORE		    "cert_root_store"
#define STREAMING_DB_SUBJECT_CN             "cert_subject_cn"
#else
#define STREAMING_DB_SSL_KEY                "ssl_key"
#define STREAMING_DB_SSL_CERT               "ssl_cert"
#define STREAMING_DB_SSL_PASSWORD           "ssl_key_password"
#endif
#define SP_SERVER_CORE_VERSION              "sp_server_core_version"

#endif
