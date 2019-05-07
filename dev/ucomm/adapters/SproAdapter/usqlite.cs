
using System;
using System.Runtime.InteropServices;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        public class CSqlServer : CAsyncDBHandler
        {
            public const uint sidMsSql = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF2; //asynchronous MS SQL stream service id
            public CSqlServer()
                : base(sidMsSql)
            {
            }

            /// <summary>
            /// You may use the protected constructor when extending this class
            /// </summary>
            /// <param name="sid">A service id</param>
            protected CSqlServer(uint sid)
                : base(sid)
            {
            }
        }

        public class CSqlite : CAsyncDBHandler
        {
            public const uint sidSqlite = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF0; //asynchronous sqlite service id

            /// <summary>
            /// A flag used with the method CAsyncDBHandler.Open for automatically attaching the opening database onto current session
            /// </summary>
            public const uint DATABASE_AUTO_ATTACHED = 0x40000000;

            public const int SQLITE_OK = 0; /* Successful result */
            /* beginning-of-error-codes */
            public const int SQLITE_ERROR = 1; /* SQL error or missing database */
            public const int SQLITE_INTERNAL = 2; /* Internal logic error in SQLite */
            public const int SQLITE_PERM = 3; /* Access permission denied */
            public const int SQLITE_ABORT = 4; /* Callback routine requested an abort */
            public const int SQLITE_BUSY = 5; /* The database file is locked */
            public const int SQLITE_LOCKED = 6; /* A table in the database is locked */
            public const int SQLITE_NOMEM = 7; /* A malloc() failed */
            public const int SQLITE_READONLY = 8; /* Attempt to write a readonly database */
            public const int SQLITE_INTERRUPT = 9; /* Operation terminated by sqlite3_interrupt()*/
            public const int SQLITE_IOERR = 10; /* Some kind of disk I/O error occurred */
            public const int SQLITE_CORRUPT = 11; /* The database disk image is malformed */
            public const int SQLITE_NOTFOUND = 12; /* Unknown opcode in sqlite3_file_control() */
            public const int SQLITE_FULL = 13; /* Insertion failed because database is full */
            public const int SQLITE_CANTOPEN = 14; /* Unable to open the database file */
            public const int SQLITE_PROTOCOL = 15; /* Database lock protocol error */
            public const int SQLITE_EMPTY = 16; /* Database is empty */
            public const int SQLITE_SCHEMA = 17; /* The database schema changed */
            public const int SQLITE_TOOBIG = 18; /* String or BLOB exceeds size limit */
            public const int SQLITE_CONSTRAINT = 19; /* Abort due to constraint violation */
            public const int SQLITE_MISMATCH = 20; /* Data type mismatch */
            public const int SQLITE_MISUSE = 21; /* Library used incorrectly */
            public const int SQLITE_NOLFS = 22; /* Uses OS features not supported on host */
            public const int SQLITE_AUTH = 23; /* Authorization denied */
            public const int SQLITE_FORMAT = 24; /* Auxiliary database format error */
            public const int SQLITE_RANGE = 25; /* 2nd parameter to sqlite3_bind out of range */
            public const int SQLITE_NOTADB = 26; /* File opened that is not a database file */
            public const int SQLITE_NOTICE = 27; /* Notifications from sqlite3_log() */
            public const int SQLITE_WARNING = 28; /* Warnings from sqlite3_log() */
            public const int SQLITE_ROW = 100; /* sqlite3_step() has another row ready */
            public const int SQLITE_DONE = 101; /* sqlite3_step() has finished executing */

            //error codes from asynchronous sqlite server side implementation
            public const int SQLITE_DB_NOT_OPENED_YET = 131;
            public const int SQLITE_BAD_END_TRANSTACTION_PLAN = 132;
            public const int SQLITE_NO_PARAMETER_SPECIFIED = 133;
            public const int SQLITE_BAD_PARAMETER_COLUMN_SIZE = 134;
            public const int SQLITE_BAD_PARAMETER_DATA_ARRAY_SIZE = 135;
            public const int SQLITE_DATA_TYPE_NOT_SUPPORTED = 136;
            public const int SQLITE_NO_DB_FILE_SPECIFIED = 137;

            //sqlite extended error codes
            public const int SQLITE_IOERR_READ = (SQLITE_IOERR | (1 << 8));
            public const int SQLITE_IOERR_SHORT_READ = (SQLITE_IOERR | (2 << 8));
            public const int SQLITE_IOERR_WRITE = (SQLITE_IOERR | (3 << 8));
            public const int SQLITE_IOERR_FSYNC = (SQLITE_IOERR | (4 << 8));
            public const int SQLITE_IOERR_DIR_FSYNC = (SQLITE_IOERR | (5 << 8));
            public const int SQLITE_IOERR_TRUNCATE = (SQLITE_IOERR | (6 << 8));
            public const int SQLITE_IOERR_FSTAT = (SQLITE_IOERR | (7 << 8));
            public const int SQLITE_IOERR_UNLOCK = (SQLITE_IOERR | (8 << 8));
            public const int SQLITE_IOERR_RDLOCK = (SQLITE_IOERR | (9 << 8));
            public const int SQLITE_IOERR_DELETE = (SQLITE_IOERR | (10 << 8));
            public const int SQLITE_IOERR_BLOCKED = (SQLITE_IOERR | (11 << 8));
            public const int SQLITE_IOERR_NOMEM = (SQLITE_IOERR | (12 << 8));
            public const int SQLITE_IOERR_ACCESS = (SQLITE_IOERR | (13 << 8));
            public const int SQLITE_IOERR_CHECKRESERVEDLOCK = (SQLITE_IOERR | (14 << 8));
            public const int SQLITE_IOERR_LOCK = (SQLITE_IOERR | (15 << 8));
            public const int SQLITE_IOERR_CLOSE = (SQLITE_IOERR | (16 << 8));
            public const int SQLITE_IOERR_DIR_CLOSE = (SQLITE_IOERR | (17 << 8));
            public const int SQLITE_IOERR_SHMOPEN = (SQLITE_IOERR | (18 << 8));
            public const int SQLITE_IOERR_SHMSIZE = (SQLITE_IOERR | (19 << 8));
            public const int SQLITE_IOERR_SHMLOCK = (SQLITE_IOERR | (20 << 8));
            public const int SQLITE_IOERR_SHMMAP = (SQLITE_IOERR | (21 << 8));
            public const int SQLITE_IOERR_SEEK = (SQLITE_IOERR | (22 << 8));
            public const int SQLITE_IOERR_DELETE_NOENT = (SQLITE_IOERR | (23 << 8));
            public const int SQLITE_IOERR_MMAP = (SQLITE_IOERR | (24 << 8));
            public const int SQLITE_IOERR_GETTEMPPATH = (SQLITE_IOERR | (25 << 8));
            public const int SQLITE_IOERR_CONVPATH = (SQLITE_IOERR | (26 << 8));
            public const int SQLITE_IOERR_VNODE = (SQLITE_IOERR | (27 << 8));
            public const int SQLITE_IOERR_AUTH = (SQLITE_IOERR | (28 << 8));
            public const int SQLITE_LOCKED_SHAREDCACHE = (SQLITE_LOCKED | (1 << 8));
            public const int SQLITE_BUSY_RECOVERY = (SQLITE_BUSY | (1 << 8));
            public const int SQLITE_BUSY_SNAPSHOT = (SQLITE_BUSY | (2 << 8));
            public const int SQLITE_CANTOPEN_NOTEMPDIR = (SQLITE_CANTOPEN | (1 << 8));
            public const int SQLITE_CANTOPEN_ISDIR = (SQLITE_CANTOPEN | (2 << 8));
            public const int SQLITE_CANTOPEN_FULLPATH = (SQLITE_CANTOPEN | (3 << 8));
            public const int SQLITE_CANTOPEN_CONVPATH = (SQLITE_CANTOPEN | (4 << 8));
            public const int SQLITE_CORRUPT_VTAB = (SQLITE_CORRUPT | (1 << 8));
            public const int SQLITE_READONLY_RECOVERY = (SQLITE_READONLY | (1 << 8));
            public const int SQLITE_READONLY_CANTLOCK = (SQLITE_READONLY | (2 << 8));
            public const int SQLITE_READONLY_ROLLBACK = (SQLITE_READONLY | (3 << 8));
            public const int SQLITE_READONLY_DBMOVED = (SQLITE_READONLY | (4 << 8));
            public const int SQLITE_ABORT_ROLLBACK = (SQLITE_ABORT | (2 << 8));
            public const int SQLITE_CONSTRAINT_CHECK = (SQLITE_CONSTRAINT | (1 << 8));
            public const int SQLITE_CONSTRAINT_COMMITHOOK = (SQLITE_CONSTRAINT | (2 << 8));
            public const int SQLITE_CONSTRAINT_FOREIGNKEY = (SQLITE_CONSTRAINT | (3 << 8));
            public const int SQLITE_CONSTRAINT_FUNCTION = (SQLITE_CONSTRAINT | (4 << 8));
            public const int SQLITE_CONSTRAINT_NOTNULL = (SQLITE_CONSTRAINT | (5 << 8));
            public const int SQLITE_CONSTRAINT_PRIMARYKEY = (SQLITE_CONSTRAINT | (6 << 8));
            public const int SQLITE_CONSTRAINT_TRIGGER = (SQLITE_CONSTRAINT | (7 << 8));
            public const int SQLITE_CONSTRAINT_UNIQUE = (SQLITE_CONSTRAINT | (8 << 8));
            public const int SQLITE_CONSTRAINT_VTAB = (SQLITE_CONSTRAINT | (9 << 8));
            public const int SQLITE_CONSTRAINT_ROWID = (SQLITE_CONSTRAINT | (10 << 8));
            public const int SQLITE_NOTICE_RECOVER_WAL = (SQLITE_NOTICE | (1 << 8));
            public const int SQLITE_NOTICE_RECOVER_ROLLBACK = (SQLITE_NOTICE | (2 << 8));
            public const int SQLITE_WARNING_AUTOINDEX = (SQLITE_WARNING | (1 << 8));
            public const int SQLITE_AUTH_USER = (SQLITE_AUTH | (1 << 8));

            public CSqlite()
                : base(sidSqlite)
            {
            }

            /// <summary>
            /// You may use the protected constructor when extending this class
            /// </summary>
            /// <param name="sid">A service id</param>
            protected CSqlite(uint sid)
                : base(sid)
            {

            }
        }

        public class CMysql : ClientSide.CAsyncDBHandler
        {
            public const uint sidMysql = SocketProAdapter.BaseServiceID.sidReserved + 0x6FFFFFF1; //asynchronous mysql service id

            public CMysql()
                : base(sidMysql)
            {

            }

            /// <summary>
            /// You may use the protected constructor when extending this class
            /// </summary>
            /// <param name="sid">A service id</param>
            protected CMysql(uint sid)
                : base(sid)
            {

            }

            /// <summary>
            /// This define is reserved for future.
            /// </summary>
            public const uint USE_REMOTE_MYSQL = 0x80000000;

            //error codes from async mysql server library
            public const int ER_NO_DB_OPENED_YET = 1981;
            public const int ER_BAD_END_TRANSTACTION_PLAN = 1982;
            public const int ER_NO_PARAMETER_SPECIFIED = 1983;
            public const int ER_BAD_PARAMETER_COLUMN_SIZE = 1984;
            public const int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985;
            public const int ER_DATA_TYPE_NOT_SUPPORTED = 1986;
            public const int ER_BAD_MANUAL_TRANSACTION_STATE = 1987;
            public const int ER_UNABLE_TO_SWITCH_TO_DATABASE = 1988;
            public const int ER_SERVICE_COMMAND_ERROR = 1989;
            public const int ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1990;
        }
    }
