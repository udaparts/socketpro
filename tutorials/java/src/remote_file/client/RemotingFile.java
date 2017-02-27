package remote_file.client;

public class RemotingFile extends SPA.ClientSide.CAsyncServiceHandler {

    public RemotingFile() {
        super(remote_file.RemFileConst.sidRemotingFile);
    }

    public final SPA.ClientSide.CStreamHelper getStreamHelper() {
        return m_sh;
    }

    private final SPA.ClientSide.CStreamHelper m_sh = new SPA.ClientSide.CStreamHelper(this);
}
