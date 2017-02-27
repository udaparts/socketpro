package SPA.ClientSide;

public class CSqlite extends CAsyncDBHandler {

    public final static int sidSqlite = SPA.BaseServiceID.sidReserved + 0x6FFFFFF0; //asynchronous sqlite service id

    public CSqlite() {
        super(sidSqlite);
    }

    public final static int SQLITE_OK = 0; /* Successful result */

    /* beginning-of-error-codes */
    public final static int SQLITE_ERROR = 1; /* SQL error or missing database */

    public final static int SQLITE_INTERNAL = 2; /* Internal logic error in SQLite */

    public final static int SQLITE_PERM = 3; /* Access permission denied */

    public final static int SQLITE_ABORT = 4; /* Callback routine requested an abort */

    public final static int SQLITE_BUSY = 5; /* The database file is locked */

    public final static int SQLITE_LOCKED = 6; /* A table in the database is locked */

    public final static int SQLITE_NOMEM = 7; /* A malloc() failed */

    public final static int SQLITE_READONLY = 8; /* Attempt to write a readonly database */

    public final static int SQLITE_INTERRUPT = 9; /* Operation terminated by sqlite3_interrupt()*/

    public final static int SQLITE_IOERR = 10; /* Some kind of disk I/O error occurred */

    public final static int SQLITE_CORRUPT = 11; /* The database disk image is malformed */

    public final static int SQLITE_NOTFOUND = 12; /* Unknown opcode in sqlite3_file_control() */

    public final static int SQLITE_FULL = 13; /* Insertion failed because database is full */

    public final static int SQLITE_CANTOPEN = 14; /* Unable to open the database file */

    public final static int SQLITE_PROTOCOL = 15; /* Database lock protocol error */

    public final static int SQLITE_EMPTY = 16; /* Database is empty */

    public final static int SQLITE_SCHEMA = 17; /* The database schema changed */

    public final static int SQLITE_TOOBIG = 18; /* String or BLOB exceeds size limit */

    public final static int SQLITE_CONSTRAINT = 19; /* Abort due to final staticraint violation */

    public final static int SQLITE_MISMATCH = 20; /* Data type mismatch */

    public final static int SQLITE_MISUSE = 21; /* Library used incorrectly */

    public final static int SQLITE_NOLFS = 22; /* Uses OS features not supported on host */

    public final static int SQLITE_AUTH = 23; /* Authorization denied */

    public final static int SQLITE_FORMAT = 24; /* Auxiliary database format error */

    public final static int SQLITE_RANGE = 25; /* 2nd parameter to sqlite3_bind out of range */

    public final static int SQLITE_NOTADB = 26; /* File opened that is not a database file */

    public final static int SQLITE_NOTICE = 27; /* Notifications from sqlite3_log() */

    public final static int SQLITE_WARNING = 28; /* Warnings from sqlite3_log() */

    public final static int SQLITE_ROW = 100; /* sqlite3_step() has another row ready */

    public final static int SQLITE_DONE = 101; /* sqlite3_step() has finished executing */

    //error codes from asynchronous sqlite server side implementation
    public final static int SQLITE_DB_NOT_OPENED_YET = 131;
    public final static int SQLITE_BAD_END_TRANSTACTION_PLAN = 132;
    public final static int SQLITE_NO_PARAMETER_SPECIFIED = 133;
    public final static int SQLITE_BAD_PARAMETER_COLUMN_SIZE = 134;
    public final static int SQLITE_BAD_PARAMETER_DATA_ARRAY_SIZE = 135;
    public final static int SQLITE_DATA_TYPE_NOT_SUPPORTED = 136;
    public final static int SQLITE_NO_DB_FILE_SPECIFIED = 137;

