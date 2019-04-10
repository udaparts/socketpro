package SPA;

public class CMasterSlaveBase<THandler extends SPA.ClientSide.CAsyncServiceHandler> extends SPA.ClientSide.CSocketPool<THandler> {

    private final String m_dbDefalut;

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb, int recvTimeout, boolean autoConn, int connTimeout, int svsId) {
        super(impl, autoConn, recvTimeout, connTimeout, svsId);
        m_dbDefalut = defaultDb;
    }

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb, int recvTimeout, boolean autoConn, int connTimeout) {
        super(impl, autoConn, recvTimeout, connTimeout);
        m_dbDefalut = defaultDb;
    }

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb, int recvTimeout, boolean autoConn) {
        super(impl, autoConn, recvTimeout);
        m_dbDefalut = defaultDb;
    }

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb, int recvTimeout) {
        super(impl, true, recvTimeout);
        m_dbDefalut = defaultDb;
    }

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb) {
        super(impl, true, SPA.ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_dbDefalut = defaultDb;
    }

    String getDefaultDBName() {
        return m_dbDefalut;
    }
}
