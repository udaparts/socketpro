package SPA.ClientSide;

public final class CPushImpl implements SPA.IUPushEx {

    private final CClientSocket m_cs;
    public volatile DOnPublish OnPublish = null;
    public volatile DOnPublishEx OnPublishEx = null;
    public volatile DOnSendUserMessage OnSendUserMessage = null;
    public volatile DOnSendUserMessageEx OnSendUserMessageEx = null;
    public volatile DOnSubscribe OnSubscribe = null;
    public volatile DOnUnsubscribe OnUnsubscribe = null;

    CPushImpl(CClientSocket cs) {
        m_cs = cs;
    }

    private CPushImpl() {
        m_cs = null;
    }

    @Override
    public boolean Publish(Object Message, int... Groups) {
        if (Groups == null) {
            Groups = new int[0];
        }
        SPA.CUQueue q = SPA.CScopeUQueue.Lock();
        byte[] bytes = q.Save(Message).getIntenalBuffer();
        boolean ok = ClientCoreLoader.Speak(m_cs.getHandle(), bytes, q.getSize(), Groups, Groups.length);
        SPA.CScopeUQueue.Unlock(q);
        return ok;
    }

    @Override
    public boolean Subscribe(int... Groups) {
        if (Groups == null) {
            Groups = new int[0];
        }
        return ClientCoreLoader.Enter(m_cs.getHandle(), Groups, Groups.length);
    }

    @Override
    public boolean Unsubscribe() {
        ClientCoreLoader.Exit(m_cs.getHandle());
        return true;
    }

    @Override
    public boolean SendUserMessage(Object Message, String UserId) {
        SPA.CUQueue q = SPA.CScopeUQueue.Lock();
        byte[] bytes = q.Save(Message).getIntenalBuffer();
        boolean ok = ClientCoreLoader.SendUserMessage(m_cs.getHandle(), UserId, bytes, q.getSize());
        SPA.CScopeUQueue.Unlock(q);
        return ok;
    }

    @Override
    public boolean Publish(byte[] Message, int... Groups) {
        if (Groups == null) {
            Groups = new int[0];
        }
        if (Message == null) {
            Message = new byte[0];
        }
        return ClientCoreLoader.SpeakEx(m_cs.getHandle(), Message, Message.length, Groups, Groups.length);
    }

    @Override
    public boolean SendUserMessage(String UserId, byte[] Message) {
        if (Message == null) {
            Message = new byte[0];
        }
        return ClientCoreLoader.SendUserMessageEx(m_cs.getHandle(), UserId, Message, Message.length);
    }
}
