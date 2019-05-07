using System;
using System.Data;
using System.Collections;

namespace SocketProAdapter
{
    internal enum tagDataTypeSupported : short
    {
        dtUnknown = 0,
        dtBoolean,
        dtByte,
        dtBytes,
        dtChar,
        dtChars,
        dtInt16,
        dtUInt16,
        dtInt32,
        dtUInt32,
        dtInt64,
        dtUInt64,
        dtFloat,
        dtDouble,
        dtDecimal,
        dtGuid,
        dtDateTime,
        dtString,

        /// <summary>
        /// An object
        /// </summary>
        dtValue,

        /// <summary>
        /// An array of objects
        /// </summary>
        dtValues,

        /// <summary>
        /// A DBNull
        /// </summary>
        dtNull,

        dtTimeSpan,

        /// <summary>
        /// User defined data type or complex data type. All of data converted into string
        /// </summary>
        dtUDT,

        dtDateTimeOffset = 200,
    }

    internal enum tagColumnBit : int
    {
        cbAllowDBNull = 1,
        cbIsAutoIncrement = 2,
        cbIsReadOnly = 4,
        cbIsUnique = 8,
        cbIsKey = 16,
        cbIsLong = 32,
        cbIsRowVersion = 64,
    }

    public class CAdoSerializationHelper
    {
        public const uint DEFAULT_BATCH_SIZE = 10240;
        public const ushort idDataTableHeaderArrive = 0x7FFF;
        public const ushort idDataTableRowsArrive = 0x7FFE;
        public const ushort idDataSetHeaderArrive = 0x7FFD;
        public const ushort idDataReaderHeaderArrive = 0x7FFC;
        public const ushort idEndDataReader = 0x7FFB;
        public const ushort idEndDataSet = 0x7FFA;
        public const ushort idDataReaderRecordsArrive = 0x7FF9;
        public const ushort idEndDataTable = 0x7FF8;
        public const ushort idDmlTriggerMessage = 0x7FF7;
        public const ushort idDdlTriggerMessage = 0x7FF6;
        public const ushort idDbEventTriggerMessage = 0x7FF5;
        public const ushort idRecordsetName = 0x7FF4;
        private bool m_bLoadingDataTable = false;
        private uint m_nBatchSize = 0;
        private CUQueue m_qBit = new CUQueue();
        private CUQueue m_qTemp = new CUQueue();
        private byte[] m_Buffer = null;
        private tagDataTypeSupported[] m_dts = null;
        private DataTable m_dt = new DataTable();
        private DataTable m_dtBackup = null;
        private uint m_nAffected = 0;
        private DataSet m_ds = new DataSet();
        private bool m_bDataSet = false;
        private bool m_bDataReader = false;

        internal CAdoSerializationHelper()
        {
        }

        private uint PopTableColNamesOnly(CUQueue UQueue, ref DataColumn[] dcs)
        {
            int n;
            int count;
            int ordinal;
            string tableName;
            uint start = UQueue.GetSize();
            UQueue.Load(out count);
            dcs = new DataColumn[count];
            for (n = 0; n < count; ++n)
            {
                UQueue.Load(out tableName);
                UQueue.Load(out ordinal);
                dcs[n] = CurrentDataSet.Tables[tableName].Columns[ordinal];
            }
            return (start - UQueue.GetSize());
        }

        private void PushTableColNamesOnly(CUQueue UQueue, DataColumn[] dcs)
        {
            UQueue.Save(dcs.Length);
            foreach (DataColumn dc in dcs)
            {
                UQueue.Save(dc.Table.TableName);
                UQueue.Save(dc.Ordinal);
            }
        }

        private void AddAAU()
        {
            int n;
            if (m_dt == null || m_dtBackup == null)
                return;
            if (m_dt.Columns.Count != m_dtBackup.Columns.Count)
                throw new InvalidOperationException("Bad operation for table column mis-matching!");
            m_dt.EndLoadData();
            int nCount = m_dt.Columns.Count;
            for (n = 0; n < nCount; n++)
            {
                DataColumn dc = m_dt.Columns[n];
                dc.AllowDBNull = m_dtBackup.Columns[n].AllowDBNull;
                dc.AutoIncrement = m_dtBackup.Columns[n].AutoIncrement;
            }

        }

        private void RemoveAAU()
        {
            if (m_dt == null)
                return;
            int n;
            int nSize = m_dt.Columns.Count;
            for (n = 0; n < nSize; n++)
            {
                DataColumn p = m_dt.Columns[n];

                //The property AllowDBNull has an important role on performance
                p.AllowDBNull = true;
                p.AutoIncrement = false;
            }
            m_dt.BeginLoadData();
        }

        /// <summary>
        /// Call the methods EndLoadData and BeginLoadData one time for loaded records during fetching records.
        /// This method is usually called at client side for reduction of latency and fast displaying beginning records.
        /// </summary>
        public void FinalizeRecords()
        {
            if (m_dt != null)
            {
                m_dt.EndLoadData();
                m_dt.BeginLoadData();
            }
        }


        /// <summary>
        /// The size of a batch of records in byte.
        /// </summary>
        public uint BatchSize
        {
            get
            {
                return m_nBatchSize;
            }
        }

        /// <summary>
        /// The number of rows, records or tables affected.
        /// </summary>
        public uint Affected
        {
            get
            {
                return m_nAffected;
            }
        }

        /// <summary>
        /// The datatable that is being fetched or has been just transferred.
        /// </summary>
        public DataTable CurrentDataTable
        {
            get
            {
                return m_dt;
            }

			//For mono, you may use the below setting plus FinalizeRecords to avoid memory leak
			set 
            {
				m_dt = value;		
			}
        }

        /// <summary>
        /// The DataSet that is being fetched or has been just transferred.
        /// </summary>
        public DataSet CurrentDataSet
        {
            get
            {
                return m_ds;
            }
        }

        public bool LoadingDataTable
        {
            get
            {
                return m_bLoadingDataTable;
            }
        }

