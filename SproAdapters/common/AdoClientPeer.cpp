#include "stdafx.h"
#include "AdoClientPeer.h"

using namespace System;
using namespace System::Data;
namespace SocketProAdapter
{
	namespace ServerSide
	{
		CAdoClientPeer::CAdoClientPeer()
		{
			m_AdoSerialier = gcnew CAsyncAdoSerializationHelper();
		}

		CAdoClientPeer::~CAdoClientPeer()
		{
			delete m_AdoSerialier;
		}

		void CAdoClientPeer::OnFastRequestArrive(short sRequestID, int nLen)
		{
			switch(sRequestID)
			{
			case CAsyncAdoSerializationHelper::idDataSetHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataReaderHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataTableHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataTableRowsArrive:
			case CAsyncAdoSerializationHelper::idDataReaderRecordsArrive:
			case CAsyncAdoSerializationHelper::idEndDataReader:
			case CAsyncAdoSerializationHelper::idEndDataTable:
			case CAsyncAdoSerializationHelper::idEndDataSet:
				if(!m_bSlowSet)
				{
					CBaseService ^p = GetBaseService();
					bool bSuc = p->AddSlowRequest(CAsyncAdoSerializationHelper::idDataReaderRecordsArrive);
					bSuc = p->AddSlowRequest(CAsyncAdoSerializationHelper::idDataTableRowsArrive);
					m_bSlowSet = true;
				}
				m_AdoSerialier->Load(sRequestID, m_UQueue);
				SendResult(sRequestID); //empty result
				break;
			default:
				break;
			}
		}

		int CAdoClientPeer::OnSlowRequestArrive(short sRequestID, int nLen)
		{
			switch(sRequestID)
			{
			case CAsyncAdoSerializationHelper::idDataSetHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataReaderHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataTableHeaderArrive:
			case CAsyncAdoSerializationHelper::idDataTableRowsArrive:
			case CAsyncAdoSerializationHelper::idDataReaderRecordsArrive:
			case CAsyncAdoSerializationHelper::idEndDataReader:
			case CAsyncAdoSerializationHelper::idEndDataTable:
			case CAsyncAdoSerializationHelper::idEndDataSet:
				m_AdoSerialier->Load(sRequestID, m_UQueue);
				SendResult(sRequestID); //empty result
				break;
			default:
				break;
			}
			return 0;
		}

		System::Int64 CAdoClientPeer::Send(IDataReader ^dr)
		{
			return Send(dr, 10240);
		}

		System::Int64 CAdoClientPeer::Send(IDataReader ^dr, int nBatchSize)
		{
			int res;
			System::Int64 nSize;
			bool bSuc;
			if (dr == nullptr)
				throw gcnew Exception("Must pass in a valid data reader interface!");
			CScopeUQueue su;
			CUQueue ^UQueue = su.UQueue;
			bool bBatching = IsBatching;
			if (!bBatching)
				bSuc = StartBatching();
			do
			{
				UQueue->SetSize(0);
				if(TransferServerException)
					UQueue->Push((long)0); //required by client
				m_AdoSerialier->PushHeader(UQueue, dr);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				UQueue->Push(nBatchSize);
				nSize = res = SendReturnData(CAsyncAdoSerializationHelper::idDataReaderHeaderArrive, UQueue);
				UQueue->SetSize(0);
				if(TransferServerException)
					UQueue->Push((long)0); //required by client
				//monitor socket close event and cancel request
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
				{
					break;
				}
				
				while (dr->Read())
				{
					m_AdoSerialier->Push(UQueue, dr);
					if (UQueue->GetSize() > nBatchSize)
					{
						res = SendReturnData(CAsyncAdoSerializationHelper::idDataReaderRecordsArrive, UQueue);
						UQueue->SetSize(0);
						if(TransferServerException)
							UQueue->Push((long)0); //required by client
						//monitor socket close event and cancel request
						if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
						{
							nSize = res;
							break;
						}
						else
						{
							nSize += res;
							if (BytesBatched > 2 * nBatchSize)
							{
								//if we find too much are stored in batch queue, we send them and start a new batching
								bSuc = CommitBatching();
								bSuc = StartBatching();
							}
						}
					}
				}
				if (UQueue->GetSize() > sizeof(long)) //remaining
				{
					res = SendReturnData(CAsyncAdoSerializationHelper::idDataReaderRecordsArrive, UQueue);
					UQueue->SetSize(0);
					if(TransferServerException)
						UQueue->Push((long)0); //required by client
					//monitor socket close event and cancel request
					if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
					{
						nSize = res;
						break;
					}
					nSize += res;
				}
			}while(false);
			UQueue->SetSize(0);
			SendResult(CAsyncAdoSerializationHelper::idEndDataReader);
			if (!bBatching)
				bSuc = CommitBatching();
			return nSize;
		}

		System::Int64 CAdoClientPeer::Send(DataTable ^dt)
		{
			return Send(dt, 10240);
		}

