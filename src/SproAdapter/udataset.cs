
using System;
using System.Collections.Generic;

namespace SocketProAdapter
{
    public class CDataSet
    {
        public const uint INVALID_VALUE = uint.MaxValue;

        protected object m_cs = new object();
        private List<KeyValuePair<string, System.Data.DataTable>> m_ds = new List<KeyValuePair<string, System.Data.DataTable>>();
        private string m_strIp = "";
        private string m_strHostName = "";
        private string m_strUpdater = "";
        private UDB.tagManagementSystem m_ms = UDB.tagManagementSystem.msUnknown;
        private bool m_bDBNameCaseSensitive = false;
        private bool m_bTableNameCaseSensitive = false;
        private bool m_bFieldNameCaseSensitive = false;
        private bool m_bDataCaseSensitive = false;

        public uint GetRowCount(string dbName, string tblName)
        {
            if (dbName == null)
                dbName = "";
            if (tblName == null)
                tblName = "";
            lock (m_cs)
            {
                foreach (KeyValuePair<string, System.Data.DataTable> p in m_ds)
                {
                    bool equal = (m_bDBNameCaseSensitive ? (string.Compare(dbName, p.Key) == 0) : (string.Compare(dbName, p.Key, StringComparison.OrdinalIgnoreCase) == 0));
                    if (!equal)
                        continue;
                    equal = (m_bTableNameCaseSensitive ? (string.Compare(tblName, p.Value.TableName) == 0) : (string.Compare(tblName, p.Value.TableName, StringComparison.OrdinalIgnoreCase) == 0));
                    if (!equal)
                        continue;
                    return (uint)p.Value.Rows.Count;
                }
            }
            return INVALID_VALUE;
        }

        public uint GetColumnCount(string dbName, string tblName)
        {
            if (dbName == null)
                dbName = "";
            if (tblName == null)
                tblName = "";
            lock (m_cs)
            {
                foreach (KeyValuePair<string, System.Data.DataTable> p in m_ds)
                {
                    bool equal = (m_bDBNameCaseSensitive ? (string.Compare(dbName, p.Key) == 0) : (string.Compare(dbName, p.Key, StringComparison.OrdinalIgnoreCase) == 0));
                    if (!equal)
                        continue;
                    equal = (m_bTableNameCaseSensitive ? (string.Compare(tblName, p.Value.TableName) == 0) : (string.Compare(tblName, p.Value.TableName, StringComparison.OrdinalIgnoreCase) == 0));
                    if (!equal)
                        continue;
                    return (uint)p.Value.Columns.Count;
                }
            }
            return INVALID_VALUE;
        }

        public void Set(string strIp, UDB.tagManagementSystem ms)
        {
            if (strIp == null)
                strIp = "";
            lock (m_cs)
            {
                m_strIp = strIp;
                m_ms = ms;
            }
        }

        public void Swap(CDataSet tc)
        {
            if (tc == null)
                return;
            lock (m_cs)
            {
                List<KeyValuePair<string, System.Data.DataTable>> temp = m_ds;
                m_ds = tc.m_ds;
                tc.m_ds = temp;
                string s = m_strIp;
                m_strIp = tc.m_strIp;
                tc.m_strIp = s;
                s = m_strHostName;
                m_strHostName = tc.m_strHostName;
                tc.m_strHostName = s;
                s = m_strUpdater;
                m_strUpdater = tc.m_strUpdater;
                tc.m_strUpdater = s;
                UDB.tagManagementSystem ms = m_ms;
                m_ms = tc.m_ms;
                tc.m_ms = ms;
            }
        }

        public List<KeyValuePair<string, string>> DBTablePair
        {
            get
            {
                List<KeyValuePair<string, string>> v = new List<KeyValuePair<string, string>>();
                lock (m_cs)
                {
                    foreach (KeyValuePair<string, System.Data.DataTable> p in m_ds)
                    {
                        v.Add(new KeyValuePair<string, string>(p.Key, p.Value.TableName));
                    }
                    return v;
                }
            }
        }

        public string DBServerIp
        {
            get
            {
                lock (m_cs)
                {
                    return m_strIp;
                }
            }
        }

        public string DBServerName
        {
            get
            {
                lock (m_cs)
                {
                    return m_strHostName;
                }
            }
            set
            {
                lock (m_cs)
                {
                    m_strHostName = value;
                    if (m_strHostName == null)
                        m_strHostName = "";
                }
            }
        }

        public string Updater
        {
            get
            {
                lock (m_cs)
                {
                    return m_strUpdater;
                }
            }
            set
            {
                lock (m_cs)
                {
                    m_strUpdater = value;
                    if (m_strUpdater == null)
                        m_strUpdater = "";
                }
            }
        }

        public void Empty()
        {
            lock (m_cs)
            {
                m_ds = m_ds = new List<KeyValuePair<string, System.Data.DataTable>>();
            }
        }

        public bool IsEmpty
        {
            get
            {
                lock (m_cs)
                {
                    return (m_ds.Count == 0);
                }
            }
        }

        public UDB.tagManagementSystem DBManagementSystem
        {
            get
            {
                lock (m_cs)
                {
                    return m_ms;
                }
            }
        }

        public bool TableNameCaseSensitive
        {
            get
            {
                lock (m_cs)
                {
                    return m_bTableNameCaseSensitive;
                }
            }

            set
            {
                lock (m_cs)
                {
                    m_bTableNameCaseSensitive = value;
                }
            }
        }

        public bool FieldNameCaseSensitive
        {
            get
            {
                lock (m_cs)
                {
                    return m_bFieldNameCaseSensitive;
                }
            }

            set
            {
                lock (m_cs)
                {
                    m_bFieldNameCaseSensitive = value;
                }
            }
        }

        public bool DataCaseSensitive
        {
            get
            {
                lock (m_cs)
                {
                    return m_bDataCaseSensitive;
                }
            }

            set
            {
                lock (m_cs)
                {
                    m_bDataCaseSensitive = value;
                }
            }
        }

        public bool DBNameCaseSensitive
        {
            get
            {
                lock (m_cs)
                {
                    return m_bDBNameCaseSensitive;
                }
            }

            set
            {
                lock (m_cs)
                {
                    m_bDBNameCaseSensitive = value;
                }
            }
        }
    }
}
