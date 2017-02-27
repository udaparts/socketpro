package SPA.ClientSide;

public interface DOnSubscribe {

    void invoke(CClientSocket sender, CMessageSender messageSender, int[] group);
}
