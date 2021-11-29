package SPA.ClientSide;

public class CPostgres extends CAsyncDBHandler {

    /**
     * Asynchronous and SQL streaming postgreSQL service id
     */
    public final static int sidPostgres = SPA.BaseServiceID.sidReserved + 0x6FFFFFF4;

    public CPostgres() {
        super(sidPostgres);
    }

    /**
     * You may use the protected constructor when extending this class
     *
     * @param sid a service id
     */
    protected CPostgres(int sid) {
        super(sid);
    }

    /**
     * An Open flag option, which is specific to PostgreSQL plug-in. It is noted
     * that this flag option is not implemented within SocketPro plug-in yet.
     */
    public final static int ROWSET_META_FLAGS_REQUIRED = 0x40000000;

    /**
     * An Open flag option, which is specific to PostgreSQL plug-in. When the
     * flag option is used with the method Open or open, it forces fetching data
     * from remote PostgreSQL server to SocketPro plug-in row-by-row instead of
     * all. The flag option should be used if there is a large number of data
     * within a record set.
     */
    public final static int USE_SINGLE_ROW_MODE = 0x20000000;

    public final static int ER_NO_DB_OPENED_YET = -1981;
    public final static int ER_BAD_END_TRANSTACTION_PLAN = -1982;
    public final static int ER_NO_PARAMETER_SPECIFIED = -1983;
    public final static int ER_BAD_PARAMETER_COLUMN_SIZE = -1984;
    public final static int ER_BAD_PARAMETER_DATA_ARRAY_SIZE = -1985;
    public final static int ER_DATA_TYPE_NOT_SUPPORTED = -1986;
    public final static int ER_BAD_TRANSTACTION_STAGE = -1987;
}
