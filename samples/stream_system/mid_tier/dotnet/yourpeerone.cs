using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using System.Threading.Tasks;

class CYourPeerOne : CCacheBasePeer
{
    private object m_csConsole = new object();
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
        uint ret;
        ulong index;
        ss.CMaxMinAvg pmma = new ss.CMaxMinAvg();
        string filter;
        q.Load(out index).Load(out filter);
        string sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.Length > 0)
            sql += (" WHERE " + filter);
        int redo = 0;
        do
        {
            CMysql handler = CYourServer.Slave.Seek();
            if (handler == null)
            {
                ret = SendResult(ss.Consts.idQueryMaxMinAvgs, index, (int)-1, "No connection to a slave database", pmma);
                return;
            }
            ++redo;
            ulong peer_handle = Handle;
            if (handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) =>
            {
                ret = SendResult(ss.Consts.idQueryMaxMinAvgs, index, r, err, pmma);
            }, (h, vData) =>
            {
                pmma.Max = double.Parse(vData[0].ToString());
                pmma.Min = double.Parse(vData[1].ToString());
                pmma.Avg = double.Parse(vData[2].ToString());
            }, (h) => { }, true, true, () =>
            {
                //front peer not closed yet
                if (peer_handle == Handle)
                {
#if DEBUG
                    //socket closed after sending
                    lock (m_csConsole)
                    {
                        Console.WriteLine("Retry rental date times ......");
                    }
#endif
                    using (CScopeUQueue sb = new CScopeUQueue())
                    {
                        //repack original request data and retry
                        sb.Save(index).Save(filter);
                        QueryPaymentMaxMinAvgs(sb.UQueue); //this will not cause recursive stack-overflow exeption
                    }
                }
            }))
            {
                redo = 0; //disable redo once request is put on wire
            }
        } while (redo > 0);
    }

    void UploadEmployees(CUQueue q)
    {
        uint ret;
        ulong index;
        KeyValuePair<int, string> error = new KeyValuePair<int, string>();
        ss.CInt64Array vId = new ss.CInt64Array();
        SocketProAdapter.UDB.CDBVariantArray vData;
        q.Load(out index).Load(out vData);
        if (vData.Count == 0)
        {
            ret = SendResult(ss.Consts.idUploadEmployees, index, (int)0, "", new ss.CInt64Array());
            return;
        }
        else if ((vData.Count % 3) != 0)
        {
            ret = SendResult(ss.Consts.idUploadEmployees, index, (int)-1, "Data array size is wrong", new ss.CInt64Array());
            return;
        }
        int redo = 0;
        do
        {
            //use master for insert, update and delete
            CMysql handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null)
            {
                ret = SendResult(ss.Consts.idUploadEmployees, index, (int)-2, "No connection to a master database", new ss.CInt64Array());
                return;
            }
            ++redo;
            do
            {
                bool ok = false;
                if (!handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) break;
                if (!handler.BeginTrans()) break;
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
                if (handler.EndTrans(SocketProAdapter.UDB.tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) =>
                {
                    if (res != 0 && error.Key == 0)
                        error = new KeyValuePair<int, string>(res, errMsg);
                    ret = SendResult(ss.Consts.idUploadEmployees, index, error.Key, error.Value, vId);
                }, () =>
                {
                    //front peer not closed yet
                    if (peer_handle == Handle)
                    {
#if DEBUG
                        //socket closed after sending
                        lock (m_csConsole)
                        {
                            Console.WriteLine("Retry UploadEmployees ......");
                        }
#endif
                        using (CScopeUQueue sb = new CScopeUQueue())
                        {
                            //repack original request data and retry
                            sb.Save(index).Save(vData);
                            UploadEmployees(sb.UQueue); //this will not cause recursive stack-overflow exeption
                        }
                    }
                }))
                {
                    redo = 0;
                    CYourServer.Master.Unlock(handler);
                }
                else
                {
                    //socket closed when sending
                }
            } while (false);
        } while (redo > 0);
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
            }, true, true, () =>
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

    [RequestAttr(ss.Consts.idGetRentalDateTimes, true)]
    string GetRentalDateTimes(ulong index, long rental_id, out ulong retIndex, out ss.CRentalDateTimes dates, out int res)
    {
        retIndex = index;
        ss.CRentalDateTimes myDates = new ss.CRentalDateTimes(rental_id);
        res = 0;
        string errMsg = "";
        string sql = "SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + rental_id;
        int redo = 0;
        do
        {
            CMysql handler = CYourServer.Slave.Seek();
            if (handler == null)
            {
                res = -1;
                errMsg = "No connection to a slave database";
                break;
            }
            ++redo;
            TaskCompletionSource<int> tcs = new TaskCompletionSource<int>();
            if (handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) =>
            {
                errMsg = err;
                tcs.SetResult(1);
            }, (h, vData) =>
            {
                myDates.Rental = (DateTime)vData[1];
                myDates.Return = (DateTime)vData[2];
                myDates.LastUpdate = (DateTime)vData[3];
            }, (h) =>
            {
                //rowset meta
            }, true, true, () =>
            {
                //socket closed after sending
                tcs.SetResult(0);
            }))
            {
                //don't use handle->WaitAll() for better completion event as a session may be shared by multiple threads
                if (!tcs.Task.Wait(20000))
                {
                    res = -2;
                    errMsg = "Querying rental date times timed out";
                    redo = 0; //no redo because of timed-out
                }
                else if (tcs.Task.Result > 0)
                {
                    redo = 0; //disable redo after result returned
                }
                else
                {
#if DEBUG
                    //socket closed after sending
                    lock (m_csConsole)
                    {
                        Console.WriteLine("Retry rental date times ......");
                    }
#endif
                }
            }
            else
            {
#if DEBUG
                //socket closed when sending SQL
                lock (m_csConsole)
                {
                    Console.WriteLine("Retry rental date times ......");
                }
#endif
            }
        } while (redo > 0);
        dates = myDates;
        return errMsg;
    }
}
