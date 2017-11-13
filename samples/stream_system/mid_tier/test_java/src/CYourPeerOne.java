
import SPA.ServerSide.*;
import SPA.ClientSide.*;
import SPA.*;
import SPA.UDB.*;

public class CYourPeerOne extends CCacheBasePeer {

    void QueryPaymentMaxMinAvgs(CUQueue q) {
        CMaxMinAvg pmma = new CMaxMinAvg();
        long index = q.LoadLong();
        String filter = q.LoadString();
        String sql = "SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment";
        if (filter != null && filter.length() > 0) {
            sql += (" WHERE " + filter);
        }
        int redo = 0;
        CUQueue sb = CScopeUQueue.Lock();
        do {
            sb.SetSize(0);
            CSqlite handler = CYourServer.Slave.Seek();
            if (handler == null) {
                sb.Save(index).Save((int) -1).Save("No connection to a slave database").Save(pmma);
                int ret = SendResult(Consts.idQueryMaxMinAvgs, sb);
                CScopeUQueue.Unlock(sb);
                return;
            }
            ++redo;
            long peer_handle = getHandle();
            if (handler.Execute(sql, (h, r, err, affected, fail_ok, vtId) -> {
                //send result if front peer not closed yet
                if (peer_handle == getHandle()) {
                    CUQueue sb0 = CScopeUQueue.Lock();
                    sb0.Save(index).Save(r).Save(err).Save(pmma);
                    int res = SendResult(Consts.idQueryMaxMinAvgs, sb0);
                    CScopeUQueue.Unlock(sb0);
                }
            }, (h, vData) -> {
                pmma.Max = Double.parseDouble(vData.get(0).toString());
                pmma.Min = Double.parseDouble(vData.get(1).toString());
                pmma.Avg = Double.parseDouble(vData.get(2).toString());
            }, (h) -> {
            }, true, true, () -> {
                //retry if front peer not closed yet
                if (peer_handle == getHandle()) {
                    CUQueue sb0 = CScopeUQueue.Lock();
                    sb0.Save(index).Save(filter);
                    QueryPaymentMaxMinAvgs(sb0);
                    CScopeUQueue.Unlock(sb0);
                }
            })) {
                redo = 0; //disable redo once request is put on wire
            }
        } while (redo > 0);
        CScopeUQueue.Unlock(sb);
    }

    void UploadEmployees(CUQueue q) {
        int ret;
        final Pair<Integer, String> p = new Pair<>(0, "");
        long index = q.LoadLong();
        CDBVariantArray vData = new CDBVariantArray();
        vData.LoadFrom(q);
        CLongArray vId = new CLongArray();
        CUQueue sb = CScopeUQueue.Lock();
        if (vData.isEmpty()) {
            ret = SendResult(Consts.idUploadEmployees, sb.Save(index).Save((int) 0).Save("").Save(vId));
        } else if (vData.size() % 3 != 0) {
            ret = SendResult(Consts.idUploadEmployees, sb.Save(index).Save((int) -1).Save("Data array size is wrong").Save(vId));
        } else {
            int redo;
            do {
                redo = 0;
                sb.SetSize(0);
                //use master for insert, update and delete
                CSqlite handler = CYourServer.Master.Lock(); //use Lock and Unlock to avoid SQL stream overlap on a session within a multi-thread environment
                if (handler == null) {
                    ret = SendResult(Consts.idUploadEmployees, sb.Save(index).Save((int) -2).Save("No connection to a master database").Save(vId));
                    break;
                }
                ++redo;
                do {
                    if (!handler.Prepare("INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)")) {
                        break;
                    }
                    if (!handler.BeginTrans()) {
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
                            SendResult(Consts.idUploadEmployees, sb0.Save(index).Save(p.first).Save(p.second).Save(vId));
                            CScopeUQueue.Unlock(sb0);
                        }
                    }, () -> {
                        //socket closed after sending

                        //retry if front peer not closed yet
                        if (peer_handle == getHandle()) {
                            CUQueue sb0 = CScopeUQueue.Lock();
                            //repack original request data and retry
                            sb0.Save(index).Save(vData);
                            UploadEmployees(sb0); //this will not cause recursive stack-overflow exeption
                            CScopeUQueue.Unlock(sb0);
                        }
                    })) {
                        redo = 0; //disable redo once request is put on wire
                        CYourServer.Master.Unlock(handler);
                    } else {
                        //socket closed when sending
                    }
                } while (false);
            } while (redo > 0);
        }
        CScopeUQueue.Unlock(sb);
    }

    @Override
    protected void OnFastRequestArrive(short reqId, int len) {
        if (reqId == Consts.idQueryMaxMinAvgs) {
            QueryPaymentMaxMinAvgs(getUQueue());
        } else if (reqId == Consts.idUploadEmployees) {
            UploadEmployees(getUQueue());
        }
    }

    @RequestAttr(RequestID = Consts.idGetMasterSlaveConnectedSessions)
    CScopeUQueue GetMasterSlaveConnectedSessions(long index) {
        CScopeUQueue sb = new CScopeUQueue();
        sb.Save(index).Save(CYourServer.Master.getConnectedSockets()).Save(CYourServer.Slave.getConnectedSockets());
        return sb;
    }

    @Override
    protected CachedTableResult GetCachedTables(String defaultDb, int flags, boolean rowset, long index) {
        CachedTableResult res = this.new CachedTableResult();
        //CSqlMasterPool<CSqlite> Master = CYourServer.Master;
        do {
            if (!rowset) {
                res.errMsg = "Client side doesn't ask for rowsets";
                break;
            }
            CConfig config = CConfig.getConfig();
            if (config.m_vFrontCachedTable.isEmpty()) {
                break;
            }
            if ((flags & CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) == CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES) {
                if (!getPush().Subscribe(CAsyncDBHandler.CACHE_UPDATE_CHAT_GROUP_ID, CAsyncDBHandler.STREAMING_SQL_CHAT_GROUP_ID)) {
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

        } while (false);

        return res;
    }

}
