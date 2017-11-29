package SPA;

public class CMasterSlaveBase<THandler extends SPA.ClientSide.CAsyncServiceHandler> extends SPA.ClientSide.CSocketPool<THandler> {

    private final String m_dbDefalut;
    private final int m_nRecvTimeout;

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb, int recvTimeout) {
        super(impl, true, recvTimeout);
        m_dbDefalut = defaultDb;
        m_nRecvTimeout = recvTimeout;
    }

    public CMasterSlaveBase(Class<THandler> impl, String defaultDb) {
        super(impl, true, SPA.ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_dbDefalut = defaultDb;
        m_nRecvTimeout = SPA.ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT;
    }

    String getDefaultDBName() {
        return m_dbDefalut;
    }

    int GetRecvTimeout() {
        return m_nRecvTimeout;
    }
}
