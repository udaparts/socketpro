from consts import piConst, hwConst
from spa.serverside import CSocketProServer, CSocketProService,\
    CClientPeer, BaseServiceID, Plugin
from spa.clientside import CAsyncQueue, CStreamingFile, CMysql, COdbc, CSqlite, CSqlServer, CPostgres
from spa.udb import DB_CONSTS
from pub_sub.ps_server.hwpeer import CHelloWorldPeer
from webdemo.myhttppeer import CMyHttpPeer
import sys
from ctypes import *
from sys import platform as os

# bool U_MODULE_OPENED WINAPI SetSPluginGlobalOptions(const char *jsonUtf8Options);
sqlite_lib = None
if os == "win32":
    sqlite_lib = WinDLL("ssqlite.dll")
else:
    sqlite_lib = WinDLL("libssqlite.so")

SetSPluginGlobalOptions = sqlite_lib.SetSPluginGlobalOptions
SetSPluginGlobalOptions.argtypes = [c_char_p]
SetSPluginGlobalOptions.restype = c_bool

# int U_MODULE_OPENED WINAPI DoSPluginAuthentication(SPA::UINT64 hSocket,
#   const wchar_t *userId, const wchar_t *password, unsigned int nSvsId,
#   const wchar_t *options);
SQLite_Auth = sqlite_lib.DoSPluginAuthentication
SQLite_Auth.argtypes = [c_uint64, c_wchar_p, c_wchar_p, c_uint, c_wchar_p]
SQLite_Auth.restype = c_int

mysql_lib = None
MySQL_Auth = None
if os == "win32":
    mysql_lib = WinDLL("smysql.dll")
else:
    mysql_lib = WinDLL("libsmysql.so")
if mysql_lib:
    MySQL_Auth = mysql_lib.DoSPluginAuthentication
    MySQL_Auth.argtypes = [c_uint64, c_wchar_p, c_wchar_p, c_uint, c_wchar_p]
    MySQL_Auth.restype = c_int

odbc_lib = None
ODBC_Auth = None
if os == "win32":
    odbc_lib = WinDLL("sodbc.dll")
else:
    odbc_lib = WinDLL("libsodbc.so")
if odbc_lib:
    ODBC_Auth = odbc_lib.DoSPluginAuthentication
    ODBC_Auth.argtypes = [c_uint64, c_wchar_p, c_wchar_p, c_uint, c_wchar_p]
    ODBC_Auth.restype = c_int

mssql_lib = None
if os == "win32":
    mssql_lib = WinDLL("usqlsvr.dll")
else:
    mssql_lib = WinDLL("libusqlsvr.so")
MsSql_Auth = mssql_lib.DoSPluginAuthentication
MsSql_Auth.argtypes = [c_uint64, c_wchar_p, c_wchar_p, c_uint, c_wchar_p]
MsSql_Auth.restype = c_int

postgres_lib = None
Postgres_Auth = None
if os == "win32":
    postgres_lib = WinDLL("spostgres.dll")
else:
    postgres_lib = WinDLL("libspostgres.so")
if postgres_lib:
    Postgres_Auth = postgres_lib.DoSPluginAuthentication
    Postgres_Auth.argtypes = [c_uint64, c_wchar_p, c_wchar_p, c_uint, c_wchar_p]
    Postgres_Auth.restype = c_int

