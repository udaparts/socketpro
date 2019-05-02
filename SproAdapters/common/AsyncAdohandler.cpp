#include "stdafx.h"
#include "AsyncAdohandler.h"

using namespace System;
using namespace System::Data;
namespace SocketProAdapter
{
	namespace ClientSide
	{
		CAsyncAdohandler::CAsyncAdohandler(int nServiceId) 
			: CAsyncServiceHandler(nServiceId)
		{
			m_AdoSerialier = gcnew CAsyncAdoSerializationHelper();
		}

		CAsyncAdohandler::CAsyncAdohandler(int nServiceId, CClientSocket ^cs)
			: CAsyncServiceHandler(nServiceId, cs)
		{
			m_AdoSerialier = gcnew CAsyncAdoSerializationHelper();
		}
		CAsyncAdohandler::CAsyncAdohandler(int nServiceId, CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler)
			: CAsyncServiceHandler(nServiceId, cs, DefaultAsyncResultsHandler)
		{
			m_AdoSerialier = gcnew CAsyncAdoSerializationHelper();
		}

		CAsyncAdohandler::~CAsyncAdohandler()
		{
			delete m_AdoSerialier;
		}

		void CAsyncAdohandler::FinalizeRecords()
		{
			if(m_AdoSerialier != nullptr)
				m_AdoSerialier->FinalizeRecords();
		}

		bool CAsyncAdohandler::Send(DataTable ^dt)
		{
			return Send(dt, 10240);
		}
		
