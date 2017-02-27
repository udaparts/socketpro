package SPA.ServerSide;

public abstract class CBaseService {

    private int m_svsId = 0;
    private static final Object m_csService = new Object();
    private static final java.util.ArrayList<CBaseService> m_lstService = new java.util.ArrayList<>();

    @Override
    protected void finalize() throws Throwable {
        RemoveMe();
        super.finalize();
    }

    protected CBaseService() {
    }

    protected abstract CSocketPeer GetPeerSocket() throws InstantiationException, IllegalAccessException;
    volatile java.util.HashMap<Short, java.lang.reflect.Method> m_dicMethod = new java.util.HashMap<>();

    private final Object m_cs = new Object();
    private volatile java.util.ArrayList<CSocketPeer> m_lstPeer = new java.util.ArrayList<>();
    private final java.util.ArrayList<CSocketPeer> m_lstDeadPeer = new java.util.ArrayList<>();

    void ReleasePeer(long hSocket, boolean bClosing, int info) {
        synchronized (m_cs) {
            for (CSocketPeer p : m_lstPeer) {
                if (p.getHandle() == hSocket) {
                    p.getUQueue().SetSize(0);
                    p.OnRelease(bClosing, info);
                    m_lstDeadPeer.add(p);
                    m_lstPeer.remove(p);
                    break;
                }
            }
        }
        ServerCoreLoader.RemovePeer(hSocket);
    }

    CSocketPeer CreatePeer(long hSocket) throws InstantiationException, IllegalAccessException {
        CSocketPeer sp = find();
        if (sp == null) {
            sp = GetPeerSocket();
            sp.m_Service = this;
        }
        sp.m_sh = hSocket;
        ServerCoreLoader.SetPeer(hSocket, sp);
        synchronized (m_cs) {
            m_lstPeer.add(sp);
        }
        return sp;
    }

    public static CBaseService SeekService(int serviceId) {
        /*
         1. Remove this lock as you will not add or remove a service after running a server under most cases.
         2. Make this lock if you will surely add or remove a service after running a server.
         3. Adding extra lock may slightly degrade performance.
         */
        //synchronized (m_csService) {
        for (CBaseService bs : m_lstService) {
            if (bs.getSvsID() == serviceId) {
                return bs;
            }
        }
        //}
        return null;
    }

    public static CBaseService SeekService(long hSocket) {
        return SeekService(ServerCoreLoader.GetSvsID(hSocket));
    }

    public final void RemoveMe() {
        synchronized (m_cs) {
            m_lstPeer.clear();
            m_lstDeadPeer.clear();
        }
        m_dicMethod.clear();
        if (m_svsId > 0) {
            ServerCoreLoader.RemoveASvsContext(m_svsId);
            synchronized (m_csService) {
                m_lstService.remove(this);
            }
            m_svsId = 0;
        }
    }

    private CSocketPeer find() {
        if (m_lstDeadPeer.size() > 0) {
            return m_lstDeadPeer.remove(0);
        }
        return null;
    }

    /**
     * Register a service
     *
     * @param svsId A service id
     * @return True if successful. Otherwise false if failed
     */
    public final boolean AddMe(int svsId) {
        return AddMe(svsId, SPA.tagThreadApartment.taNone);
    }

    /**
     * Register a service
     *
     * @param svsId A service id
     * @param ta Thread apartment for windows default to
     * tagThreadApartment.taNone. It is ignored on non-windows platforms
     * @return True if successful. Otherwise false if failed
     */
    public boolean AddMe(int svsId, SPA.tagThreadApartment ta) {
        if (m_svsId == 0 && svsId != 0 && ServerCoreLoader.AddSvsContext(svsId, ta.getValue())) {
            m_svsId = svsId;
            synchronized (m_csService) {
                m_lstService.add(this);
            }
            return true;
        }
        return false;
    }

    public final int getSvsID() {
        return m_svsId;
    }

    public final int getCountOfSlowRequests() {
        return ServerCoreLoader.GetCountOfSlowRequests(m_svsId);
    }

    public final short[] getAllSlowRequestIds() {
        short[] sr = new short[getCountOfSlowRequests() + 8];
        int res = ServerCoreLoader.GetAllSlowRequestIds(m_svsId, sr, sr.length);
        short[] s = new short[res];
        for (int n = 0; n < res; ++n) {
            s[n] = sr[n];
        }
        return s;
    }

    public final boolean getReturnRandom() {
        return ServerCoreLoader.GetReturnRandom(m_svsId);
    }

    public final void setReturnRandom(boolean value) {
        ServerCoreLoader.SetReturnRandom(m_svsId, value);
    }

    /**
     * Register a slow request
     *
     * @param reqId A request id
     * @return True if successful. Otherwise false if failed
     */
    public final boolean AddSlowRequest(short reqId) {
        return ServerCoreLoader.AddSlowRequest(m_svsId, reqId);
    }

    public final void RemoveSlowRequest(short reqId) {
        ServerCoreLoader.RemoveSlowRequest(m_svsId, reqId);
    }

    public final void RemoveAllSlowRequests() {
        ServerCoreLoader.RemoveAllSlowRequests(m_svsId);
    }

    /**
     * Make a request processed at router for a routee
     *
     * @param reqId A request id
     * @return True if successful. Otherwise, false if failed
     */
    public final boolean AddAlphaRequest(short reqId) {
        return ServerCoreLoader.AddAlphaRequest(m_svsId, reqId);
    }

    public final short[] getAlphaRequestIds() {
        short[] sr = new short[4097];
        int res = ServerCoreLoader.GetAlphaRequestIds(m_svsId, sr, 4097);
        short[] s = new short[res];
        for (int n = 0; n < res; ++n) {
            s[n] = sr[n];
        }
        return s;
    }

    public final CSocketPeer Seek(long hSocket) {
        synchronized (m_cs) {
            for (CSocketPeer sp : m_lstPeer) {
                if (sp.getHandle() == hSocket) {
                    return sp;
                }
            }
        }
        return null;
    }
}
