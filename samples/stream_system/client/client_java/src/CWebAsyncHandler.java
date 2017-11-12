
public class CWebAsyncHandler extends SPA.ClientSide.CCachedBaseHandler {

    public CWebAsyncHandler() {
        super(Consts.sidStreamSystem);
    }

    public interface DMyCanceled {

        public void invoke(long index);
    }

    public interface DMaxMinAvg {

        public void invoke(long index, CMaxMinAvg mma, int res, String errMsg);
    }

    public interface DConnectedSessions {

        public void invoke(long index, CMaxMinAvg mma, int m_connections, int s_connections);
    }

    public interface DUploadEmployees {

        public void invoke(long index, int res, String errMsg, CLongArray vId);
    }

    public interface DRentalDateTimes {

        public void invoke(long index, CRentalDateTimes dates, int res, String errMsg);
    }
    
    private static final Object m_csSS = new Object();
    private static long m_ssIndex = 0;//protected by m_csSS
}
