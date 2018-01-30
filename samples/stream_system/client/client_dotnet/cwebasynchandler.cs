using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class CWebAsyncHandler : CCachedBaseHandler
{
    public CWebAsyncHandler()
        : base(ss.Consts.sidStreamSystem)
    {
    }

    public delegate void DMaxMinAvg(ss.CMaxMinAvg mma, int res, string errMsg);
    public bool QueryPaymentMaxMinAvgs(string filter, DMaxMinAvg mma, DDiscarded discarded = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            int res;
            string errMsg;
            ss.CMaxMinAvg m_m_a;
            ar.Load(out res).Load(out errMsg).Load(out m_m_a);
            if (mma != null)
                mma(m_m_a, res, errMsg);
        };
        return SendRequest(ss.Consts.idQueryMaxMinAvgs, filter, arh, discarded, (DOnExceptionFromServer)null);
    }

    public delegate void DConnectedSessions(uint m_connection, uint s_connection);
    public bool GetMasterSlaveConnectedSessions(DConnectedSessions cs, DDiscarded discarded = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            uint master_connections, slave_conenctions;
            ar.Load(out master_connections).Load(out slave_conenctions);
            if (cs != null)
                cs(master_connections, slave_conenctions);
        };
        return SendRequest(ss.Consts.idGetMasterSlaveConnectedSessions, arh, discarded, (DOnExceptionFromServer)null);
    }

    public delegate void DUploadEmployees(int res, string errMsg, ss.CInt64Array vId);
    public bool UploadEmployees(SocketProAdapter.UDB.CDBVariantArray vData, DUploadEmployees res, DDiscarded discarded = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            int errCode;
            string errMsg;
            ss.CInt64Array vId;
            ar.Load(out errCode).Load(out errMsg).Load(out vId);
            if (res != null)
                res(errCode, errMsg, vId);
        };
        return SendRequest(ss.Consts.idUploadEmployees, vData, arh, discarded, (DOnExceptionFromServer)null);
    }

    public delegate void DRentalDateTimes(ss.CRentalDateTimes dates, int res, string errMsg);
    public bool GetRentalDateTimes(long rentalId, DRentalDateTimes rdt, DDiscarded discarded = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            int errCode;
            string errMsg;
            ss.CRentalDateTimes dates;
            ar.Load(out dates).Load(out errCode).Load(out errMsg);
            if (rdt != null)
                rdt(dates, errCode, errMsg);
        };
        return SendRequest(ss.Consts.idGetRentalDateTimes, rentalId, arh, discarded, (DOnExceptionFromServer)null);
    }
}
