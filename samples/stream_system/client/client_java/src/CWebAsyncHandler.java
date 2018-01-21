
import SPA.ClientSide.*;
import SPA.*;
import SPA.UDB.CDBVariantArray;

public class CWebAsyncHandler extends CCachedBaseHandler {

    public CWebAsyncHandler() {
        super(Consts.sidStreamSystem);
    }

    public interface DMaxMinAvg {

        public void invoke(CMaxMinAvg mma, int res, String errMsg);
    }

    public boolean QueryPaymentMaxMinAvgs(String filter, DMaxMinAvg mma, DDiscarded discarded) {
        CUQueue q = CScopeUQueue.Lock();
        DAsyncResultHandler arh = (ar) -> {
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            CMaxMinAvg m_m_a = ar.Load(CMaxMinAvg.class);
            if (mma != null) {
                mma.invoke(m_m_a, res, errMsg);
            }
        };
        q.Save(filter);
        boolean ok = SendRequest(Consts.idQueryMaxMinAvgs, q, arh, discarded);
        CScopeUQueue.Unlock(q);
        return ok;
    }

    public boolean QueryPaymentMaxMinAvgs(String filter, DMaxMinAvg mma) {
        return QueryPaymentMaxMinAvgs(filter, mma, null);
    }

    public interface DConnectedSessions {

        public void invoke(int m_connections, int s_connections);
    }

    public boolean GetMasterSlaveConnectedSessions(DConnectedSessions cs, DDiscarded discarded) {
        DAsyncResultHandler arh = (ar) -> {
            int master_connections = ar.LoadInt();
            int slave_connections = ar.LoadInt();
            if (cs != null) {
                cs.invoke(master_connections, slave_connections);
            }
        };
        return SendRequest(Consts.idGetMasterSlaveConnectedSessions, arh, discarded);
    }

    public boolean GetMasterSlaveConnectedSessions(DConnectedSessions cs) {
        return GetMasterSlaveConnectedSessions(cs, null);
    }

    public interface DUploadEmployees {

        public void invoke(int res, String errMsg, CLongArray vId);
    }

    public boolean UploadEmployees(CDBVariantArray vData, DUploadEmployees ue, DDiscarded discarded) {
        if (vData == null) {
            vData = new CDBVariantArray();
        }
        CUQueue q = CScopeUQueue.Lock();
        DAsyncResultHandler arh = (ar) -> {
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            CLongArray vId = ar.Load(CLongArray.class);
            if (ue != null) {
                ue.invoke(res, errMsg, vId);
            }
        };
        vData.SaveTo(q);
        boolean ok = SendRequest(Consts.idUploadEmployees, q, arh, discarded);
        CScopeUQueue.Unlock(q);
        return ok;
    }

    public boolean UploadEmployees(CDBVariantArray vData, DUploadEmployees ue) {
        return UploadEmployees(vData, ue, null);
    }

    public interface DRentalDateTimes {

        public void invoke(CRentalDateTimes dates, int res, String errMsg);
    }

    public boolean GetRentalDateTimes(long rentalId, DRentalDateTimes rdt, DDiscarded discarded) {
        CUQueue q = CScopeUQueue.Lock();
        DAsyncResultHandler arh = (ar) -> {
            CRentalDateTimes dates = ar.Load(CRentalDateTimes.class);
            int res = ar.LoadInt();
            String errMsg = ar.LoadString();
            if (rdt != null) {
                rdt.invoke(dates, res, errMsg);
            }
        };
        q.Save(rentalId);
        boolean ok = SendRequest(Consts.idGetRentalDateTimes, q, arh, discarded);
        CScopeUQueue.Unlock(q);
        return ok;
    }

    public boolean GetRentalDateTimes(long rentalId, DRentalDateTimes rdt) {
        return GetRentalDateTimes(rentalId, rdt, null);
    }
}
