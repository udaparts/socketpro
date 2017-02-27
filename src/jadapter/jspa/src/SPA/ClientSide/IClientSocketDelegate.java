package SPA.ClientSide;

public interface IClientSocketDelegate {

    void OnSocketClosed(CClientSocket sender, int errorCode);

    void OnHandShakeCompleted(CClientSocket sender, int errorCode);

    void OnSocketConnected(CClientSocket sender, int errorCode);

    void OnRequestProcessed(CClientSocket sender, short reqId, int len);

    void OnBaseRequestProcessed(CClientSocket sender, SPA.tagBaseRequestID reqId);

    void OnServerException(CClientSocket sender, short reqId, String errMessage, String errWhere, int errCode);

    void OnAllRequestsProcessed(CClientSocket sender, short lastReqId);
}
