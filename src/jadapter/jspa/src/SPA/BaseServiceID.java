package SPA;

public final class BaseServiceID {

    public static final int sidReserved1 = 1;
    public static final int sidStartup = 0x100;
    public static final int sidChat = (sidStartup + 1);
    public static final int sidHTTP = (sidChat + 1);
    public static final int sidFile = (sidHTTP + 1);
    public static final int sidODBC = (sidFile + 1);
    public static final int sidReserved = 0x10000000;
    public static final int sidQueue = sidChat;
}
