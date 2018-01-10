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

    public delegate void DMyCanceled(ulong index);

    private static ulong m_ssIndex = 0; //protected by IndexLocker

    public delegate void DMaxMinAvg(ulong index, ss.CMaxMinAvg mma, int res, string errMsg);
    private Dictionary<ulong, KeyValuePair<DMaxMinAvg, DMyCanceled>> m_mapMMA = new Dictionary<ulong, KeyValuePair<DMaxMinAvg, DMyCanceled>>();
    public ulong QueryPaymentMaxMinAvgs(string filter, DMaxMinAvg mma, DMyCanceled canceled = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            ulong index;
            int res;
            string errMsg;
            ss.CMaxMinAvg m_m_a;
            ar.Load(out index).Load(out res).Load(out errMsg).Load(out m_m_a);
            KeyValuePair<DMaxMinAvg, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapMMA[index];
                m_mapMMA.Remove(index);
            }
            if (p.Key != null)
                p.Key.Invoke(index, m_m_a, res, errMsg);
        };
        ulong callIndex;
        lock (IndexLocker)
        {
            callIndex = ++m_ssIndex;
        }
        lock (m_csCache)
        {
            m_mapMMA[callIndex] = new KeyValuePair<DMaxMinAvg, DMyCanceled>(mma, canceled);
        }
        if (!SendRequest(ss.Consts.idQueryMaxMinAvgs, callIndex, filter, arh, () =>
        {
            //socket closed or request canceled after request is put on wire
            KeyValuePair<DMaxMinAvg, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapMMA[callIndex];
                m_mapMMA.Remove(callIndex);
            }
            if (p.Value != null)
                p.Value.Invoke(callIndex);
        }, (DOnExceptionFromServer)null))
        {
            //socket is already closed before sending request
            lock (m_csCache)
            {
                m_mapMMA.Remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public delegate void DConnectedSessions(ulong index, uint m_connection, uint s_connection);
    private Dictionary<ulong, KeyValuePair<DConnectedSessions, DMyCanceled>> m_mapSession = new Dictionary<ulong, KeyValuePair<DConnectedSessions, DMyCanceled>>();
    public ulong GetMasterSlaveConnectedSessions(DConnectedSessions cs, DMyCanceled canceled = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            ulong index;
            uint master_connections, slave_conenctions;
            ar.Load(out index).Load(out master_connections).Load(out slave_conenctions);
            KeyValuePair<DConnectedSessions, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapSession[index];
                m_mapSession.Remove(index);
            }
            if (p.Key != null)
                p.Key.Invoke(index, master_connections, slave_conenctions);
        };
        ulong callIndex;
        lock (IndexLocker)
        {
            callIndex = ++m_ssIndex;
        }
        lock (m_csCache)
        {
            m_mapSession[callIndex] = new KeyValuePair<DConnectedSessions, DMyCanceled>(cs, canceled);
        }
        if (!SendRequest(ss.Consts.idGetMasterSlaveConnectedSessions, callIndex, arh, () =>
        {
            //socket closed or request canceled after request is put on wire
            KeyValuePair<DConnectedSessions, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapSession[callIndex];
                m_mapSession.Remove(callIndex);
            }
            if (p.Value != null)
                p.Value.Invoke(callIndex);
        }, (DOnExceptionFromServer)null))
        {
            //socket is already closed before sending request
            lock (m_csCache)
            {
                m_mapSession.Remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public delegate void DUploadEmployees(ulong index, int res, string errMsg, ss.CInt64Array vId);
    private Dictionary<ulong, KeyValuePair<DUploadEmployees, DMyCanceled>> m_mapUpload = new Dictionary<ulong, KeyValuePair<DUploadEmployees, DMyCanceled>>();
    public ulong UploadEmployees(SocketProAdapter.UDB.CDBVariantArray vData, DUploadEmployees res, DMyCanceled canceled = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            ulong index;
            int errCode;
            string errMsg;
            ss.CInt64Array vId;
            ar.Load(out index).Load(out errCode).Load(out errMsg).Load(out vId);
            KeyValuePair<DUploadEmployees, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapUpload[index];
                m_mapUpload.Remove(index);
            }
            if (p.Key != null)
                p.Key.Invoke(index, errCode, errMsg, vId);
        };
        ulong callIndex;
        lock (IndexLocker)
        {
            callIndex = ++m_ssIndex;
        }
        lock (m_csCache)
        {
            m_mapUpload[callIndex] = new KeyValuePair<DUploadEmployees, DMyCanceled>(res, canceled);
        }
        if (!SendRequest(ss.Consts.idUploadEmployees, callIndex, vData, arh, () =>
        {
            //socket closed or request canceled after request is put on wire
            KeyValuePair<DUploadEmployees, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapUpload[callIndex];
                m_mapUpload.Remove(callIndex);
            }
            if (p.Value != null)
                p.Value.Invoke(callIndex);
        }, (DOnExceptionFromServer)null))
        {
            //socket is already closed before sending request
            lock (m_csCache)
            {
                m_mapUpload.Remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }

    public delegate void DRentalDateTimes(ulong index, ss.CRentalDateTimes dates, int res, string errMsg);
    private Dictionary<ulong, KeyValuePair<DRentalDateTimes, DMyCanceled>> m_mapRentalDateTimes = new Dictionary<ulong, KeyValuePair<DRentalDateTimes, DMyCanceled>>();
    public ulong GetRentalDateTimes(long rentalId, DRentalDateTimes rdt, DMyCanceled canceled = null)
    {
        DAsyncResultHandler arh = (ar) =>
        {
            ulong index;
            int errCode;
            string errMsg;
            ss.CRentalDateTimes dates;
            ar.Load(out index).Load(out dates).Load(out errCode).Load(out errMsg);
            KeyValuePair<DRentalDateTimes, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapRentalDateTimes[index];
                m_mapRentalDateTimes.Remove(index);
            }
            if (p.Key != null)
                p.Key.Invoke(index, dates, errCode, errMsg);
        };
        ulong callIndex;
        lock (IndexLocker)
        {
            callIndex = ++m_ssIndex;
        }
        lock (m_csCache)
        {
            m_mapRentalDateTimes[callIndex] = new KeyValuePair<DRentalDateTimes, DMyCanceled>(rdt, canceled);
        }
        if (!SendRequest(ss.Consts.idGetRentalDateTimes, callIndex, rentalId, arh, () =>
        {
            //socket closed or request canceled after request is put on wire
            KeyValuePair<DRentalDateTimes, DMyCanceled> p;
            lock (m_csCache)
            {
                p = m_mapRentalDateTimes[callIndex];
                m_mapRentalDateTimes.Remove(callIndex);
            }
            if (p.Value != null)
                p.Value.Invoke(callIndex);
        }, (DOnExceptionFromServer)null))
        {
            //socket is already closed before sending request
            lock (m_csCache)
            {
                m_mapRentalDateTimes.Remove(callIndex);
            }
            return 0;
        }
        return callIndex;
    }
}