		bool CAsyncAdohandler::Send(DataTable ^dt, int nBatchSize)
		{
			bool bSuc = false;
			if (GetAttachedClientSocket() == nullptr)
				throw gcnew Exception("The asynchronous handler must be attached to an instance of CClientSocket first!");
			if (dt == nullptr)
				throw gcnew Exception("Must pass in a valid data table object!");

			bool bBatching = GetAttachedClientSocket()->IsBatching();
			if (!bBatching)
				bSuc = GetAttachedClientSocket()->BeginBatching();
			CScopeUQueue	UQueue;
			CUQueue ^AdoUQueue = UQueue.m_UQueue;
			do
			{
				AdoUQueue->SetSize(0);
				//m_AdoSerialier->PushHeader(AdoUQueue, dt, bNeedParentRelations, bNeedChildRelations);
				m_AdoSerialier->PushHeader(AdoUQueue, dt, false, false);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				AdoUQueue->Push(nBatchSize);
				bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataTableHeaderArrive, AdoUQueue);
				AdoUQueue->SetSize(0);
				if(!bSuc)
					break;
				for each (DataRow ^dr in dt->Rows)
				{
					m_AdoSerialier->Push(AdoUQueue, dr);
					if (AdoUQueue->GetSize() > nBatchSize)
					{
						bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataTableRowsArrive, AdoUQueue);
						AdoUQueue->SetSize(0);
						if(!bSuc)
							break;
						if (GetAttachedClientSocket()->GetBytesBatched() > 2 * nBatchSize)
						{
							//if we find too much are stored in batch queue, we send them and start a new batching
							bool b = GetAttachedClientSocket()->Commit(true);
							b = GetAttachedClientSocket()->BeginBatching();
						}
						int nBytesInSendBuffer = GetAttachedClientSocket()->GetUSocket()->BytesInSndMemory;
						if (nBytesInSendBuffer > 60*1024) //60k
						{
							bool b = GetAttachedClientSocket()->Commit(true);
							//if we find there are too much data in sending buffer, we wait until all of data are sent and processed.
							GetAttachedClientSocket()->WaitAll();
							b = GetAttachedClientSocket()->BeginBatching();
						}
					}
				}
				if(!bSuc)
					break;
				if (AdoUQueue->GetSize() > 0)
				{
					bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataTableRowsArrive, AdoUQueue);
					AdoUQueue->SetSize(0);
				}
			}while(false);
			SendRequest(CAsyncAdoSerializationHelper::idEndDataTable);
			if (!bBatching)
				GetAttachedClientSocket()->Commit(true);
			return bSuc;
		}

		void CAsyncAdohandler::OnResultReturned(short sRequestID, CUQueue ^UQueue)
		{
			m_AdoSerialier->Load(sRequestID, UQueue);
		}

		bool CAsyncAdohandler::EndDataSet(DataSet ^ds, bool bNeedRelations)
		{
			if (GetAttachedClientSocket() == nullptr)
				throw gcnew Exception("The asynchronous handler must be attached to an instance of CClientSocket first!");
			if (ds == nullptr)
				throw gcnew Exception("Must pass in a valid dataset object!");
			CScopeUQueue	UQueue;
			CUQueue ^AdoUQueue = UQueue.m_UQueue;
			if (bNeedRelations)
				m_AdoSerialier->Push(AdoUQueue, ds->Relations);
			return SendRequest(CAsyncAdoSerializationHelper::idEndDataSet, AdoUQueue);
		}

		bool CAsyncAdohandler::Send(DataSet ^ds)
		{
			return Send(ds, false, 10240);
		}

		bool CAsyncAdohandler::Send(DataSet ^ds, bool bNeedRelations)
		{
			return Send(ds, bNeedRelations, 10240);
		}

		bool CAsyncAdohandler::Send(DataSet ^ds, bool bNeedRelations, int nBatchSize)
		{
			bool bSuc;
			bool b = false;
			if (GetAttachedClientSocket() == nullptr)
				throw gcnew Exception("The asynchronous handler must be attached to an instance of CClientSocket first!");
			if (ds == nullptr)
				throw gcnew Exception("Must pass in an valid DataSet object!");
			bool bBatching = GetAttachedClientSocket()->IsBatching();
			if (!bBatching)
				bSuc = GetAttachedClientSocket()->BeginBatching();
			CScopeUQueue	UQueue;
			CUQueue ^AdoUQueue = UQueue.m_UQueue;
			do
			{
				m_AdoSerialier->PushHeader(AdoUQueue, ds);
				b = SendRequest(CAsyncAdoSerializationHelper::idDataSetHeaderArrive, AdoUQueue);
				AdoUQueue->SetSize(0);
				if (!b)
					break;
				for each (DataTable ^dt in ds->Tables)
				{
					b = Send(dt, nBatchSize);
					AdoUQueue->SetSize(0);
					if (!b)
						break;
				}
				if (!b)
					break;
				b = EndDataSet(ds, bNeedRelations);
				AdoUQueue->SetSize(0);
			}while(false);
			if (!bBatching)
				bSuc = GetAttachedClientSocket()->Commit(true);
			return b;
		}

		bool CAsyncAdohandler::Send(IDataReader ^dr)
		{
			return Send(dr, 10240);
		}

		bool CAsyncAdohandler::Send(IDataReader ^dr, int nBatchSize)
		{
			bool bSuc = false;
			if (dr == nullptr)
				throw gcnew Exception("Must pass in a valid data reader interface!");
			if (GetAttachedClientSocket() == nullptr)
				throw gcnew Exception("The asynchronous handler must be attached to an instance of CClientSocket first!");
			bool bBatching = GetAttachedClientSocket()->IsBatching();
			if (!bBatching)
				GetAttachedClientSocket()->BeginBatching();
			CScopeUQueue	UQueue;
			CUQueue ^AdoUQueue = UQueue.m_UQueue;
			do
			{
				m_AdoSerialier->PushHeader(AdoUQueue, dr);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				AdoUQueue->Push(nBatchSize);
				bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataReaderHeaderArrive, AdoUQueue);
				AdoUQueue->SetSize(0);
				//monitor socket close event
				if (!bSuc)
					break;
				while (dr->Read())
				{
					m_AdoSerialier->Push(AdoUQueue, dr);
					if (AdoUQueue->GetSize() > nBatchSize)
					{
						bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataReaderRecordsArrive, AdoUQueue);
						AdoUQueue->SetSize(0);
						if(!bSuc)
							break;
						if (GetAttachedClientSocket()->GetBytesBatched() > 2 * nBatchSize)
						{
							//if we find too much are stored in batch queue, we send them and start a new batching
							bool b = GetAttachedClientSocket()->Commit(true);
							b = GetAttachedClientSocket()->BeginBatching();
						}
						int nBytesInSendBuffer = GetAttachedClientSocket()->GetUSocket()->BytesInSndMemory;
						if (nBytesInSendBuffer > 60 * 1024)
						{
							bool b = GetAttachedClientSocket()->Commit(true);
							//if we find there are too much data in sending buffer, we wait until all of data are sent and processed.
							GetAttachedClientSocket()->WaitAll();
							b = GetAttachedClientSocket()->BeginBatching();
						}
					}
				}
				if(!bSuc)
					break;
				if (AdoUQueue->GetSize() > 0) //remaining
				{
					bSuc = SendRequest(CAsyncAdoSerializationHelper::idDataReaderRecordsArrive, AdoUQueue);
					AdoUQueue->SetSize(0);
				}
			}while(false);
			SendRequest(CAsyncAdoSerializationHelper::idEndDataReader);
			if (!bBatching)
			{
				GetAttachedClientSocket()->Commit(true);
			}
			return bSuc;
		}
	}
}