        public bool LoadingDataSet
        {
            get
            {
                return m_bDataSet;
            }
        }

        public bool LoadingDataReader
        {
            get
            {
                return m_bDataReader;
            }
        }

        internal void Load(ushort sRequestID, CUQueue UQueue)
        {
            if (UQueue == null)
                throw new ArgumentException("Invalid input parameter UQueue");
            switch (sRequestID)
            {
                case idDataSetHeaderArrive:
                    if (UQueue.GetSize() > 0)
                    {
                        m_bDataSet = true;
                        if (m_dt != null && m_dt.Columns.Count > 0)
                            m_dt = new DataTable();
                        LoadDataSetHeader(UQueue);
                    }
                    break;
                case idDataReaderHeaderArrive:
                    if (UQueue.GetSize() > 0)
                    {
                        if (m_ds != null && m_ds.Tables.Count > 0)
                            m_ds = new DataSet();
                        m_bDataReader = true;
                        LoadDataReaderHeader(UQueue);
                        m_dtBackup = m_dt.Clone();

                        //for better performance
                        RemoveAAU();
                    }
                    break;
                case idDataTableHeaderArrive:
                    if (UQueue.GetSize() > 0)
                    {
                        if (!m_bDataSet && m_ds != null && m_ds.Tables.Count > 0)
                            m_ds = new DataSet();
                        m_bLoadingDataTable = true;
                        LoadDataTableHeader(UQueue);
                        m_dtBackup = m_dt.Clone();
                        //for better performance
                        RemoveAAU();

                        if (m_bDataSet)
                            m_ds.Tables.Add(m_dt);
                    }
                    break;
                case idDataTableRowsArrive:
                case idDataReaderRecordsArrive:
                    if (UQueue.GetSize() > 0)
                        LoadRows(UQueue);
                    break;
                case idEndDataReader:
                    if (m_bDataReader)
                    {
                        if (m_dt != null)
                            m_dt.AcceptChanges();
                        AddAAU(); //reset datatable
                        m_bDataReader = false;
                    }
                    break;
                case idEndDataTable:
                    if (m_bLoadingDataTable)
                    {
                        AddAAU(); //reset datatable
                        m_bLoadingDataTable = false;
                    }
                    break;
                case idEndDataSet:
                    if (m_bDataSet)
                    {
                        DataRelationCollection drc = LoadDataSetRelations(UQueue);
                        if (drc != null)
                        {
                            int n;
                            int nSize = drc.Count;
                            for (n = 0; n < nSize; n++)
                            {
                                DataRelation dr = drc[n];
                                m_ds.Relations.Add(dr);
                            }
                        }
                        m_bDataSet = false;
                    }
                    break;
                default:
                    break;
            }
        }

        private DataRelationCollection LoadDataSetRelations(CUQueue UQueue)
        {
            DataRelationCollection drc = null;
            if (UQueue != null && UQueue.GetSize() > 0)
            {
                drc = CurrentDataSet.Relations;
                drc.Clear();
                Pop(UQueue, ref drc);
            }
            return drc;
        }

        private int LoadRows(CUQueue UQueue)
        {
            if (m_bLoadingDataTable)
                return LoadDataTableRows(UQueue);
            return LoadDataReaderRecords(UQueue);
        }

