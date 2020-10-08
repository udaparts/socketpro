package SPA.ClientSide;

public interface IUFExtra {

    public short getReqId();

    public void setException(SPA.CServerError ex);

    public void setException(CSocketError ex);

    public int getState();
}
