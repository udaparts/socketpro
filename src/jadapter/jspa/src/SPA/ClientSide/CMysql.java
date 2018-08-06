package SPA.ClientSide;

public class CMysql extends CAsyncDBHandler {

    public final static int sidMysql = SPA.BaseServiceID.sidReserved + 0x6FFFFFF1; //asynchronous MySQL/MariaDB service id

    //error codes from async MySQL/MariaDB server library
    public final static int ER_NO_DB_OPENED_YET = 1981;
    public final static int ER_BAD_END_TRANSTACTION_PLAN = 1982;
    public final static int ER_NO_PARAMETER_SPECIFIED = 1983;
    public final static int ER_BAD_PARAMETER_COLUMN_SIZE = 1984;
    public final static int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985;
    public final static int ER_DATA_TYPE_NOT_SUPPORTED = 1986;
    public final static int ER_BAD_MANUAL_TRANSACTION_STATE = 1987;
    public final static int ER_UNABLE_TO_SWITCH_TO_DATABASE = 1988;
    public final static int ER_SERVICE_COMMAND_ERROR = 1989;

    //The following defines are required by non MySQL database plugin and MariaDB SQL-stream technologies
    public final static int ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1990;
    /**
     * This define is reserved for future
     */
    public final static int USE_REMOTE_MYSQL = 0x80000000;

    public CMysql() {
        super(sidMysql);
    }

    /**
     * You may use the protected constructor when extending this class
     *
     * @param sid a service id
     */
    protected CMysql(int sid) {
        super(sid);
    }
}
