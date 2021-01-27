package SPA;

public final class BaseServiceID {

    public static final int sidReserved1 = 1;
    public static final int sidStartup = 0x100;
    public static final int sidChat = (sidStartup + 1);
    public static final int sidHTTP = (sidChat + 1);
    public static final int sidFile = (sidHTTP + 1);
    public static final int sidODBC = (sidFile + 1);

    /**
     * Your non-db service ids should be between (sidReserved + 1) and
     * (sidDB_RESERVED - 1)
     */
    public static final int sidReserved = 0x10000000;
    public static final int sidQueue = sidChat;

    /**
     * Your db streaming service ids must be between sidDB_RESERVED and
     * (sidDB_UDAParts_RESERVED - 1)
     */
    public static final int sidDB_RESERVED = (sidReserved + 0x6FFFFF00);

    public static final int sidDB_UDAParts_RESERVED = (sidReserved + 0x6FFFFFD0);

    /**
     * UDAParts reserves sidODBC and db streaming service ids from
     * sidDB_UDAParts_RESERVED through sidDB_MAX
     */
    public static final int sidDB_MAX = (sidReserved + 0x6FFFFFFF);
}
