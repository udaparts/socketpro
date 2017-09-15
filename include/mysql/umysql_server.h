
#pragma once

#include "../spa_module.h"
#include "umysql.h"

#ifndef _MYSQL_MODULE_IMPLEMENTATION_BASIC_HEADER_H_
#define _MYSQL_MODULE_IMPLEMENTATION_BASIC_HEADER_H_

#ifdef __cplusplus
extern "C" {
#endif

    /**
     * Set mysql global connection string
     * @param dbConnection a global or default string for either embedded mysql database name or remote mysql connection string
     * @param remote true for setting a connection string to a remote mysql database; and false for setting an embedded mysql database name
     */
    void WINAPI SetMysqlDBGlobalConnectionString(const wchar_t *dbConnection, bool remote);

    /**
     * Set embedded mysql server initial options
     * @param options Embedded mysql initial options string like 'datadir=.;language=./share;default-storage-engine=MyISAM;skip-innodb;key-buffer-size=64M;console'. If the string is null or empty, the options string will not be changed
     * @return options string for embedded mysql server initialization
     */
    const char* WINAPI SetMysqlEmbeddedOptions(const wchar_t *options);
#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetMysqlDBGlobalConnectionString)(const wchar_t *dbConnection, bool remote);
typedef const char* (WINAPI *PSetMysqlEmbeddedOptions)(const wchar_t *options);

namespace SPA {
    namespace ServerSide {
        namespace Mysql {

            static const unsigned int DISABLE_REMOTE_MYSQL = 0x1;
            static const unsigned int DISABLE_EMBEDDED_MYSQL = 0x2;

        } //namespace Mysql
    } //namespace ServerSide
} //namespace SPA

#endif