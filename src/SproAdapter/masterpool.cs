using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    public class CMasterPool<THandler, TDataSet> : CMasterSlaveBase<THandler>
        where THandler : ClientSide.CCachedBaseHandler, new()
        where TDataSet : CDataSet, new()
    {
        private bool m_bMidTier;
        public bool MidTier
        {
            get
            {
                return m_bMidTier;
            }
        }

        public TDataSet Cache
        {
            get
            {
                return m_MasterCache;
            }
        }

        public CMasterPool(string defaultDB, bool midTier, uint recvTimeout)
            : base(defaultDB, recvTimeout)
        {
            m_bMidTier = midTier;
        }

        public CMasterPool(string defaultDB, bool midTier)
            : base(defaultDB)
        {
            m_bMidTier = midTier;
        }

        public CMasterPool(string defaultDB)
            : base(defaultDB)
        {
            m_bMidTier = false;
        }

        private TDataSet m_MasterCache = new TDataSet();

        protected TDataSet m_cache = new TDataSet();
        protected UDB.CDBColumnInfoArray m_meta = new UDB.CDBColumnInfoArray();

        private THandler m_hander = null;

        protected override void OnSocketPoolEvent(ClientSide.tagSocketPoolEvent spe, THandler handler)
        {
            if (spe == ClientSide.tagSocketPoolEvent.speUSocketCreated)
            {
                if (handler == AsyncHandlers[0])
                {
                    m_hander = handler;
                    handler.AttachedClientSocket.Push.OnPublish += new ClientSide.DOnPublish(Push_OnPublish);
                }
            }
            else if (spe == ClientSide.tagSocketPoolEvent.speConnected && handler.AttachedClientSocket.ErrorCode == 0)
            {
                if (handler == AsyncHandlers[0])
                {
#if WINCE
#else
                    if (m_bMidTier)
                    {
                        object vtMessage = null;
                        ServerSide.CSocketProServer.PushManager.Publish(vtMessage, UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID);
                    }
#endif
                    m_cache.Updater = "";
                    m_cache.Empty();
                    SetInitialCache();
                }
                else
                {
                    handler.GetCachedTables(DefaultDBName, null, null, null, (uint)0);
                }
            }
            base.OnSocketPoolEvent(spe, handler);
        }

        void Push_OnPublish(ClientSide.CClientSocket sender, ClientSide.CMessageSender messageSender, uint[] group, object msg)
        {
            if (group[0] == UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID)
            {
#if WINCE
#else
                if (m_bMidTier)
                {
                    ServerSide.CSocketProServer.PushManager.Publish(msg, UDB.DB_CONSTS.CACHE_UPDATE_CHAT_GROUP_ID);
                }
#endif
                SetInitialCache();
                return;
            }
#if WINCE
#else
            if (m_bMidTier)
            {
                //push message onto front clients which may be interested in the message
                ServerSide.CSocketProServer.PushManager.Publish(msg, UDB.DB_CONSTS.STREAMING_SQL_CHAT_GROUP_ID);
            }
#endif
            //vData[0] == event type; vData[1] == host; vData[2] = database user; vData[3] == db name; vData[4] == table name
            object[] vData = (object[])msg;
            UDB.tagUpdateEvent eventType = (UDB.tagUpdateEvent)((int)vData[0]);
            if (m_MasterCache.DBServerName == null || m_MasterCache.DBServerName.Length == 0)
            {
                if (vData[1] is sbyte[])
                {
                    m_MasterCache.DBServerName = CUQueue.ToString((sbyte[])vData[1]);
                }
                else if (vData[1] is string)
                {
                    m_MasterCache.DBServerName = (string)vData[1];
                }
            }

            if (vData[2] is sbyte[])
            {
                m_MasterCache.Updater = CUQueue.ToString((sbyte[])vData[2]);
            }
            else if (vData[2] is string)
            {
                m_MasterCache.Updater = (string)vData[2];
            }
            else
            {
                m_MasterCache.Updater = "";
            }

            string dbName = "";
            if (vData[3] is sbyte[])
            {
                dbName = CUQueue.ToString((sbyte[])vData[3]);
            }
            else if (vData[3] is string)
            {
                dbName = (string)vData[3];
            }
            string tblName = "";
            if (vData[4] is sbyte[])
            {
                tblName = CUQueue.ToString((sbyte[])vData[4]);
            }
            else if (vData[4] is string)
            {
                tblName = (string)vData[4];
            }
            uint ret = 0;
            switch (eventType)
            {
                case UDB.tagUpdateEvent.ueDelete:
                    {
                        List<Object> v = new List<object>();
                        for (int n = 5; n < vData.Length; ++n)
                        {
                            v.Add(vData[n]);
                        }
                        ret = m_MasterCache.DeleteARow(dbName, tblName, v.ToArray());
                    }
                    break;
                case UDB.tagUpdateEvent.ueInsert:
                    {
                        List<Object> v = new List<object>();
                        for (int n = 5; n < vData.Length; ++n)
                        {
                            v.Add(vData[n]);
                        }
                        ret = m_MasterCache.AddRows(dbName, tblName, v);
                    }
                    break;
                case UDB.tagUpdateEvent.ueUpdate:
                    {
                        List<Object> v = new List<object>();
                        for (int n = 5; n < vData.Length; ++n)
                        {
                            v.Add(vData[n]);
                        }
                        ret = m_MasterCache.UpdateARow(dbName, tblName, v.ToArray());
                    }
                    break;
                default:
                    break;
            }
        }

        void SetInitialCache()
        {
            //open default database and subscribe for table update events (update, delete and insert) by setting flag ClientSide.CAsyncDBHandler.ENABLE_TABLE_UPDATE_MESSAGES
            bool ok = m_hander.GetCachedTables(DefaultDBName, (res, errMsg) =>
            {
                uint port;
                string ip = m_hander.AttachedClientSocket.GetPeerName(out port);
                ip += ":";
                ip += port;
                m_cache.Set(ip, m_hander.DBManagementSystem);
                m_cache.DBServerName = m_hander.AttachedClientSocket.ConnectionContext.Host;
                if (res == 0)
                {
                    m_MasterCache.Swap(m_cache); //exchange between master Cache and this m_cache
                    m_cache.Set(ip, m_hander.DBManagementSystem);
                }
            }, (vData) =>
            {
                m_cache.AddRows(m_meta[0].DBPath, m_meta[0].TablePath, vData);
            }, (meta) =>
            {
                m_cache.AddEmptyRowset(meta);
                m_meta = meta;
            }, UDB.DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
        }

        public class CSlavePool : CMasterSlaveBase<THandler>
        {
            public CSlavePool(string defaultDb, uint recvTimeout)
                : base(defaultDb, recvTimeout)
            {
            }
            public CSlavePool(string defaultDb)
                : base(defaultDb)
            {
            }
        }
    }
}
