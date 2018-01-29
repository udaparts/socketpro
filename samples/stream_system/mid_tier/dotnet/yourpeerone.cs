using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using System.Threading.Tasks;
using SocketProAdapter.UDB;

class CYourPeerOne : CCacheBasePeer
{
    private object m_csConsole = new object();
    protected override void OnFastRequestArrive(ushort reqId, uint len)
    {
        if (reqId == ss.Consts.idQueryMaxMinAvgs)
        {
            QueryPaymentMaxMinAvgs(UQueue, CurrentRequestIndex);
        }
        else if (reqId == ss.Consts.idGetRentalDateTimes)
        {
            GetRentalDateTimes(UQueue, CurrentRequestIndex);
        }
        else if (reqId == ss.Consts.idUploadEmployees)
        {
            UploadEmployees(UQueue, CurrentRequestIndex);
        }
    }

    [RequestAttr(ss.Consts.idGetMasterSlaveConnectedSessions)]
    uint GetMasterSlaveConnectedSessions(out uint masters)
    {
        masters = CYourServer.Master.ConnectedSockets;
        return CYourServer.Slave.ConnectedSockets;
    }

    void QueryPaymentMaxMinAvgs(CUQueue q, ulong reqIndex)
    {
        uint ret;
        string filter;
        q.Load(out filter);
        //assuming slave pool has queue name set (request backup)
        System.Diagnostics.Debug.Assert(CYourServer.Slave.QueueName.Length > 0);
        ss.CMaxMinAvg pmma = new ss.CMaxMinAvg();
        string sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.Length > 0)
            sql += (" WHERE " + filter);
        var handler = CYourServer.Slave.SeekByQueue();
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idQueryMaxMinAvgs, (int)-1, "No connection to a slave database", pmma);
            return;
        }
        ulong peer_handle = Handle;
        bool ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) =>
        {
            //send result if front peer not closed yet
            if (peer_handle == Handle)
                ret = SendResultIndex(reqIndex, ss.Consts.idQueryMaxMinAvgs, r, err, pmma);
        }, (h, vData) =>
        {
            pmma.Max = double.Parse(vData[0].ToString());
            pmma.Min = double.Parse(vData[1].ToString());
            pmma.Avg = double.Parse(vData[2].ToString());
        });
        //should always be true because slave pool has queue name set for request backup
        System.Diagnostics.Debug.Assert(ok);
    }

    void GetRentalDateTimes(CUQueue q, ulong reqIndex)
    {
        uint ret;
        long rental_id;
        q.Load(out rental_id);
        //assuming slave pool has queue name set (request backup)
        System.Diagnostics.Debug.Assert(CYourServer.Slave.QueueName.Length > 0);
        ss.CRentalDateTimes myDates = new ss.CRentalDateTimes();
        string sql = "SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + rental_id;
        var handler = CYourServer.Slave.SeekByQueue();
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idGetRentalDateTimes, myDates, (int)-1, "No connection to a slave database");
            return;
        }
        ulong peer_handle = Handle;
        bool ok = handler.Execute(sql, (h, res, errMsg, affected, fail_ok, vtId) =>
        {
            //send result if front peer not closed yet
            if (peer_handle == Handle)
                ret = SendResultIndex(reqIndex, ss.Consts.idGetRentalDateTimes, myDates, res, errMsg);
        }, (h, vData) =>
        {
            myDates.rental_id = (long)vData[0];
            myDates.Rental = (DateTime)vData[1];
            myDates.Return = (DateTime)vData[2];
            myDates.LastUpdate = (DateTime)vData[3];
        });
        //should always be true because slave pool has queue name set for request backup
        System.Diagnostics.Debug.Assert(ok);
    }
    
    void UploadEmployees(CUQueue q, ulong reqIndex)
    {
        uint ret;
        KeyValuePair<int, string> error = new KeyValuePair<int, string>();
        ss.CInt64Array vId = new ss.CInt64Array();
        CDBVariantArray vData;
        q.Load(out vData);
        if (vData.Count == 0)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)0, "", vId);
            return;
        }
        else if ((vData.Count % 3) != 0)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)-1, "Data array size is wrong", vId);
            return;
        }
        //use master for insert, update and delete
        var handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)-2, "No connection to a master database", vId);
            return;
        }
        CClientSocket cs = handler.AttachedClientSocket;
        do
        {
            if (!handler.BeginTrans() || !handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)"))
                break;
            bool ok = true;
            CDBVariantArray v = new CDBVariantArray();
            int rows = vData.Count / 3;
            for (int n = 0; n < rows; ++n)
            {
                v.Add(vData[n * 3 + 0]);
                v.Add(vData[n * 3 + 1]);
                v.Add(vData[n * 3 + 2]);
                ok = handler.Execute(v, (h, r, err, affected, fail_ok, vtId) =>
                {
                    if (r != 0)
                    {
                        if (error.Key == 0)
                        {
                            error = new KeyValuePair<int, string>(r, err);
                        }
                        vId.Add(-1);
                    }
                    else
                    {
                        vId.Add(long.Parse(vtId.ToString()));
                    }
                });
                if (!ok)
                    break;
                v.Clear();
            }
            if (!ok)
                break;
            ulong peer_handle = Handle;
            if (!handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) =>
            {
                //send result if front peer not closed yet
                if (peer_handle == Handle)
                {
                    if (res != 0 && error.Key == 0)
                        error = new KeyValuePair<int, string>(res, errMsg);
                    ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, error.Key, error.Value, vId);
                }
            }, (h, canceled) =>
            {
                //send error message if front peer not closed yet
                if (peer_handle == Handle)
                {
                    //socket closed after requests are put on wire
                    if (error.Key == 0)
                        error = new KeyValuePair<int, string>(cs.ErrorCode, cs.ErrorMsg);
                    ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, error.Key, error.Value, vId);
                }
            }))
                break;
            //put handler back into pool as soon as possible for reuse as long as socket connection is not closed yet
            CYourServer.Master.Unlock(handler);
            return;
        } while (false);
        ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, cs.ErrorCode, cs.ErrorMsg, vId);
    }
    
    //manual retry for better fault tolerance
    /*
    void UploadEmployees(CUQueue q, ulong reqIndex)
    {
        uint ret;
        KeyValuePair<int, string> error = new KeyValuePair<int, string>();
        ss.CInt64Array vId = new ss.CInt64Array();
        CDBVariantArray vData;
        q.Load(out vData);
        if (vData.Count == 0)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)0, "", vId);
            return;
        }
        else if ((vData.Count % 3) != 0)
        {
            ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)-1, "Data array size is wrong", vId);
            return;
        }
        int redo = 0;
        do
        {
            //use master for insert, update and delete
            var handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null)
            {
                ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, (int)-2, "No connection to a master database", vId);
                return;
            }
            ++redo;
            do
            {
                bool ok = true;
                if (!handler.BeginTrans() || !handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)"))
                    break;
                SocketProAdapter.UDB.CDBVariantArray v = new SocketProAdapter.UDB.CDBVariantArray();
                int rows = vData.Count / 3;
                for (int n = 0; n < rows; ++n)
                {
                    v.Add(vData[n * 3 + 0]);
                    v.Add(vData[n * 3 + 1]);
                    v.Add(vData[n * 3 + 2]);
                    ok = handler.Execute(v, (h, r, err, affected, fail_ok, vtId) =>
                    {
                        if (r != 0)
                        {
                            if (error.Key == 0)
                            {
                                error = new KeyValuePair<int, string>(r, err);
                            }
                            vId.Add(-1);
                        }
                        else
                        {
                            vId.Add(long.Parse(vtId.ToString()));
                        }
                    });
                    if (!ok)
                        break;
                    v.Clear();
                }
                if (!ok)
                    break;
                ulong peer_handle = Handle;
                if (!handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) =>
                {
                    //send result if front peer not closed yet
                    if (peer_handle == Handle)
                    {
                        if (res != 0 && error.Key == 0)
                            error = new KeyValuePair<int, string>(res, errMsg);
                        ret = SendResultIndex(reqIndex, ss.Consts.idUploadEmployees, error.Key, error.Value, vId);
                    }
                }, (h, canceled) =>
                {
                    //retry if front peer not closed yet
                    if (peer_handle == Handle)
                    {
#if DEBUG
                        //socket closed after requests are put on wire
                        lock (m_csConsole)
                        {
                            Console.WriteLine("Retry UploadEmployees ......");
                        }
#endif
                        using (CScopeUQueue sb = new CScopeUQueue())
                        {
                            //repack original request data and retry
                            sb.Save(vData);
                            UploadEmployees(sb.UQueue, reqIndex); //this will not cause recursive stack-overflow exeption
                        }
                    }
                }))
                {
                    //socket just closed when sending last request EndTrans
                }
                else
                {
                    redo = 0; //no redo as all requests are successfully put on wire
                    //put handler back into pool as soon as possible for reuse, as long as socket connection is not closed yet
                    CYourServer.Master.Unlock(handler);
                }
            } while (false);
        } while (redo > 0);
    }
    */
    protected override string GetCachedTables(string defaultDb, uint flags, ulong index, out int dbMS, out int res)
    {
        res = 0;
        dbMS = (int)SocketProAdapter.UDB.tagManagementSystem.msUnknown;
        string errMsg = "";
        do
        {
            CConfig config = CConfig.GetConfig();
            if (config.m_vFrontCachedTable.Count == 0 || (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) != DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
                break;
            if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            {
                if (!Push.Subscribe(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID))
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
            var handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null)
            {
                res = -1;
                errMsg = "No connection to a master database";
                break;
            }
            dbMS = (int)handler.DBManagementSystem;
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
            }, true, true, (h, canceled) =>
            {
                tcs.SetResult(-2);
            }))
            {
                res = handler.AttachedClientSocket.ErrorCode;
                errMsg = handler.AttachedClientSocket.ErrorMsg;
                break;
            }
            CYourServer.Master.Unlock(handler); //put back locked handler and its socket back into pool for reuse as soon as possible
            Task<int> task = tcs.Task;
            if (!task.Wait(25000)) //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
            {
                res = -3;
                errMsg = "Querying cached table data timeout";
            }
            else if (task.Result == -2)
            {
                res = -2;
                errMsg = "Request canceled or socket closed";
            }
        } while (false);
        return errMsg;
    }
}
