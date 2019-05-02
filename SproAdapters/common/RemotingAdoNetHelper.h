#pragma once

using namespace System;
using namespace System::Data;
using namespace System::Collections;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace SocketProAdapter
{
	private enum class tagDataTypeSupported : short
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
		dtValue,
		dtValues,
		dtNull,
		dtTimeSpan,
		dtDateTimeOffset = 200,
	};
	private enum class tagColumnBit : int
	{
		cbAllowDBNull = 1,
		cbIsAutoIncrement = 2,
		cbIsReadOnly = 4,
		cbIsUnique = 8,
		cbIsKey = 16,
		cbIsLong = 32,
		cbIsRowVersion = 64,
	};

#ifndef __NO_OPEN_MP__
	private ref class CPRecordContext
	{
	public:
		CUQueue						^m_UQueue;
		List<array<Object^>^>		^m_lstRow;
	};

	private ref class CPDataRowContext : CPRecordContext
	{
	public:
		List<DataRowState>	^m_lstState;
		List<String^>		^m_lstError;
	};

#endif

	public ref class CAsyncAdoSerializationHelper
	{
	public:
		literal short idDataTableHeaderArrive = 0x7FFF;
		literal short idDataTableRowsArrive = 0x7FFE;
		literal short idDataSetHeaderArrive = 0x7FFD;
		literal short idEndDataSet = 0x7FFA;
		literal short idDataReaderHeaderArrive = 0x7FFC;
		literal short idEndDataReader = 0x7FFB;
		literal short idDataReaderRecordsArrive = 0x7FF9;
		literal short idEndDataTable = 0x7FF8;

	internal:
		CAsyncAdoSerializationHelper();
		
		/// <summary>
		/// Construct an instance of CAsyncAdoSerializationHelper and extract field data type information from an existing instance of CAsyncAdoSerializationHelper if it is available.
		/// </summary>
		CAsyncAdoSerializationHelper(CAsyncAdoSerializationHelper ^AsyncAdoSerializationHelper);

		/// <summary>
		/// Construct an instance of CAsyncAdoSerializationHelper and extract field data type information from the parameter dt if a DataTable object is available.
		/// </summary>
		CAsyncAdoSerializationHelper(DataTable ^dt);

		/// <summary>
		/// Construct an instance of CAsyncAdoSerializationHelper and extract field data type information from the parameter dr if a DataReader object is available.
		/// </summary>
		CAsyncAdoSerializationHelper(IDataReader ^dr);
		virtual ~CAsyncAdoSerializationHelper();

	internal:
		void Load(short sRequestID, CUQueue ^UQueue);

	public:
		/// <summary>
		/// Call the methods EndLoadData and BeginLoadData one time for loaded records during fetching records.
		/// This method is usually called at client side for reduction of latency and fast displaying beginning records.
		/// </summary>
		void FinalizeRecords();

		/// <summary>
		/// The size of a batch of records in byte.
		/// </summary>
		property int BatchSize
		{
			int get()
			{
				return m_nBatchSize;
			}
		}

		/// <summary>
		/// The datatable that is being fetched or has been just transferred.
		/// </summary>
		property DataTable^ CurrentDataTable
		{
			DataTable^ get()
			{
				return m_dt;
			}
		}

		/// <summary>
		/// The DataSet that is being fetched or has been just transferred.
		/// </summary>
		property DataSet^ CurrentDataSet
		{
			DataSet^ get()
			{
				return m_ds;
			}
		}

		/// <summary>
		/// The number of rows, records or tables affected.
		/// </summary>
		property int Affected
		{
			int get()
			{
				return m_nAffected;
			}
		}

		property bool LoadingDataTable
		{
			bool get()
			{
				return m_bLoadingDataTable;
			}
		}

		property bool LoadingDataSet
		{
			bool get()
			{
				return m_bDataSet;
			}
		}
		
		property bool LoadingDataReader
		{
			bool get()
			{
				return m_bDataReader;
			}
		}

		/// <summary>
		/// Indicates if loading in parallel is enabled.
		/// </summary>
		property bool LoadingInParallel
		{
			bool get()
			{
				return m_bParallel;
			}
		}


#ifndef __NO_OPEN_MP__
	private:
		/// <summary>
		/// Turn on loading data in parallel if a PC has a multi-core CPU. If a PC does not have a multi-core CPU, the call does nothing.
		/// Note this feature may help an application performance only if a single thread does not have enough power for loading data. 
		/// </summary>
		void StartLoadingInParallel();
		
		/// <summary>
		/// Turn off loading data in parallel. If there is data in memory queue to be loaded, calling the method will load them.
		/// </summary>
		int EndLoadingInParallel();

		int PopDataTableRow(CUQueue ^UQueue, array<Object^> ^%aData, DataRowState %drs, String ^%strError);
		static CUQueue ^LockUQueue();
		static void UnlockUQueue(CUQueue ^UQueue);
		int LoadDataTableRows(List<CUQueue^> ^lstUQueue);
		int LoadDataReaderRecords(List<CUQueue^> ^lstUQueue);
		static System::Collections::Generic::Stack<CUQueue^> ^m_sMQ = gcnew System::Collections::Generic::Stack<CUQueue^>();
		List<CUQueue^> ^m_lstUQueue;
#endif
		
	internal:
		void Push(CUQueue ^UQueue, IDataReader ^dr);
		void Push(CUQueue ^UQueue, DataRow ^dr);
		void PushHeader(CUQueue ^UQueue, DataTable ^dt, bool bNeedParentRelations, bool bNeedChildRelations);
		void PushHeader(CUQueue ^UQueue, IDataReader ^dr);
		void PushHeader(CUQueue ^UQueue, DataSet ^ds);
		void Push(CUQueue ^UQueue, DataRelationCollection ^drc);
		
	private:
		int Pop(CUQueue ^UQueue, DataColumn ^%dc);
		int Pop(CUQueue ^UQueue, array<DataColumn^> ^%dcs);
		int Pop(CUQueue ^UQueue, ForeignKeyConstraint ^%fkc);
		int Pop(CUQueue ^UQueue, UniqueConstraint ^%uc);
		int Pop(CUQueue ^UQueue, DataRelationCollection ^%drc);
		int PopTableColNamesOnly(CUQueue ^UQueue, array<DataColumn^> ^%dcs);
		void PushTableColNamesOnly(CUQueue ^UQueue, array<DataColumn^> ^dcs);
		void Push(CUQueue ^UQueue, DataColumn ^dc);
		void Push(CUQueue ^UQueue, array<DataColumn^> ^dcs);
		void Push(CUQueue ^UQueue, ForeignKeyConstraint ^fkc);
		void Push(CUQueue ^UQueue, UniqueConstraint ^uc);
		int Pop(CUQueue ^UQueue, DataColumnCollection ^%Cols);
		void Push(CUQueue ^UQueue, DataColumnCollection ^Cols);
		int CountNames(ArrayList ^al, System::String ^strColName);
		static tagDataTypeSupported GetDT(Type ^type);
		static Type ^GetType(tagDataTypeSupported dt);
		static tagDataTypeSupported GetDT(System::String ^strType);
		array<tagDataTypeSupported>	^m_dts;
		void RemoveAAU();
		void AddAAU();
		DataRelationCollection ^LoadDataSetRelations(CUQueue ^UQueue);
		int LoadDataTableRows(CUQueue ^UQueue);
		int LoadDataReaderRecords(CUQueue ^UQueue);
		DataSet^ LoadDataSetHeader(CUQueue ^UQueue);
		DataTable^ LoadDataTableHeader(CUQueue ^UQueue);
		DataTable^ LoadDataReaderHeader(CUQueue ^UQueue);
		int LoadRows(CUQueue ^UQueue);
		int PopDataRecord(CUQueue ^UQueue, array<System::Object^> ^%aData);
		int Pop(CUQueue ^UQueue, array<Object^> ^%aData, DataRowState %drs);

	private:
		int					m_nAffected;
		bool				m_bLoadingDataTable;
		int					m_nBatchSize;
		CUQueue				^m_qBit;
		CUQueue				^m_qTemp;
		array<System::Byte> ^m_Buffer;
		bool				m_bDataSet;
		DataTable			^m_dtBackup;
		DataSet				^m_ds;
		DataTable			^m_dt;
		bool				m_bDataReader;
		bool				m_bParallel;
	};
}
