package SPA.ServerSide;

public class CSocketProService<TPeer extends CSocketPeer> extends CBaseService {

    private final Class<TPeer> m_PeerClass;

    public CSocketProService(Class<TPeer> cls) {
        m_PeerClass = cls;
    }

    @Override
    protected TPeer GetPeerSocket() throws InstantiationException, IllegalAccessException {
        TPeer p = m_PeerClass.newInstance();
        return p;
    }

    private void SetMethods() {
        java.lang.reflect.Method[] mis = m_PeerClass.getDeclaredMethods();
        for (java.lang.reflect.Method mi : mis) {
            //This may not work for parent class!!!!
            RequestAttr ras = mi.getAnnotation(RequestAttr.class);
            if (ras != null) {
                short reqId = ras.RequestID();
                if (reqId > 0 && reqId <= SPA.tagBaseRequestID.idReservedTwo) {
                    throw new UnsupportedOperationException("The request ID must be larger than ocketProAdapter.tagBaseRequestID.idReservedTwo");
                }
                if (m_dicMethod.containsKey(reqId)) {
                    throw new UnsupportedOperationException("The request ID (" + reqId + ") can not be duplicated within the same service");
                }
                m_dicMethod.put(reqId, mi);
                mi.setAccessible(true);
                if (ras.SlowRequest()) {
                    AddSlowRequest(reqId);
                }
            }
        }
    }

    @Override
    public boolean AddMe(int svsId, SPA.tagThreadApartment ta) {
        m_dicMethod.clear();
        if (super.AddMe(svsId, ta)) {
            if (svsId == SPA.BaseServiceID.sidHTTP) {
                AddSlowRequest(tagHttpRequestID.idPost.getValue());
                AddSlowRequest(tagHttpRequestID.idGet.getValue());
                AddSlowRequest(tagHttpRequestID.idConnect.getValue());
                AddSlowRequest(tagHttpRequestID.idHead.getValue());
                AddSlowRequest(tagHttpRequestID.idMultiPart.getValue());
                AddSlowRequest(tagHttpRequestID.idOptions.getValue());
                AddSlowRequest(tagHttpRequestID.idPut.getValue());
                AddSlowRequest(tagHttpRequestID.idTrace.getValue());
                AddSlowRequest(tagHttpRequestID.idUserRequest.getValue());
                AddSlowRequest(tagHttpRequestID.idDelete.getValue());
            } else {
                SetMethods();
            }
            return true;
        }
        return false;
    }
}
