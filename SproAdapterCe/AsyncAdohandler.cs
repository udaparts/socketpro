using System;
using System.Data;


namespace SocketProAdapter
{
    namespace ClientSide
    {
        public abstract class CAsyncAdohandler : CAsyncServiceHandler
        {
            public CAsyncAdohandler(int nServiceId)
                :base(nServiceId)
            {
            }

            public CAsyncAdohandler(int nServiceId, CClientSocket cs)
                : base(nServiceId, cs)
            {
            }

            public CAsyncAdohandler(int nServiceId, CClientSocket cs, IAsyncResultsHandler DefaultAsyncResultsHandler)
                : base(nServiceId, cs, DefaultAsyncResultsHandler)
            {
            }

            /// <summary>
            /// Call the methods EndLoadData and BeginLoadData one time for loaded records during fetching records.
            /// This method is usually called at client side for reduction of latency and fast displaying beginning records.
            /// </summary>
            public void FinalizeRecords()
            {
                if (m_AdoSerialier != null)
                    m_AdoSerialier.FinalizeRecords();
            }

            protected CUQueue m_AdoUQueue = new CUQueue();
            protected CAsyncAdoSerializationHelper m_AdoSerialier = new CAsyncAdoSerializationHelper();
            public bool Send(DataTable dt)
            {
                return Send(dt, false, false, 10240);
            }

            public virtual bool Send(DataTable dt, bool bNeedParentRelations, bool bNeedChildRelations, int nBatchSize)
            {
                bool bSuc = true;
                if (GetAttachedClientSocket() == null)
                    throw new Exception("The asynchronous handlers must be attached to an instance of CClientSocket firts!");
                if (dt == null)
                    throw new Exception("Must pass in a valid data table object!");
                
                bool bBatching = GetAttachedClientSocket().IsBatching();
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().BeginBatching();
                }
                m_AdoUQueue.SetSize(0);
                m_AdoSerialier.PushHeader(m_AdoUQueue, dt, false, false);
                //m_AdoSerialier.PushHeader(m_AdoUQueue, dt, bNeedParentRelations, bNeedChildRelations);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				m_AdoUQueue.Push(nBatchSize);
				if (!SendRequest(CAsyncAdoSerializationHelper.idDataTableHeaderArrive, m_AdoUQueue))
                {
                    if (!bBatching)
                    {
                        bSuc = GetAttachedClientSocket().Commit(true);
                    }
                    return false;
                }
                m_AdoUQueue.SetSize(0);
                foreach (DataRow dr in dt.Rows)
                {
                    m_AdoSerialier.Push(m_AdoUQueue, dr);
                    if (m_AdoUQueue.GetSize() > nBatchSize)
                    {
                        if (!SendRequest(CAsyncAdoSerializationHelper.idDataTableRowsArrive, m_AdoUQueue))
                        {
                            if (!bBatching)
                            {
                                bSuc = GetAttachedClientSocket().Commit(true);
                            }
                            return false;
                        }

                        if (GetAttachedClientSocket().GetBytesBatched() > 2 * nBatchSize)
                        {
                            //if we found too much are stored in batch queue, we send them and start a new batching
                            bool b = GetAttachedClientSocket().Commit(true);
                            b = GetAttachedClientSocket().BeginBatching();
                        }
                        m_AdoUQueue.SetSize(0);
                        int nBytesInSendBuffer = GetAttachedClientSocket().GetUSocket().BytesInSndMemory;
                        if (nBytesInSendBuffer > 40*1024) //40k
                        {
                            bool b = GetAttachedClientSocket().Commit(true);
                            //if we found there are too much data in sending buffer, we wait until all of data are sent and processed.
                            GetAttachedClientSocket().WaitAll();
                            b = GetAttachedClientSocket().BeginBatching();
                        }
                    }
                }
                if (m_AdoUQueue.GetSize() > 0)
                {
                    bSuc = SendRequest(CAsyncAdoSerializationHelper.idDataTableRowsArrive, m_AdoUQueue);
                    m_AdoUQueue.SetSize(0);
                }
				SendRequest(CAsyncAdoSerializationHelper.idEndDataTable);
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().Commit(true);
                }
                return bSuc;
            }

            private bool EndDataSet(DataSet ds, bool bNeedRelations)
            {
                if (GetAttachedClientSocket() == null)
                    throw new Exception("The asynchronous handlers must be attached to an instance of CClientSocket firts!");
                if (ds == null)
                    throw new Exception("Must pass in a valid dataset object!"); 
                m_AdoUQueue.SetSize(0);
                if (bNeedRelations)
                    m_AdoSerialier.Push(m_AdoUQueue, ds.Relations);
                return SendRequest(CAsyncAdoSerializationHelper.idEndDataSet, m_AdoUQueue);
            }

