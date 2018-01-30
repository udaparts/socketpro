
import SPA.*;
import SPA.UDB.*;
import SPA.ServerSide.*;
import SPA.ClientSide.*;
import java.util.concurrent.*;

public class CYourPeerOne extends CCacheBasePeer {

    private void QueryPaymentMaxMinAvgs(CUQueue q, long reqIndex) {
        CMaxMinAvg pmma = new CMaxMinAvg();
        String filter = q.LoadString();
        String sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.length() > 0) {
            sql += (" WHERE " + filter);
        }
        CSqlite handler = CYourServer.Slave.SeekByQueue();
        if (handler == null) {
            CUQueue sb = CScopeUQueue.Lock();
            sb.Save((int) -1).Save("No connection to a slave database").Save(pmma);
            int ret = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, sb);
            CScopeUQueue.Unlock(sb);
            return;
        }
        long peer_handle = getHandle();
        boolean ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
            //send result if front peer not closed yet
            if (peer_handle == getHandle()) {
                CUQueue sb = CScopeUQueue.Lock();
                sb.Save(r).Save(err).Save(pmma);
                int res = SendResultIndex(reqIndex, Consts.idQueryMaxMinAvgs, sb);
                CScopeUQueue.Unlock(sb);
            }
        }, (h, vData) -> {
            pmma.Max = Double.parseDouble(vData.get(0).toString());
            pmma.Min = Double.parseDouble(vData.get(1).toString());
            pmma.Avg = Double.parseDouble(vData.get(2).toString());
        });
        assert ok; //always be true if pool has local queue for request backup
    }

    private void UploadEmployees(CUQueue q, long reqIndex) {
        int ret;
        Pair<Integer, String> p = new Pair<>(0, "");
        CDBVariantArray vData = new CDBVariantArray();
        vData.LoadFrom(q);
        CLongArray vId = new CLongArray();
        CUQueue sb = CScopeUQueue.Lock();
        if (vData.isEmpty()) {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) 0).Save("").Save(vId));
        } else if (vData.size() % 3 != 0) {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -1).Save("Data array size is wrong").Save(vId));
        } else {
            do {
                CSqlite handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
                if (handler == null) {
                    ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -2).Save("No connection to a master database").Save(vId));
                    break;
                }
                CClientSocket cs = handler.getAttachedClientSocket();
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
                        CUQueue sb0 = CScopeUQueue.Lock();
                        SendResultIndex(reqIndex, Consts.idUploadEmployees, sb0.Save(p.first).Save(p.second).Save(vId));
                        CScopeUQueue.Unlock(sb0);
                    }
                }, (h, canceled) -> {
                    //socket closed after requests are put on wire

                    //send result if front peer not closed yet
                    if (peer_handle == getHandle()) {
                        CUQueue sb0 = CScopeUQueue.Lock();
                        SendResultIndex(reqIndex, Consts.idUploadEmployees, sb0.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                        CScopeUQueue.Unlock(sb0);
                    }
                })) {
                    ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save(cs.getErrorCode()).Save(cs.getErrorMsg()).Save(vId));
                    break;
                }
                //put handler back into pool as soon as possible for reuse, as long as socket connection is not closed yet
                CYourServer.Master.Unlock(handler);
            } while (false);
        }
        CScopeUQueue.Unlock(sb);
    }

    //manual retry for better fault tolerance
    /*
    private void UploadEmployees(CUQueue q, long reqIndex) {
        int ret;
        Pair<Integer, String> p = new Pair<>(0, "");
        CDBVariantArray vData = new CDBVariantArray();
        vData.LoadFrom(q);
        CLongArray vId = new CLongArray();
        CUQueue sb = CScopeUQueue.Lock();
        if (vData.isEmpty()) {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) 0).Save("").Save(vId));
        } else if (vData.size() % 3 != 0) {
            ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -1).Save("Data array size is wrong").Save(vId));
        } else {
            int redo;
            do {
                redo = 0;
                sb.SetSize(0);
                //use master for insert, update and delete
                CSqlite handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
                if (handler == null) {
                    ret = SendResultIndex(reqIndex, Consts.idUploadEmployees, sb.Save((int) -2).Save("No connection to a master database").Save(vId));
                    break;
                }
                ++redo;
                do {
                    if (!handler.BeginTrans()) {
                        break;
                    }
                    if (!handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) {
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
                        break;
                    }
                    long peer_handle = getHandle();
                    if (handler.EndTrans(tagRollbackPlan.rpRollbackErrorAll, (h, res, errMsg) -> {
                        if (res != 0 && p.first == 0) {
                            p.first = res;
                            p.second = errMsg;
                        }
                        //send result if front peer not closed yet
                        if (peer_handle == getHandle()) {
                            CUQueue sb0 = CScopeUQueue.Lock();
                            SendResultIndex(reqIndex, Consts.idUploadEmployees, sb0.Save(p.first).Save(p.second).Save(vId));
                            CScopeUQueue.Unlock(sb0);
                        }
                    }, (h, canceled) -> {
                        //socket closed after requests are put on wire

                        //retry if front peer not closed yet
                        if (peer_handle == getHandle()) {
                            CUQueue sb0 = CScopeUQueue.Lock();
                            //repack original request data and retry
                            sb0.Save(vData);
                            UploadEmployees(sb0, reqIndex); //this will not cause recursive stack-overflow exeption
                            CScopeUQueue.Unlock(sb0);
                        }
                    })) {
                        redo = 0; //disable redo once request is put on wire
                        //put handler back into pool as soon as possible for reuse, as long as socket connection is not closed yet
                        CYourServer.Master.Unlock(handler);
                    } else {
                        //socket closed when sending
                    }
                } while (false);
            } while (redo > 0);
        }
        CScopeUQueue.Unlock(sb);
    }
    */
    
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
        CRentalDateTimes myDates = new CRentalDateTimes();
        String sql = "SELECT rental_id,rental_date,return_date,last_update FROM rental where rental_id=" + rental_id;
        CSqlite handler = CYourServer.Slave.SeekByQueue();
        if (handler == null) {
            CUQueue sb = CScopeUQueue.Lock();
            sb.Save(myDates).Save((int) -1).Save("No connection to a slave database");
            int ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, sb);
            CScopeUQueue.Unlock(sb);
            return;
        }
        long peer_handle = getHandle();
        boolean ok = handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
            //send result if front peer not closed yet
            if (peer_handle == getHandle()) {
                CUQueue sb = CScopeUQueue.Lock();
                sb.Save(myDates).Save(r).Save(err);
                int ret = SendResultIndex(reqIndex, Consts.idGetRentalDateTimes, sb);
                CScopeUQueue.Unlock(sb);
            }
        }, (h, vData) -> {
            myDates.rental_id = (long) vData.get(0);
            myDates.Rental = (java.sql.Timestamp) vData.get(1);
            myDates.Return = (java.sql.Timestamp) vData.get(2);
            myDates.LastUpdate = (java.sql.Timestamp) vData.get(3);
        });
        assert ok; //always be true if pool has local queue for request backup
    }

    @Override
    @RequestAttr(RequestID = DB_CONSTS.idGetCachedTables, SlowRequest = true) //true -- slow request
    protected CachedTableResult GetCachedTables(String defaultDb, int flags, long index) {
        CachedTableResult res = this.new CachedTableResult();
        do {
            CConfig config = CConfig.getConfig();
            if (config.m_vFrontCachedTable.isEmpty()) {
                break;
            }
            if ((flags & DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) == DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES) {
                if (!getPush().Subscribe(DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID)) {
                    res.errMsg = "Failed in subscribing for table events"; //warning message
                }
            }
            String sql = "";
            for (String s : config.m_vFrontCachedTable) {
                if (sql.length() != 0) {
                    sql += ";";
                }
                sql += "SELECT * FROM " + s;
            }
            CSqlite handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
            if (handler == null) {
                res.res = -1;
                res.errMsg = "No connection to a master database";
                break;
            }
            res.ms = handler.getDBManagementSystem();
            UFuture<Integer> f = new UFuture<>();
            if (!handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
                res.errMsg = err;
                res.res = r;
                f.set(0);
            }, (h, vData) -> {
                SendRows(vData);
            }, (h) -> {
                SendMeta(h.getColumnInfo(), index);
            }, true, true, (h, canceled) -> {
                res.res = -2;
                res.errMsg = canceled ? "Request canceled" : "Socket closed";
                f.set(-2);
            })) {
                res.res = handler.getAttachedClientSocket().getErrorCode();
                res.errMsg = handler.getAttachedClientSocket().getErrorMsg();
                break;
            }
            CYourServer.Master.Unlock(handler);//put back locked handler and its socket back into pool for reuse as soon as possible
            try {
                int ret = f.get(25000, TimeUnit.MILLISECONDS);
            } catch (InterruptedException | TimeoutException | ExecutionException err) {
                res.res = -3;
                res.errMsg = err.getMessage();
            }
        } while (false);
        return res;
    }
}
