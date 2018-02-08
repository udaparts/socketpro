package SPA;

import SPA.ClientSide.*;
import SPA.UDB.DB_CONSTS;

public class CSqlMasterPool<THandler extends CAsyncDBHandler> extends CMasterSlaveBase<THandler> {

    private boolean m_bMidTier = false;
    private CDataSet Cache = new CDataSet();
    protected CDataSet m_cache = new CDataSet();
    protected final SPA.UDB.CDBColumnInfoArray m_meta = new SPA.UDB.CDBColumnInfoArray();
    protected THandler m_hander = null;
    private final Class<THandler> m_impl;

    public class CSlavePool extends CMasterSlaveBase<THandler> {

        public CSlavePool(String defaultDB, int recvTimeout) {
            super(m_impl, defaultDB, recvTimeout);
        }

        public CSlavePool(String defaultDB) {
            super(m_impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        }

        @Override
        protected void OnSocketPoolEvent(tagSocketPoolEvent spe, THandler handler) {
            switch (spe) {
                case speConnected:
                    handler.Open(getDefaultDBName(), null); //open a session to backend database by default 
                    break;
                default:
                    break;
            }
            super.OnSocketPoolEvent(spe, handler);
        }
    }

    public CDataSet getCache() {
        return Cache;
    }

    public CSqlMasterPool(Class<THandler> impl, String defaultDB, boolean midTier, int recvTimeout) {
        super(impl, defaultDB, recvTimeout);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CSqlMasterPool(Class<THandler> impl, String defaultDB, boolean midTier) {
        super(impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CSqlMasterPool(Class<THandler> impl, String defaultDB) {
        super(impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_impl = impl;
    }

    void SetInitialCache() {
        //open default database and subscribe for table update events (update, delete and insert) by setting flag ClientSide.CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES
        boolean ok = m_hander.Open(getDefaultDBName(), new CAsyncDBHandler.DResult() {
            @Override
            public void invoke(CAsyncDBHandler h, int res, String errMsg) {
                m_cache.setUpdater("");
                m_cache.Empty();
                m_cache.setDBServerName(h.getAttachedClientSocket().getConnectionContext().Host);
                SPA.RefObject<Integer> port = new SPA.RefObject<>(0);
                String ip = m_hander.getAttachedClientSocket().GetPeerName(port);
                ip += ":";
                ip += port.Value;
                m_cache.Set(ip, m_hander.getDBManagementSystem());
            }
        }, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);

        //bring all cached table data into m_cache first for initial cache, and exchange it with Cache if there is no error
        ok = m_hander.Execute("", new CAsyncDBHandler.DExecuteResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                if (res == 0) {
                    Cache.Swap(m_cache); //exchange between master Cache and this m_cache
                }
            }
        }, new CAsyncDBHandler.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, SPA.UDB.CDBVariantArray vData) {
                SPA.UDB.CDBColumnInfoArray meta = dbHandler.getColumnInfo();
                m_cache.AddRows(meta.get(0).DBPath, meta.get(0).TablePath, vData);
            }
        }, new CAsyncDBHandler.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler h) {
                m_cache.AddEmptyRowset(h.getColumnInfo());
            }
        });
    }

    @Override
    protected void OnSocketPoolEvent(tagSocketPoolEvent spe, THandler handler) {
        if (spe == tagSocketPoolEvent.speUSocketCreated) {
            if (handler == getAsyncHandlers()[0]) {
                m_hander = handler;
                handler.getAttachedClientSocket().getPush().OnPublish = new DOnPublish() {
                    @Override
                    public void invoke(CClientSocket sender, CMessageSender messageSender, int[] group, Object msg) {
                        if (group[0] == DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID) {
                            if (m_bMidTier) {
                                SPA.ServerSide.CSocketProServer.PushManager.Publish(msg, DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID);
                            }
                            SetInitialCache();
                            return;
                        }
                        if (m_bMidTier) {
                            //push message onto front clients which may be interested in the message
                            SPA.ServerSide.CSocketProServer.PushManager.Publish(msg, DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
                        }
                        //vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
                        Object[] vData = (Object[]) msg;
                        SPA.UDB.tagUpdateEvent eventType = SPA.UDB.tagUpdateEvent.forValue((int) vData[0]);
                        if (Cache.getDBServerName() == null || Cache.getDBServerName().length() == 0) {
                            if (vData[1] instanceof byte[]) {
                                Cache.setDBServerName(CUQueue.ToString((byte[]) vData[1]));
                            } else if (vData[1] instanceof String) {
                                Cache.setDBServerName((String) vData[1]);
                            }
                        }
                        if (vData[2] instanceof byte[]) {
                            Cache.setUpdater(CUQueue.ToString((byte[]) vData[2]));
                        } else if (vData[2] instanceof String) {
                            Cache.setUpdater((String) vData[2]);
                        } else {
                            Cache.setUpdater("");
                        }
                        String dbName = "";
                        if (vData[3] instanceof byte[]) {
                            dbName = CUQueue.ToString((byte[]) vData[3]);
                        } else if (vData[3] instanceof String) {
                            dbName = (String) vData[3];
                        }
                        String tblName = "";
                        if (vData[4] instanceof byte[]) {
                            tblName = CUQueue.ToString((byte[]) vData[4]);
                        } else if (vData[4] instanceof String) {
                            tblName = (String) vData[4];
                        }
                        int ret = 0;
                        java.util.ArrayList<Object> v = new java.util.ArrayList<>();
                        switch (eventType) {
                            case ueDelete: {
                                for (int n = 5; n < vData.length; ++n) {
                                    v.add(vData[n]);
                                }
                                if (v.size() == Cache.GetColumnCount(dbName, tblName)) {
                                    ret = Cache.DeleteARow(dbName, tblName, v.toArray());
                                } else if (v.size() == 1) {
                                    ret = Cache.DeleteARow(dbName, tblName, v.get(0));
                                } else if (v.size() == 2) {
                                    ret = Cache.DeleteARow(dbName, tblName, v.get(0), v.get(1));
                                }
                            }
                            break;
                            case ueInsert: {
                                for (int n = 5; n < vData.length; ++n) {
                                    v.add(vData[n]);
                                }
                                ret = Cache.AddRows(dbName, tblName, v);
                            }
                            break;
                            case ueUpdate: {
                                for (int n = 5; n < vData.length; ++n) {
                                    v.add(vData[n]);
                                }
                                ret = Cache.UpdateARow(dbName, tblName, v);
                            }
                            break;
                            default:
                                break;
                        }
                    }
                };
            }
        } else if (spe == tagSocketPoolEvent.speConnected && handler.getAttachedClientSocket().getErrorCode() == 0) {
            if (handler == getAsyncHandlers()[0]) {
                if (m_bMidTier) {
                    Object vtMessage = null;
                    int[] Groups = {DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID};
                    SPA.ServerSide.CSocketProServer.PushManager.Publish(vtMessage, Groups);
                }
                SetInitialCache();
            } else {
                handler.Open(getDefaultDBName(), null);
            }
        }
        super.OnSocketPoolEvent(spe, handler);
    }
}