			protected override void OnResultReturned(short sRequestID, CUQueue UQueue) 
			{
				//if(SocketProServerException.HResult != 0) return; //exception transfered from SocketPro server
				m_AdoSerialier.Load(sRequestID, UQueue);
			}

            public bool Send(DataSet ds)
            {
                return Send(ds, false, 10240);
            }

			public bool Send(DataSet ds, bool bNeedRelations)
			{
				return Send(ds, bNeedRelations, 10240);
			}

            public virtual bool Send(DataSet ds, bool bNeedRelations, int nBatchSize)
            {
                bool bSuc;
                if (GetAttachedClientSocket() == null)
                    throw new Exception("The asynchronous handlers must be attached to an instance of CClientSocket firts!");
                if (ds == null)
                    throw new Exception("Must pass in an valid DataSet object!");
                bool bBatching = GetAttachedClientSocket().IsBatching();
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().BeginBatching();
                }
                m_AdoUQueue.SetSize(0);
                m_AdoSerialier.PushHeader(m_AdoUQueue, ds);
                bool b = SendRequest(CAsyncAdoSerializationHelper.idDataSetHeaderArrive, m_AdoUQueue);
                m_AdoUQueue.SetSize(0);
                if (!b)
                {
                    if (!bBatching)
                    {
                        bSuc = GetAttachedClientSocket().Commit(true);
                    }
                    return false;
                }
                foreach (DataTable dt in ds.Tables)
                {
                    b = Send(dt, bNeedRelations, bNeedRelations, nBatchSize);
                    if (!b)
                    {
                        if (!bBatching)
                        {
                            bSuc = GetAttachedClientSocket().Commit(true);
                        }
                        return false;
                    }
                    m_AdoUQueue.SetSize(0);
                }
                b = EndDataSet(ds, bNeedRelations);
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().Commit(true);
                }
                m_AdoUQueue.SetSize(0);
                return b;
            }
            public bool Send(IDataReader dr)
            {
                return Send(dr, 10240);
            }
            public virtual bool Send(IDataReader dr, int nBatchSize)
            {
                bool bSuc = false;
                if (dr == null)
                    throw new Exception("Must pass in a valid data reader interface!");
                if (GetAttachedClientSocket() == null)
                    throw new Exception("The asynchronous handlers must be attached to an instance of CClientSocket firts!");
                m_AdoUQueue.SetSize(0);
                bool bBatching = GetAttachedClientSocket().IsBatching();
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().BeginBatching();
                }
                m_AdoSerialier.PushHeader(m_AdoUQueue, dr);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				m_AdoUQueue.Push(nBatchSize);
                bSuc = SendRequest(CAsyncAdoSerializationHelper.idDataReaderHeaderArrive, m_AdoUQueue);

                //monitor socket close event
                if (!bSuc)
                {
                    if (!bBatching)
                    {
                        bSuc = GetAttachedClientSocket().Commit(true);
                    }
                    return false;
                }
                m_AdoUQueue.SetSize(0);
                while (dr.Read())
                {
                    m_AdoSerialier.Push(m_AdoUQueue, dr);
                    if (m_AdoUQueue.GetSize() > nBatchSize)
                    {
                        bSuc = SendRequest(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive, m_AdoUQueue);
                        if (GetAttachedClientSocket().GetBytesBatched() > 2 * nBatchSize)
                        {
                            //if we found too much are stored in batch queue, we send them and start a new batching
                            bool b = GetAttachedClientSocket().Commit(true);
                            b = GetAttachedClientSocket().BeginBatching();
                        }

                        int nBytesInSendBuffer = GetAttachedClientSocket().GetUSocket().BytesInSndMemory;
                        if (nBytesInSendBuffer > 40 * 1024)
                        {
                            bool b = GetAttachedClientSocket().Commit(true);
                            //if we found there are too much data in sending buffer, we wait until all of data are sent and processed.
                            GetAttachedClientSocket().WaitAll();
                            b = GetAttachedClientSocket().BeginBatching();
                        }

                        //monitor socket close event and cancel request
                        if (!bSuc)
                        {
                            if (!bBatching)
                            {
                                bSuc = GetAttachedClientSocket().Commit(true);
                            }
                            return false;
                        }
                        m_AdoUQueue.SetSize(0);
                    }
                }
                if (m_AdoUQueue.GetSize() > 0) //remaining
                {
                    bSuc = SendRequest(CAsyncAdoSerializationHelper.idDataReaderRecordsArrive, m_AdoUQueue);
                    m_AdoUQueue.SetSize(0);
                }
				SendRequest(CAsyncAdoSerializationHelper.idEndDataReader);
                if (!bBatching)
                {
                    bSuc = GetAttachedClientSocket().Commit(true);
                }
                return bSuc;
            }
        }
    }
}
