package SPA;

public interface IUPushEx extends IUPush {

    boolean Publish(byte[] message, int... groups);

    boolean SendUserMessage(String userId, byte[] message);
}
