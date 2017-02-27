package SPA.ClientSide;

public interface DOnPublish {

    void invoke(CClientSocket sender, CMessageSender messageSender, int[] group, Object msg);
}
