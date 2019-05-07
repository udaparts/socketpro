using System;
using System.Data;
using System.Diagnostics;

namespace SocketProAdapter.ClientSide
{
    public class CAsyncAdohandler : CAsyncServiceHandler
    {
        public delegate void DAdonetLoaded(CAsyncAdohandler sender, ushort reqId);
        public event DAdonetLoaded OnAdonetLoaded;

        private DAsyncResultHandler m_arh = (ar) =>
        {
            switch (ar.RequestId)
            {
                case CAdoSerializationHelper.idEndDataReader:
                case CAdoSerializationHelper.idEndDataSet:
                case CAdoSerializationHelper.idEndDataTable:
                case CAdoSerializationHelper.idDataReaderHeaderArrive:
                case CAdoSerializationHelper.idDataReaderRecordsArrive:
                case CAdoSerializationHelper.idDataSetHeaderArrive:
                case CAdoSerializationHelper.idDataTableHeaderArrive:
                case CAdoSerializationHelper.idDataTableRowsArrive:
                    if (ar.UQueue.GetSize() > 0)
                    {
                        Debug.WriteLine("Warning: ADO.NET request id = " + ar.RequestId + ", size = " + ar.UQueue.GetSize());
                    }
                    break;
                default:
                    break;
            }
        };

        protected CAsyncAdohandler(uint serviceId)
            : base(serviceId)
        {
            m_AdoSerializer = new CAdoSerializationHelper();
        }

        protected override void OnResultReturned(ushort sRequestId, CUQueue UQueue)
        {
            m_AdoSerializer.Load(sRequestId, UQueue);
            if (OnAdonetLoaded != null)
                OnAdonetLoaded.Invoke(this, sRequestId);
        }

        public CAdoSerializationHelper AdoSerializer
        {
            get
            {
                return m_AdoSerializer;
            }
        }

        private CAdoSerializationHelper m_AdoSerializer;
        private bool EndDataSet(DataSet ds, bool needRelations)
        {
            if (AttachedClientSocket == null)
                throw new InvalidOperationException("The asynchronous handler must be attached to an instance of CClientSocket first!");
            if (ds == null)
                throw new ArgumentNullException("Must pass in a valid dataset object!");
            using (CScopeUQueue UQueue = new CScopeUQueue())
            {
                CUQueue AdoUQueue = UQueue.UQueue;
                if (needRelations)
                    m_AdoSerializer.Push(AdoUQueue, ds.Relations);
                if (RouteeRequest)
                    return SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idEndDataSet);
                return SendRequest(CAdoSerializationHelper.idEndDataSet, AdoUQueue, m_arh);
            }
        }

        public void FinalizeRecords()
        {
            m_AdoSerializer.FinalizeRecords();
        }

