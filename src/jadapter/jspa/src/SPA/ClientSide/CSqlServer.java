package SPA.ClientSide;

public class CSqlServer extends CAsyncDBHandler {

    //asynchronous MS SQL service id
    public final static int sidMsSql = SPA.BaseServiceID.sidReserved + 0x6FFFFFF2;

    public final static int READ_ONLY = 0x20000000;
    public final static int USE_ENCRYPTION = 0x40000000;

    public CSqlServer() {
        super(sidMsSql);
    }

    /**
     * You may use the protected constructor when extending this class
     *
     * @param sid a service id
     */
    protected CSqlServer(int sid) {
        super(sid);
    }
}