#if WINCE
#else
    namespace ServerSide
    {
        public static class Sqlite
        {
            [DllImport("ssqlite")]
            public static extern void SetSqliteDBGlobalConnectionString([In] [MarshalAs(UnmanagedType.LPWStr)] string sqliteDbFile);

            /// <summary>
            /// Don't use sqlite update hook at server side by default.
            /// If update hook is enabled, all connected clients will be notified when there is a record insert, update or delete.
            /// For details, refer to the documentation of sqlite function sqlite3_update_hook
            /// </summary>
            public const uint ENABLE_GLOBAL_SQLITE_UPDATE_HOOK = 0x1;

            /// <summary>
            /// Don't use sqlite shared cache at server side by default
            /// </summary>
            public const uint USE_SHARED_CACHE_MODE = 0x2;

            /// <summary>
            /// Sqlite server will return extended error codes to client by default
            /// </summary>
            public const uint DO_NOT_USE_EXTENDED_ERROR_CODE = 0x4;

            /// <summary>
            /// Sqlite server will use utf8 encoding by default
            /// </summary>
            public const uint USE_UTF16_ENCODING = 0x8;
        }

        public static class Mysql
        {
            /// <summary>
            /// Set mysql global or default string for either embedded mysql database name or remote mysql connection string
            /// </summary>
            /// <param name="dbConnection">a global or default string for either embedded mysql database name or remote mysql connection string</param>
            /// <param name="remote">true for setting a connection string to a remote mysql database; and false for setting an embedded mysql database name</param>
            [DllImport("smysql")]
            public static extern void SetMysqlDBGlobalConnectionString([In] [MarshalAs(UnmanagedType.LPWStr)] string dbConnection, bool remote);

            [DllImport("smysql", EntryPoint = "SetMysqlEmbeddedOptions")]
            private static extern IntPtr SetEmbeddedOptions([In] [MarshalAs(UnmanagedType.LPWStr)] string options);

            /// <summary>
            /// Set embedded mysql server initial options
            /// </summary>
            /// <param name="options">Embedded mysql initial options string like 'datadir=.;language=./share;default-storage-engine=MyISAM;skip-innodb;key-buffer-size=64M;console'. If the string is null or empty, the options string will not be changed</param>
            /// <returns>options string for embedded mysql server initialization</returns>
            public static string SetMysqlEmbeddedOptions(string options)
            {
                return Marshal.PtrToStringAnsi(SetEmbeddedOptions(options));
            }

            public const uint DISABLE_REMOTE_MYSQL = 0x1;
            public const uint DISABLE_EMBEDDED_MYSQL = 0x2;
        }
    }
#endif
}