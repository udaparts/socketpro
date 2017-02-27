package SPA.ClientSide;

public interface IPushDelegate {

    void OnSubscribe(CClientSocket sender, CMessageSender messageSender, int[] groups);

    void OnUnsubscribe(CClientSocket sender, CMessageSender messageSender, int[] groups);

    void OnSendUserMessage(CClientSocket sender, CMessageSender messageSender, Object message);

    void OnSendUserMessageEx(CClientSocket sender, CMessageSender messageSender, byte[] message);

    void OnPublishEx(CClientSocket sender, CMessageSender messageSender, int[] groups, byte[] message);

    void OnPublish(CClientSocket sender, CMessageSender messageSender, int[] groups, Object message);
}
