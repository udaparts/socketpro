package SPA.ClientSide;

public interface DOnSendUserMessage {

    void invoke(CClientSocket sender, CMessageSender messageSender, Object msg);
}
