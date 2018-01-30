using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    namespace ClientSide
    {
        using UDB;
        public class CCachedBaseHandler : CAsyncServiceHandler
        {
            protected CCachedBaseHandler(uint ServiceId)
                : base(ServiceId)
            {
            }

            protected const uint ONE_MEGA_BYTES = 0x100000;
            protected const uint BLOB_LENGTH_NOT_AVAILABLE = 0xffffffe0;

            public delegate void DResult(int res, string errMsg);
            public delegate void DRowsetHeader(CDBColumnInfoArray meta);
            public delegate void DRows(CDBVariantArray vData);

            protected Dictionary<ulong, KeyValuePair<DRowsetHeader, DRows>> m_mapRowset = new Dictionary<ulong, KeyValuePair<DRowsetHeader, DRows>>();
            protected CUQueue m_Blob = new CUQueue();
            protected CDBVariantArray m_vData = new CDBVariantArray();
            protected object m_csCache = new object();
            protected ulong m_indexRowset = 0;

            private UDB.tagManagementSystem m_ms = tagManagementSystem.msUnknown;
            public UDB.tagManagementSystem DBManagementSystem
            {
                get
                {
                    lock (m_csCache)
                    {
                        return m_ms;
                    }
                }
            }

            public override uint CleanCallbacks()
            {
                lock (m_csCache)
                {
                    m_mapRowset.Clear();
                }
                return base.CleanCallbacks();
            }

            protected override void OnMergeTo(CAsyncServiceHandler to)
            {
                CCachedBaseHandler dbTo = (CCachedBaseHandler)to;
                lock (dbTo.m_csCache)
                {
                    lock (m_csCache)
                    {
                        foreach (ulong callIndex in m_mapRowset.Keys)
                        {
                            dbTo.m_mapRowset.Add(callIndex, m_mapRowset[callIndex]);
                        }
                        m_mapRowset.Clear();
                    }
                }
            }

            protected override void OnResultReturned(ushort reqId, CUQueue mc)
            {
                switch (reqId)
                {
                    case DB_CONSTS.idRowsetHeader:
                        {
                            m_Blob.SetSize(0);
                            if (m_Blob.MaxBufferSize > ONE_MEGA_BYTES)
                            {
                                m_Blob.Realloc(ONE_MEGA_BYTES);
                            }
                            CDBColumnInfoArray vColInfo;
                            mc.Load(out vColInfo).Load(out m_indexRowset);
                            KeyValuePair<DRowsetHeader, DRows> p = new KeyValuePair<DRowsetHeader, DRows>();
                            lock (m_csCache)
                            {
                                m_vData.Clear();
                                if (m_mapRowset.ContainsKey(m_indexRowset))
                                    p = m_mapRowset[m_indexRowset];
                            }
                            if (p.Key != null)
                                p.Key.Invoke(vColInfo);
                        }
                        break;
                    case DB_CONSTS.idBeginRows:
                        m_Blob.SetSize(0);
                        m_vData.Clear();
                        break;
                    case DB_CONSTS.idTransferring:
                        while (mc.GetSize() > 0)
                        {
                            object vt;
                            mc.Load(out vt);
                            m_vData.Add(vt);
                        }
                        break;
                    case DB_CONSTS.idEndRows:
                        if (mc.GetSize() > 0 || m_vData.Count > 0)
                        {
                            object vt;
                            while (mc.GetSize() > 0)
                            {
                                mc.Load(out vt);
                                m_vData.Add(vt);
                            }
                            DRows row = null;
                            lock (m_csCache)
                            {
                                if (m_mapRowset.ContainsKey(m_indexRowset))
                                {
                                    row = m_mapRowset[m_indexRowset].Value;
                                }
                            }
                            if (row != null)
                            {
                                row.Invoke(m_vData);
                            }
                        }
                        m_vData.Clear();
                        break;
                    case DB_CONSTS.idStartBLOB:
                        {
                            m_Blob.SetSize(0);
                            uint len;
                            mc.Load(out len);
                            if (len != uint.MaxValue && len > m_Blob.MaxBufferSize)
                            {
                                m_Blob.Realloc(len);
                            }
                            m_Blob.Push(mc.IntenalBuffer, mc.HeadPosition, mc.GetSize());
                            mc.SetSize(0);
                        }
                        break;
                    case DB_CONSTS.idChunk:
                        m_Blob.Push(mc.IntenalBuffer, mc.GetSize());
                        mc.SetSize(0);
                        break;
                    case DB_CONSTS.idEndBLOB:
                        if (mc.GetSize() > 0 || m_Blob.GetSize() > 0)
                        {
                            m_Blob.Push(mc.IntenalBuffer, mc.GetSize());
                            mc.SetSize(0);
                            unsafe
                            {
                                fixed (byte* p = m_Blob.IntenalBuffer)
                                {
                                    uint* len = (uint*)(p + m_Blob.HeadPosition + sizeof(ushort));
                                    if (*len >= BLOB_LENGTH_NOT_AVAILABLE)
                                    {
                                        //length should be reset if BLOB length not available from server side at beginning
                                        *len = (m_Blob.GetSize() - sizeof(ushort) - sizeof(uint));
                                    }
                                }
                            }
                            object vt;
                            m_Blob.Load(out vt);
                            m_vData.Add(vt);
                        }
                        break;
                    default:
                        base.OnResultReturned(reqId, mc);
                        break;
                }
            }

            public bool GetCachedTables(string defaultDb, DResult handler, DRows row, DRowsetHeader rh)
            {
                return GetCachedTables(defaultDb, handler, row, rh, DB_CONSTS.ENABLE_TABLE_UPDATE_MESSAGES);
            }

            public virtual bool GetCachedTables(string defaultDb, DResult handler, DRows row, DRowsetHeader rh, uint flags)
            {
                ulong index = GetCallIndex();
                lock (m_csCache)
                {
                    //don't make m_csCache locked across calling SendRequest, which may lead to cross-SendRequest dead-lock
                    //in case a client asynchronously sends lots of requests without use of client side queue.
                    m_mapRowset[index] = new KeyValuePair<DRowsetHeader, DRows>(rh, row);
                }
                if (!SendRequest(DB_CONSTS.idGetCachedTables, defaultDb, flags, index, (ar) =>
                {
                    int res, dbMS;
                    string errMsg;
                    ar.Load(out dbMS).Load(out res).Load(out errMsg);
                    lock (m_csCache)
                    {
                        m_ms = (UDB.tagManagementSystem)dbMS;
                        m_mapRowset.Remove(index);
                    }
                    if (handler != null)
                    {
                        handler(res, errMsg);
                    }
                }))
                {
                    lock (m_csCache)
                    {
                        m_mapRowset.Remove(index);
                    }
                    return false;
                }
                return true;
            }
        }
    }
}
