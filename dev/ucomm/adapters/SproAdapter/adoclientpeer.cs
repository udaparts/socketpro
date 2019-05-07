using System;
using System.Data;

namespace SocketProAdapter.ServerSide
{
    public class CAdoClientPeer : CClientPeer
    {
        public delegate void DAdonetLoaded(CAdoClientPeer sender, ushort reqId);
        public event DAdonetLoaded OnAdonetLoaded;

        public CAdoSerializationHelper AdoSerializer
        {
            get
            {
                return m_AdoSerializer;
            }
        }

        [RequestAttr(CAdoSerializationHelper.idDataSetHeaderArrive)]
        private void LoadDataSetHeader()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idDataSetHeaderArrive, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idDataSetHeaderArrive);
        }

        [RequestAttr(CAdoSerializationHelper.idDataTableHeaderArrive)]
        private void LoadDataTableHeader()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idDataTableHeaderArrive, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idDataTableHeaderArrive);
        }

        [RequestAttr(CAdoSerializationHelper.idDataReaderHeaderArrive)]
        private void LoadDataReaderHeader()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idDataReaderHeaderArrive, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idDataReaderHeaderArrive);
        }

        [RequestAttr(CAdoSerializationHelper.idEndDataSet)]
        private void LoadEndDataSet()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idEndDataSet, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idEndDataSet);
        }

        [RequestAttr(CAdoSerializationHelper.idEndDataTable)]
        private void LoadEndDataTable()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idEndDataTable, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idEndDataTable);
        }

        [RequestAttr(CAdoSerializationHelper.idEndDataReader)]
        private void LoadEndDataReader()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idEndDataReader, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idEndDataReader);
        }

        [RequestAttr(CAdoSerializationHelper.idDataTableRowsArrive, true)]
        private void LoadDataTableRows()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idDataTableRowsArrive, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idDataTableRowsArrive);
        }

        [RequestAttr(CAdoSerializationHelper.idDataReaderRecordsArrive, true)]
        private void LoadDataReaderRecords()
        {
            m_AdoSerializer.Load(CAdoSerializationHelper.idDataReaderRecordsArrive, m_qBuffer);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, CAdoSerializationHelper.idDataReaderRecordsArrive);
        }

        public ulong Send(IDataReader dr)
        {
            return Send(dr, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public virtual ulong Send(IDataReader dr, uint batchSize)
        {
            uint res;
            ulong nSize = 0;
            bool bSuc;
            if (dr == null)
                throw new ArgumentException("Must pass in a valid data reader interface!");
            using (CScopeUQueue su = new CScopeUQueue())
            {
                CUQueue UQueue = su.UQueue;
                bool bBatching = Batching;
                if (!bBatching)
                    bSuc = StartBatching();
                do
                {
                    UQueue.SetSize(0);
                    m_AdoSerializer.PushHeader(UQueue, dr);
                    if (batchSize < 2048)
                        batchSize = 2048;
                    UQueue.Save(batchSize);
                    nSize = res = SendResult(CAdoSerializationHelper.idDataReaderHeaderArrive, UQueue);
                    UQueue.SetSize(0);

                    //monitor socket close event and cancel request
                    if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        break;

                    while (dr.Read())
                    {
                        m_AdoSerializer.Push(UQueue, dr);
                        if (UQueue.GetSize() > batchSize)
                        {
                            res = SendResult(CAdoSerializationHelper.idDataReaderRecordsArrive, UQueue);
                            UQueue.SetSize(0);

                            //monitor socket close event and cancel request
                            if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                            {
                                nSize = res;
                                break;
                            }
                            else
                            {
                                nSize += res;
                                if (BytesBatched > 2 * batchSize)
                                {
                                    //if we find too much are stored in batch queue, we send them and start a new batching
                                    bSuc = CommitBatching();
                                    bSuc = StartBatching();
                                }
                            }
                        }
                    }
                    if (UQueue.GetSize() > 0) //remaining
                    {
                        res = SendResult(CAdoSerializationHelper.idDataReaderRecordsArrive, UQueue);
                        UQueue.SetSize(0);

                        //monitor socket close event and cancel request
                        if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        {
                            nSize = res;
                            break;
                        }
                        nSize += res;
                    }
                } while (false);
                UQueue.SetSize(0);
                res = SendResult(CAdoSerializationHelper.idEndDataReader);
                //monitor socket close event and cancel request
                if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                    nSize = res;
                else
                    nSize += res;

                if (!bBatching)
                    bSuc = CommitBatching();
            }
            return nSize;
        }

        public virtual ulong Send(DataSet ds, bool bNeedRelations, uint batchSize)
        {
            bool bSuc;
            uint res;
            ulong nSize = 0;
            if (ds == null)
                throw new ArgumentException("Must pass in an valid DataSet object!");
            using (CScopeUQueue su = new CScopeUQueue())
            {
                CUQueue UQueue = su.UQueue;
                m_AdoSerializer.PushHeader(UQueue, ds);
                bool bBatching = Batching;
                if (!bBatching)
                    bSuc = StartBatching();
                do
                {
                    nSize = res = SendResult(CAdoSerializationHelper.idDataSetHeaderArrive, UQueue);
                    UQueue.SetSize(0);

                    //monitor socket close event and cancel request
                    if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        break;
                    foreach (DataTable dt in ds.Tables)
                    {
                        ulong rtn = Send(dt, batchSize);
                        UQueue.SetSize(0);

                        if (rtn == CClientPeer.REQUEST_CANCELED || rtn == CClientPeer.SOCKET_NOT_FOUND)
                        {
                            nSize = rtn;
                            break;
                        }
                        else
                            nSize += rtn;
                    }

                    res = EndDataSet(ds, bNeedRelations, UQueue);
                    if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                    {
                        nSize = res;
                        break;
                    }
                    else
                        nSize += res;
                } while (false);
                UQueue.SetSize(0);
                if (!bBatching && Batching)
                    bSuc = CommitBatching();
            }
            return nSize;
        }

        uint EndDataSet(DataSet ds, bool bNeedRelations, CUQueue UQueue)
        {
            UQueue.SetSize(0);
            if (bNeedRelations && ds.Relations != null)
                m_AdoSerializer.Push(UQueue, ds.Relations);
            return SendResult(CAdoSerializationHelper.idEndDataSet, UQueue);
        }

        public ulong Send(DataSet ds, bool bNeedRelations)
        {
            return Send(ds, bNeedRelations, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public ulong Send(DataSet ds)
        {
            return Send(ds, false, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public ulong Send(DataTable dt)
        {
            return Send(dt, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public virtual ulong Send(DataTable dt, uint batchSize)
        {
            uint res;
            ulong nSize;
            bool bSuc;
            if (dt == null)
                throw new ArgumentException("Must pass in an valid DataTable object!");
            using (CScopeUQueue su = new CScopeUQueue())
            {
                CUQueue UQueue = su.UQueue;
                bool bBatching = Batching;
                if (!bBatching)
                    bSuc = StartBatching();
                do
                {
                    //m_AdoSerializer->PushHeader(UQueue, dt, bNeedParentRelations, bNeedChildRelations);
                    m_AdoSerializer.PushHeader(UQueue, dt, false, false);
                    if (batchSize < 2048)
                        batchSize = 2048;
                    UQueue.Save(batchSize);
                    nSize = res = SendResult(CAdoSerializationHelper.idDataTableHeaderArrive, UQueue);
                    UQueue.SetSize(0);

                    //monitor socket close event and cancel request
                    if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        break;

                    foreach (DataRow dr in dt.Rows)
                    {
                        m_AdoSerializer.Push(UQueue, dr);
                        if (UQueue.GetSize() > batchSize)
                        {
                            res = SendResult(CAdoSerializationHelper.idDataTableRowsArrive, UQueue);
                            UQueue.SetSize(0);

                            //monitor socket close event and cancel request
                            if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                            {
                                nSize = res;
                                break;
                            }
                            else
                            {
                                if (BytesBatched > 2 * batchSize)
                                {
                                    bSuc = CommitBatching();
                                    bSuc = StartBatching();
                                }
                                nSize += res;
                            }
                        }
                    }
                    if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        break;
                    if (UQueue.GetSize() > 0) //remaining
                    {
                        res = SendResult(CAdoSerializationHelper.idDataTableRowsArrive, UQueue);
                        UQueue.SetSize(0);

                        //monitor socket close event and cancel request
                        if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                        {
                            nSize = res;
                            break;
                        }
                        else
                            nSize += res;
                    }
                } while (false);
                UQueue.SetSize(0);
                res = SendResult(CAdoSerializationHelper.idEndDataTable);
                if (res == CClientPeer.REQUEST_CANCELED || res == CClientPeer.SOCKET_NOT_FOUND)
                    nSize = res;
                else
                    nSize += res;
                if (!bBatching)
                    bSuc = CommitBatching();
            }
            return nSize;
        }
        private CAdoSerializationHelper m_AdoSerializer = new CAdoSerializationHelper();
    }
}
