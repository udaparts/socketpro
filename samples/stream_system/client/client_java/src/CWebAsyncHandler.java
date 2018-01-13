
import SPA.ClientSide.*;
import SPA.*;
import java.util.HashMap;
import SPA.UDB.CDBVariantArray;

public class CWebAsyncHandler extends CCachedBaseHandler {

    public CWebAsyncHandler() {
        super(Consts.sidStreamSystem);
    }

    public interface DMyCanceled {

        public void invoke(long index);
    }

    private final HashMap<Long, Pair<DMaxMinAvg, DMyCanceled>> m_mapMMA = new HashMap<>();

    public interface DMaxMinAvg {

        public void invoke(long index, CMaxMinAvg mma, int res, String errMsg);
    }

    public long QueryPaymentMaxMinAvgs(String filter, DMaxMinAvg mma, DMyCanceled canceled) {
        CUQueue q = CScopeUQueue.Lock();
        CAsyncServiceHandler.DAsyncResultHandler arh = (ar) -> {
            long index = ar.LoadLong();
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            CMaxMinAvg m_m_a = ar.Load(CMaxMinAvg.class);
            Pair<DMaxMinAvg, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapMMA.remove(index);
            }
            if (p != null && p.first != null) {
                p.first.invoke(index, m_m_a, res, errMsg);
            }
        };
        final long callIndex;
        synchronized (m_csSS) {
            callIndex = ++m_ssIndex;
        }
        synchronized (m_csCache) {
            m_mapMMA.put(callIndex, new Pair<>(mma, canceled));
        }
        q.Save(callIndex).Save(filter);
        boolean ok = SendRequest(Consts.idQueryMaxMinAvgs, q, arh, (h, c) -> {
            Pair<DMaxMinAvg, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapMMA.remove(callIndex);
            }
            if (p != null && p.second != null) {
                p.second.invoke(callIndex);
            }
        });
        CScopeUQueue.Unlock(q);
        if (!ok) {
            synchronized (m_csCache) {
                m_mapMMA.remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public long QueryPaymentMaxMinAvgs(String filter, DMaxMinAvg mma) {
        return QueryPaymentMaxMinAvgs(filter, mma, null);
    }

    public interface DConnectedSessions {

        public void invoke(long index, int m_connections, int s_connections);
    }
    private final HashMap<Long, Pair<DConnectedSessions, DMyCanceled>> m_mapSession = new HashMap<>();

    public long GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyCanceled canceled) {
        CUQueue q = CScopeUQueue.Lock();
        CAsyncServiceHandler.DAsyncResultHandler arh = (ar) -> {
            long index = ar.LoadLong();
            int master_connections = ar.LoadInt();
            int slave_connections = ar.LoadInt();
            Pair<DConnectedSessions, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapSession.remove(index);
            }
            if (p != null && p.first != null) {
                p.first.invoke(index, master_connections, slave_connections);
            }
        };
        final long callIndex;
        synchronized (m_csSS) {
            callIndex = ++m_ssIndex;
        }
        synchronized (m_csCache) {
            m_mapSession.put(callIndex, new Pair<>(cs, canceled));
        }
        q.Save(callIndex);
        boolean ok = SendRequest(Consts.idGetMasterSlaveConnectedSessions, q, arh, (h, c) -> {
            Pair<DConnectedSessions, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapSession.remove(callIndex);
            }
            if (p != null && p.second != null) {
                p.second.invoke(callIndex);
            }
        });
        CScopeUQueue.Unlock(q);
        if (!ok) {
            synchronized (m_csCache) {
                m_mapSession.remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public long GetMasterSlaveConnectedSessions(DConnectedSessions cs) {
        return GetMasterSlaveConnectedSessions(cs, null);
    }

    public interface DUploadEmployees {

        public void invoke(long index, int res, String errMsg, CLongArray vId);
    }
    private final HashMap<Long, Pair<DUploadEmployees, DMyCanceled>> m_mapUpload = new HashMap<>();

    public long UploadEmployees(CDBVariantArray vData, DUploadEmployees ue, DMyCanceled canceled) {
        if (vData == null) {
            vData = new CDBVariantArray();
        }
        CUQueue q = CScopeUQueue.Lock();
        CAsyncServiceHandler.DAsyncResultHandler arh = (ar) -> {
            long index = ar.LoadLong();
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            CLongArray vId = ar.Load(CLongArray.class);
            Pair<DUploadEmployees, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapUpload.remove(index);
            }
            if (p != null && p.first != null) {
                p.first.invoke(index, res, errMsg, vId);
            }
        };
        final long callIndex;
        synchronized (m_csSS) {
            callIndex = ++m_ssIndex;
        }
        synchronized (m_csCache) {
            m_mapUpload.put(callIndex, new Pair<>(ue, canceled));
        }
        q.Save(callIndex);
        vData.SaveTo(q);
        boolean ok = SendRequest(Consts.idUploadEmployees, q, arh, (h, c) -> {
            Pair<DUploadEmployees, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapUpload.remove(callIndex);
            }
            if (p != null && p.second != null) {
                p.second.invoke(callIndex);
            }
        });
        CScopeUQueue.Unlock(q);
        if (!ok) {
            synchronized (m_csCache) {
                m_mapUpload.remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public long UploadEmployees(CDBVariantArray vData, DUploadEmployees ue) {
        return UploadEmployees(vData, ue, null);
    }

    public interface DRentalDateTimes {

        public void invoke(long index, CRentalDateTimes dates, int res, String errMsg);
    }
    private final HashMap<Long, Pair<DRentalDateTimes, DMyCanceled>> m_mapRentalDateTimes = new HashMap<>();

    public long GetRentalDateTimes(long rentalId, DRentalDateTimes rdt, DMyCanceled canceled) {
        CUQueue q = CScopeUQueue.Lock();
        CAsyncServiceHandler.DAsyncResultHandler arh = (ar) -> {
            long index = ar.LoadLong();
            CRentalDateTimes dates = ar.Load(CRentalDateTimes.class);
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            Pair<DRentalDateTimes, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapRentalDateTimes.remove(index);
            }
            if (p != null && p.first != null) {
                p.first.invoke(index, dates, res, errMsg);
            }
        };
        final long callIndex;
        synchronized (m_csSS) {
            callIndex = ++m_ssIndex;
        }
        synchronized (m_csCache) {
            m_mapRentalDateTimes.put(callIndex, new Pair<>(rdt, canceled));
        }
        q.Save(callIndex).Save(rentalId);
        boolean ok = SendRequest(Consts.idGetRentalDateTimes, q, arh, (h, c) -> {
            Pair<DRentalDateTimes, DMyCanceled> p;
            synchronized (m_csCache) {
                p = m_mapRentalDateTimes.remove(callIndex);
            }
            if (p != null && p.second != null) {
                p.second.invoke(callIndex);
            }
        });
        CScopeUQueue.Unlock(q);
        if (!ok) {
            synchronized (m_csCache) {
                m_mapRentalDateTimes.remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public long GetRentalDateTimes(long rentalId, DRentalDateTimes rdt) {
        return GetRentalDateTimes(rentalId, rdt, null);
    }
    private static final Object m_csSS = new Object();
    private static long m_ssIndex = 0;//protected by m_csSS
}
