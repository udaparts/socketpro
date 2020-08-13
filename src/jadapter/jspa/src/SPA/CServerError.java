package SPA;

public class CServerError extends java.util.concurrent.ExecutionException {

    private final int m_ec;
    private final String m_errWhere;
    private final short m_reqId;

    public CServerError(int errCode, String errMsg, String errWhere, short reqId) {
        super(errMsg);
        m_ec = errCode;
        m_errWhere = errWhere;
        m_reqId = reqId;
    }

    public CServerError(int errCode, String errMsg) {
        super(errMsg);
        m_ec = errCode;
        m_errWhere = "";
        m_reqId = 0;
    }

    public int getErrCode() {
        return m_ec;
    }

    public String getWhere() {
        return m_errWhere;
    }

    public short getReqId() {
        return m_reqId;
    }

    @Override
    public String toString() {
        String s = "ec: " + String.valueOf(m_ec) + ", em: " + getMessage() + ", where: " + m_errWhere + ", reqId: " + String.valueOf(m_reqId);
        return s;
    }
}