		System::Int64 CAdoClientPeer::Send(DataTable ^dt, int nBatchSize)
		{
			int res;
			System::Int64 nSize;
			bool bSuc;
			if (dt == nullptr)
				throw gcnew Exception("Must pass in an valid DataTable object!");
			CScopeUQueue su;
			CUQueue ^UQueue = su.UQueue;
			if(TransferServerException)
				UQueue->Push((long)0); //required by client
			bool bBatching = IsBatching;
			if (!bBatching)
				bSuc = StartBatching();
			do
			{
				//m_AdoSerialier->PushHeader(UQueue, dt, bNeedParentRelations, bNeedChildRelations);
				m_AdoSerialier->PushHeader(UQueue, dt, false, false);
				if (nBatchSize < 2048)
					nBatchSize = 2048;
				UQueue->Push(nBatchSize);
				nSize = res = SendReturnData(CAsyncAdoSerializationHelper::idDataTableHeaderArrive, UQueue);
				UQueue->SetSize(0);
				if(TransferServerException)
					UQueue->Push((long)0); //required by client

				//monitor socket close event and cancel request
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
					break;
				for each (DataRow ^dr in dt->Rows)
				{
					m_AdoSerialier->Push(UQueue, dr);
					if (UQueue->GetSize() > nBatchSize)
					{
						res = SendReturnData(CAsyncAdoSerializationHelper::idDataTableRowsArrive, UQueue);
						UQueue->SetSize(0);
						if(TransferServerException)
							UQueue->Push((long)0); //required by client
						//monitor socket close event and cancel request
						if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
						{
							nSize = res;
							break;
						}
						else
						{
							if(BytesBatched > 2*nBatchSize)
							{
								bSuc = CommitBatching(); 
								bSuc = StartBatching();
							}
							nSize += res;
						}
					}
				}
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
					break;
				if (UQueue->GetSize() > sizeof(long)) //remaining
				{
					res = SendReturnData(CAsyncAdoSerializationHelper::idDataTableRowsArrive, UQueue);
					UQueue->SetSize(0);
					if(TransferServerException)
						UQueue->Push((long)0); //required by client
					//monitor socket close event and cancel request
					if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
					{
						nSize = res;
						break;
					}
					nSize += res;
				}
			}while(false);
			UQueue->SetSize(0);
			SendResult(CAsyncAdoSerializationHelper::idEndDataTable);
			if (!bBatching)
				bSuc = CommitBatching();
			return nSize;
		}

		System::Int64 CAdoClientPeer::Send(DataSet ^ds, bool bNeedRelations)
		{
			return Send(ds, bNeedRelations, 10240);
		}

		System::Int64 CAdoClientPeer::Send(DataSet ^ds, bool bNeedRelations, int nBatchSize)
		{
			bool bSuc;
			int res;
			if (ds == nullptr)
				throw gcnew Exception("Must pass in an valid DataSet object!");
			CScopeUQueue su;
			CUQueue ^UQueue = su.UQueue;
			System::Int64 nSize = 0;
			if(TransferServerException)
				UQueue->Push((long)0); //required by client
			m_AdoSerialier->PushHeader(UQueue, ds);
			bool bBatching = IsBatching;
			if (!bBatching)
				bSuc = StartBatching();
			do
			{
				nSize = res = SendReturnData(CAsyncAdoSerializationHelper::idDataSetHeaderArrive, UQueue);
				UQueue->SetSize(0);
				if(TransferServerException)
					UQueue->Push((long)0); //required by client
				//monitor socket close event and cancel request
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
				{
					break;
				}
				for each (DataTable ^dt in ds->Tables)
				{
					System::Int64 rtn = Send(dt, nBatchSize);
					UQueue->SetSize(0);
					if(TransferServerException)
						UQueue->Push((long)0); //required by client
					if (rtn == CClientPeer::REQUEST_CANCELED || rtn == CClientPeer::SOCKET_NOT_FOUND)
					{
						nSize = rtn;
						break;
					}
					nSize += rtn;
				}
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
				{
					break;
				}
				res = EndDataSet(ds, bNeedRelations, UQueue);
				if (res == CClientPeer::REQUEST_CANCELED || res == CClientPeer::SOCKET_NOT_FOUND)
				{
					nSize = res;
					break;
				}
				nSize += res;
			}while(false);
			UQueue->SetSize(0);
			if (!bBatching && IsBatching)
				bSuc = CommitBatching();
			return nSize;
		}

		int CAdoClientPeer::EndDataSet(DataSet ^ds, bool bNeedRelations, CUQueue ^UQueue)
		{
			UQueue->SetSize(0);
			if(TransferServerException)
				UQueue->Push((long)0); //required by client
			if (bNeedRelations && ds->Relations != nullptr)
				m_AdoSerialier->Push(UQueue, ds->Relations);
			return SendReturnData(CAsyncAdoSerializationHelper::idEndDataSet, UQueue);
		}

		System::Int64 CAdoClientPeer::Send(DataSet ^ds)
		{
			return Send(ds, false, 10240);
		}
	}
}