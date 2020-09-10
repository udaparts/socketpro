
import SPA.*;
import SPA.UDB.*;
import SPA.ServerSide.*;
import SPA.ClientSide.*;
import java.util.concurrent.*;

public class CYourPeerOne extends CCacheBasePeer {

    private void QueryPaymentMaxMinAvgs(CUQueue q, long reqIndex) {
        final CMaxMinAvg pmma = new CMaxMinAvg();
        String filter = q.LoadString();
        String sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.length() > 0) {
            sql += (" WHERE " + filter);
        }
        CMysql handler = CYourServer.Slave.SeekByQueue();
        if (handler == null) {
            try (CScopeUQueue sb = new CScopeUQueue()) {
                sb.Save((int) -2).Save("No connection to anyone of slave databases").Save(pmma);
                int ret = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, sb);
            }
            return;
        }
        long peer_handle = getHandle();
        boolean ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
            //send result if front peer not closed yet
            if (peer_handle == getHandle()) {
                try (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(r).Save(err).Save(pmma);
                    int res = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, sb);
                }
            }
        }, (h, vData) -> {
            pmma.Max = Double.parseDouble(vData.get(0).toString());
            pmma.Min = Double.parseDouble(vData.get(1).toString());
            pmma.Avg = Double.parseDouble(vData.get(2).toString());
        });
        assert ok; //always be true if pool has local queue for request backup
    }

    @Override
    protected void OnFastRequestArrive(short reqId, int len) {
        if (reqId == Consts.idQueryMaxMinAvgs) {
            QueryPaymentMaxMinAvgs(getUQueue(), getCurrentRequestIndex());
        } else if (reqId == Consts.idUploadEmployees) {
            UploadEmployees(getUQueue(), getCurrentRequestIndex());
        } else if (reqId == Consts.idGetRentalDateTimes) {
            GetRentalDateTimes(getUQueue(), getCurrentRequestIndex());
        }
    }

    @RequestAttr(RequestID = Consts.idGetMasterSlaveConnectedSessions)
    private CScopeUQueue GetMasterSlaveConnectedSessions() {
        CScopeUQueue sb = new CScopeUQueue();
        sb.Save(CYourServer.Master.getConnectedSockets()).Save(CYourServer.Slave.getConnectedSockets());
        return sb;
    }

    private void GetRentalDateTimes(CUQueue q, long reqIndex) {
        long rental_id = q.LoadLong();
        final CRentalDateTimes myDates = new CRentalDateTimes();
        myDates.rental_id = rental_id;
        String sql = "SELECT rental_date,return_date,last_update FROM rental where rental_id=" + rental_id;
        CMysql handler = CYourServer.Slave.SeekByQueue();
        if (handler == null) {
            try (CScopeUQueue sb = new CScopeUQueue()) {
                sb.Save(myDates).Save((int) -2).Save("No connection to anyone of slave databases");
                int ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, sb);
            }
            return;
        }
        long peer_handle = getHandle();
        boolean ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
            //send result if front peer not closed yet
            if (peer_handle == getHandle()) {
                try (CScopeUQueue sb = new CScopeUQueue()) {
                    sb.Save(myDates).Save(r).Save(err);
                    int ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, sb);
                }
            }
        }, (h, vData) -> {
            myDates.Rental = (java.sql.Timestamp) vData.get(0);
            Object obj = vData.get(1);
            if (obj != null && obj instanceof java.sql.Timestamp) {
                myDates.Return = (java.sql.Timestamp) obj;
            }
            myDates.LastUpdate = (java.sql.Timestamp) vData.get(2);
        });
        assert ok; //always be true if pool has local queue for request backup
    }

    @Override
    @RequestAttr(RequestID = DB_CONSTS.idGetCachedTables, SlowRequest = true) //true -- slow request
    protected CachedTableResult GetCachedTables(String defaultDb, int flags, long index) {
        final CachedTableResult res = this.new CachedTableResult();
        if (CYourServer.FrontCachedTables.isEmpty() || DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES != (flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES)) {
            return res;
        }
        if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) {
            if (!getPush().Subscribe(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID)) {
                res.errMsg = "Failed in subscribing for table events"; //warning message
            }
        }
        String sql = "";
        for (String s : CYourServer.FrontCachedTables) {
            if (sql.length() != 0) {
                sql += ";";
            }
            sql += "SELECT * FROM " + s;
        }
        //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
        CMysql handler = CYourServer.Master.Lock();
        if (handler == null) {
            res.res = -1;
            res.errMsg = "No connection to a master database";
            return res;
        }
        res.ms = handler.getDBManagementSystem();
        try {
            UFuture<CMysql.SQLExeInfo> f = handler.execute(sql, (h, vData) -> {
                SendRows(vData);
            }, (h) -> {
                SendMeta(h.getColumnInfo(), index);
            });
            //put back locked handler and its socket back into pool for reuse as soon as possible
            CYourServer.Master.Unlock(handler);
            CMysql.SQLExeInfo sei = f.get(30, TimeUnit.SECONDS);
            res.errMsg = sei.em;
            res.res = sei.ec;
        } catch (java.util.concurrent.TimeoutException ex) {
            res.errMsg = ex.getMessage();
            res.res = -3;
        } catch (CServerError ex) {
            res.errMsg = ex.getMessage();
            res.res = ex.getErrCode();
        } catch (CSocketError ex) {
            res.errMsg = ex.getMessage();
            res.res = ex.getErrCode();
        } catch (Exception ex) {
            res.errMsg = ex.getMessage();
            res.res = -1;
        }
        return res;
    }

    private void UploadEmployees(CUQueue q, long reqIndex) {
        final Pair<Integer, String> p = new Pair<>(0, "");
        final CDBVariantArray vData = new CDBVariantArray();
        vData.LoadFrom(q);
        final CLongArray vId = new CLongArray();
        try (CScopeUQueue sb = new CScopeUQueue()) {
            int ret;
            if (vData.isEmpty()) {
                ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) 0).Save("").Save(vId));
            } else if (vData.size() % 3 != 0) {
                ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -1).Save("Data array size is wrong").Save(vId));
            } else {
                do {
                    //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
                    CMysql handler = CYourServer.Master.Lock();
                    if (handler == null) {
                        ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -2).Save("No connection to a master database").Save(vId));
                        break;
                    }
                    CClientSocket cs = handler.getSocket();
                    if (!handler.BeginTrans() || !handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) {
                        ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                        break;
                    }
                    boolean ok = false;
                    CDBVariantArray v = new CDBVariantArray();
                    int rows = vData.size() / 3;
                    for (int n = 0; n < rows; ++n) {
                        v.add(vData.get(n * 3 + 0));
                        v.add(vData.get(n * 3 + 1));
                        v.add(vData.get(n * 3 + 2));
                        ok = handler.Execute(v, (h, r, err, affected, fail_ok, vtId) -> {
                            if (r != 0) {
                                if (p.first == 0) {
                                    p.first = r;
                                    p.second = err;
                                }
                                vId.add((long) -1);
                            } else {
                                vId.add(Long.parseLong(vtId.toString()));
                            }
                        });
                        if (!ok) {
                            break;
                        }
                        v.clear();
                    }
                    if (!ok) {
                        ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                        break;
                    }
                    long peer_handle = getHandle();
                    if (!handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) -> {
                        if (res != 0 && p.first == 0) {
                            p.first = res;
                            p.second = errMsg;
                        }
                        //send result if front peer not closed yet
                        if (peer_handle == getHandle()) {
                            try (CScopeUQueue sb0 = new CScopeUQueue()) {
                                SendResultIndex(reqIndex, Consts.idUploadEmployees, sb0.Save(p.first).Save(p.second).Save(vId));
                            }
                        }
                    }, (h, canceled) -> {
                        //socket closed after requests are put on wire

                        //send result if front peer not closed yet
                        if (peer_handle == getHandle()) {
                            try (CScopeUQueue sb0 = new CScopeUQueue()) {
                                SendResultIndex(reqIndex, Consts.idUploadEmployees, sb0.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                            }
                        }
                    })) {
                        ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                        break;
                    }
                    //put handler back into pool as soon as possible for reuse, as long as socket connection is not closed yet
                    CYourServer.Master.Unlock(handler);
                } while (false);
            }
        }
    }
}
