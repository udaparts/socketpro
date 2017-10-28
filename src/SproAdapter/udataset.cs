
using System;
using System.Collections.Generic;
using System.Data;

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

        public void AddEmptyRowset(UDB.CDBColumnInfoArray meta)
        {
            if (meta == null || meta.Count == 0)
                return;
            if (meta[0].DBPath == null || meta[0].DBPath.Length == 0 || meta[0].TablePath == null || meta[0].TablePath.Length == 0)
                throw new Exception("The first column meta must contain database name and table name");
            System.Data.DataTable tbl = ClientSide.CAsyncDBHandler.MakeDataTable(meta, meta[0].TablePath);
            if (tbl.PrimaryKey == null || tbl.PrimaryKey.Length == 0)
                throw new Exception("Column meta information doesn't contain any key");
            lock (m_cs)
            {
                tbl.CaseSensitive = m_bDataCaseSensitive;
                m_ds.Add(new KeyValuePair<string, System.Data.DataTable>(meta[0].DBPath, tbl));
            }
        }

        public DataRow[] Find(string dbName, string tblName, string filterExpression, string sort)
        {
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return null;
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
                    return p.Value.Select(filterExpression, sort);
                }
            }
            return null;
        }

        public DataColumnCollection GetColumnMeta(string dbName, string tblName)
        {
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return null;
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
                    return p.Value.Columns;
                }
            }
            return null;
        }

        public DataRow[] Find(string dbName, string tblName, string filterExpression)
        {
            return Find(dbName, tblName, filterExpression, "");
        }

        public DataRow[] Find(string dbName, string tblName)
        {
            return Find(dbName, tblName, "", "");
        }

        public uint AddRows(string dbName, string tblName, List<object> vData)
        {
            if (vData == null || vData.Count == 0)
                return 0;
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return INVALID_VALUE;
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
                    ClientSide.CAsyncDBHandler.AppendRowDataIntoDataTable(vData, p.Value);
                    return (uint)(vData.Count / p.Value.Columns.Count);
                }
            }
            return INVALID_VALUE;
        }

        public uint AddRows(string dbName, string tblName, object[] vData)
        {
            if (vData == null || vData.Length == 0)
                return 0;
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return INVALID_VALUE;
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
                    ClientSide.CAsyncDBHandler.AppendRowDataIntoDataTable(vData, p.Value);
                    return (uint)(vData.Length / p.Value.Columns.Count);
                }
            }
            return INVALID_VALUE;
        }

        public uint UpdateARow(string dbName, string tblName, object[] oldnewValues)
        {
            if (oldnewValues == null || oldnewValues.Length == 0)
                return 0;
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return INVALID_VALUE;
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
                    DataColumn[] keyColumns = p.Value.PrimaryKey;
                    if (oldnewValues.Length != p.Value.Columns.Count * 2)
                        throw new Exception("Wrong data number for update");
                    List<KeyValuePair<DataColumn, object>> vKeyVal = new List<KeyValuePair<DataColumn, object>>();
                    foreach (DataColumn dc in keyColumns)
                    {
                        KeyValuePair<DataColumn, object> kv = new KeyValuePair<DataColumn, object>(dc, oldnewValues[dc.Ordinal * 2]);
                        vKeyVal.Add(kv);
                    }
                    string filter;
                    DataRow row = FindRowByKeys(p.Value, vKeyVal, UDB.tagUpdateEvent.ueDelete, out filter);
                    if (row != null)
                    {
                        object[] newValues = new object[p.Value.Columns.Count];
                        for (int n = 0; n < p.Value.Columns.Count; ++n)
                        {
                            newValues[n] = oldnewValues[n * 2 + 1];
                        }
                        row.ItemArray = newValues;
                        return 1;
                    }
                    return 0;
                }
            }
            return INVALID_VALUE;
        }

        public uint DeleteARow(string dbName, string tblName, object[] keys)
        {
            if (keys == null || keys.Length == 0)
                return 0;
            if (dbName == null || dbName.Length == 0 || tblName == null || tblName.Length == 0)
                return INVALID_VALUE;
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
                    DataColumn[] keyColumns = p.Value.PrimaryKey;
                    if (keyColumns == null || keyColumns.Length != keys.Length)
                        throw new Exception("Wrong key number");
                    List<KeyValuePair<DataColumn, object>> vKeyVal = new List<KeyValuePair<DataColumn, object>>();
                    int index = 0;
                    foreach (DataColumn dc in keyColumns)
                    {
                        KeyValuePair<DataColumn, object> kv = new KeyValuePair<DataColumn, object>(dc, keys[index]);
                        vKeyVal.Add(kv);
                        ++index;
                    }
                    string filter;
                    DataRow row = FindRowByKeys(p.Value, vKeyVal, UDB.tagUpdateEvent.ueDelete, out filter);
                    if (row != null)
                    {
                        p.Value.Rows.Remove(row);
                        return 1;
                    }
                    return 0;
                }
            }
            return INVALID_VALUE;
        }

        private static DataRow FindRowByKeys(DataTable dt, List<KeyValuePair<DataColumn, object>> vKey, UDB.tagUpdateEvent ue, out string filter)
        {
            DataRow[] rows = null;
            filter = "";
            foreach (var kv in vKey)
            {
                if (filter.Length > 0)
                    filter += " AND ";
                filter += ("`" + kv.Key.ColumnName + "`=");
                if (kv.Value is long || kv.Value is decimal || kv.Value is double)
                    filter += kv.Value.ToString();
                else if (kv.Value is string)
                    filter += ("'" + kv.Value.ToString() + "'");
                else
                    throw new Exception("Other key column not supported");
            }
            if (ue != UDB.tagUpdateEvent.ueInsert)
                rows = dt.Select(filter);
            if (rows != null && rows.Length == 1)
                return rows[0];
            else if (rows != null && rows.Length > 1)
                throw new Exception("Multiple rows found beyond our expectation");
            return null;
        }

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

        public DataColumn[] FindKeys(string dbName, string tblName)
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
                    return p.Value.PrimaryKey;
                }
            }
            return null;
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
