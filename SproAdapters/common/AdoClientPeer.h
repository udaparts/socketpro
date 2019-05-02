#pragma once
#include "RemotingAdoNetHelper.h"
using namespace System;
using namespace System::Data;

namespace SocketProAdapter
{
	namespace ServerSide
	{
		/// <summary>
		/// An class makes remoting ADO.NET objects, DataSet, DataTable and IDataReader onto a remote client much easier.
		/// To use this class at server side, your client peer class must derive from CAdoClientPeer instead of CClientPeer.
		/// </summary>
		public ref class CAdoClientPeer abstract: CClientPeer
		{
		public:
			CAdoClientPeer();
			virtual ~CAdoClientPeer();

		public:
			/// <summary>
			/// Send a set of records through the interface IDataReader onto a remote client.
			/// If proper, this method is preferred for reduction of memory footprint and latency without pre-loading data records into memory.
			/// </summary>
			System::Int64 Send(IDataReader ^dr);

			/// <summary>
			/// Send a set of records through the interface IDataReader onto a remote client. Parameter nBatchSize is defaulted to 10240.
			/// The parameter nBatchSize controls network communication batch size in byte.
			/// If proper, this method is preferred for reduction of memory footprint and latency without pre-loading data records into memory.
			/// </summary>
			virtual System::Int64 Send(IDataReader ^dr, int nBatchSize);
			
			/// <summary>
			/// Send a DataTable object onto a remote client.
			/// </summary>
			System::Int64 Send(DataTable ^dt);

			/// <summary>
			/// Send a DataTable object onto a remote client. Parameter nBatchSize is defaulted to 10240, which controls network communication batch size in byte.
			/// </summary>
			virtual System::Int64 Send(DataTable ^dt, int nBatchSize);
			
			/// <summary>
			/// Send a DataSet object onto a remote client. Parameters bNeedRelations and nBatchSize are defaulted to false and 10240, repectively.
			/// If bNeedRelations is set to true, relationships among tables are remoted.
			/// The parameter nBatchSize controls network communication batch size in byte.
			/// By default, nBatchSize is 10240.
			/// </summary>
			virtual System::Int64 Send(DataSet ^ds, bool bNeedRelations, int nBatchSize);

			/// <summary>
			/// Send a DataSet object onto a remote client. The parameter bNeedRelations is defaulted to false.
			/// If bNeedRelations is set to true, relationships among tables are remoted.
			/// </summary>
			virtual System::Int64 Send(DataSet ^ds, bool bNeedRelations);

			/// <summary>
			/// Send a DataSet object onto a remote client without serializing relationships.
			/// </summary>
			System::Int64 Send(DataSet ^ds);

		protected:
			virtual void OnFastRequestArrive(short sRequestID, int nLen) override;
			virtual int OnSlowRequestArrive(short sRequestID, int nLen) override;

		private:
			int EndDataSet(DataSet ^ds, bool bNeedRelations, CUQueue ^UQueue);

		protected:
			CAsyncAdoSerializationHelper ^m_AdoSerialier;

		private:
			static bool	m_bSlowSet = false;
		};
	}
}
