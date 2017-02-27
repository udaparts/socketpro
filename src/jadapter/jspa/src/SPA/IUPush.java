package SPA;

public interface IUPush {

    boolean Publish(Object message, int... groups);

    boolean Subscribe(int... groups);

    boolean Unsubscribe();

    boolean SendUserMessage(Object message, String userId);
}
