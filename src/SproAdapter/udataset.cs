
using System;

namespace SocketProAdapter
{
    public class CDataSet
    {
        public const ulong INVALID_VALUE = ulong.MaxValue;

        protected object m_cs = new object();
        System.Data.DataSet m_ds = new System.Data.DataSet("udata_set");

        private string m_strIp = "";
        private string m_strHostName = "";
        private string m_strUpdater = "";
        private UDB.tagManagementSystem m_ms = UDB.tagManagementSystem.msUnknown;
        private bool m_bDBNameCaseSensitive = false;
        private bool m_bTableNameCaseSensitive = false;
        private bool m_bFieldNameCaseSensitive = false;
        private bool m_bDataCaseSensitive = false;

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
        }

        public void Empty()
        {
            lock (m_cs)
            {
                m_ds = new System.Data.DataSet("udata_set");
            }
        }

        public bool IsEmpty
        {
            get
            {
                lock (m_cs)
                {
                    return (m_ds.Tables.Count == 0);
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
