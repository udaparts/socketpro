package SPA.ClientSide;

public class CSocketError extends java.util.concurrent.ExecutionException {
    private final int m_ec;
    private final short m_reqId;
    public CSocketError(int errCode, String errMsg, short reqId) {
        super(errMsg);
        m_ec = errCode;
        m_reqId = reqId;
    }
    
    public int getErrCode() {
        return m_ec;
    }
    
    public short getReqId() {
        return m_reqId;
    }
}
