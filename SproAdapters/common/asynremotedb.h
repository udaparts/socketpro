#ifndef ___ASYN_REMOTE_DB_HANDLER_H__
#define ___ASYN_REMOTE_DB_HANDLER_H__

#ifndef __DONT_NEED_REMOTE_DB_SERVICE__

#include "udb.h"
using namespace System::Data;
using namespace System::Collections;
using namespace System::Threading;
using namespace System::Windows::Forms;

namespace SocketProAdapter
{
namespace ClientSide
{
namespace RemoteDB
{

[CLSCompliantAttribute(true)] 
public delegate void DOutputDataCome(array<Object^> ^arrayOutput);

[CLSCompliantAttribute(true)] 
public delegate void DTableOpened(DataTable ^dt);

[CLSCompliantAttribute(true)] 
public delegate void DFetchingData(DataTable ^dt, array<DataRow^> ^arrayRow);

[CLSCompliantAttribute(true)]
public value struct SchemaGuid
{
public:
	static initonly Guid DBSCHEMA_ASSERTIONS = Guid("{0xc8b52210,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_CHARACTER_SETS = Guid("{0xc8b52212,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_COLLATIONS = Guid("{0xc8b52213,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_COLUMNS = Guid("{0xc8b52214,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_CHECK_CONSTRAINTS = Guid("{0xc8b52215,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_CONSTRAINT_COLUMN_USAGE = Guid("{0xc8b52216,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_CONSTRAINT_TABLE_USAGE = Guid("{0xc8b52217,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_KEY_COLUMN_USAGE = Guid("{0xc8b52218,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_REFERENTIAL_CONSTRAINTS = Guid("{0xc8b52219,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_TABLE_CONSTRAINTS = Guid("{0xc8b5221a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_COLUMN_DOMAIN_USAGE = Guid("{0xc8b5221b,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_INDEXES = Guid("{0xc8b5221e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_COLUMN_PRIVILEGES = Guid("{0xc8b52221,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_TABLE_PRIVILEGES = Guid("{0xc8b52222,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_USAGE_PRIVILEGES = Guid("{0xc8b52223,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_PROCEDURES = Guid("{0xc8b52224,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_SCHEMATA = Guid("{0xc8b52225,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_SQL_LANGUAGES = Guid("{0xc8b52226,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_STATISTICS = Guid("{0xc8b52227,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_TABLES = Guid("{0xc8b52229,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_TRANSLATIONS = Guid("{0xc8b5222a,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_PROVIDER_TYPES = Guid("{0xc8b5222c,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_VIEWS = Guid("{0xc8b5222d,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_VIEW_COLUMN_USAGE = Guid("{0xc8b5222e,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_VIEW_TABLE_USAGE = Guid("{0xc8b5222f,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_PROCEDURE_PARAMETERS = Guid("{0xc8b522b8,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_FOREIGN_KEYS = Guid("{0xc8b522c4,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_PRIMARY_KEYS = Guid("{0xc8b522c5,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_PROCEDURE_COLUMNS = Guid("{0xc8b522c9,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_TABLE_STATISTICS = Guid("{0xc8b522ff,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
	static initonly Guid DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE = Guid("{0xc8b52301,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}}");
};

[CLSCompliantAttribute(true)] 
public ref class CAsynDBException : public Exception
{
public:
	CAsynDBException(int hr, String ^strMessage) : Exception(strMessage)
	{
		HResult = hr;
	}

public:
	property int HResult
	{
		int get() new
		{
			return (int)Exception::HResult;
		}
	}
};

[CLSCompliantAttribute(true)] 
public ref class CDBRequestError
{
public:
	CDBRequestError()
	{
		m_nRequestID = (UDBLib::tagDBRequestID)0;
		m_nErrorCode = 0;
		m_strErrorMsg = nullptr;
	}
	CDBRequestError(UDBLib::tagDBRequestID sRequestID, int nErrorCode, String ^strErrorMsg)
	{
		m_nRequestID = sRequestID;
		m_nErrorCode = nErrorCode;
		m_strErrorMsg = strErrorMsg;
	}
public:
	UDBLib::tagDBRequestID		m_nRequestID;
	int							m_nErrorCode;
	String						^m_strErrorMsg;
};

[CLSCompliantAttribute(true)] 
public ref class CParamInfo
{
public:
	CParamInfo()
	{
		m_sDBType = UDBLib::tagSockDataType::sdVT_EMPTY;
		m_nParamIO = UDBLib::tagSockDBParamType::sdParamInput;
		m_nLen = 0;

//		m_bPrecision = 0;
//		m_bScale = 0;
	}

	CParamInfo(UDBLib::tagSockDataType sDBType)
	{
		m_sDBType = sDBType;
		m_nParamIO = UDBLib::tagSockDBParamType::sdParamInput;
		m_nLen = 0;

//		m_bPrecision = 0;
//		m_bScale = 0;
	}

	CParamInfo(UDBLib::tagSockDataType sDBType, UDBLib::tagSockDBParamType nParamIO)
	{
		m_sDBType = sDBType;
		m_nParamIO = nParamIO;
		m_nLen = 0;

//		m_bPrecision = 0;
//		m_bScale = 0;
	}

	CParamInfo(UDBLib::tagSockDataType sDBType, UDBLib::tagSockDBParamType nParamIO, int nLen)
	{
		m_sDBType = sDBType;
		m_nParamIO = nParamIO;
		m_nLen = nLen;

//		m_bPrecision = 0;
//		m_bScale = 0;
	}

public:
	UDBLib::tagSockDataType		m_sDBType;
	UDBLib::tagSockDBParamType	m_nParamIO;		//1 - input, 2 - output, 3 - input/output
	
	/// <summary>
	/// Parameter max data length in byte. It is used only for data types sdVT_STR, sdVT_BYTES, and sdVT_WSTR. -1 means that the parameter would be a large BLOB or text. For other data types, it is ignored.
	/// </summary>
	int							m_nLen;			//in byte

//	BYTE						m_bPrecision;
//	BYTE						m_bScale;
};

/// <summary>
/// This class is a wrapper of key features in the file udb.dll, 
/// which simplifies accessing a remote backend database from win32 or MS device platforms.
/// With the help of the class, you can use one piece of code to access all of data sources through MS OLEDB or MS ODBC technology.
/// </summary>
[CLSCompliantAttribute(true)] 
public ref class CAsynDBLite : public CAsyncServiceHandler //, public IAsyncResult
{
public:
	CAsynDBLite();
	CAsynDBLite(CClientSocket ^cs);
	CAsynDBLite(CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler);
	virtual ~CAsynDBLite();

public:
	/// <summary>
	/// A readonly rowset.
	/// </summary>
	static const int Readonly = 2;

	/// <summary>
	/// Use delay mode to update rowset.
	/// </summary>
	static const int UseDelayUpdate = 4;

	/// <summary>
	/// A scrollable rowset that supports two methods LastBatch and PrevBatch.
	/// </summary>
	static const int Scrollable = 8;

	/// <summary>
	/// A rowset with bookmark support.
	/// </summary>
	static const int UseBookmark = 16;

	/// <summary>
	/// Use MS OLEDD client cursor engine.
	/// </summary>
	static const int UseClientCursorEngine = 32;

	/// <summary>
	/// Automatically retrieve the latest record inserted from a rwoset.
	/// </summary>
	static const int RetrieveServerDataOnInsert = 64;

	/// <summary>
	/// Enable quick rowset search on server side.
	/// </summary>
	static const int EnableQuickFind = 0x20000000;

	/// <summary>
	/// The fastest way to bring a rowset data from remote SocketPro server to client.
	/// Once the whole rowset is fetched, the server cursor and rowset resources are released.
	/// </summary>
	static const int AsynFetch = 0x40000000;

public:
	//virtual int GetSvsID() override;
	virtual bool Attach(CClientSocket ^pClientSocket) override;
	virtual void Detach() override;

public:
	/// <summary>
	/// Connect to a remote DB through a remote SocketPro server in middle with MS OLEDB technology. Note that remote database service may use a global OLEDB connection string instead if the input strOLEDBConnString is null or empty string.
	/// </summary>
	/// <param name="strOLEDBConnString">A connection string to a backend database</param>
	virtual void ConnectDB(String ^strOLEDBConnString);
	
	/// <summary>
	/// Disconnect the connection to a backend database.
	/// </summary>
	virtual void DisconnectDB();
	
	/// <summary>
	/// Start a manual transaction with read committed isoletion level.
	/// </summary>
	virtual void BeginTrans();

	/// <summary>
	/// Start a manual transaction with a given isoletion level, nIsolationLevel.
	/// </summary>
	virtual void BeginTrans(UDBLib::tagIsolationLevel nIsolationLevel);
	
	/// <summary>
	/// Commit a manual transaction.
	/// </summary>
	virtual void Commit();

	/// <summary>
	/// Abort a manual transaction.
	/// </summary>
	virtual void Rollback();

	/// <summary>
	/// Send a matrix (multiple sets) of parameter data onto a backend database through a parameterized statement. 
	/// You can use the method for fast add, update and delete as well as executing store procedures.
	/// </summary>
	virtual void DoBatch(array<Object^> ^aParameterData);
	
	/// <summary>
	/// Open a command with a parameterized statement using an array of parameter infos. 
	/// Later, you can use the method DoBatch to execute the parameterized statement in batch
	/// </summary>
	virtual void OpenCommandWithParameters(String ^strParameterizedStatement, List<CParamInfo^> ^lstParamInfo);
	
	/// <summary>
	/// Open a command with a parameterized statement using an array of parameter infos. 
	/// Later, you can use the method DoBatch to execute the parameterized statement in batch, or call the method OpenRowsetFromParameters for a rowset.
	/// </summary>
	virtual void OpenCommandWithParameters(String ^strParameterizedStatement, List<CParamInfo^> ^lstParamInfo, bool bPrepare);
	
	/// <summary>
	/// Open a rowset from one array (one set) of parameter data
	/// </summary>
	virtual void OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName);
	
	/// <summary>
	/// Execute a SQL statement for a rowset. If the rowset is opened with the hint AsynFetch, a rowset will be asynchronously fetched at the fastest speed and be closed afterwards. Otherwise, the rowset will be fetched with the first batch of records.
	/// </summary>
	/// <param name="aParameterData"><c>aParameterData</c> is one set of parameter data.</param>
	/// <param name="strTableName"><c>strTableName</c> is a table name.</param>
	/// <param name="ct"><c>ct</c> is a type of cursor (ctForwardOnly, ctStatic, ctKeyset, ctDynamic).</param>
	/// <param name="nHints"><c>nHints</c> represents a set of rowset properties like Readonly, Scrollable, AsynFetch, ......, and so on.</param>
	/// <param name="sBatchSize"><c>sBatchSize</c> is the number of records per batch. If it is 0, remote database service will automatically determine the size.</param>
	/// <param name="nRowsExpected"><c>nRowsExpected</c> is a given number of records to be fetched. -1 means fetching all of records.</param>
	virtual void OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName, UDBLib::tagCursorType ct, int nHints, short sBatchSize, int nRowsExpected);
	
	/// <summary>
	/// Execute a SQL statement without opening a rowset
	/// </summary>
	virtual void ExecuteNonQuery(String ^strSQL);
	
	/// <summary>
	/// Execute a SQL statement for a readonly rowset. Fetch all of records at the fastest speed and close rowset afterwards
	/// </summary>
	virtual void OpenRowset(String ^strSQL, String ^strTableName);

	/// <summary>
	/// Execute a SQL statement for a rowset. If the rowset is opened with the hint AsynFetch, a rowset will be asynchronously fetched at the fastest speed and be closed afterwards. Otherwise, the rowset will be fetched with the first batch of records.
	/// </summary>
	/// <param name="strSQL"><c>strSQL</c> is a SQL query statement.</param>
	/// <param name="strTableName"><c>strTableName</c> is a table name.</param>
	/// <param name="ct"><c>ct</c> is a type of cursor (ctForwardOnly, ctStatic, ctKeyset, ctDynamic).</param>
	/// <param name="nHints"><c>nHints</c> represents a set of rowset properties like Readonly, Scrollable, AsynFetch, ......, and so on.</param>
	/// <param name="sBatchSize"><c>sBatchSize</c> is the number of records per batch. If it is 0, remote database service will automatically determine the size.</param>
	/// <param name="nRowsExpected"><c>nRowsExpected</c> is a given number of records to be fetched. -1 means fetching all of records.</param>
	virtual void OpenRowset(String ^strSQL, String ^strTableName, UDBLib::tagCursorType ct, int nHints, short sBatchSize, int nRowsExpected);
	
	/// <summary>
	/// Open a meta rowset specified by guidSchema with an array of given restrictions arrRestrictions. 
	/// Refer to MS OLEDB documentation OLE DB Programmer's Reference\Part 5: Appendixes\Appendix B: Schema Rowsets.
	/// </summary>
	virtual void OpenSchema(Guid guidSchema, array<Object^> ^arrRestrictions);
	
	/// <summary>
	/// Start to batch a set of requests by calling the method CClientSocket::BeginBatching. 
	/// </summary>
	virtual bool BeginBatch();

	/// <summary>
	/// Commit a set of requests with a given callback cb by calling the method CClientSocket::Commit. 
	/// </summary>
	virtual bool CommitBatch(AsyncCallback ^cb);

	/// <summary>
	/// Abort a set of requests by calling the method CClientSocket::Rollback. 
	/// </summary>
	virtual bool AbortBatch();

	/// <summary>
	/// Get the first batch of records
	/// </summary>
	virtual void FirstBatch();

	/// <summary>
	/// Get the last batch of records. The method may fail if an opened rowset is not scrollable.
	/// </summary>
	virtual void LastBatch();

	/// <summary>
	/// Get the next batch of records with a specified number of batch records skipped. 
	/// If the input lSkip is less than -1, the method may fail if an opened rowset is not scrollable.
	/// </summary>
	virtual void NextBatch(long lSkip);

	/// <summary>
	/// Get the next batch of records
	/// </summary>
	virtual void NextBatch();

	/// <summary>
	/// Get the previous batch of records. The call is exactly equal to the call MoveNext(-2).
	/// </summary>
	virtual void PrevBatch();
	
	/// <summary>
	/// Delete a record from a given 0-based index nRowIndex.
	/// </summary>
	virtual void DeleteRow(int nRowIndex);

	/// <summary>
	/// Update a record with with a new value objNewValue. All of indices are 0-based.
	/// </summary>
	virtual void Update(int nRowIndex, int nColIndex, Object ^objNewValue);
	
	/// <summary>
	/// Add a record onto a database. The input aRow must contains an array of proper data, which must match rowset data types.
	/// </summary>
	virtual void AddRecord(array<Object^> ^aRow);

	/// <summary>
	/// Add a record onto a database. The input aRow must contains an array of proper data, which must match rowset data types.
	/// If the input parameter bNeedNewRecord is set true and RetrieveServerDataOnInsert is included with rowset opening hint, you will get the latest added rowset immediately.
	/// </summary>
	virtual void AddRecord(array<Object^> ^aRow, bool bNeedNewRecord);

public:
	UDBLib::UDataSourceClass^ GetDataSource();
	UDBLib::USessionClass^ GetSession();
	UDBLib::UCommandClass^ GetCommand();
	UDBLib::URowsetClass^ GetRowset();

public:
/*
	property bool IsCompleted
	{
		virtual bool get()
		{
			if (m_pClientSocket == nullptr)
				return true;
			long lCount = 0;
			IUSocket *pIUSocket = (IUSocket *)m_pClientSocket->GetIUSocket().ToPointer();
			if(pIUSocket == NULL)
				return true;
			pIUSocket->get_CountOfRequestsInQueue(&lCount);
			return (lCount == 0);
		}
	}

	property Object^ AsyncState
	{
		virtual Object^ get()
		{
			return nullptr;
		}
	}

	property WaitHandle^ AsyncWaitHandle
	{
		virtual WaitHandle^ get()
		{
			return nullptr;
		}
	}

	property bool CompletedSynchronously
	{
		virtual bool get()
		{
			return false;
		}
	}*/

public:
	/// <summary>
	/// The property indicates if there is a database connection from a remote SocketPro server to a backend database.
	/// </summary>
	property bool DBConnected
	{
		bool get()
		{
			long lDSHandle = 0;
			long lSessionHandle = 0;
			if(m_pIUDataSource != NULL)
			{
				m_pIUDataSource->get_Handle(&lDSHandle);
			}
			if(m_pIUSession != NULL)
			{
				m_pIUSession->get_Handle(&lSessionHandle);
			}
			return (lDSHandle != 0 && lSessionHandle != 0);
		}
	}
	
	/// <summary>
	/// The property indicates if the current rowset is updateable.
	/// </summary>
	property bool IsRowsetReadonly
	{
		bool get()
		{
			if(m_pIURowset == NULL)
				return false;
			VARIANT_BOOL bReadOnly;
			m_pIURowset->IsReadOnly(&bReadOnly);
			return (bReadOnly != VARIANT_FALSE);
		}
	}

	/// <summary>
	/// The property indicates if the current rowset is opened.
	/// </summary>
	property bool IsRowsetOpened
	{
		bool get()
		{
			if(m_pIURowset == NULL)
				return false;
			long lHandle;
			m_pIURowset->get_Handle(&lHandle);
			return (lHandle != 0);
		}
	}

	/// <summary>
	/// The property indicates if the current rowset is scrollable.
	/// </summary>
	property bool IsRowsetScrollable
	{
		bool get()
		{
			if(!IsRowsetOpened)
				return false;
			CAutoLock AutoLock(&m_pCS->m_sec);
			if(m_RowsetPropertyInfo[0] != nullptr && m_RowsetPropertyInfo[0]->GetType() == bool::typeid)
			{
				return (bool)(m_RowsetPropertyInfo[0]);
			}
			return false;
		}
	}

	/// <summary>
	/// The property indicates if the current rowset is bookmarkable.
	/// </summary>
	property bool IsRowsetBookmarkable
	{
		bool get()
		{
			if(!IsRowsetOpened)
				return false;
			CAutoLock AutoLock(&m_pCS->m_sec);
			if(m_RowsetPropertyInfo[1] != nullptr && m_RowsetPropertyInfo[1]->GetType() == bool::typeid)
			{
				return (bool)(m_RowsetPropertyInfo[1]);
			}
			return false;
		}
	}

	/// <summary>
	/// The property indicates if the number of records are fetched.
	/// </summary>
	property int RowsFetched
	{
		int get()
		{
			if(!IsRowsetOpened)
				return 0;
			CAutoLock AutoLock(&m_pCS->m_sec);
			short sObtained;
			m_pIURowset->GetRowsFetched(&sObtained);
			return (unsigned short)sObtained;
		}
	}
	
	/// <summary>
	/// The property indicates a list of error records. The first one is usually the most important one if available.
	/// </summary>
	property List<CDBRequestError^>^ DBErrors
	{
		List<CDBRequestError^>^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_lstError;
		}
	}

	/// <summary>
	/// The property indicates a dataset containing a list of datatables created by executing a list of sql queries.
	/// </summary>
	property System::Data::DataSet^ DataSet
	{
		System::Data::DataSet^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_DataSet;
		}
	}

	/// <summary>
	/// The property indicates the current datatable created by executing a SQL query statement.
	/// </summary>
	property System::Data::DataTable^ CurrentDataTable
	{
		System::Data::DataTable^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_CurrentTable;
		}
	}
	
	/// <summary>
	/// The property indicates the name of OLEDB provider used by remote database service at a SocketPro server.
	/// </summary>
	property String^ DBProvider
	{
		String^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_DataSourceInfo[1];
		}
	}
	
	/// <summary>
	/// The property indicates the name of connected database.
	/// </summary>
	property String^ Database
	{
		String^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_DataSourceInfo[3];
		}
	}
	
	/// <summary>
	/// The property indicates the name of backend database server.
	/// </summary>
	property String^ DBDataSource
	{
		String^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_DataSourceInfo[2];
		}
	}
	
	/// <summary>
	/// The property indicates the version of backend database server.
	/// </summary>
	property String^ DBServerVersion
	{
		String^ get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return m_DataSourceInfo[0];
		}
	}
	
	/// <summary>
	/// The property indicates if setting multiple sets of parameters is possible.
	/// </summary>
	property bool Batchable
	{
		bool get()
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			return ((String^)m_DataSourceInfo[5] == "True");
		}
	}

	/// <summary>
	/// The property indicates the sub-batch size. By default, it is zero. 
	/// When setting the property to a small number, it will reduce records data arrival latency for low bandwidth networks.
	/// If a network bandwidth is high, you should default this property to zero.
	/// </summary>
	property short SubBatchSize
	{
		short get()
		{
			return (short)m_sSubBatchSize;
		}
		void set(short sSubBatchSize)
		{
			m_sSubBatchSize = (unsigned short)sSubBatchSize;
		}
	}

protected:
	virtual void OnResultReturned(short sRequestID, CUQueue ^UQueue) override;
	virtual void OnRDBResult(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib::tagReturnFlag ReturnFlag);

private:
	array<DataRow^>^ AddRows(short sRequestID);
	void OnSocketClosed(int hSocket, int nError);
	bool HasOutput();
	DataTable^ CreateEmptyDataTable(String ^strTableName);
	int GetColNameIndex(DataTable ^dt, String ^strColName);
	void HandleOneBlob();
	bool IsBlob(int nCol);

public:
	DTableOpened		^m_OnTableOpened;
	DFetchingData		^m_OnFetchingData;
	DOutputDataCome		^m_OnOutputDataCome;

private:
	UDBLib::UDataSourceClass	^m_DataSource;
	UDBLib::USessionClass		^m_Session;
	UDBLib::UCommandClass		^m_Command;
	UDBLib::URowsetClass		^m_Rowset;
	System::Data::DataSet		^m_DataSet;
	array<Object^>				^m_aData;
	array<short>				^m_aDBType;
	array<String^>				^m_DataSourceInfo;
	array<Object^>				^m_RowsetPropertyInfo;
	System::Data::DataTable		^m_CurrentTable;
	int							m_nIndex;
	CComAutoCriticalSection		*m_pCS;
	List<String^>				^m_lstTableNames;
	List<CDBRequestError^>		^m_lstError;
	unsigned short				m_sSubBatchSize;
	int							m_nBlobIndex;
	unsigned short				m_sBatchIndex;
	List<int>					^m_lstBlobColumns;

internal:
	bool						m_bAsynFetching;
	IUDataSource				*m_pIUDataSource;
	IUSession					*m_pIUSession;
	IUCommand					*m_pIUCommand;
	IURowset					*m_pIURowset;
	IUDataReader				*m_pIUDataReader;
};

[CLSCompliantAttribute(true)] 
public ref class CAsynDBLiteEx : public CAsynDBLite
{
public:
	CAsynDBLiteEx();
	CAsynDBLiteEx(CClientSocket ^cs);
	CAsynDBLiteEx(CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler);
	virtual ~CAsynDBLiteEx();

public:
	/// <summary>
	/// Open an updateable rowset from one array (one set) of parameter data with the first batch of records
	/// </summary>
	virtual void OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName) override;
	
	/// <summary>
	/// Execute a SQL statement for an updateable rowset with the first batch of records
	/// </summary>
	virtual void OpenRowset(String ^strSQL, String ^strTableName) override;
	
	/// <summary>
	/// Delete a record indicated by a given data grid view object dgvRow.
	/// </summary>
	virtual void DeleteRow(DataGridViewRow ^dgvRow);
	
	/// <summary>
	/// Update a record from a given data grid view cell object dgvCell.
	/// </summary>
	virtual void Update(DataGridViewCell ^dgvCell);

	/// <summary>
	/// Add a record onto a database from a given data grid view row object dgvRow.
	/// </summary>
	virtual void AddRecord(DataGridViewRow ^dgvRow);

	/// <summary>
	/// Add a record onto a database from a given data grid view row object dgvRow.
	/// If the input parameter bNeedNewRecord is set true and RetrieveServerDataOnInsert is included with rowset opening hint, you will get the latest added rowset immediately.
	/// </summary>
	virtual void AddRecord(DataGridViewRow ^dgvRow, bool bNeedNewRecord);
	
	/// <summary>
	/// Get the index of a batch of rowset records from a given data grid view row index nGridViewRowIndex. It can return -1, an invalid index.
	/// </summary>
	int GetRecordRowIndex(int nGridViewRowIndex);

public:
	/// <summary>
	/// Attach or detach a DataGridView object with CurrentDataTable
	/// </summary>
	property DataGridView^ AttachedDataGridView
	{
		DataGridView^ get()
		{
			return m_dgv;
		}
		void set(DataGridView ^dgv)
		{
			RemoveDGVDelegates();
			m_dgv = dgv;
			if(m_dgv != nullptr)
			{
				bool b = IsRowsetReadonly;
				m_dgv->AllowUserToAddRows = (!b);
				m_dgv->AllowUserToDeleteRows = (!b);
			}
			AddDGVDelegates();
		}
	}

protected:
	//You may override the following virtual functions
	virtual void OnResultReturned(short sRequestID, CUQueue ^UQueue) override;
	virtual void OnDataError(Object^ sender, DataGridViewDataErrorEventArgs^ e);
	virtual void OnCellValueChanged(Object^ sender, DataGridViewCellEventArgs^ e);
	virtual void OnCellClick(Object ^sender, DataGridViewCellEventArgs ^e);
	virtual void OnKeyDown(Object^ sender, KeyEventArgs ^e);
	virtual void OnUserDeletingRow(Object^ sender, DataGridViewRowCancelEventArgs^ e);
	virtual void OnRowEnter(Object^ sender, DataGridViewCellEventArgs^ e);

private:
	void RemoveDGVDelegates();
	void AddDGVDelegates();
	int GetErrorColumn(String ^strErrorText);
	void RemovePrevErrors();

private:
	DataGridView	^m_dgv;
	bool			m_bInternalChanging;
	unsigned int	m_nStart;
};

}
}
}
#endif

#endif