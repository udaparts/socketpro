#pragma once

using namespace System;
using namespace System::Data;
#include "RemotingAdoNetHelper.h"

namespace SocketProAdapter
{
	namespace ClientSide
	{
		/// <summary>
		/// An abstract class makes remoting ADO.NET objects, DataSet, DataTable and IDataReader onto a remote host much easier.
		/// To use this class at client side, your asynchronous handler must inheriet CAsyncAdohandler instead of CAsyncServiceHandler.
		/// </summary>
		public ref class CAsyncAdohandler : CAsyncServiceHandler
		{
		public:
			CAsyncAdohandler(int nServiceId);
			CAsyncAdohandler(int nServiceId, CClientSocket ^cs);
			CAsyncAdohandler(int nServiceId, CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler);
			virtual ~CAsyncAdohandler();

		public:
			/// <summary>
			/// Call the methods EndLoadData and BeginLoadData one time for loaded records during fetching records.
			/// This method is usually called at client side for reduction of latency and fast displaying beginning records.
			/// </summary>
			void FinalizeRecords();

			/// <summary>
			/// Send a DataTable object onto a remote host. 
			/// </summary>
			bool Send(DataTable ^dt);
			
			/// <summary>
			/// Send a DataTable object onto a remote host. Parameter nBatchSize is defaulted to 10240, which controls network communication batch size in byte.
			/// </summary>
			virtual bool Send(DataTable ^dt, int nBatchSize);

			/// <summary>
			/// Send a DataSet object onto a remote host.
			/// </summary>
			bool Send(DataSet ^ds);
			
			/// <summary>
			/// Send a DataSet object onto a remote host.
			/// The parameter bNeedRelations is defaulted to false.
			/// If the parameter bNeedRelations is set to true, relationships among tables are remoted.
			/// </summary>
			bool Send(DataSet ^ds, bool bNeedRelations);

			/// <summary>
			/// Send a DataSet object onto a remote host.
			/// The parameter bNeedRelations is defaulted to false.
			/// If the parameter bNeedRelations is set to true, relationships among tables are remoted.
			/// By default, nBatchSize is 10240.
			/// </summary>
			virtual bool Send(DataSet ^ds, bool bNeedRelations, int nBatchSize);
			
			/// <summary>
			/// Send a set of records through an interface IDataReader onto a remote host.
			/// If proper, this method is preferred for reduction of memory footprint and latency without pre-loading data records into memory.
			/// </summary>
			bool Send(IDataReader ^dr);
			
			/// <summary>
			/// Send a set of records through an interface IDataReader onto a remote host.
			/// The parameter nBatchSize controls network communication batch size in byte and it is defaulted to 10240.
			/// If proper, this method is preferred for reduction of memory footprint and latency without pre-loading data records into memory.
			/// </summary>
			virtual bool Send(IDataReader ^dr, int nBatchSize);

		protected:
			virtual void OnResultReturned(short sRequestID, CUQueue ^UQueue) override;

		private:
			bool EndDataSet(DataSet ^ds, bool bNeedRelations);

		protected:
			CAsyncAdoSerializationHelper ^m_AdoSerialier;
		};
	}
}
