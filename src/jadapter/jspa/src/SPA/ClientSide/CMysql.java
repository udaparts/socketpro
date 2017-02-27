package SPA.ClientSide;

public class CMysql extends CAsyncDBHandler {

    public final static int sidMysql = SPA.BaseServiceID.sidReserved + 0x6FFFFFF1; //asynchronous mysql service id

    /**
     * Use Mysql embedded at SocketPro server side by default. Use this final
     * value for the input parameter flags with the method of
     * CAsyncDBHandler::Open at client side to open a connection to remote Mysql
     * server at SocketPro server instead of embedded Mysql
     */
    public final static int USE_REMOTE_MYSQL = 0x1;

    //error codes from async mysql server library
    public final static int ER_NO_DB_OPENED_YET = 1981;
    public final static int ER_BAD_END_TRANSTACTION_PLAN = 1982;
    public final static int ER_NO_PARAMETER_SPECIFIED = 1983;
    public final static int ER_BAD_PARAMETER_COLUMN_SIZE = 1984;
    public final static int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = 1985;
    public final static int ER_DATA_TYPE_NOT_SUPPORTED = 1986;
    public final static int ER_NO_DB_NAME_SPECIFIED = 1987;
    public final static int ER_MYSQL_LIBRARY_NOT_INITIALIZED = 1988;
    public final static int ER_BAD_MANUAL_TRANSACTION_STATE = 1989;

    public CMysql() {
        super(sidMysql);
    }
}
