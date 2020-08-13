package SPA.ClientSide;

public class CSocketError extends java.util.concurrent.ExecutionException {

    private final int m_ec;
    private final short m_reqId;
    private final boolean m_before;

    public CSocketError(int errCode, String errMsg, short reqId, boolean before) {
        super(errMsg);
        m_ec = errCode;
        m_reqId = reqId;
        m_before = before;
    }

    public CSocketError(int errCode, String errMsg, short reqId) {
        super(errMsg);
        m_ec = errCode;
        m_reqId = reqId;
        m_before = true;
    }

    public int getErrCode() {
        return m_ec;
    }

    public short getReqId() {
        return m_reqId;
    }

    public boolean getBefore() {
        return m_before;
    }

    @Override
    public String toString() {
        String s = "ec: " + String.valueOf(m_ec) + ", em: " + getMessage();
        s += ", reqId: " + String.valueOf(m_reqId);
        s += ", before: " + String.valueOf(m_before);
        return s;
    }
}
