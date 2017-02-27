package SPA.ClientSide;

public interface DOnUnsubscribe {

    void invoke(CClientSocket sender, CMessageSender messageSender, int[] group);
}
