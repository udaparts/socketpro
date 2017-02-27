package SPA.ClientSide;

public interface DOnSendUserMessageEx {

    void invoke(CClientSocket sender, CMessageSender messageSender, byte[] msg);
}