with CSocketProServer() as server:
    def OnClose(hSocket, errCode):
        bs = CSocketProService.SeekService(hSocket)
        if bs:
            sp = bs.Seek(hSocket)
            # ......
    server.OnClose = OnClose

    def OnIsPermitted(hSocket, userId, pwd, svsId):
        auth_res = Plugin.AUTHENTICATION_NOT_IMPLEMENTED
        if svsId == hwConst.sidHelloWorld or svsId == BaseServiceID.sidHTTP or svsId == piConst.sidPi or svsId == piConst.sidPiWorker:
            # give permission to known services without authentication
            auth_res = Plugin.AUTHENTICATION_OK
        elif svsId == CAsyncQueue.sidQueue or svsId == CStreamingFile.sidFile:
            # give permission to known services without authentication
            auth_res = Plugin.AUTHENTICATION_OK
        elif svsId == CPostgres.sidPostgres:
            auth_res = Postgres_Auth(hSocket, userId, pwd, svsId, 'database=sakila;server=localhost;timeout=45;max_SQLs_batched=16')
        elif svsId == CSqlServer.sidMsSql:
            auth_res = MsSql_Auth(hSocket, userId, pwd, svsId, 'database=sakila;server=localhost;timeout=45;max_SQLs_batched=16')
        elif svsId == COdbc.sidOdbc:
            auth_res = ODBC_Auth(hSocket, userId, pwd, svsId, 'DRIVER={ODBC Driver 13 for SQL Server};Server=windesk;database=sakila;max_sqls_batched=16')
        elif svsId == CMysql.sidMysql:
            auth_res = MySQL_Auth(hSocket, userId, pwd, svsId, 'database=sakila;server=windesk;max_sqls_batched=16')
        elif svsId == CSqlite.sidSqlite:
            auth_res = SQLite_Auth(hSocket, userId, pwd, svsId, 'usqlite.db')
            if auth_res == Plugin.AUTHENTICATION_PROCESSED:
                # give permission without authentication
                auth_res = Plugin.AUTHENTICATION_OK
        if auth_res >= Plugin.AUTHENTICATION_OK:
            print(userId + "'s connecting permitted, and DB handle opened and cached")
        elif auth_res == Plugin.AUTHENTICATION_PROCESSED:
            print(userId + "'s connecting denied: no authentication implemented but DB handle opened and cached")
        elif auth_res == Plugin.AUTHENTICATION_FAILED:
            print(userId + "'s connecting denied: bad password or user id")
        elif auth_res == Plugin.AUTHENTICATION_INTERNAL_ERROR:
            print(userId + "'s connecting denied: plugin internal error")
        elif auth_res == Plugin.AUTHENTICATION_NOT_IMPLEMENTED:
            print(userId + "'s connecting denied: no authentication implemented")
        else:
            print(userId + "'s connecting denied: unknown reseaon with res --" + str(auth_res))
        return auth_res >= Plugin.AUTHENTICATION_OK
    server.OnIsPermitted = OnIsPermitted

    def do_configuration():
        CSocketProServer.PushManager.AddAChatGroup(1, "R&D Department")
        CSocketProServer.PushManager.AddAChatGroup(2, "Sales Department")
        CSocketProServer.PushManager.AddAChatGroup(3, "Management Department")
        CSocketProServer.PushManager.AddAChatGroup(7, "HR Department")
        CSocketProServer.PushManager.AddAChatGroup(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, "Subscribe/publish for front clients")
        return True # True -- ok; False -- no listening server
    server.OnSettingServer = do_configuration

    mapIdMethod = {
        hwConst.idSayHello: 'sayHello',
        hwConst.idSleep: ['sleep', True],  # or ('sleep', True)
        hwConst.idEcho: 'echo'
    }
    server.hw = CSocketProService(CHelloWorldPeer, hwConst.sidHelloWorld, mapIdMethod)

    # HTTP/WebSocket service
    server.HttpSvs = CSocketProService(CMyHttpPeer, BaseServiceID.sidHTTP, None)

    mapIdReq = {}
    server.Pi = CSocketProService(CClientPeer, piConst.sidPi, mapIdReq)
    server.PiWorker = CSocketProService(CClientPeer, piConst.sidPiWorker, mapIdReq)
    if not CSocketProServer.Router.SetRouting(piConst.sidPi, piConst.sidPiWorker):
        print('Setting routing failed')

    # load file streaming library at the directory ../bin/free_services/file
    # 16 * 1024 dequeue batch size in bytes
    server.aq = CSocketProServer.DllManager.AddALibrary('uasyncqueue', 16 * 1024)

    # load async sqlite library located at the directory ../bin/free_services/sqlite
    server.sqlite = CSocketProServer.DllManager.AddALibrary("ssqlite")
    if server.sqlite:
        # monitoring sakila.db table events (DELETE, INSERT and UPDATE) for
        # tables actor, language, category, country and film_actor
        jsonOptions = '{"global_connection_string":"usqlite.db","monitored_tables":\
"sakila.db.actor;sakila.db.language;sakila.db.category;sakila.db.country;sakila.db.film_actor"}'
        SetSPluginGlobalOptions(jsonOptions.encode('utf-8'))

    # load persistent message queue library at the directory ../bin/free_services/queue
    server.file = CSocketProServer.DllManager.AddALibrary('ustreamfile')

    # load MySQL/MariaDB server plugin library at the directory ../bin/free_services/mm_middle
    server.mysql = CSocketProServer.DllManager.AddALibrary("smysql")

    # load ODBC server plugin library at the directory ../bin/win or ../bin/linux
    server.odbc = CSocketProServer.DllManager.AddALibrary("sodbc")

    # load MS sql server plugin library at the directory ../bin/win or ../bin/linux
    server.mssql = CSocketProServer.DllManager.AddALibrary("usqlsvr")

    # load PostgreSQL plugin library at the directory ../bin/win/win64 or ../bin/linux
    server.postgres = CSocketProServer.DllManager.AddALibrary("spostgres")

    if not server.Run(20901):
        print('Error message = ' + CSocketProServer.ErrorMessage)
    print('Read a line to shutdown the application ......')
    line = sys.stdin.readline()