        public bool Send(DataSet ds)
        {
            return Send(ds, false, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public bool Send(DataSet ds, bool needRelations)
        {
            return Send(ds, needRelations, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public virtual bool Send(DataSet ds, bool needRelations, uint batchSize)
        {
            bool b = false;
            if (AttachedClientSocket == null)
                throw new InvalidOperationException("The asynchronous handler must be attached to an instance of CClientSocket first!");
            if (ds == null)
                throw new ArgumentNullException("Must pass in an valid DataSet object!");
            bool bBatching = Batching;
            if (!bBatching)
                StartBatching();
            using (CScopeUQueue UQueue = new CScopeUQueue())
            {
                CUQueue AdoUQueue = UQueue.UQueue;
                do
                {
                    m_AdoSerializer.PushHeader(AdoUQueue, ds);
                    if (RouteeRequest)
                        b = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataSetHeaderArrive);
                    else
                        b = SendRequest(CAdoSerializationHelper.idDataSetHeaderArrive, AdoUQueue, m_arh);
                    AdoUQueue.SetSize(0);
                    if (!b)
                        break;
                    foreach (DataTable dt in ds.Tables)
                    {
                        b = Send(dt, batchSize);
                        AdoUQueue.SetSize(0);
                        if (!b)
                            break;
                    }
                    if (!b)
                        break;
                    b = EndDataSet(ds, needRelations);
                    AdoUQueue.SetSize(0);
                } while (false);
                if (!bBatching)
                    CommitBatching(true);
            }
            return b;
        }

        public bool Send(DataTable dt)
        {
            return Send(dt, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public virtual bool Send(DataTable dt, uint batchSize)
        {
            bool bSuc = false;
            if (AttachedClientSocket == null)
                throw new InvalidOperationException("The asynchronous handler must be attached to an instance of CClientSocket first!");
            if (dt == null)
                throw new ArgumentNullException("Must pass in a valid data table object!");
            bool rr = RouteeRequest;
            bool bBatching = Batching;
            if (!bBatching)
                StartBatching();
            using (CScopeUQueue UQueue = new CScopeUQueue())
            {
                CUQueue AdoUQueue = UQueue.UQueue;
                do
                {
                    AdoUQueue.SetSize(0);
                    m_AdoSerializer.PushHeader(AdoUQueue, dt, false, false);
                    if (batchSize < 2048)
                        batchSize = 2048;
                    AdoUQueue.Save(batchSize);
                    if (rr)
                        bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataTableHeaderArrive);
                    else
                        bSuc = SendRequest(CAdoSerializationHelper.idDataTableHeaderArrive, AdoUQueue, m_arh);
                    AdoUQueue.SetSize(0);
                    if (!bSuc)
                        break;
                    foreach (DataRow dr in dt.Rows)
                    {
                        m_AdoSerializer.Push(AdoUQueue, dr);
                        if (AdoUQueue.GetSize() > batchSize)
                        {
                            if (rr)
                                bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataTableRowsArrive);
                            else
                                bSuc = SendRequest(CAdoSerializationHelper.idDataTableRowsArrive, AdoUQueue, m_arh);
                            AdoUQueue.SetSize(0);
                            if (!bSuc)
                                break;
                            if (AttachedClientSocket.BytesBatched > 2 * batchSize)
                            {
                                //if we find too much are stored in batch queue, we send them and start a new batching
                                CommitBatching(true);
                                StartBatching();
                            }
                            uint nBytesInSendBuffer = AttachedClientSocket.BytesInSendingBuffer;
                            if (nBytesInSendBuffer > 6 * CAdoSerializationHelper.DEFAULT_BATCH_SIZE) //60k
                            {
                                CommitBatching(true);
                                //if we find there are too much data in sending buffer, we wait until all of data are sent and processed.
                                WaitAll();
                                StartBatching();
                            }
                        }
                    }
                    if (!bSuc)
                        break;
                    if (AdoUQueue.GetSize() > 0)
                    {
                        if (rr)
                            bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataTableRowsArrive);
                        else
                            bSuc = SendRequest(CAdoSerializationHelper.idDataTableRowsArrive, AdoUQueue, m_arh);
                        AdoUQueue.SetSize(0);
                    }
                    if (!bSuc)
                        break;
                } while (false);
                if (bSuc)
                {
                    if (rr)
                        SendRouteeResult(CAdoSerializationHelper.idEndDataTable);
                    else
                        SendRequest(CAdoSerializationHelper.idEndDataTable, m_arh);
                }
                if (!bBatching)
                    CommitBatching(true);
            }
            return bSuc;
        }

        public bool Send(IDataReader dr)
        {
            return Send(dr, CAdoSerializationHelper.DEFAULT_BATCH_SIZE);
        }

        public virtual bool Send(IDataReader dr, uint batchSize)
        {
            bool bSuc = false;
            if (dr == null)
                throw new ArgumentNullException("Must pass in a valid data reader interface!");
            if (AttachedClientSocket == null)
                throw new InvalidOperationException("The asynchronous handler must be attached to an instance of CClientSocket first!");
            bool rr = RouteeRequest;
            bool bBatching = Batching;
            if (!bBatching)
                StartBatching();
            using (CScopeUQueue UQueue = new CScopeUQueue())
            {
                CUQueue AdoUQueue = UQueue.UQueue;
                do
                {
                    m_AdoSerializer.PushHeader(AdoUQueue, dr);
                    if (batchSize < 2048)
                        batchSize = 2048;
                    AdoUQueue.Save(batchSize);
                    if (rr)
                        bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataReaderHeaderArrive);
                    else
                        bSuc = SendRequest(CAdoSerializationHelper.idDataReaderHeaderArrive, AdoUQueue, m_arh);
                    AdoUQueue.SetSize(0);
                    //monitor socket close event
                    if (!bSuc)
                        break;
                    while (dr.Read())
                    {
                        m_AdoSerializer.Push(AdoUQueue, dr);
                        if (AdoUQueue.GetSize() > batchSize)
                        {
                            if (rr)
                                bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataReaderRecordsArrive);
                            else
                                bSuc = SendRequest(CAdoSerializationHelper.idDataReaderRecordsArrive, AdoUQueue, m_arh);
                            AdoUQueue.SetSize(0);
                            if (!bSuc)
                                break;
                            if (AttachedClientSocket.BytesBatched > 2 * batchSize)
                            {
                                //if we find too much are stored in batch queue, we send them and start a new batching
                                CommitBatching(true);
                                StartBatching();
                            }
                            if (AttachedClientSocket.BytesInSendingBuffer > 60 * 1024)
                            {
                                CommitBatching(true);
                                //if we find there are too much data in sending buffer, we wait until all of data are sent and processed.
                                WaitAll();
                                StartBatching();
                            }
                        }
                    }
                    if (!bSuc)
                        break;
                    if (AdoUQueue.GetSize() > 0) //remaining
                    {
                        if (rr)
                            bSuc = SendRouteeResult(AdoUQueue, CAdoSerializationHelper.idDataReaderRecordsArrive);
                        else
                            bSuc = SendRequest(CAdoSerializationHelper.idDataReaderRecordsArrive, AdoUQueue, m_arh);
                        AdoUQueue.SetSize(0);
                    }
                    if (!bSuc)
                        break;
                } while (false);
                if (bSuc)
                {
                    if (rr)
                        bSuc = SendRouteeResult(CAdoSerializationHelper.idEndDataReader);
                    else
                        bSuc = SendRequest(CAdoSerializationHelper.idEndDataReader, m_arh);
                }
                if (!bBatching)
                    CommitBatching(true);
            }
            return bSuc;
        }
    }
}