        private int LoadDataTableRows(CUQueue UQueue)
        {
            if (UQueue.GetSize() == 0)
                return 0;
            int nSize = 0;
            DataTable dt = m_dt;
            DataRowState drs = DataRowState.Detached;
            //dt.BeginLoadData();
            while (UQueue != null && UQueue.GetSize() > 0)
            {
                DataRow dr = dt.NewRow();
                Pop(UQueue, ref dr, ref drs);
                dt.Rows.Add(dr);
                switch (drs)
                {
                    case DataRowState.Added:
                        break;
                    case DataRowState.Unchanged:
                        dr.AcceptChanges();
                        break;
                    case DataRowState.Modified:
                        dr.AcceptChanges();
                        {
                            int n;
                            object obj;
                            int nCount = dt.Columns.Count;
                            for (n = 0; n < nCount; ++n)
                            {
                                if (!dt.Columns[n].ReadOnly)
                                {
                                    obj = dr[0];
                                    dr[0] = obj;
                                    break;
                                }
                            }
                        }
                        break;
                    case DataRowState.Deleted:
                        dr.AcceptChanges();
                        dr.Delete();
                        break;
                    default: //DataRowState.Detached
                        throw new InvalidOperationException("Wrong DataRow state");
                }
                ++nSize;
            }
            //dt.EndLoadData();
            return nSize;
        }
        private int LoadDataReaderRecords(CUQueue UQueue)
        {
            if (UQueue.GetSize() == 0)
                return 0;
            DataTable dt = m_dt;
            int nSize = 0;
            object[] aData = null;
            //dt.BeginLoadData();
            while (UQueue != null && UQueue.GetSize() > 0)
            {
                PopDataRecord(UQueue, ref aData);
                dt.Rows.Add(aData);
                ++nSize;
            }
            //dt.EndLoadData();
            return nSize;
        }
        static private tagDataTypeSupported GetDT(Type type)
        {
            if (type == null)
                return tagDataTypeSupported.dtUnknown;
            return GetDT(type.FullName);
        }
        static private Type GetType(tagDataTypeSupported dt)
        {
            switch (dt)
            {
                case tagDataTypeSupported.dtBoolean:
                    return typeof(bool);
                case tagDataTypeSupported.dtByte:
                    return typeof(byte);
                case tagDataTypeSupported.dtBytes:
                    return typeof(byte[]);
                case tagDataTypeSupported.dtChar:
                    return typeof(char);
                case tagDataTypeSupported.dtChars:
                    return typeof(char[]);
                case tagDataTypeSupported.dtDateTime:
                    return typeof(DateTime);
                case tagDataTypeSupported.dtDecimal:
                    return typeof(decimal);
                case tagDataTypeSupported.dtDouble:
                    return typeof(double);
                case tagDataTypeSupported.dtFloat:
                    return typeof(float);
                case tagDataTypeSupported.dtGuid:
                    return typeof(Guid);
                case tagDataTypeSupported.dtInt16:
                    return typeof(short);
                case tagDataTypeSupported.dtInt32:
                    return typeof(int);
                case tagDataTypeSupported.dtInt64:
                    return typeof(long);
                case tagDataTypeSupported.dtUInt16:
                    return typeof(ushort);
                case tagDataTypeSupported.dtUInt32:
                    return typeof(uint);
                case tagDataTypeSupported.dtUInt64:
                    return typeof(ulong);
                case tagDataTypeSupported.dtString:
                    return typeof(string);
                case tagDataTypeSupported.dtValue:
                    return typeof(object);
                case tagDataTypeSupported.dtValues:
                    return typeof(object[]);
                case tagDataTypeSupported.dtNull:
                    return typeof(DBNull);
                case tagDataTypeSupported.dtTimeSpan:
                    return typeof(TimeSpan);
                case tagDataTypeSupported.dtUDT:
                    return typeof(string);
                default:
                    throw new InvalidOperationException("Unsupported data type");
            }
        }
        static private tagDataTypeSupported GetDT(string strType)
        {
            switch (strType)
            {
                case "System.Boolean":
                    return tagDataTypeSupported.dtBoolean;
                case "System.Byte":
                    return tagDataTypeSupported.dtByte;
                case "System.Byte[]":
                    return tagDataTypeSupported.dtBytes;
                case "System.String":
                    return tagDataTypeSupported.dtString;
                case "System.Single":
                    return tagDataTypeSupported.dtFloat;
                case "System.DateTime":
                    return tagDataTypeSupported.dtDateTime;
                case "System.Double":
                    return tagDataTypeSupported.dtDouble;
                case "System.Decimal":
                    return tagDataTypeSupported.dtDecimal;
                case "System.Char":
                    return tagDataTypeSupported.dtChar;
                case "System.Char[]":
                case "System.UInt16[]":
                    return tagDataTypeSupported.dtChars;
                case "System.Guid":
                    return tagDataTypeSupported.dtGuid;
                case "System.Int16":
                    return tagDataTypeSupported.dtInt16;
                case "System.Int32":
                    return tagDataTypeSupported.dtInt32;
                case "System.Int64":
                    return tagDataTypeSupported.dtInt64;
                case "System.UInt16":
                    return tagDataTypeSupported.dtUInt16;
                case "System.UInt32":
                    return tagDataTypeSupported.dtUInt32;
                case "System.UInt64":
                    return tagDataTypeSupported.dtUInt64;
                case "System.Object":
                    return tagDataTypeSupported.dtValue;
                case "System.Object[]":
                    return tagDataTypeSupported.dtValues;
                case "System.DBNull":
                    return tagDataTypeSupported.dtNull;
                case "System.TimeSpan":
                    return tagDataTypeSupported.dtTimeSpan;
                default:
                    return tagDataTypeSupported.dtUDT;
                //throw new InvalidOperationException("Unsupported data type");
            }
        }

