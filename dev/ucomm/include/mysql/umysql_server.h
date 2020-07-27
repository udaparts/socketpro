
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
     * @param remote reserved for the future use
     */
    void U_MODULE_OPENED WINAPI SetMysqlDBGlobalConnectionString(const wchar_t *dbConnection, bool remote);

    /**
     * Do authentication through MySQL/Mariadb C connector
     * @param hSocket A server peer SocketPro socket handle
     * @param userId A string for user id
     * @param password A string for user password
     * @param nSvsId A service Id
     * @param dbConnection A connection string to MySQL/Mariadb database
     * @return true for success authentication and false for failure authentication
     */
    bool U_MODULE_OPENED WINAPI DoMySQLAuthentication(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *dbConnection);
#ifdef __cplusplus
}
#endif

typedef void (WINAPI *PSetMysqlDBGlobalConnectionString)(const wchar_t *dbConnection, bool remote);
typedef bool (WINAPI *PDoMySQLAuthentication)(USocket_Server_Handle hSocket, const wchar_t *userId, const wchar_t *password, unsigned int nSvsId, const wchar_t *dbConnection);

namespace SPA {
    namespace ServerSide {
        namespace Mysql {

            //The two following defines are reserved for the future use

            static const unsigned int DISABLE_REMOTE_MYSQL = 0x80000000;
            static const unsigned int DISABLE_EMBEDDED_MYSQL = 0x40000000;

        } //namespace Mysql
    } //namespace ServerSide
} //namespace SPA

#endif