package SPA.ClientSide;

public class ReplicationSetting {

    public static final int DEAFULT_TTL = (int) 30 * 24 * 3600; //30 days

    /**
     * An absolute path to a directory containing message queue files.
     */
    public String QueueDir = CClientSocket.QueueConfigure.getWorkDirectory();

    /**
     * False for auto socket connecting. Otherwise, there is no auto connection.
     */
    public boolean NoAutoConn = false;

    /**
     * Time-to-live in seconds. It is ignored if persistent message queue
     * feature is not used. If the value is not set or zero, the value will
     * default to DEFAULT_TTL (30 days).
     */
    public int TTL = DEAFULT_TTL;

    /**
     * A timeout for receiving result from remote SocketPro server. If the value
     * is not set or it is zero, the value will default to
     * CClientSocket.DEFAULT_RECV_TIMEOUT (30 seconds).
     */
    public int RecvTimeout = CClientSocket.DEFAULT_RECV_TIMEOUT;

    /**
     * A timeout for connecting to remote SocketPro server. If the value is not
     * set or it is zero, the value will default to
     * CClientSocket.DEFAULT_CONN_TIMEOUT (30 seconds).
     */
    public int ConnTimeout = CClientSocket.DEFAULT_CONN_TIMEOUT;

    public final ReplicationSetting Copy() {
        ReplicationSetting rs = new ReplicationSetting();
        rs.ConnTimeout = ConnTimeout;
        rs.NoAutoConn = NoAutoConn;
        rs.QueueDir = QueueDir;
        rs.RecvTimeout = RecvTimeout;
        rs.TTL = TTL;
        return rs;
    }
}