        private uint PopDataRecord(CUQueue UQueue, ref object[] aData)
        {
            int n;
            byte bData = 0;
            byte bOne = 1;
            if (m_dts == null)
                throw new InvalidOperationException("DataTable header is not de-serialized yet");
            uint nSize = UQueue.GetSize();
            int nLen = m_dts.Length;
            if (aData == null || aData.Length != nLen)
                aData = new object[nLen];
            uint nBits = (uint)(m_dts.Length / 8 + (((m_dts.Length % 8) != 0) ? 1 : 0));
            byte[] aBit = new byte[nBits];
            UQueue.Pop(out aBit, nBits);
            for (n = 0; n < nLen; n++)
            {
                if ((n % 8) == 0)
                    bData = aBit[n / 8];
                if ((bData & (bOne << (byte)(n % 8))) != 0)
                {
                    aData[n] = DBNull.Value;
                }
                else
                {
                    switch (m_dts[n])
                    {
                        case tagDataTypeSupported.dtBoolean:
                            {
                                bool myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtByte:
                            {
                                byte myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtChar:
                            {
                                char myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDateTime:
                            {
                                DateTime myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDecimal:
                            {
                                decimal myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDouble:
                            {
                                double myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtFloat:
                            {
                                float myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtGuid:
                            {
                                Guid myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt16:
                            {
                                short myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt32:
                            {
                                int myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt64:
                            {
                                long myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtUDT:
                        case tagDataTypeSupported.dtString:
                            {
                                string myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtBytes:
                            {
                                byte[] buffer;
                                uint nBytes;
                                UQueue.Load(out nBytes);
                                UQueue.Pop(out buffer, nBytes);
                                aData[n] = buffer;
                            }
                            break;
                        case tagDataTypeSupported.dtUInt64:
                        case tagDataTypeSupported.dtUInt32:
                        case tagDataTypeSupported.dtUInt16:
                        case tagDataTypeSupported.dtValue:
                        case tagDataTypeSupported.dtValues:
                        case tagDataTypeSupported.dtTimeSpan:
                            UQueue.Load(out aData[n]);
                            break;
                        default:
                            throw new InvalidOperationException("Unsupported data type for serialization");
                    }
                }
            }
            return (nSize - UQueue.GetSize());
        }
        internal void Push(CUQueue UQueue, IDataReader dr)
        {
            int n;
            bool bNull;
            if (dr == null)
                throw new ArgumentNullException("Datarow object can't be null");
            if (m_dts == null)
                throw new ArgumentNullException("DataTable header is not serialized yet");
            if (m_dts.Length != dr.FieldCount)
                throw new InvalidOperationException("The size of the input data type array does not match the size of data row");
            byte b = 0;
            byte bOne = 1;
            m_qBit.SetSize(0);
            m_qTemp.SetSize(0);
            int nLen = m_dts.Length;
            for (n = 0; n < nLen; n++)
            {
                bNull = dr.IsDBNull(n);
                if (bNull)
                {
                    b += (byte)(bOne << (byte)(n % 8));
                }
                if ((n % 8) == 7)
                {
                    m_qBit.Save(b);
                    b = 0;
                }
                if (bNull)
                    continue;
                switch (m_dts[n])
                {
                    case tagDataTypeSupported.dtBoolean:
                        m_qTemp.Save(dr.GetBoolean(n));
                        break;
                    case tagDataTypeSupported.dtByte:
                        m_qTemp.Save(dr.GetByte(n));
                        break;
                    case tagDataTypeSupported.dtChar:
                        m_qTemp.Save(dr.GetChar(n));
                        break;
                    case tagDataTypeSupported.dtDateTime:
                        m_qTemp.Save(dr.GetDateTime(n));
                        break;
                    case tagDataTypeSupported.dtDecimal:
                        m_qTemp.Save(dr.GetDecimal(n));
                        break;
                    case tagDataTypeSupported.dtDouble:
                        m_qTemp.Save(dr.GetDouble(n));
                        break;
                    case tagDataTypeSupported.dtFloat:
                        m_qTemp.Save(dr.GetFloat(n));
                        break;
                    case tagDataTypeSupported.dtGuid:
                        m_qTemp.Save(dr.GetGuid(n));
                        break;
                    case tagDataTypeSupported.dtInt16:
                        m_qTemp.Save(dr.GetInt16(n));
                        break;
                    case tagDataTypeSupported.dtInt32:
                        m_qTemp.Save(dr.GetInt32(n));
                        break;
                    case tagDataTypeSupported.dtInt64:
                        m_qTemp.Save(dr.GetInt64(n));
                        break;
                    case tagDataTypeSupported.dtString:
                        {
                            string str = dr.GetString(n);
                            m_qTemp.Save(str);
                        }
                        break;
                    case tagDataTypeSupported.dtBytes:
                        {
                            uint nBytes = (uint)dr.GetBytes(n, (long)0, null, 0, 0);
                            if (m_Buffer == null || nBytes > m_Buffer.Length)
                                m_Buffer = new byte[nBytes + 1024];
                            dr.GetBytes(n, (long)0, m_Buffer, 0, (int)nBytes);
                            m_qTemp.Save(nBytes);
                            m_qTemp.Push(m_Buffer, nBytes);
                        }
                        break;
                    case tagDataTypeSupported.dtUInt64:
                    case tagDataTypeSupported.dtUInt32:
                    case tagDataTypeSupported.dtUInt16:
                    case tagDataTypeSupported.dtValue:
                    case tagDataTypeSupported.dtValues:
                    case tagDataTypeSupported.dtTimeSpan:
                        {
                            object obj = dr.GetValue(n);
                            m_qTemp.Save(obj, false, false);
                        }
                        break;
                    case tagDataTypeSupported.dtUDT:
                        {
                            object obj = dr.GetValue(n);
                            m_qTemp.Save(obj.ToString());
                        }
                        break;
                    default:
                        throw new InvalidOperationException("Unsupported data type for serialization");
                }
            }
            if ((n % 8) != 0)
                m_qBit.Save(b);
            UQueue.Push(m_qBit.m_bytes, m_qBit.GetSize());
            UQueue.Push(m_qTemp.m_bytes, m_qTemp.GetSize());
        }
        internal void Push(CUQueue UQueue, DataRow dr)
        {
            int n;
            bool bNull;
            if (dr == null)
                throw new ArgumentNullException("Datarow object can't be null");
            if (m_dts == null)
                throw new ArgumentNullException("DataTable header is not serialized yet");
            if (m_dts.Length != dr.ItemArray.Length)
                throw new InvalidOperationException("The size of the input data type array does not match the size of data row");
            byte b = 0;
            byte bOne = 1;
            m_qBit.SetSize(0);
            m_qTemp.SetSize(0);
            bool bDelete = (dr.RowState == DataRowState.Deleted);
            if (bDelete)
                dr.RejectChanges();
            object[] data = dr.ItemArray;
            int nLen = m_dts.Length;
            for (n = 0; n < nLen; n++)
            {
                object myData = data[n];
                bNull = (myData == null || myData.Equals(DBNull.Value));
                if (bNull)
                {
                    b += (byte)(bOne << (byte)(n % 8));
                }
                if ((n % 8) == 7)
                {
                    m_qBit.Save(b);
                    b = 0;
                }
                if (bNull)
                    continue;
                switch (m_dts[n])
                {
                    case tagDataTypeSupported.dtBoolean:
                        m_qTemp.Save((bool)myData);
                        break;
                    case tagDataTypeSupported.dtByte:
                        m_qTemp.Save((byte)myData);
                        break;
                    case tagDataTypeSupported.dtChar:
                        m_qTemp.Save((char)myData);
                        break;
                    case tagDataTypeSupported.dtDateTime:
                        m_qTemp.Save((DateTime)myData);
                        break;
                    case tagDataTypeSupported.dtDecimal:
                        m_qTemp.Save((decimal)myData);
                        break;
                    case tagDataTypeSupported.dtDouble:
                        m_qTemp.Save((double)myData);
                        break;
                    case tagDataTypeSupported.dtFloat:
                        m_qTemp.Save((float)myData);
                        break;
                    case tagDataTypeSupported.dtGuid:
                        m_qTemp.Save((Guid)myData);
                        break;
                    case tagDataTypeSupported.dtUInt16:
                        m_qTemp.Save((ushort)myData);
                        break;
                    case tagDataTypeSupported.dtUInt32:
                        m_qTemp.Save((uint)myData);
                        break;
                    case tagDataTypeSupported.dtUInt64:
                        m_qTemp.Save((ulong)myData);
                        break;
                    case tagDataTypeSupported.dtInt16:
                        m_qTemp.Save((short)myData);
                        break;
                    case tagDataTypeSupported.dtInt32:
                        m_qTemp.Save((int)myData);
                        break;
                    case tagDataTypeSupported.dtInt64:
                        m_qTemp.Save((long)myData);
                        break;
                    case tagDataTypeSupported.dtString:
                        m_qTemp.Save((string)myData);
                        break;
                    case tagDataTypeSupported.dtValue:
                    case tagDataTypeSupported.dtValues:
                    case tagDataTypeSupported.dtChars:
                    case tagDataTypeSupported.dtBytes:
                    case tagDataTypeSupported.dtTimeSpan:
                        m_qTemp.Save(myData, false, false);
                        break;
                    case tagDataTypeSupported.dtUDT:
                        m_qTemp.Save(myData.ToString());
                        break;
                    default:
                        throw new InvalidOperationException("Unsupported data type for serialization");
                }
            }
            if ((n % 8) != 0)
                m_qBit.Save(b);
            UQueue.Push(m_qBit.m_bytes, m_qBit.GetSize());
            UQueue.Push(m_qTemp.m_bytes, m_qTemp.GetSize());
            if (bDelete)
                dr.Delete();
            UQueue.Save((byte)dr.RowState);
            UQueue.Save(dr.HasErrors);
            if (dr.HasErrors)
                UQueue.Save(dr.RowError);
        }
        private uint Pop(CUQueue UQueue, ref DataRow dr, ref DataRowState drs)
        {
            int n;
            bool b;
            string str;
            byte bData = 0;
            byte bOne = 1;
            if (m_dts == null)
                throw new ArgumentNullException("DataTable header is not de-serialized yet");
            uint nSize = UQueue.GetSize();
            if (dr == null)
                throw new ArgumentNullException("Datarow object can't be null");
            int nLen = m_dts.Length;
            if (dr.ItemArray == null || dr.ItemArray.Length != m_dts.Length)
                throw new InvalidOperationException("Wrong data row object");
            object[] aData = new object[nLen];
            uint nBits = (uint)(m_dts.Length / 8 + (((m_dts.Length % 8) != 0) ? 1 : 0));
            byte[] aBit = new byte[nBits];
            UQueue.Pop(out aBit, nBits);
            for (n = 0; n < nLen; n++)
            {
                if ((n % 8) == 0)
                    bData = aBit[n / 8];
                if ((bData & (bOne << (byte)(n % 8))) != 0)
                {
                    aData[n] = DBNull.Value;
                }
                else
                {
                    switch (m_dts[n])
                    {
                        case tagDataTypeSupported.dtBoolean:
                            {
                                bool myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtByte:
                            {
                                byte myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtChar:
                            {
                                char myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDateTime:
                            {
                                DateTime myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDecimal:
                            {
                                decimal myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtDouble:
                            {
                                double myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtFloat:
                            {
                                float myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtGuid:
                            {
                                Guid myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtUInt16:
                            {
                                ushort myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtUInt32:
                            {
                                uint myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtUInt64:
                            {
                                ulong myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt16:
                            {
                                short myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt32:
                            {
                                int myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtInt64:
                            {
                                long myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtUDT:
                        case tagDataTypeSupported.dtString:
                            {
                                string myData;
                                UQueue.Load(out myData);
                                aData[n] = myData;
                            }
                            break;
                        case tagDataTypeSupported.dtValue:
                        case tagDataTypeSupported.dtValues:
                        case tagDataTypeSupported.dtChars:
                        case tagDataTypeSupported.dtBytes:
                        case tagDataTypeSupported.dtTimeSpan:
                            UQueue.Load(out aData[n]);
                            break;
                        default:
                            throw new InvalidOperationException("Unsupported data type for serialization");
                    }
                }
            }
            dr.ItemArray = aData;
            
            UQueue.Load(out bData);
            drs = (DataRowState)bData;
            UQueue.Load(out b);
            if (b)
            {
                UQueue.Load(out str);
                dr.RowError = str;
            }

            return (nSize - UQueue.GetSize());
        }
        internal void PushHeader(CUQueue UQueue, DataTable dt, bool bNeedParentRelations, bool bNeedChildRelations)
        {
            int n;
            int nSize;
            if (dt == null)
                return;
            UQueue.SetSize(0);
            m_dts = new tagDataTypeSupported[dt.Columns.Count];
            UQueue.Save(dt.Rows.Count);
            byte bData = 0;
            /*          if (dt.RemotingFormat == SerializationFormat.Xml)
                          bData += 1;*/
            if (bNeedParentRelations)
                bData += 2;
            if (bNeedChildRelations)
                bData += 4;
            UQueue.Save(bData);
            UQueue.Save(dt.TableName);
            Push(UQueue, dt.Columns);
            for (n = 0; n < dt.Columns.Count; n++)
            {
                m_dts[n] = GetDT(dt.Columns[n].DataType.FullName);
            }
            UQueue.Save(dt.DisplayExpression);
            UQueue.Save(dt.MinimumCapacity);
            UQueue.Save(dt.Namespace);
            UQueue.Save(dt.Prefix);
            nSize = dt.PrimaryKey.Length;
            UQueue.Save(nSize);
            for (n = 0; n < nSize; n++)
            {
                UQueue.Save(dt.PrimaryKey[n].Ordinal);
            }
            if (bNeedParentRelations)
            {
                Push(UQueue, dt.ParentRelations);
            }
            if (bNeedChildRelations)
            {
                Push(UQueue, dt.ChildRelations);
            }
        }
        DataSet LoadDataSetHeader(CUQueue UQueue)
        {
            if (UQueue.GetSize() == 0)
                return null;
            string str = null;
            byte bData = 0;
            DataSet ds = new DataSet();
            m_ds = ds;
            UQueue.Load(out m_nAffected);
            UQueue.Load(out bData);
            ds.CaseSensitive = ((bData & 2) == 2);
            ds.EnforceConstraints = ((bData & 4) == 4);
            UQueue.Load(out str);
            ds.DataSetName = str;
            UQueue.Load(out str);
            ds.Namespace = str;
            UQueue.Load(out str);
            ds.Prefix = str;
            return ds;
        }
        private DataTable LoadDataTableHeader(CUQueue UQueue)
        {
            int n;
            bool bNeedChildRelations;
            bool bNeedbParentRelations;
            if (UQueue.GetSize() == 0)
                return null;
            int nData = 0;
            byte bData = 0;
            string str = null;
            DataTable dt = new DataTable();
            m_dt = dt;
            m_bLoadingDataTable = true;
            UQueue.Load(out m_nAffected);
            UQueue.Load(out bData);
            bNeedChildRelations = ((bData & 2) == 2);
            bNeedbParentRelations = ((bData & 4) == 4);
            DataColumnCollection dcc = dt.Columns;
            UQueue.Load(out str);
            dt.TableName = str;
            Pop(UQueue, ref dcc);
            m_dts = new tagDataTypeSupported[dcc.Count];
            for (n = 0; n < dcc.Count; n++)
            {
                m_dts[n] = GetDT(dcc[n].DataType.FullName);
            }
            UQueue.Load(out str);
            dt.DisplayExpression = str;
            UQueue.Load(out nData);
            dt.MinimumCapacity = nData;
            UQueue.Load(out str);
            dt.Namespace = str;
            UQueue.Load(out str);
            dt.Prefix = str;
            UQueue.Load(out nData);
            DataColumn[] pk = new DataColumn[nData];
            for (n = 0; n < nData; n++)
            {
                UQueue.Load(out nData);
                pk[n] = dt.Columns[nData];
            }
            dt.PrimaryKey = pk;
            if (bNeedbParentRelations)
            {
                DataRelationCollection drc = dt.ParentRelations;
                Pop(UQueue, ref drc);
            }
            if (bNeedChildRelations)
            {
                DataRelationCollection drc = dt.ChildRelations;
                Pop(UQueue, ref drc);
            }
            if (UQueue.GetSize() >= 4)
            {
                UQueue.Load(out m_nBatchSize);
            }
            else
            {
                m_nBatchSize = 0;
            }
            return dt;
        }

        private DataTable LoadDataReaderHeader(CUQueue UQueue)
        {
            int n;
            int nData;
            short sData;
            string str;
            int nFieldCount;
            if (UQueue.GetSize() == 0)
                return null;
            m_dt = new DataTable();
            DataTable dt = m_dt;
            m_bLoadingDataTable = false;
            UQueue.Load(out nFieldCount);
            UQueue.Load(out m_nAffected);
            m_dts = new tagDataTypeSupported[nFieldCount];
            for (n = 0; n < nFieldCount; n++)
            {
                UQueue.Load(out sData);
                m_dts[n] = (tagDataTypeSupported)sData;
            }
            m_qTemp.SetSize(0);
            for (n = 0; n < nFieldCount; n++)
            {
                UQueue.Load(out nData);
                DataColumn dc = new DataColumn();
                dc.DataType = GetType(m_dts[n]);
                dc.AllowDBNull = ((nData & (int)tagColumnBit.cbAllowDBNull) == (int)tagColumnBit.cbAllowDBNull);
                dc.AutoIncrement = ((nData & (int)tagColumnBit.cbIsAutoIncrement) == (int)tagColumnBit.cbIsAutoIncrement);
                dc.ReadOnly = ((nData & (int)tagColumnBit.cbIsReadOnly) == (int)tagColumnBit.cbIsReadOnly);
                dc.Unique = ((nData & (int)tagColumnBit.cbIsUnique) == (int)tagColumnBit.cbIsUnique);
                bool cbIsLong = ((nData & (int)tagColumnBit.cbIsLong) == (int)tagColumnBit.cbIsLong);
                if ((nData & (int)tagColumnBit.cbIsKey) == (int)tagColumnBit.cbIsKey)
                {
                    m_qTemp.Save(n);
                }
                UQueue.Load(out nData);
                if (nData > 0 && !cbIsLong && (m_dts[n] == tagDataTypeSupported.dtString || m_dts[n] == tagDataTypeSupported.dtChars))
                {
                    dc.MaxLength = nData; //ColumnSize
                }
                UQueue.Load(out str);
                dc.ColumnName = str;
                dt.Columns.Add(dc);
            }

            if (m_qTemp.GetSize() > 0)
            {
                int nIndex = 0;
                DataColumn[] dcs = new DataColumn[m_qTemp.GetSize() / 4];
                while (m_qTemp.GetSize() > 0)
                {
                    m_qTemp.Load(out nData);
                    DataColumn dc = dt.Columns[nData];
                    dcs[nIndex] = dc;
                    ++nIndex;
                }
                dt.PrimaryKey = dcs;
            }
            if (UQueue.GetSize() >= 4)
            {
                UQueue.Load(out m_nBatchSize);
            }
            else
            {
                m_nBatchSize = 0;
            }
            return dt;
        }

        private int CountNames(ArrayList al, string strColName)
        {
            int n;
            int nCount = 0;
            int nSize = al.Count;
            for (n = 0; n < nSize; n++)
            {
                string str = (string)al[n];
                if (str != null && string.Compare(str, strColName, true) == 0)
                {
                    ++nCount;
                }
            }
            return nCount;
        }

        internal void PushHeader(CUQueue UQueue, IDataReader dr)
        {
            int n;
            int nCount;
            if (dr == null)
                throw new ArgumentNullException("Must pass in a valid data reader object");
            ArrayList al = new ArrayList();
            UQueue.SetSize(0);
            UQueue.Save(dr.FieldCount);
            UQueue.Save(dr.RecordsAffected);
            DataTable dtSchema = dr.GetSchemaTable();
            int nColumnName = dtSchema.Columns.IndexOf("ColumnName");
            int nColumnSize = dtSchema.Columns.IndexOf("ColumnSize");
            int nDataType = dtSchema.Columns.IndexOf("DataType");
            int nIsLong = dtSchema.Columns.IndexOf("IsLong");
            int nAllowDBNull = dtSchema.Columns.IndexOf("AllowDBNull");
            int nIsReadOnly = dtSchema.Columns.IndexOf("IsReadOnly");
            int nIsRowVersion = dtSchema.Columns.IndexOf("IsRowVersion");
            int nIsUnique = dtSchema.Columns.IndexOf("IsUnique");
            int nIsKey = dtSchema.Columns.IndexOf("IsKey");
            int nIsAutoIncrement = dtSchema.Columns.IndexOf("IsAutoIncrement");
            m_dts = new tagDataTypeSupported[dr.FieldCount];
            for (n = 0; n < dr.FieldCount; n++)
            {
                tagDataTypeSupported dts = GetDT(dr.GetFieldType(n));
                m_dts[n] = dts;
                UQueue.Save((short)dts);
            }
            foreach (DataRow row in dtSchema.Rows)
            {
                int nData = 0;
                string str = null;
                if (nIsAutoIncrement != -1)
                {
                    if (row[nIsAutoIncrement].Equals(true))
                        nData |= (int)tagColumnBit.cbIsAutoIncrement;
                }

                if (nIsKey != -1)
                {
                    if (row[nIsKey].Equals(true))
                        nData |= (int)tagColumnBit.cbIsKey;
                }

                if (nAllowDBNull != -1)
                {
                    if (row[nAllowDBNull].Equals(true))
                        nData |= (int)tagColumnBit.cbAllowDBNull;
                }

                if (nIsReadOnly != -1)
                {
                    if (row[nIsReadOnly].Equals(true))
                        nData |= (int)tagColumnBit.cbIsReadOnly;
                }

                if (nIsRowVersion != -1)
                {
                    if (row[nIsRowVersion].Equals(true))
                        nData |= (int)tagColumnBit.cbIsRowVersion;
                }

                if (nIsUnique != -1)
                {
                    if (row[nIsUnique].Equals(true))
                        nData |= (int)tagColumnBit.cbIsUnique;
                }

                if (nIsLong != -1)
                {
                    if (row[nIsLong].Equals(true))
                        nData |= (int)tagColumnBit.cbIsLong;
                }

                UQueue.Save(nData);

                nData = 0;
                if (nColumnSize != -1)
                {
                    nData = (int)row[nColumnSize];
                }
                UQueue.Save(nData);

                if (nColumnName != -1)
                {
                    str = (string)row[nColumnName];
                    nCount = CountNames(al, str);
                    if (nCount > 0)
                        str += nCount.ToString();
                    al.Add(str);
                }
                UQueue.Save(str);
            }
        }
        internal void PushHeader(CUQueue UQueue, DataSet ds)
        {
            byte bData = 0;
            UQueue.SetSize(0);
            /*          if (ds.RemotingFormat == SerializationFormat.Xml)
                          bData += 1;*/
            if (ds.CaseSensitive)
                bData += 2;
            if (ds.EnforceConstraints)
                bData += 4;
            UQueue.Save(ds.Tables.Count);
            UQueue.Save(bData);
            UQueue.Save(ds.DataSetName);
            UQueue.Save(ds.Namespace);
            UQueue.Save(ds.Prefix);
        }
        private void Push(CUQueue UQueue, DataColumn dc)
        {
            bool bNull = (dc == null);
            UQueue.Save(bNull);
            if (bNull)
                return;
            byte bData = 0;
            if (dc.AllowDBNull)
                bData += (byte)tagColumnBit.cbAllowDBNull;
            if (dc.AutoIncrement)
                bData += (byte)tagColumnBit.cbIsAutoIncrement;
            if (dc.ReadOnly)
                bData += (byte)tagColumnBit.cbIsReadOnly;
            if (dc.Unique)
                bData += (byte)tagColumnBit.cbIsUnique;
            UQueue.Save(bData);
            UQueue.Save(dc.AutoIncrementSeed);
            UQueue.Save(dc.AutoIncrementStep);
            UQueue.Save(dc.Caption);
            UQueue.Save((byte)dc.ColumnMapping);
            UQueue.Save(dc.ColumnName);
            UQueue.Save((short)GetDT(dc.DataType.FullName));
            //           UQueue.Push((byte)dc.DateTimeMode);
            UQueue.Save(dc.DefaultValue, false, false);
            UQueue.Save(dc.Expression);
            //dc.ExtendedProperties ignored. If needed, write your own code
            UQueue.Save(dc.MaxLength);
            UQueue.Save(dc.Namespace);
            UQueue.Save(dc.Prefix);
        }
        private uint Pop(CUQueue UQueue, ref DataColumn dc)
        {
            bool bNull;
            uint nLen = UQueue.GetSize();
            UQueue.Load(out bNull);
            if (bNull)
                dc = null;
            else
            {
                int nData;
                object ob;
                short sData;
                string str;
                long lData;
                byte bData;
                UQueue.Load(out bData);
                if (dc == null)
                    dc = new DataColumn();
                dc.AllowDBNull = ((bData & (int)tagColumnBit.cbAllowDBNull) == (int)tagColumnBit.cbAllowDBNull);
                dc.AutoIncrement = ((bData & (int)tagColumnBit.cbIsAutoIncrement) == (int)tagColumnBit.cbIsAutoIncrement);
                dc.ReadOnly = ((bData & (int)tagColumnBit.cbIsReadOnly) == (int)tagColumnBit.cbIsReadOnly);
                dc.Unique = ((bData & (int)tagColumnBit.cbIsUnique) == (int)tagColumnBit.cbIsUnique);
                UQueue.Load(out lData);
                dc.AutoIncrementSeed = lData;
                UQueue.Load(out lData);
                dc.AutoIncrementStep = lData;
                UQueue.Load(out str);
                dc.Caption = str;
                UQueue.Load(out bData);
                dc.ColumnMapping = (MappingType)bData;
                UQueue.Load(out str);
                dc.ColumnName = str;
                UQueue.Load(out sData);
                dc.DataType = GetType((tagDataTypeSupported)sData);
                /*             UQueue.Pop(out bData);
                               dc.DateTimeMode = (DataSetDateTime)bData;*/
                UQueue.Load(out ob);
                dc.DefaultValue = ob;
                UQueue.Load(out str);
                dc.Expression = str;
                UQueue.Load(out nData);
                dc.MaxLength = nData;
                UQueue.Load(out str);
                dc.Namespace = str;
                UQueue.Load(out str);
                dc.Prefix = str;
            }
            return (nLen - UQueue.GetSize());
        }
        private uint Pop(CUQueue UQueue, ref DataColumnCollection Cols)
        {
            bool bNull = false;
            uint nSize = UQueue.GetSize();
            UQueue.Load(out bNull);
            if (bNull)
                Cols = null;
            else
            {
                int n;
                int nLen;
                DataColumn dc = null;
                Cols.Clear();
                UQueue.Load(out nLen);
                for (n = 0; n < nLen; n++)
                {
                    Pop(UQueue, ref dc);
                    Cols.Add(dc);
                    dc = null;
                }
            }
            return (nSize - UQueue.GetSize());
        }
        private void Push(CUQueue UQueue, DataColumnCollection Cols)
        {
            bool bNull = (Cols == null);
            UQueue.Save(bNull);
            if (!bNull)
            {
                int n;
                int nLen = 0;
                if (Cols != null && Cols.Count > 0)
                    nLen = Cols.Count;
                UQueue.Save(nLen);
                for (n = 0; n < nLen; n++)
                {
                    Push(UQueue, Cols[n]);
                }
            }
        }
        private void Push(CUQueue UQueue, DataColumn[] dcs)
        {
            if (dcs == null)
            {
                UQueue.Save((int)-1);
            }
            else
            {
                UQueue.Save(dcs.Length);
                foreach (DataColumn dc in dcs)
                {
                    Push(UQueue, dc);
                }
            }
        }
        private uint Pop(CUQueue UQueue, ref DataColumn[] dcs)
        {
            int n;
            int nSize;
            uint nLen = UQueue.GetSize();
            UQueue.Load(out nSize);
            if (nSize == -1)
            {
                dcs = null;
            }
            else
            {
                if (dcs == null || dcs.Length != nSize)
                    dcs = new DataColumn[nSize];
                for (n = 0; n < nSize; n++)
                {
                    Pop(UQueue, ref dcs[n]);
                }
            }
            return (nLen - UQueue.GetSize());
        }
        private void Push(CUQueue UQueue, ForeignKeyConstraint fkc)
        {
            bool bNull = (fkc == null);
            UQueue.Save(bNull);
            if (!bNull)
            {
                Push(UQueue, fkc.Columns);
                Push(UQueue, fkc.RelatedColumns);
                UQueue.Save(fkc.ConstraintName);
                UQueue.Save((byte)fkc.DeleteRule);
                UQueue.Save((byte)fkc.AcceptRejectRule);
                UQueue.Save((byte)fkc.UpdateRule);
            }
        }
        private uint Pop(CUQueue UQueue, out ForeignKeyConstraint fkc)
        {
            bool b;
            uint nSize = UQueue.GetSize();
            UQueue.Load(out b);
            if (b) //null
            {
                fkc = null;
            }
            else
            {
                byte bData = 0;
                string str = null;
                DataColumn[] dcsChild = null;
                Pop(UQueue, ref dcsChild);
                DataColumn[] dcsParent = null;
                Pop(UQueue, ref dcsParent);
                fkc = new ForeignKeyConstraint(dcsParent, dcsChild);
                UQueue.Load(out str);
                fkc.ConstraintName = str;
                UQueue.Load(out bData);
                fkc.AcceptRejectRule = (AcceptRejectRule)bData;

                UQueue.Load(out bData);
                fkc.UpdateRule = (Rule)bData;

                UQueue.Load(out bData);
                fkc.DeleteRule = (Rule)bData;
            }
            return (nSize - UQueue.GetSize());
        }
        private void Push(CUQueue UQueue, UniqueConstraint uc)
        {
            bool bNull = (uc == null);
            UQueue.Save(bNull);
            if (!bNull)
            {
                Push(UQueue, uc.Columns);
                UQueue.Save(uc.ConstraintName);
                UQueue.Save(uc.IsPrimaryKey);
            }
        }
        private uint Pop(CUQueue UQueue, ref UniqueConstraint uc)
        {
            bool bNull = false;
            uint nSize = UQueue.GetSize();
            UQueue.Load(out bNull);
            if (!bNull)
            {
                string str;
                DataColumn[] dcs = null;
                bool b;
                Pop(UQueue, ref dcs);
                UQueue.Load(out str);
                UQueue.Load(out b);
                uc = new UniqueConstraint(str, dcs, b);
            }
            return nSize - UQueue.GetSize();
        }
        internal void Push(CUQueue UQueue, DataRelationCollection drc)
        {
            UQueue.Save(drc.Count);
            foreach (DataRelation dr in drc)
            {
                PushTableColNamesOnly(UQueue, dr.ChildColumns);
                UQueue.Save(dr.Nested);
                UQueue.Save(dr.RelationName);
                PushTableColNamesOnly(UQueue, dr.ParentColumns);
            }
        }
        private uint Pop(CUQueue UQueue, ref DataRelationCollection drc)
        {
            int n;
            string str;
            bool b;
            int nData;
            uint nSize = UQueue.GetSize();
            UQueue.Load(out nData);
            drc.Clear();
            for (n = 0; n < nData; n++)
            {
                DataColumn[] dcsChild = null;
                PopTableColNamesOnly(UQueue, ref dcsChild);

                UQueue.Load(out b);
                UQueue.Load(out str);

                DataColumn[] dcsParent = null;
                PopTableColNamesOnly(UQueue, ref dcsParent);

                DataRelation dr = new DataRelation(str, dcsParent, dcsChild);
                dr.Nested = b;
                drc.Add(dr);
            }
            return (nSize - UQueue.GetSize());
        }
    }
}
