﻿using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using SocketProAdapter.ClientSide;
using System.Collections.Generic;
using SocketProAdapter.UDB;
using ss;

class CYourPeerOne : CCacheBasePeer
{
    private object m_csConsole = new object();
    protected override void OnFastRequestArrive(ushort reqId, uint len)
    {
        if (reqId == Consts.idQueryMaxMinAvgs)
            QueryPaymentMaxMinAvgs(UQueue, CurrentRequestIndex);
        else if (reqId == Consts.idGetRentalDateTimes)
            GetRentalDateTimes(UQueue, CurrentRequestIndex);
        else if (reqId == Consts.idUploadEmployees)
            UploadEmployees(UQueue, CurrentRequestIndex);
    }

    [RequestAttr(Consts.idGetMasterSlaveConnectedSessions)]
    uint GetMasterSlaveConnectedSessions(out uint masters)
    {
        masters = CYourServer.Master.ConnectedSockets;
        return CYourServer.Slave.ConnectedSockets;
    }

    void QueryPaymentMaxMinAvgs(CUQueue q, ulong reqIndex)
    {
        uint ret;
        string filter = q.Load<string>();
        //assuming slave pool has queue name set (request backup)
        System.Diagnostics.Debug.Assert(CYourServer.Slave.QueueName.Length > 0);
        CMaxMinAvg pmma = new CMaxMinAvg();
        string sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.Length > 0)
            sql += (" WHERE " + filter);
        var handler = CYourServer.Slave.SeekByQueue();
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, (int)-2, "No connection to anyone of slave databases", pmma);
            return;
        }
        ulong peer_handle = Handle;
        bool ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) =>
        {
            //send result if front peer not closed yet
            if (peer_handle == Handle)
                ret = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, r, err, pmma);
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
        long rental_id = q.Load<long>();
        //assuming slave pool has queue name set (request backup)
        System.Diagnostics.Debug.Assert(CYourServer.Slave.QueueName.Length > 0);
        CRentalDateTimes myDates = new CRentalDateTimes(rental_id);
        string sql = "SELECT rental_date,return_date,last_update FROM rental where rental_id=" + rental_id;
        var handler = CYourServer.Slave.SeekByQueue();
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, myDates, (int)-2, "No connection to anyone of slave databases");
            return;
        }
        ulong peer_handle = Handle;
        bool ok = handler.Execute(sql, (h, res, errMsg, affected, fail_ok, vtId) =>
        {
            //send result if front peer not closed yet
            if (peer_handle == Handle)
                ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, myDates, res, errMsg);
        }, (h, vData) =>
        {
            myDates.Rental = (DateTime)vData[0];
            if (vData[1] != null && vData[1] is DateTime)
                myDates.Return = (DateTime)vData[1];
            myDates.LastUpdate = (DateTime)vData[2];
        });
        //should always be true because slave pool has queue name set for request backup
        System.Diagnostics.Debug.Assert(ok);
    }

    void UploadEmployees(CUQueue q, ulong reqIndex)
    {
        uint ret;
        KeyValuePair<int, string> error = new KeyValuePair<int, string>();
        CInt64Array vId = new CInt64Array();
        CDBVariantArray vData = q.Load<CDBVariantArray>();
        if (vData.Count == 0)
        {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, (int)0, "", vId);
            return;
        }
        else if ((vData.Count % 3) != 0)
        {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, (int)-1, "Data array size is wrong", vId);
            return;
        }
        //use master for insert, update and delete
        var handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
        if (handler == null)
        {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, (int)-2, "No connection to a master database", vId);
            return;
        }
        CClientSocket cs = handler.Socket;
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
                    ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, error.Key, error.Value, vId);
                }
            }, (h, canceled) =>
            {
                //send error message if front peer not closed yet
                if (peer_handle == Handle)
                {
                    //socket closed after requests are put on wire
                    if (error.Key == 0)
                        error = new KeyValuePair<int, string>(cs.ErrorCode, cs.ErrorMsg);
                    ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, error.Key, error.Value, vId);
                }
            }))
                break;
            //put handler back into pool as soon as possible for reuse as long as socket connection is not closed yet
            CYourServer.Master.Unlock(handler);
            return;
        } while (false);
        ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, cs.ErrorCode, cs.ErrorMsg, vId);
    }

    protected override string GetCachedTables(string defaultDb, uint flags, ulong index, out int dbMS, out int res)
    {
        res = 0;
        dbMS = (int)SocketProAdapter.UDB.tagManagementSystem.msUnknown;
        string errMsg = "";
        if (CYourServer.FrontCachedTables.Count == 0 || (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) != DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            return errMsg;
        if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)
            Push.Subscribe(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
        string sql = "";
        List<string> v = CYourServer.FrontCachedTables;
        foreach (string s in v)
        {
            if (sql.Length != 0)
                sql += ";";
            sql += "SELECT * FROM " + s;
        }
        //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
        var handler = CYourServer.Master.Lock();
        if (handler == null)
        {
            res = -2;
            return "No connection to a master database";
        }
        dbMS = (int)handler.DBManagementSystem;
        try
        {
            var t = handler.execute(sql, (h, vData) =>
            {
                SendRows(vData);
            }, (h) =>
            {
                SendMeta(h.ColumnInfo, index);
            });
            //put back locked handler and its socket back into pool for reuse as soon as possible
            CYourServer.Master.Unlock(handler);
            if (t.Wait(30000))
            {
                var result = t.Result;
                res = result.ec;
                errMsg = result.em;
            }
            else
            {
                res = -3;
                errMsg = "Querying cached table data timeout";
            }
        }
        catch (CSocketError ex)
        {
            res = ex.ErrCode;
            errMsg = ex.Message;
        }
        return errMsg;
    }
}
