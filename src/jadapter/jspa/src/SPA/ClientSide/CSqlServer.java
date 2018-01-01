package SPA.ClientSide;

public class CSqlServer extends CAsyncDBHandler {

    public final static int sidMsSql = SPA.BaseServiceID.sidReserved + 0x6FFFFFF2; //asynchronous MS SQL stream service id

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
