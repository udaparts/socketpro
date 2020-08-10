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
    
    public int getErrCode() {
        return m_ec;
    }
    
    public int getWhere() {
        return m_ec;
    }
    
    public short getReqId() {
        return m_reqId;
    }
}
