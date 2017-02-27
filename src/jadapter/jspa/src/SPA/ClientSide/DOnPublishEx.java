package SPA.ClientSide;

public interface DOnPublishEx {

    void invoke(CClientSocket sender, CMessageSender messageSender, int[] group, byte[] msg);
}
