using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using System.Threading.Tasks;

class CYourPeerOne : CCacheBasePeer
{
    protected override void OnFastRequestArrive(ushort reqId, uint len)
    {
        if (reqId == ss.Consts.idQueryMaxMinAvgs)
        {
            QueryPaymentMaxMinAvgs(UQueue);
        }
        else if (reqId == ss.Consts.idUploadEmployees)
        {
            UploadEmployees(UQueue);
        }
    }

    void QueryPaymentMaxMinAvgs(CUQueue q)
    {

    }

    void UploadEmployees(CUQueue q)
    {

    }

    [RequestAttr(ss.Consts.idGetMasterSlaveConnectedSessions)]
    uint GetMasterSlaveConnectedSessions(ulong index, out ulong retIndex, out uint masters)
    {
        retIndex = index;
        masters = CYourServer.Master.ConnectedSockets;
        return CYourServer.Slave.ConnectedSockets;
    }

    protected override string GetCachedTables(string defaultDb, uint flags, bool rowset, ulong index, out int res)
    {
        res = 0;
        string errMsg = "";
        do
        {
            if (!rowset)
                break;
            CConfig config = CConfig.GetConfig();
            if (config.m_vFrontCachedTable.Count == 0)
                break;
            if ((flags & CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) == CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES)
            {
                if (!Push.Subscribe(CAsyncDBHandler.CACHE_UPDATE_CHAT_GROUP_ID, CAsyncDBHandler.STREAMING_SQL_CHAT_GROUP_ID))
                    errMsg = "Failed in subscribing for table events"; //warning message
            }
            string sql = "";
            List<string> v = config.m_vFrontCachedTable;
            foreach (string s in v)
            {
                if (sql.Length != 0)
                    sql += ";";
                sql += "SELECT * FROM " + s;
            }
            CMysql handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null)
            {
                res = -1;
                errMsg = "No connection to a master database";
                break;
            }
            TaskCompletionSource<int> tcs = new TaskCompletionSource<int>();
            if (!handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) =>
            {
                errMsg = err;
                tcs.SetResult(r);
            }, (h, vData) =>
            {
                SendRows(vData);
            }, (h) =>
            {
                SendMeta(h.ColumnInfo, index);
            }, true, true, () => {
                errMsg = "Request canceled or socket closed";
                tcs.SetResult(-2);
            }))
            {
                res = handler.AttachedClientSocket.ErrorCode;
                errMsg = handler.AttachedClientSocket.ErrorMsg;
                break;
            }
            CYourServer.Master.Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
            if (!tcs.Task.Wait(25000)) //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
            {
                res = -3;
                errMsg = "Querying cached table data timeout";
            }
        } while (false);
        return errMsg;
    }

    [RequestAttr(ss.Consts.idGetRentalDateTimes, true)]
    string GetRentalDateTimes(ulong index, long rentalId, out ulong retIndex, out ss.CRentalDateTimes dates, out int res)
    {
        retIndex = index;

        dates = new ss.CRentalDateTimes();
        res = 0;
        return "";
    }
}