    //sqlite extended error codes
    public final static int SQLITE_IOERR_READ = (SQLITE_IOERR | (1 << 8));
    public final static int SQLITE_IOERR_SHORT_READ = (SQLITE_IOERR | (2 << 8));
    public final static int SQLITE_IOERR_WRITE = (SQLITE_IOERR | (3 << 8));
    public final static int SQLITE_IOERR_FSYNC = (SQLITE_IOERR | (4 << 8));
    public final static int SQLITE_IOERR_DIR_FSYNC = (SQLITE_IOERR | (5 << 8));
    public final static int SQLITE_IOERR_TRUNCATE = (SQLITE_IOERR | (6 << 8));
    public final static int SQLITE_IOERR_FSTAT = (SQLITE_IOERR | (7 << 8));
    public final static int SQLITE_IOERR_UNLOCK = (SQLITE_IOERR | (8 << 8));
    public final static int SQLITE_IOERR_RDLOCK = (SQLITE_IOERR | (9 << 8));
    public final static int SQLITE_IOERR_DELETE = (SQLITE_IOERR | (10 << 8));
    public final static int SQLITE_IOERR_BLOCKED = (SQLITE_IOERR | (11 << 8));
    public final static int SQLITE_IOERR_NOMEM = (SQLITE_IOERR | (12 << 8));
    public final static int SQLITE_IOERR_ACCESS = (SQLITE_IOERR | (13 << 8));
    public final static int SQLITE_IOERR_CHECKRESERVEDLOCK = (SQLITE_IOERR | (14 << 8));
    public final static int SQLITE_IOERR_LOCK = (SQLITE_IOERR | (15 << 8));
    public final static int SQLITE_IOERR_CLOSE = (SQLITE_IOERR | (16 << 8));
    public final static int SQLITE_IOERR_DIR_CLOSE = (SQLITE_IOERR | (17 << 8));
    public final static int SQLITE_IOERR_SHMOPEN = (SQLITE_IOERR | (18 << 8));
    public final static int SQLITE_IOERR_SHMSIZE = (SQLITE_IOERR | (19 << 8));
    public final static int SQLITE_IOERR_SHMLOCK = (SQLITE_IOERR | (20 << 8));
    public final static int SQLITE_IOERR_SHMMAP = (SQLITE_IOERR | (21 << 8));
    public final static int SQLITE_IOERR_SEEK = (SQLITE_IOERR | (22 << 8));
    public final static int SQLITE_IOERR_DELETE_NOENT = (SQLITE_IOERR | (23 << 8));
    public final static int SQLITE_IOERR_MMAP = (SQLITE_IOERR | (24 << 8));
    public final static int SQLITE_IOERR_GETTEMPPATH = (SQLITE_IOERR | (25 << 8));
    public final static int SQLITE_IOERR_CONVPATH = (SQLITE_IOERR | (26 << 8));
    public final static int SQLITE_IOERR_VNODE = (SQLITE_IOERR | (27 << 8));
    public final static int SQLITE_IOERR_AUTH = (SQLITE_IOERR | (28 << 8));
    public final static int SQLITE_LOCKED_SHAREDCACHE = (SQLITE_LOCKED | (1 << 8));
    public final static int SQLITE_BUSY_RECOVERY = (SQLITE_BUSY | (1 << 8));
    public final static int SQLITE_BUSY_SNAPSHOT = (SQLITE_BUSY | (2 << 8));
    public final static int SQLITE_CANTOPEN_NOTEMPDIR = (SQLITE_CANTOPEN | (1 << 8));
    public final static int SQLITE_CANTOPEN_ISDIR = (SQLITE_CANTOPEN | (2 << 8));
    public final static int SQLITE_CANTOPEN_FULLPATH = (SQLITE_CANTOPEN | (3 << 8));
    public final static int SQLITE_CANTOPEN_CONVPATH = (SQLITE_CANTOPEN | (4 << 8));
    public final static int SQLITE_CORRUPT_VTAB = (SQLITE_CORRUPT | (1 << 8));
    public final static int SQLITE_READONLY_RECOVERY = (SQLITE_READONLY | (1 << 8));
    public final static int SQLITE_READONLY_CANTLOCK = (SQLITE_READONLY | (2 << 8));
    public final static int SQLITE_READONLY_ROLLBACK = (SQLITE_READONLY | (3 << 8));
    public final static int SQLITE_READONLY_DBMOVED = (SQLITE_READONLY | (4 << 8));
    public final static int SQLITE_ABORT_ROLLBACK = (SQLITE_ABORT | (2 << 8));
    public final static int SQLITE_CONSTRAINT_CHECK = (SQLITE_CONSTRAINT | (1 << 8));
    public final static int SQLITE_CONSTRAINT_COMMITHOOK = (SQLITE_CONSTRAINT | (2 << 8));
    public final static int SQLITE_CONSTRAINT_FOREIGNKEY = (SQLITE_CONSTRAINT | (3 << 8));
    public final static int SQLITE_CONSTRAINT_FUNCTION = (SQLITE_CONSTRAINT | (4 << 8));
    public final static int SQLITE_CONSTRAINT_NOTNULL = (SQLITE_CONSTRAINT | (5 << 8));
    public final static int SQLITE_CONSTRAINT_PRIMARYKEY = (SQLITE_CONSTRAINT | (6 << 8));
    public final static int SQLITE_CONSTRAINT_TRIGGER = (SQLITE_CONSTRAINT | (7 << 8));
    public final static int SQLITE_CONSTRAINT_UNIQUE = (SQLITE_CONSTRAINT | (8 << 8));
    public final static int SQLITE_CONSTRAINT_VTAB = (SQLITE_CONSTRAINT | (9 << 8));
    public final static int SQLITE_CONSTRAINT_ROWID = (SQLITE_CONSTRAINT | (10 << 8));
    public final static int SQLITE_NOTICE_RECOVER_WAL = (SQLITE_NOTICE | (1 << 8));
    public final static int SQLITE_NOTICE_RECOVER_ROLLBACK = (SQLITE_NOTICE | (2 << 8));
    public final static int SQLITE_WARNING_AUTOINDEX = (SQLITE_WARNING | (1 << 8));
    public final static int SQLITE_AUTH_USER = (SQLITE_AUTH | (1 << 8));
}
