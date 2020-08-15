package SPA;

import SPA.ClientSide.*;
import SPA.UDB.DB_CONSTS;

public class CMasterPool<THandler extends CCachedBaseHandler> extends CMasterSlaveBase<THandler> {

    private boolean m_bMidTier = false;
    private CDataSet Cache = new CDataSet();
    protected CDataSet m_cache = new CDataSet();
    protected SPA.UDB.CDBColumnInfoArray m_meta = new SPA.UDB.CDBColumnInfoArray();
    protected THandler m_hander = null;
    private final Class<THandler> m_impl;

    public class CSlavePool extends CMasterSlaveBase<THandler> {

        public CSlavePool(String defaultDB, int recvTimeout, boolean autoConn, int connTimeout, int svsId) {
            super(m_impl, defaultDB, recvTimeout, autoConn, connTimeout, svsId);
        }

        public CSlavePool(String defaultDB, int recvTimeout, boolean autoConn, int connTimeout) {
            super(m_impl, defaultDB, recvTimeout, autoConn, connTimeout);
        }

        public CSlavePool(String defaultDB, int recvTimeout, boolean autoConn) {
            super(m_impl, defaultDB, recvTimeout, autoConn);
        }

        public CSlavePool(String defaultDB, int recvTimeout) {
            super(m_impl, defaultDB, recvTimeout);
        }

        public CSlavePool(String defaultDB) {
            super(m_impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        }
    }

    public CDataSet getCache() {
        return Cache;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB, boolean midTier, int recvTimeout, boolean autoConn, int connTimeout, int svsId) {
        super(impl, defaultDB, recvTimeout, autoConn, connTimeout, svsId);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB, boolean midTier, int recvTimeout, boolean autoConn, int connTimeout) {
        super(impl, defaultDB, recvTimeout, autoConn, connTimeout);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB, boolean midTier, int recvTimeout, boolean autoConn) {
        super(impl, defaultDB, recvTimeout, autoConn);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB, boolean midTier, int recvTimeout) {
        super(impl, defaultDB, recvTimeout);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB, boolean midTier) {
        super(impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_bMidTier = midTier;
        m_impl = impl;
    }

    public CMasterPool(Class<THandler> impl, String defaultDB) {
        super(impl, defaultDB, CClientSocket.DEFAULT_RECV_TIMEOUT);
        m_impl = impl;
    }

    void SetInitialCache() {
        m_cache.setDBServerName("");
        m_cache.setUpdater("");
        m_cache.Empty();
        //open default database and subscribe for table update events (update, delete and insert) by setting flag ClientSide.CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES
        boolean ok = m_hander.GetCachedTables(getDefaultDBName(), new CCachedBaseHandler.DResult() {
            @Override
            public void invoke(int res, String errMsg) {
                SPA.RefObject<Integer> port = new SPA.RefObject<>(0);
                String ip = m_hander.getSocket().GetPeerName(port);
                ip += ":";
                ip += port.Value;
                m_cache.Set(ip, m_hander.getDBManagementSystem());
                m_cache.setDBServerName(m_hander.getSocket().getConnectionContext().Host);
                if (res == 0) {
                    Cache.Swap(m_cache); //exchange between master Cache and this m_cache
                    m_cache.Set(ip, m_hander.getDBManagementSystem());
                }
            }
        }, new CCachedBaseHandler.DRows() {
            @Override
            public void invoke(SPA.UDB.CDBVariantArray vData) {
                m_cache.AddRows(m_meta.get(0).DBPath, m_meta.get(0).TablePath, vData);
            }
        }, new CCachedBaseHandler.DRowsetHeader() {
            @Override
            public void invoke(SPA.UDB.CDBColumnInfoArray meta) {
                m_cache.AddEmptyRowset(meta);
                m_meta = meta;
            }
        }, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
    }

    @Override
    protected void OnSocketPoolEvent(tagSocketPoolEvent spe, THandler handler) {
        if (spe == tagSocketPoolEvent.speUSocketCreated) {
            if (handler == getAsyncHandlers()[0]) {
                m_hander = handler;
                handler.getSocket().getPush().OnPublish = new DOnPublish() {
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
        } else if (spe == tagSocketPoolEvent.speConnected && handler.getSocket().getErrorCode() == 0) {
            if (handler == getAsyncHandlers()[0]) {
                if (m_bMidTier) {
                    Object vtMessage = null;
                    int[] Groups = {DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID};
                    SPA.ServerSide.CSocketProServer.PushManager.Publish(vtMessage, Groups);
                }
                SetInitialCache();
            } else {
                handler.GetCachedTables(getDefaultDBName(), null, null, null, 0);
            }
        }
        super.OnSocketPoolEvent(spe, handler);
    }
}
