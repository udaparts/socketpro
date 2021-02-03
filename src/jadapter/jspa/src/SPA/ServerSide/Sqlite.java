package SPA.ServerSide;

public final class Sqlite {
    /**
     * Don't use sqlite update hook at server side by default. If update hook is
     * enabled, all connected clients will be notified when there is a record
     * insert, update or delete. For details, refer to the documentation of
     * sqlite function sqlite3_update_hook
     */
    public final static int ENABLE_GLOBAL_SQLITE_UPDATE_HOOK = 0x1;

    /**
     * Don't use sqlite shared cache at server side by default
     */
    public final static int USE_SHARED_CACHE_MODE = 0x2;

    /**
     * Sqlite server will return extended error codes to client by default
     */
    public final static int DO_NOT_USE_EXTENDED_ERROR_CODE = 0x4;

    /**
     * Sqlite server will use utf8 encoding by default
     */
    public final static int USE_UTF16_ENCODING = 0x8;
}
