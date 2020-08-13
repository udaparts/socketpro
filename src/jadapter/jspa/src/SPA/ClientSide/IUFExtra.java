package SPA.ClientSide;

public interface IUFExtra {

    public String getMethodName();

    public short getReqId();

    public void setException(SPA.CServerError ex);

    public void setException(CSocketError ex);

    public int getState();
}
