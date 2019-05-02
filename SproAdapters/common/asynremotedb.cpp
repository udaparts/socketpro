#include "stdafx.h"

#ifndef __DONT_NEED_REMOTE_DB_SERVICE__

#include "asynremotedb.h"

//#define DBINITCONSTANTS
//#include <oledb.h>

const GUID DBPROPSET_DATASOURCE = {0xc8b522ba,0x5cf3,0x11ce,{0xad,0xe5,0x00,0xaa,0x00,0x44,0x77,0x3d}};

namespace SocketProAdapter
{
namespace ClientSide
{
namespace RemoteDB
{

class CField
{
public:
	ULONG	m_ulLen;
	BYTE	*m_pData;
	bool	m_bModified;
	bool	m_bVT;
};

CAsynDBLite::CAsynDBLite() : CAsyncServiceHandler(sidOleDB)
{
	m_sSubBatchSize = 0;
	m_nIndex = 0;
	m_pIUDataSource = NULL;
	m_pIUSession = NULL;
	m_pIUCommand = NULL;
	m_pIURowset = NULL;
	m_pIUDataReader = NULL;

	m_OnTableOpened = nullptr;
	m_OnFetchingData = nullptr;

	m_DataSource = nullptr;
	m_Session = nullptr;
	m_Command = nullptr;
	m_Rowset = nullptr;

	m_pCS = new CComAutoCriticalSection();

	m_lstError = gcnew List<CDBRequestError^>();
	m_lstTableNames = gcnew List<String^>();
	m_DataSet = gcnew System::Data::DataSet();
	m_DataSet->EnforceConstraints = false;
	m_CurrentTable = gcnew System::Data::DataTable();
	m_DataSourceInfo = gcnew array<String^>(6);
	m_RowsetPropertyInfo = gcnew array<Object^>(2);

	m_lstBlobColumns = gcnew List<int>();

	IUDataSource *pIUDataSource = NULL;
	HRESULT hr = ::CoCreateInstance(__uuidof(UDataSource), NULL, CLSCTX_ALL, __uuidof(IUDataSource), (void**)&pIUDataSource);
	if(FAILED(hr))
	{
#ifdef _WIN64
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x64) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#else
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x32) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#endif
	}
	m_pIUDataSource = pIUDataSource;

	IUSession *pIUSession = NULL;
	hr = ::CoCreateInstance(__uuidof(USession), NULL, CLSCTX_ALL, __uuidof(IUSession), (void**)&pIUSession);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUSession = pIUSession;

	IUCommand *pIUCommand = NULL;
	hr = ::CoCreateInstance(__uuidof(UCommand), NULL, CLSCTX_ALL, __uuidof(IUCommand), (void**)&pIUCommand);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUCommand = pIUCommand;

	IURowset *pIURowset = NULL;
	hr = ::CoCreateInstance(__uuidof(URowset), NULL, CLSCTX_ALL, __uuidof(IURowset), (void**)&pIURowset);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIURowset = pIURowset;
	
	IUDataReader *pIUDataReader = NULL;
	hr = m_pIURowset->QueryInterface(__uuidof(IUDataReader), (void**)&pIUDataReader);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll version is too old!"));
	m_pIUDataReader = pIUDataReader;
}

CAsynDBLite::CAsynDBLite(CClientSocket ^cs) : CAsyncServiceHandler(sidOleDB, cs)
{
	m_sSubBatchSize = 0;
	m_nIndex = 0;
	m_pIUDataSource = NULL;
	m_pIUSession = NULL;
	m_pIUCommand = NULL;
	m_pIURowset = NULL;
	m_pIUDataReader = NULL;

	m_OnTableOpened = nullptr;
	m_OnFetchingData = nullptr;

	m_DataSource = nullptr;
	m_Session = nullptr;
	m_Command = nullptr;
	m_Rowset = nullptr;

	m_pCS = new CComAutoCriticalSection();

	m_lstError = gcnew List<CDBRequestError^>();
	m_lstTableNames = gcnew List<String^>();
	m_DataSet = gcnew System::Data::DataSet();
	m_DataSet->EnforceConstraints = false;
	m_CurrentTable = gcnew System::Data::DataTable();
	m_DataSourceInfo = gcnew array<String^>(6);
	m_RowsetPropertyInfo = gcnew array<Object^>(2);

	m_lstBlobColumns = gcnew List<int>();

	IUDataSource *pIUDataSource = NULL;
	HRESULT hr = ::CoCreateInstance(__uuidof(UDataSource), NULL, CLSCTX_ALL, __uuidof(IUDataSource), (void**)&pIUDataSource);
	if(FAILED(hr))
	{
#ifdef _WIN64
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x64) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#else
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x32) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#endif
	}
	m_pIUDataSource = pIUDataSource;

	IUSession *pIUSession = NULL;
	hr = ::CoCreateInstance(__uuidof(USession), NULL, CLSCTX_ALL, __uuidof(IUSession), (void**)&pIUSession);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUSession = pIUSession;

	IUCommand *pIUCommand = NULL;
	hr = ::CoCreateInstance(__uuidof(UCommand), NULL, CLSCTX_ALL, __uuidof(IUCommand), (void**)&pIUCommand);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUCommand = pIUCommand;

	IURowset *pIURowset = NULL;
	hr = ::CoCreateInstance(__uuidof(URowset), NULL, CLSCTX_ALL, __uuidof(IURowset), (void**)&pIURowset);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIURowset = pIURowset;
	
	IUDataReader *pIUDataReader = NULL;
	hr = m_pIURowset->QueryInterface(__uuidof(IUDataReader), (void**)&pIUDataReader);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll version is too old!"));
	m_pIUDataReader = pIUDataReader;
}

CAsynDBLite::CAsynDBLite(CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler) : CAsyncServiceHandler(sidOleDB, cs, DefaultAsyncResultsHandler)
{
	m_sSubBatchSize = 0;
	m_nIndex = 0;
	m_pIUDataSource = NULL;
	m_pIUSession = NULL;
	m_pIUCommand = NULL;
	m_pIURowset = NULL;
	m_pIUDataReader = NULL;

	m_OnTableOpened = nullptr;
	m_OnFetchingData = nullptr;

	m_DataSource = nullptr;
	m_Session = nullptr;
	m_Command = nullptr;
	m_Rowset = nullptr;

	m_pCS = new CComAutoCriticalSection();

	m_lstError = gcnew List<CDBRequestError^>();
	m_lstTableNames = gcnew List<String^>();
	m_DataSet = gcnew System::Data::DataSet();
	m_DataSet->EnforceConstraints = false;
	m_CurrentTable = gcnew System::Data::DataTable();
	m_DataSourceInfo = gcnew array<String^>(6);
	m_RowsetPropertyInfo = gcnew array<Object^>(2);

	m_lstBlobColumns = gcnew List<int>();

	IUDataSource *pIUDataSource = NULL;
	HRESULT hr = ::CoCreateInstance(__uuidof(UDataSource), NULL, CLSCTX_ALL, __uuidof(IUDataSource), (void**)&pIUDataSource);
	if(FAILED(hr))
	{
#ifdef _WIN64
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x64) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#else
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll (x32) not registerred properly! Execute regsvr32 udb.dll from DOS COMMAND"));
#endif
	}
	m_pIUDataSource = pIUDataSource;

	IUSession *pIUSession = NULL;
	hr = ::CoCreateInstance(__uuidof(USession), NULL, CLSCTX_ALL, __uuidof(IUSession), (void**)&pIUSession);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUSession = pIUSession;

	IUCommand *pIUCommand = NULL;
	hr = ::CoCreateInstance(__uuidof(UCommand), NULL, CLSCTX_ALL, __uuidof(IUCommand), (void**)&pIUCommand);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIUCommand = pIUCommand;

	IURowset *pIURowset = NULL;
	hr = ::CoCreateInstance(__uuidof(URowset), NULL, CLSCTX_ALL, __uuidof(IURowset), (void**)&pIURowset);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll not registerred properly!"));
	m_pIURowset = pIURowset;
	
	IUDataReader *pIUDataReader = NULL;
	hr = m_pIURowset->QueryInterface(__uuidof(IUDataReader), (void**)&pIUDataReader);
	if(FAILED(hr))
		throw gcnew CAsynDBException(hr, gcnew String("UDB.dll version is too old!"));
	m_pIUDataReader = pIUDataReader;
}

CAsynDBLite::~CAsynDBLite()
{
	m_CurrentTable->Clear();
	m_lstError->Clear();
	m_lstTableNames->Clear();
	m_DataSet->Clear();
	
	Detach();

	if(m_DataSource != nullptr)
	{
		System::Runtime::InteropServices::Marshal::ReleaseComObject(m_DataSource);
		m_DataSource = nullptr;
	}
	
	if(m_Session != nullptr)
	{
		System::Runtime::InteropServices::Marshal::ReleaseComObject(m_Session);
		m_Session = nullptr;
	}
	
	if(m_Command != nullptr)
	{
		System::Runtime::InteropServices::Marshal::ReleaseComObject(m_Command);
		m_Command = nullptr;
	}

	if(m_Rowset != nullptr)
	{
		System::Runtime::InteropServices::Marshal::ReleaseComObject(m_Rowset);
		m_Rowset = nullptr;
	}

	if(m_pIUDataReader != NULL)
	{
		m_pIUDataReader->Release();
		m_pIUDataReader = NULL;
	}

	if(m_pIURowset != NULL)
	{
		m_pIURowset->Release();
		m_pIURowset = NULL;
	}

	if(m_pIUCommand != NULL)
	{
		m_pIUCommand->Release();
		m_pIUCommand = NULL;
	}

	if(m_pIUSession != NULL)
	{
		m_pIUSession->Release();
		m_pIUSession = NULL;
	}

	if(m_pIUDataSource != NULL)
	{
		m_pIUDataSource->Release();
		m_pIUDataSource = NULL;
	}
	if( m_pCS != NULL)
	{
		delete m_pCS;
		m_pCS = NULL;
	}
}

//int CAsynDBLite::GetSvsID()
//{
//	return sidOleDB;
//}

void CAsynDBLite::OnResultReturned(short sRequestID, CUQueue ^UQueue)
{
	
}

array<DataRow^>^ CAsynDBLite::AddRows(short sRequestID)
{
	long lPointer;
	unsigned short n;
	long nCol;
	long nCols;
	short nRows;
	HRESULT hr = m_pIURowset->GetRowsFetched(&nRows);
	if(nRows == 0)
		return nullptr;
	if(m_sSubBatchSize == 0)
		m_sSubBatchSize = (unsigned short)nRows;
	ATLASSERT(nRows > m_sBatchIndex);
	hr = m_pIURowset->GetCols(&nCols);
	hr = m_pIUDataReader->get_InternalDataPointer(&lPointer);
	array<DataRow^> ^aRows = gcnew array<DataRow^>((unsigned short)nRows - m_sBatchIndex);
	for (n = m_sBatchIndex; n < (unsigned short)nRows; n++)
	{
		bool bAdd = (m_bAsynFetching || n >= m_CurrentTable->Rows->Count);
		int nInternalDataPos = (nCols + 1)*n;
		for (nCol = 0; nCol < nCols; nCol++)
		{
			//This hack helps performance at client side because it avoids coverting native data into VARIANT data array.
			CField *pField = *((CField **)(lPointer + (nInternalDataPos + nCol + 1)*sizeof(void*)));
			if(pField->m_ulLen == (~0))
			{
				m_aData[nCol] = DBNull::Value;
			}
			else
			{
				switch (m_aDBType[nCol])
				{
				case sdVT_I2:
					{
						m_aData[nCol] = *((short*)pField->m_pData);
					}
					break;
				case sdVT_I4:
					{
						m_aData[nCol] = *((long*)pField->m_pData);
					}
					break;
				case sdVT_R4:
					{
						m_aData[nCol] = *((float*)pField->m_pData);
					}
					break;
				case sdVT_R8:
					{
						m_aData[nCol] = *((double*)pField->m_pData);
					}
					break;
				case sdVT_CY:
					{
						Decimal cyMoney;
						__int64 lData = *((__int64*)pField->m_pData);
						cyMoney = lData;
						cyMoney /= 10000;
						m_aData[nCol] = cyMoney;
					}
					break;
				case sdVT_DATE:
					{
						VARIANT vtDate;
						::VariantInit(&vtDate);
						vtDate.vt = VT_DATE;
						vtDate.date = *((DATE*)pField->m_pData);
						m_aData[nCol] = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtDate));
					}
					break;
				case sdVT_BOOL:
					{
						m_aData[nCol] = *((VARIANT_BOOL*)pField->m_pData) ? true : false;
					}
					break;
				case sdVT_VARIANT:
					m_aData[nCol] = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(pField->m_pData));
					break;
				case sdVT_DECIMAL:
					{
						Decimal decData;
						pin_ptr<Decimal> ptr = &decData;
						memcpy(ptr, pField->m_pData, sizeof(DECIMAL));
						m_aData[nCol] = decData;
					}
					break;
				case sdVT_I1:
					{
						m_aData[nCol] = *((char*)pField->m_pData);
					}
					break;
				case sdVT_UI1:
					{
						m_aData[nCol] = *((unsigned char*)pField->m_pData);
					}
					break;
				case sdVT_UI2:
					{
						m_aData[nCol] = *((unsigned short*)pField->m_pData);
					}
					break;
				case sdVT_UI4:
					{
						m_aData[nCol] = *((unsigned long*)pField->m_pData);
					}
					break;
				case sdVT_I8:
					{
						m_aData[nCol] = *((LONGLONG*)pField->m_pData);
					}
					break;
				case sdVT_UI8:
					{
						m_aData[nCol] = *((ULONGLONG*)pField->m_pData);
					}
					break;
				case sdVT_BYTES:
					if(pField->m_pData != NULL && !IsBlob(nCol + 1))
					{
						array<BYTE> ^aByte = gcnew array<BYTE>(pField->m_ulLen);
						pin_ptr<BYTE> pByte = &aByte[0];
						memcpy(pByte, pField->m_pData, pField->m_ulLen);
						m_aData[nCol] = aByte;
					}
					else
					{
						m_aData[nCol] = DBNull::Value;
					}
					break;
				case sdVT_STR:
					if(pField->m_pData != NULL && !IsBlob(nCol + 1))
					{
						m_aData[nCol] = gcnew String((char*)pField->m_pData, 0, pField->m_ulLen);
					}
					else
					{
						m_aData[nCol] = DBNull::Value;
					}
					break;
				case sdVT_WSTR:
					if(pField->m_pData != NULL && !IsBlob(nCol + 1))
					{
						m_aData[nCol] = gcnew String((WCHAR*)pField->m_pData, 0, pField->m_ulLen/sizeof(WCHAR));
					}
					else
					{
						m_aData[nCol] = DBNull::Value;
					}
					break;
				default:
					ATLASSERT(FALSE);
					break;	
				}
			}
			if(m_aData[nCol] == nullptr)
			{
				m_aData[nCol] = DBNull::Value;
			}
		}
		if(m_CurrentTable->Columns->Count > nCols)
		{
			m_aData[nCols] = (int)n;
		}

		if(bAdd)
		{
			aRows[n-m_sBatchIndex] = m_CurrentTable->Rows->Add(m_aData);
		}
		else
		{
			DataRow ^row = m_CurrentTable->Rows[n];
			row->ItemArray = m_aData;
			aRows[n-m_sBatchIndex] = m_CurrentTable->Rows[n];
		}
	}
	m_sBatchIndex = (unsigned short)nRows;
	return aRows;
}

int CAsynDBLite::GetColNameIndex(DataTable ^dt, String ^strColName)
{
	int n = 0;
	for each (DataColumn ^dc in dt->Columns)
	{
		if(String::Compare(dc->ColumnName, strColName, true) == 0)
		{
			n++;
		}
	}
	return n;
}

bool CAsynDBLite::IsBlob(int nCol)
{
	for each(int nColBlob in m_lstBlobColumns)
	{
		if(nCol == nColBlob)
			return true;
	}
	return false;
}

void CAsynDBLite::HandleOneBlob()
{
	long lPointer;
	long nCols;
	short sDBType;
	CAutoLock AutoLock(&m_pCS->m_sec);
	long nRow = m_nBlobIndex/m_lstBlobColumns->Count;
	long nCol = (m_nBlobIndex%m_lstBlobColumns->Count);
	nCol = m_lstBlobColumns[nCol];
	HRESULT hr = m_pIUDataReader->get_InternalDataPointer(&lPointer);
	hr = m_pIURowset->GetCols(&nCols);
	hr = m_pIURowset->GetDataType(nCol, &sDBType);
	long nInternalDataPos = (nCols + 1) * nRow;

	DataRow ^dr = m_CurrentTable->Rows[nRow];

	bool bReadOnly = m_CurrentTable->Columns[nCol-1]->ReadOnly;
	if(bReadOnly)
		m_CurrentTable->Columns[nCol-1]->ReadOnly = false;

	//This hack helps performance at client side because it avoids coverting native data into VARIANT data array.
	CField *pField = *((CField **)(lPointer + (nInternalDataPos + nCol)*sizeof(void*)));
	switch(sDBType)
	{
	case sdVT_BYTES:
		if(pField->m_pData != NULL && pField->m_ulLen != (~0))
		{
			array<BYTE> ^aByte = gcnew array<BYTE>(pField->m_ulLen);
			pin_ptr<BYTE> pByte = &aByte[0];
			memcpy(pByte, pField->m_pData, pField->m_ulLen);
			dr[nCol-1] = aByte;
		}
		break;
	case sdVT_STR:
		if(pField->m_pData != NULL && pField->m_ulLen != (~0))
		{
			dr[nCol-1] = gcnew String((char*)pField->m_pData, 0, pField->m_ulLen);
		}
		break;
	case sdVT_WSTR:
		if(pField->m_pData != NULL && pField->m_ulLen != (~0))
		{
			dr[nCol-1] = gcnew String((WCHAR*)pField->m_pData, 0, pField->m_ulLen/sizeof(WCHAR));
		}
		break;
	default:
		ATLASSERT(FALSE);
		break;
	}

	if(bReadOnly)
		m_CurrentTable->Columns[nCol-1]->ReadOnly = true;
}

DataTable^ CAsynDBLite::CreateEmptyDataTable(String ^strTableName)
{
	long nCol;
	short sDBType;
	long nMaxLen;
	VARIANT_BOOL b;
	long nCols = 0;
	VARIANT_BOOL bBlob;
	CComBSTR bstrColName;
	VARIANT_BOOL bReadOnly = VARIANT_FALSE;
	m_sSubBatchSize = 0;
	m_lstBlobColumns->Clear();
	HRESULT hr = m_pIURowset->GetCols(&nCols);
	hr = m_pIURowset->IsReadOnly(&bReadOnly);
	m_aDBType = gcnew array<short> (nCols);
	DataTable ^dt = gcnew DataTable(strTableName);
	for (nCol = 1; nCol <= nCols; nCol++)
	{
		System::Type ^tType = nullptr;
		hr = m_pIURowset->GetColName(nCol, &bstrColName);
		String ^strColName = gcnew String(bstrColName);
		int nFound = GetColNameIndex(dt, strColName);
		if(nFound > 0)
		{
			strColName += "(";
			strColName += nFound.ToString();
			strColName += ")";
		}
		DataColumn ^dc = gcnew DataColumn(strColName);
		hr = m_pIURowset->GetDataType(nCol, &sDBType);
		hr = m_pIURowset->IsBLOB(nCol, &bBlob);
		if(bBlob)
		{
			m_lstBlobColumns->Add(nCol);
		}
		m_aDBType[nCol-1] = sDBType;
		switch(sDBType)
		{
		case sdVT_BOOL:
			tType = bool::typeid;
			break;
		case sdVT_BYTES:
			tType = array<BYTE>::typeid;
			break;
		case sdVT_CY:
		case sdVT_DECIMAL:
			tType = Decimal::typeid;
			break;
		case sdVT_DATE:
			tType = DateTime::typeid;
			break;
		case sdVT_I1:
			tType = SByte::typeid;
			break;
		case sdVT_UI1:
			tType = Byte::typeid;
			break;
		case sdVT_I2:
			tType = short::typeid;
			break;
		case sdVT_UI2:
			tType = unsigned short::typeid;
			break;
		case sdVT_I4:
			tType = int::typeid;
			break;
		case sdVT_UI4:
			tType = unsigned int::typeid;
			break;
		case sdVT_I8:
			tType = __int64::typeid;
			break;
		case sdVT_UI8:
			tType = unsigned __int64::typeid;
			break;
		case sdVT_R4:
			tType = float::typeid;
			break;
		case sdVT_R8:
			tType = double::typeid;
			break;
		case sdVT_STR:
			tType = String::typeid;
			hr = m_pIURowset->GetMaxLen(nCol, &nMaxLen);
			dc->MaxLength = nMaxLen - 1;
			break;
		case sdVT_WSTR:
			tType = String::typeid;
			hr = m_pIURowset->GetMaxLen(nCol, &nMaxLen);
			dc->MaxLength = nMaxLen/2 - 1;
			break;
		case sdVT_VARIANT:
			tType = Object::typeid;
			break;
		default:
			ATLASSERT(FALSE);
			break;
		}
		dc->DataType = tType;
		hr = m_pIURowset->IsNullable(nCol, &b);
		dc->AllowDBNull = (b != VARIANT_FALSE);
		hr = m_pIURowset->IsWritable(nCol, &b);
		dc->ReadOnly = ((b == VARIANT_FALSE) || (bReadOnly != VARIANT_FALSE));
		dt->Columns->Add(dc);
		bstrColName.Empty();
	}
	return dt;
}

void CAsynDBLite::OnSocketClosed(int hSocket, int nError)
{
	CAutoLock AutoLock(&m_pCS->m_sec);
	m_lstTableNames->Clear();
}

void CAsynDBLite::OnRDBResult(int hSocket, short sRequestID, int nLen, int nLenInBuffer, USOCKETLib::tagReturnFlag ReturnFlag)
{
	if(ReturnFlag != USOCKETLib::tagReturnFlag::rfCompleted)
		return;
	long lError = 0;
	switch(sRequestID)
	{
	case idDSOpenFromHandle:
	case idDSOpen:
		{
			m_pIUDataSource->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIUDataSource->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
		}
		break;
	case idDSGetProperty:
		{
			m_pIUDataSource->get_Rtn(&lError);
			CAutoLock AutoLock(&m_pCS->m_sec);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIUDataSource->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
			else
			{
				CComVariant vtProp;
				m_pIUDataSource->get_Property(&vtProp);
				if(vtProp.vt == VT_BSTR && m_nIndex < 4)
				{
					m_DataSourceInfo[m_nIndex] = gcnew String(vtProp.bstrVal);
				}
				else if(vtProp.vt == VT_I4 && m_nIndex == 4)
				{
					Object ^ob = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtProp));
					m_DataSourceInfo[m_nIndex] = ob->ToString();
				}
				else if(vtProp.vt == VT_BOOL && m_nIndex == 5)
				{
					Object ^ob = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtProp));
					m_DataSourceInfo[m_nIndex] = ob->ToString();
					m_nIndex = -1;
				}
			}
			m_nIndex++;
		}
		break;
	case idDSClose:
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_DataSourceInfo[0] = m_DataSourceInfo[1] = m_DataSourceInfo[2] = m_DataSourceInfo[3] = nullptr;
		}
		break;
	case idSessionBeginTrans:
	case idSessionCommit:
	case idSessionRollback:
	case idSessionOpen:
	case idSessionOpenFromHandle:
		{
			m_pIUSession->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIUSession->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
		}
		break;
	case idCmndGetOutputParams:
		{
			m_pIUCommand->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIUCommand->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
			else if(m_OnOutputDataCome != nullptr)
			{
				long lCount = 0;
				m_pIUCommand->GetCountOutputData(&lCount);
				if(lCount > 0)
				{
					long n;
					CComVariant vtData;
					array<Object^> ^aOutput = gcnew array<Object^>(lCount);
					for(n=0; n<lCount; n++)
					{
						m_pIUCommand->GetOutputData(n, &vtData);
						aOutput[n] = System::Runtime::InteropServices::Marshal::GetObjectForNativeVariant(IntPtr(&vtData));
						vtData.Clear();
					}
					m_OnOutputDataCome->Invoke(aOutput);
				}
			}
		}
		break;
	case idCmndDoBatch:
	case idCmndExecuteSQL:
	case idCmndOpen:
	case idCmndOpenFromHandle:
		{
			m_pIUCommand->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIUCommand->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
		}
		break;
	case idRowsetGetProperty:
		{
			CComVariant vtProp;
			m_pIURowset->get_Property(&vtProp);
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_RowsetPropertyInfo[m_nIndex] = Marshal::GetObjectForNativeVariant(IntPtr(&vtProp));
			if(m_nIndex == 1)
			{
				m_nIndex = -1;
			}
			m_nIndex++;
		}
		break;
	case idRowsetBookmark:
	case idRowsetDelete:
	case idRowsetUpdate:
		{
			m_pIURowset->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIURowset->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
			}
		}
		break;
	case idRowsetOpenFromHandle:
	case idRowsetGetProviders:
	case idRowsetGetSchemaRowset:
	case idRowsetOpen:
		{
			m_pIURowset->get_Rtn(&lError);
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_bAsynFetching = true;
			m_nIndex = 0;
			m_RowsetPropertyInfo[0] = m_RowsetPropertyInfo[1] = false;
			
			String ^strTableName = m_lstTableNames[0];
			m_lstTableNames->RemoveAt(0);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIURowset->get_ErrorMsg(&bstrError);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
				m_CurrentTable = gcnew DataTable();
			}
			else
			{
				m_CurrentTable = CreateEmptyDataTable(strTableName);
				if(m_DataSet->Tables[strTableName] != nullptr)
				{
					//remove the old table having the same table name
					m_DataSet->Tables->Remove(strTableName);
				}
				m_DataSet->Tables->Add(m_CurrentTable);
				
				if(m_OnTableOpened != nullptr)
				{
					m_OnTableOpened->Invoke(m_CurrentTable);
				}
			}
		}
		break;
	case idRowsetSendBLOB:
		HandleOneBlob();
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_nBlobIndex++;
		}
		break;
	case idRowsetStartFetchingBatch:
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_nBlobIndex = 0;
			m_sBatchIndex = 0;
		}
		if(!m_bAsynFetching)
		{
			long nCol;
			long nCols;
			m_pIURowset->GetCols(&nCols);
			CAutoLock AutoLock(&m_pCS->m_sec);
			for(nCol = 0; nCol<nCols; nCol++)
			{
				DataColumn ^dc = m_CurrentTable->Columns[nCol];
				if(dc->ReadOnly)
				{
					dc->ReadOnly = false;
				}
			}
		}
		break;
	case idRowsetSendSubBatch:
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_CurrentTable->BeginLoadData();
			array<DataRow^> ^aRows = AddRows(sRequestID);
			m_CurrentTable->EndLoadData();
			if(m_OnFetchingData != nullptr && aRows != nullptr)
			{
				m_OnFetchingData->Invoke(m_CurrentTable, aRows);
			}
		}
		break;
	case idRowsetClose:
		break;
	case idRowsetAdd:
	case idRowsetMoveFirst:
	case idRowsetMoveNext:
	case idRowsetMoveLast:
	case idRowsetMovePrev:
		{
			m_pIURowset->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIURowset->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
				m_CurrentTable->Rows->Clear();
			}
			else
			{
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_CurrentTable->Rows->Clear();
				array<DataRow^> ^aRows = AddRows(sRequestID);
				if(m_OnFetchingData != nullptr && aRows != nullptr)
				{
					m_OnFetchingData->Invoke(m_CurrentTable, aRows);
				}
			}
		}
		break;
	case idRowsetGetRowsAt:
	case idRowsetGetBatchRecords:
	case idRowsetGetBatchRecordsEx:
	case idRowsetGetBatchRecordsLast:
		{
			long nCol;
			long nCols;
			VARIANT_BOOL b;
			VARIANT_BOOL bReadOnly;
			m_bAsynFetching = false;
			short sRows = 0;
			m_pIURowset->GetRowsFetched(&sRows);
			m_pIURowset->GetCols(&nCols);
			m_pIURowset->IsReadOnly(&bReadOnly);

			CAutoLock AutoLock(&m_pCS->m_sec);
			while(m_CurrentTable->Rows->Count > (unsigned short)sRows)
			{
				m_CurrentTable->Rows->RemoveAt(m_CurrentTable->Rows->Count - 1);
			}

			for(nCol = 0; nCol<nCols; nCol++)
			{
				DataColumn ^dc = m_CurrentTable->Columns[nCol];
				m_pIURowset->IsWritable(nCol + 1, &b);
				bool bRO = ((b == VARIANT_FALSE) || (bReadOnly != VARIANT_FALSE));
				if(bRO != dc->ReadOnly)
					dc->ReadOnly = bRO;
			}
		}
	case idRowsetAsynFetch:
		{
			m_pIURowset->get_Rtn(&lError);
			if(FAILED(lError))
			{
				CComBSTR bstrError;
				m_pIURowset->get_ErrorMsg(&bstrError);
				CAutoLock AutoLock(&m_pCS->m_sec);
				m_lstError->Add(gcnew CDBRequestError((UDBLib::tagDBRequestID)sRequestID, lError, gcnew String(bstrError)));
				m_CurrentTable->Rows->Clear();
			}
		}
		break;
	default:
		break;
	}
	CScopeUQueue sq;
	OnResultReturned(sRequestID, sq.UQueue);
	switch(sRequestID)
	{
	case idRowsetOpenFromHandle:
	case idRowsetGetProviders:
	case idRowsetGetSchemaRowset:
	case idRowsetOpen:
		{
			CAutoLock AutoLock(&m_pCS->m_sec);
			m_aData = gcnew array<Object^> (m_CurrentTable->Columns->Count);
		}
		break;
	default:
		break;
	}
}

bool CAsynDBLite::Attach(CClientSocket ^pClientSocket)
{
	CAutoLock al(&m_pCS->m_sec);
	m_pClientSocket = pClientSocket;
	if(pClientSocket != nullptr)
	{
		IUnknown *pIUSocket = (IUnknown*)pClientSocket->GetIUSocket().ToPointer();
		HRESULT hr = m_pIUDataSource->AttachSocket(pIUSocket);
		hr = m_pIUSession->AttachSocket(pIUSocket);
		hr = m_pIUCommand->AttachSocket(pIUSocket);
		hr = m_pIURowset->AttachSocket(pIUSocket);
		pClientSocket->m_OnRequestProcessed += gcnew DOnRequestProcessed(this, &CAsynDBLite::OnRDBResult);
		pClientSocket->m_OnSocketClosed += gcnew DOnSocketClosed(this, &CAsynDBLite::OnSocketClosed);
		return true;
	}
	return false;
}

void CAsynDBLite::Detach()
{
	CClientSocket ^p = GetAttachedClientSocket();
	if(p != nullptr)
	{
		p->m_OnSocketClosed -= gcnew DOnSocketClosed(this, &CAsynDBLite::OnSocketClosed);
		p->m_OnRequestProcessed -= gcnew DOnRequestProcessed(this, &CAsynDBLite::OnRDBResult);
	}

	if(m_pIURowset != NULL)
	{
		m_pIURowset->AttachSocket(NULL);
	}

	if(m_pIUCommand != NULL)
	{
		m_pIUCommand->AttachSocket(NULL);
	}

	if(m_pIUSession != NULL)
	{
		m_pIUSession->AttachSocket(NULL);
	}

	if(m_pIUDataSource != NULL)
	{
		m_pIUDataSource->AttachSocket(NULL);
	}
	m_pClientSocket = nullptr;
}

void CAsynDBLite::ConnectDB(String ^strOLEDBConnString)
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	
	CClientSocket ^pClientSocket = GetAttachedClientSocket();

	if(pClientSocket == nullptr)
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("No USocket object attached!"));
/*	if(strOLEDBConnString == nullptr || strOLEDBConnString->Length == 0)
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Connection string to a DB can't be null or empty!"));*/
	if(pClientSocket->IsConnected())
	{
		bool bBatching = pClientSocket->IsBatching();
		if(!bBatching)
			pClientSocket->BeginBatching();
		pin_ptr<const wchar_t> str = PtrToStringChars(strOLEDBConnString);
		m_pIUDataSource->Open(CComBSTR(str), 0);
		m_pIUDataSource->GetProperty(41);		//DBPROP_DBMSVER
		m_pIUDataSource->GetProperty(235);		//DBPROP_PROVIDERFRIENDLYNAME
		m_pIUDataSource->GetProperty(250); //DBPROP_SERVERNAME
		m_pIUDataSource->GetProperty(37, CComBSTR(DBPROPSET_DATASOURCE)); //DBPROP_CURRENTCATALOG
		m_pIUDataSource->GetProperty(184); //DBPROP_OUTPUTPARAMETERAVAILABILITY
		m_pIUDataSource->GetProperty(191); //DBPROP_MULTIPLEPARAMSETS
		m_pIUSession->Open(NULL, 0);
		m_pIUCommand->Open(NULL, 0);
		if(!bBatching)
			pClientSocket->Commit(true);
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_nIndex = 0;
	}
}

void CAsynDBLite::DisconnectDB()
{
	CClientSocket ^pClientSocket = GetAttachedClientSocket();
	if(pClientSocket == nullptr)
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("No USocket object attached!"));
	bool bBatching = pClientSocket->IsBatching();
	if(!bBatching)
		pClientSocket->BeginBatching();
	m_pIUCommand->Close();
	m_pIUSession->Close();
	m_pIUDataSource->Close();
	if(!bBatching)
		pClientSocket->Commit(true);
}

void CAsynDBLite::OpenSchema(Guid guidSchema, array<Object^> ^arrRestrictions)
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	
	GUID guid;
	Object ^ob = arrRestrictions;
	pin_ptr<Guid> p = &guidSchema;
	memcpy(&guid, p, sizeof(GUID));
	ULONG nRestrictions = 0;
	CComVariant vtRestrictions;
	if(arrRestrictions != nullptr && arrRestrictions->Length > 0)
	{
		System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(ob, IntPtr(&vtRestrictions));
	}
	CClientSocket ^pClientSocket = GetAttachedClientSocket();
	bool bBatching = pClientSocket->IsBatching();
	if(!bBatching)
		pClientSocket->BeginBatching();
	CComBSTR bstr(guid);
	HRESULT hr = m_pIURowset->GetSchemaRowset(bstr, vtRestrictions);
	hr = m_pIURowset->AsynFetch(VARIANT_TRUE, 0, -1);
	//The following request leads to release all of resources for a cursor after asynchronously fetching all of records
	hr = m_pIURowset->Close();
	if(!bBatching)
		pClientSocket->Commit(true);

	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstTableNames->Add(guidSchema.ToString());
	}
}

void CAsynDBLite::BeginTrans(UDBLib::tagIsolationLevel nIsolationLevel)
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	m_pIUSession->BeginTrans((int)nIsolationLevel);
}

void CAsynDBLite::BeginTrans()
{
	BeginTrans(UDBLib::tagIsolationLevel::ilReadCommitted);
}

void CAsynDBLite::Commit()
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	m_pIUSession->Commit();
}

void CAsynDBLite::Rollback()
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	m_pIUSession->Rollback();
}

void CAsynDBLite::OpenCommandWithParameters(String ^strSQL, List<CParamInfo^> ^lstParamInfo)
{
	OpenCommandWithParameters(strSQL, lstParamInfo, true);
}

void CAsynDBLite::OpenCommandWithParameters(String ^strSQL, List<CParamInfo^> ^lstParamInfo, bool bPrepare)
{
	int n;
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	int nCount = 0;
	if(lstParamInfo != nullptr)
		nCount = lstParamInfo->Count;
	HRESULT hr = m_pIUCommand->CleanParamData();
	hr = m_pIUCommand->CleanParamInfo();
	for(n=0; n<nCount; n++)
	{
		hr = m_pIUCommand->AddParamInfo((short)lstParamInfo[n]->m_sDBType, (long)lstParamInfo[n]->m_nParamIO, lstParamInfo[n]->m_nLen, NULL, 0/*lstParamInfo[n]->m_bPrecision*/, 0/*lstParamInfo[n]->m_bScale*/);
	}
	int nHint = bPrepare ? (int)UDBLib::tagCommandHint::chCommandPrepare : 0;
	pin_ptr<const wchar_t> str = PtrToStringChars(strSQL);
	hr = m_pIUCommand->Open(CComBSTR(str), nHint);
}

void CAsynDBLite::OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName)
{
	OpenRowsetFromParameters(aParameterData, strTableName, UDBLib::tagCursorType::ctForwardOnly, CAsynDBLite::Readonly|CAsynDBLite::AsynFetch, 0, -1);
}

void CAsynDBLite::OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName, UDBLib::tagCursorType ct, int nHints, short sBatchSize, int nRowsExpected)
{
	long n;
	long lCount = 0;
	bool bDelayOutput;

	CClientSocket ^pClientSocket = GetAttachedClientSocket();

	if(pClientSocket == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	
	if(strTableName == nullptr || strTableName->Length == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Table name must be valid, and can't be null or empty!"));
	}

	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		if(m_lstTableNames->Contains(strTableName))
		{
			throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Table name must be unique for each of queries!"));
		}
		m_lstError->Clear();
		bDelayOutput = (m_DataSourceInfo[4] == "4");
		m_nIndex = 0;
	}

	HRESULT hr = m_pIUCommand->CleanParamData();
	hr = m_pIUCommand->GetCountParamInfos(&lCount);
	if(lCount == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Open a command with parameterized statement first!"));
	}
	if(aParameterData == nullptr || aParameterData->Length == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Must input an array of parameter data!"));
	}
	
	if(aParameterData->Length != lCount)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The size of an array of parameter data must be equal to the number of parameters!"));
	}
	
	for(n=0; n<lCount; n++)
	{
		{
			CComVariant vtData;
			if(aParameterData[n] != nullptr)
				System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(aParameterData[n], IntPtr(&vtData));
			hr = m_pIUCommand->AppendParamData(vtData);
			if(FAILED(hr))
			{
				CComBSTR bstrError;
				m_pIUCommand->get_ErrorMsg(&bstrError);
				throw gcnew CAsynDBException(hr, gcnew String(bstrError));
			}
		}
	}
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstTableNames->Add(strTableName);
	}
	if((nHints & AsynFetch) == AsynFetch)
	{
		nHints = (CAsynDBLite::Readonly | CAsynDBLite::AsynFetch);
		ct = UDBLib::tagCursorType::ctForwardOnly;
	}
	long lHint = nHints;
	if(DBProvider != nullptr && DBProvider->Contains(gcnew String(" Jet")) && (nHints & CAsynDBLite::Readonly))
	{
		//Jet OLEDB provider may fetch one record per batch if rowset is readonly, which will lead to performance degrade
		//The following code prevents Jet performance from degrade.
		lHint |= CAsynDBLite::Scrollable;
		if(ct == UDBLib::tagCursorType::ctForwardOnly)
			ct = UDBLib::tagCursorType::ctStatic;
	}
	bool bBatching = pClientSocket->IsBatching();
	if(!bBatching)
		pClientSocket->BeginBatching();
	hr = m_pIUCommand->DoBatch(coRowset, (short)ct, lHint);
	bool bHasOutput = HasOutput();
	if(!bDelayOutput && bHasOutput)
	{
		hr = m_pIUCommand->GetOutputParams();
	}
	hr = m_pIURowset->Open(NULL, 0, 0, sBatchSize);
	hr = m_pIURowset->GetProperty(0x15); //DBPROP_CANSCROLLBACKWARDS
	hr = m_pIURowset->GetProperty(0x82); //DBPROP_IRowsetLocate
	if((lHint & AsynFetch) == AsynFetch)
	{
		hr = m_pIURowset->AsynFetch(VARIANT_TRUE, 0, nRowsExpected);
		//The following two requests lead to release all of resources for a cursor after asynchronously fetching all of records
		hr = m_pIURowset->Close();
		hr = m_pIUCommand->ReleaseCreatedObject();
		if(bDelayOutput && bHasOutput)
		{
			hr = m_pIUCommand->GetOutputParams();
		}
	}
	else
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		hr = m_pIURowset->GetBatchRecords(m_sSubBatchSize, VARIANT_TRUE);
	}
	if(!bBatching)
		pClientSocket->Commit(true);
}

void CAsynDBLite::OpenRowset(String ^strSQL, String ^strTableName)
{
	OpenRowset(strSQL, strTableName, UDBLib::tagCursorType::ctForwardOnly, CAsynDBLite::Readonly|CAsynDBLite::AsynFetch, 0, -1);
}

bool CAsynDBLite::HasOutput()
{
	long n;
	HRESULT hr;
	long lDBParamIO = 0;
	long lCount = 0;
	short sDBType;
	m_pIUCommand->GetCountParamInfos(&lCount);
	for(n=0; n<lCount; n++)
	{
		hr = m_pIUCommand->GetParamInfo(n, &sDBType, &lDBParamIO);
		if(lDBParamIO == sdParamOutput || lDBParamIO == sdParamInputOutput)
			return true;
	}
	return false;
}

void CAsynDBLite::OpenRowset(String ^strSQL, String ^strTableName, UDBLib::tagCursorType ct, int nHints, short nBatchSize, int nRowsExpected)
{
	if(strTableName == nullptr || strTableName->Length == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Table name must be valid, and can't be null or empty!"));
	}
	
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		if(m_lstTableNames->Contains(strTableName))
		{
			throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Table name must be unique for each of queries!"));
		}
		m_lstError->Clear();
		m_nIndex = 0;
	}
	pin_ptr<const wchar_t> str = PtrToStringChars(strSQL);
	HRESULT hr = m_pIUCommand->CleanParamData();
	hr = m_pIUCommand->CleanParamInfo();
	if((nHints & AsynFetch) == AsynFetch)
	{
		nHints = (CAsynDBLite::Readonly | CAsynDBLite::AsynFetch);
		ct = UDBLib::tagCursorType::ctForwardOnly;
	}
	long lHint = nHints;
	if(DBProvider != nullptr && DBProvider->Contains(gcnew String(" Jet")) && (nHints & CAsynDBLite::Readonly))
	{
		//Jet OLEDB provider may fetch one record per batch if rowset is readonly, which will lead to performance degrade
		//The following code prevents Jet performance from degrade.
		lHint |= CAsynDBLite::Scrollable;
		if(ct == UDBLib::tagCursorType::ctForwardOnly)
			ct = UDBLib::tagCursorType::ctStatic;
	}
	CClientSocket ^pClientSocket = GetAttachedClientSocket();
	bool bBatching = pClientSocket->IsBatching();
	if(!bBatching)
		pClientSocket->BeginBatching();
	hr = m_pIUCommand->ExecuteSQL(CComBSTR(str), coRowset, (short)ct, lHint);
	hr = m_pIURowset->Open(NULL, (short)0, 0, nBatchSize, false);
	hr = m_pIURowset->GetProperty(0x15); //DBPROP_CANSCROLLBACKWARDS
	hr = m_pIURowset->GetProperty(0x82); //DBPROP_IRowsetLocate
	if((lHint & AsynFetch) == AsynFetch)
	{
		hr = m_pIURowset->AsynFetch(VARIANT_TRUE, 0, nRowsExpected);
		//The following two requests lead to release all of resources for a cursor after asynchronously fetching all of records
		hr = m_pIURowset->Close();
		hr = m_pIUCommand->ReleaseCreatedObject();
	}
	else
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		hr = m_pIURowset->GetBatchRecords(m_sSubBatchSize, VARIANT_TRUE);
	}
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstTableNames->Add(strTableName);
	}
	if(!bBatching)
		pClientSocket->Commit(true);
}

void CAsynDBLite::ExecuteNonQuery(String ^strSQL)
{
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	pin_ptr<const wchar_t> str = PtrToStringChars(strSQL);
	HRESULT hr = m_pIUCommand->CleanParamData();
	hr = m_pIUCommand->CleanParamInfo();
	hr = m_pIUCommand->ExecuteSQL(CComBSTR(str), (short)UDBLib::tagCreatedObject::coNothing, 0, 0);
}

UDBLib::UDataSourceClass^ CAsynDBLite::GetDataSource()
{
	if(m_DataSource == nullptr && GetAttachedClientSocket() != nullptr && m_pIUDataSource != NULL)
	{
		m_DataSource = (UDBLib::UDataSourceClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIUDataSource), UDBLib::UDataSourceClass::typeid);
	}
	return m_DataSource;
}

UDBLib::USessionClass^ CAsynDBLite::GetSession()
{
	if(m_Session == nullptr && GetAttachedClientSocket() != nullptr && m_pIUSession != NULL)
	{
		m_Session = (UDBLib::USessionClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIUSession), UDBLib::USessionClass::typeid);
	}
	return m_Session;
}

UDBLib::UCommandClass^ CAsynDBLite::GetCommand()
{
	if(m_Command == nullptr && GetAttachedClientSocket() != nullptr && m_pIUCommand != NULL)
	{
		m_Command = (UDBLib::UCommandClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIUCommand), UDBLib::UCommandClass::typeid);
	}
	return m_Command;
}

UDBLib::URowsetClass^ CAsynDBLite::GetRowset()
{
	if(m_Rowset == nullptr && GetAttachedClientSocket() != nullptr && m_pIURowset != NULL)
	{
		m_Rowset = (UDBLib::URowsetClass^)Marshal::GetTypedObjectForIUnknown(IntPtr(m_pIURowset), UDBLib::URowsetClass::typeid);
	}
	return m_Rowset;
}

bool CAsynDBLite::BeginBatch()
{
	CAutoLock AutoLock(&m_pCS->m_sec);
	if(GetAttachedClientSocket() == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	return GetAttachedClientSocket()->BeginBatching();
}

bool CAsynDBLite::CommitBatch(AsyncCallback ^cb)
{
	CAutoLock AutoLock(&m_pCS->m_sec);
	CClientSocket ^p = GetAttachedClientSocket();
	if(p == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	IUSocket *pIUSocket = (IUSocket *)p->GetIUSocket().ToPointer();
	m_lstError->Clear();
	m_CurrentTable->Clear();
	m_DataSet->Clear();
	p->Callback = cb;
	return p->Commit(true);
}

bool CAsynDBLite::AbortBatch()
{
	CAutoLock AutoLock(&m_pCS->m_sec);
	CClientSocket ^p = GetAttachedClientSocket();
	if(p == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	m_lstTableNames->Clear();
	return p->Rollback();
}

void CAsynDBLite::DoBatch(array<Object^> ^aParameterData)
{
	HRESULT hr;
	long lCount = 0;
	long lIndex;
	{
		CAutoLock AutoLock(&m_pCS->m_sec);
		m_lstError->Clear();
	}
	CClientSocket ^p = GetAttachedClientSocket();
	if(p == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	m_pIUCommand->CleanParamData();
	m_pIUCommand->GetCountParamInfos(&lCount);
	if(lCount == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("UCommand object doesn't contain any parameter information!"));
	}
	if(aParameterData == nullptr || aParameterData->Length == 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Can't pass in null or empty array of parameter data"));
	}
	
	if((aParameterData->Length % lCount) != 0)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Must pass in a matrix or vector of parameter data"));
	}
	
	lCount = aParameterData->Length;
	for(lIndex = 0; lIndex < lCount; lIndex++)
	{
		{
			CComVariant vtData;
			Object ^obData = aParameterData[lIndex];
			if(obData != nullptr)
				System::Runtime::InteropServices::Marshal::GetNativeVariantForObject(obData, IntPtr(&vtData));
			hr = m_pIUCommand->AppendParamData(vtData);
			if(FAILED(hr))
			{
				CComBSTR bstrError;
				m_pIUCommand->get_ErrorMsg(&bstrError);
				throw gcnew CAsynDBException(hr, gcnew String(bstrError));
			}
		}
	}
	bool bHasOutput = HasOutput();
	if(bHasOutput)
	{
		bool bBatching = GetAttachedClientSocket()->IsBatching();
		if(!bBatching)
			GetAttachedClientSocket()->BeginBatching();
		hr = m_pIUCommand->DoBatch(coNothing, 0, 0);
		if(bHasOutput && !FAILED(hr))
		{
			m_pIUCommand->GetOutputParams();
		}
		if(!bBatching)
			GetAttachedClientSocket()->Commit(true);
	}
	else
	{
		hr = m_pIUCommand->DoBatch(coNothing, 0, 0);
	}
	if(FAILED(hr))
	{
		CComBSTR bstrError;
		m_pIUCommand->get_ErrorMsg(&bstrError);
		throw gcnew CAsynDBException(hr, gcnew String(bstrError));
	}
}

void CAsynDBLite::FirstBatch()
{
	if(GetAttachedClientSocket() == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	CAutoLock AutoLock(&m_pCS->m_sec);
	HRESULT hr = m_pIURowset->GetBatchRecords(m_sSubBatchSize, VARIANT_TRUE);
}

void CAsynDBLite::LastBatch()
{
	if(GetAttachedClientSocket() == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}

	if(!IsRowsetScrollable)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Rowset is not scrollable!"));
	}
	CAutoLock AutoLock(&m_pCS->m_sec);
	HRESULT hr = m_pIURowset->GetBatchRecordsLast(m_sSubBatchSize);
}

void CAsynDBLite::NextBatch()
{
	NextBatch(0);
}

void CAsynDBLite::NextBatch(long lSkip)
{
	if(GetAttachedClientSocket() == nullptr)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("The object is not attached with a USocket object yet!"));
	}
	
	if(!IsRowsetScrollable && lSkip < -1)
	{
		throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Rowset is not scrollable!"));
	}
	CAutoLock AutoLock(&m_pCS->m_sec);
	HRESULT hr = m_pIURowset->GetBatchRecordsEx(m_sSubBatchSize, lSkip);
}

void CAsynDBLite::PrevBatch()
{
	NextBatch(-2);
}

void CAsynDBLiteEx::OpenRowsetFromParameters(array<Object^> ^aParameterData, String ^strTableName)
{
	//open a updateable rowset, and fetch the first batch of reocrds
	OpenRowsetFromParameters(aParameterData, strTableName, UDBLib::tagCursorType::ctStatic, 0, 0, -1);
}

void CAsynDBLiteEx::OpenRowset(String ^strSQL, String ^strTableName)
{
	//open a updateable rowset, and fetch the first batch of reocrds
	OpenRowset(strSQL, strTableName, UDBLib::tagCursorType::ctStatic, 0, 0, -1);
}

void CAsynDBLiteEx::AddDGVDelegates()
{
	if(m_dgv != nullptr)
	{
		m_dgv->CellValueChanged += gcnew DataGridViewCellEventHandler(this, &CAsynDBLiteEx::OnCellValueChanged);
		m_dgv->DataError += gcnew DataGridViewDataErrorEventHandler(this, &CAsynDBLiteEx::OnDataError);
		m_dgv->CellClick += gcnew DataGridViewCellEventHandler(this, &CAsynDBLiteEx::OnCellClick);
		m_dgv->KeyDown += gcnew KeyEventHandler(this, &CAsynDBLiteEx::OnKeyDown);
		m_dgv->UserDeletingRow += gcnew DataGridViewRowCancelEventHandler(this, &CAsynDBLiteEx::OnUserDeletingRow);
		m_dgv->RowEnter += gcnew DataGridViewCellEventHandler(this, &CAsynDBLiteEx::OnRowEnter);
	}
}

void CAsynDBLiteEx::OnRowEnter(Object^ sender, DataGridViewCellEventArgs^ e)
{
	if(e == nullptr || m_dgv == nullptr)
		return;
	if(e->RowIndex >= m_dgv->Rows->Count)
		return;
	if(m_dgv->Rows[e->RowIndex]->ErrorText->Length > 0)
	{
		m_dgv->Rows[e->RowIndex]->ErrorText = nullptr;
	}
}


void CAsynDBLiteEx::OnCellClick(Object ^sender, DataGridViewCellEventArgs ^e)
{
	if(e == nullptr || m_bInternalChanging)
		return;
	if(e->RowIndex < 0 || e->ColumnIndex < 0)
		return;
	if(m_dgv == nullptr || GetRowset()->Handle == 0)
		return;
	DataGridViewCell ^cell = m_dgv[e->ColumnIndex, e->RowIndex];
	if(cell->ErrorText->Length > 0)
	{
		cell->ErrorText = nullptr;
	}
}

int CAsynDBLiteEx::GetErrorColumn(String ^strErrorText)
{
	int nCol;
	int nCols = m_dgv->ColumnCount;
	String ^strColName;
	for(nCol=0; nCol<nCols; nCol++)
	{
		strColName = " '";
		strColName += m_dgv->Columns[nCol]->Name;
		strColName += "' ";
		if(strErrorText->Contains(strColName))
		{
			return nCol;
		}
	}
	return -1;
}

int CAsynDBLiteEx::GetRecordRowIndex(int nCellRowIndex)
{
	if(m_dgv == nullptr)
		return -1;
	if(nCellRowIndex < 0)
		return -1;
	if(nCellRowIndex >= m_dgv->Rows->Count)
		return -1;
	int nCols = (int)m_dgv->Rows[nCellRowIndex]->Cells->Count;
	DataGridViewRow ^row = m_dgv->Rows[nCellRowIndex];
	if(row == nullptr)
		return -1;
	DataGridViewCell ^cell = row->Cells[nCols-1];
	if(cell == nullptr)
		return -1;
	if(cell->Value == nullptr)
		return -1;
	int nRecordIndex = (int)cell->Value;
	nRecordIndex -= m_nStart;
	return nRecordIndex;
}

void CAsynDBLiteEx::OnCellValueChanged(Object^ sender, DataGridViewCellEventArgs^ e)
{
	HRESULT hr;
	if(e == nullptr || m_bInternalChanging)
		return;
	if(e->RowIndex < 0 || e->ColumnIndex < 0)
		return;
	if(m_dgv == nullptr || GetRowset()->Handle == 0 || IsRowsetReadonly)
		return;
	DataTable ^dt = (DataTable ^)m_dgv->DataSource;
	if(dt == nullptr)
		return;
	if(e->RowIndex >= dt->Rows->Count)
		return;
	int nRecordRowIndex = GetRecordRowIndex(e->RowIndex);
	if(nRecordRowIndex == -1)
		return;
	int nCol = e->ColumnIndex;
	DataGridViewCell ^cell = m_dgv[e->ColumnIndex, e->RowIndex];
	Object ^objOriginal = GetRowset()->GetData((short)((unsigned short)nRecordRowIndex), nCol + 1);
	Object ^vtData = cell->Value;
	if(vtData == DBNull::Value)
		vtData = nullptr;
	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	if(objOriginal != vtData && nRecordRowIndex < (unsigned short)sObtained)
	{
		DBErrors->Clear();
		if(vtData != nullptr && vtData->GetType() == Decimal::typeid)
		{
			if(GetRowset()->GetDataType(nCol + 1) == sdVT_CY)
			{
				Decimal dec = (Decimal)vtData;
				dec += (Decimal)0.00005;
				__int64 lMoney = (__int64)(dec*10000);
				CComVariant vtCY;
				vtCY.vt = VT_CY;
				vtCY.cyVal.int64 = lMoney;
				hr = m_pIURowset->SetData((short)((unsigned short)nRecordRowIndex), nCol + 1, vtCY);
				dec = lMoney;
				cell->Value = (dec/10000);
			}
			else
			{
				GetRowset()->SetData((short)((unsigned short)nRecordRowIndex), nCol + 1, vtData);
			}
		}
		else
		{
			GetRowset()->SetData((short)((unsigned short)nRecordRowIndex), nCol + 1, vtData);
		}
		GetRowset()->Update((short)((unsigned short)nRecordRowIndex));
		GetAttachedClientSocket()->WaitAll();
		if(DBErrors->Count > 0)
		{
			cell->ErrorText = DBErrors[0]->m_strErrorMsg;
			GetRowset()->SetData((short)((unsigned short)nRecordRowIndex), nCol + 1, objOriginal);
			if(objOriginal == nullptr)
				objOriginal = DBNull::Value;
			m_bInternalChanging = true;
			cell->Value = objOriginal;
			m_bInternalChanging = false;
		}
	}
}

void CAsynDBLiteEx::OnDataError(Object^ sender, DataGridViewDataErrorEventArgs^ e)
{
	if(m_dgv == nullptr || e == nullptr)
		return;
	e->Cancel = true;
	e->ThrowException = false;
	if(e->ColumnIndex > -1 && e->RowIndex > -1 && e->Exception != nullptr && e->Exception->Message != nullptr && e->Exception->Message->Length > 0)
	{
		DataGridViewCell ^cell = nullptr;
		int nCol = GetErrorColumn(e->Exception->Message);
		if(nCol != -1)
		{
			cell = m_dgv[nCol, e->RowIndex];
			if(cell != nullptr)
			{
				cell->ErrorText = e->Exception->Message;
			}
		}
		else
		{
			if(m_dgv->ShowCellErrors)
			{
				MessageBox::Show(e->Exception->Message, Application::ProductName, MessageBoxButtons::OK, MessageBoxIcon::Error);
			}
		}
	}
}
void CAsynDBLiteEx::OnUserDeletingRow(Object^ sender, DataGridViewRowCancelEventArgs^ e)
{
	if(e == nullptr)
		return;
	if(m_dgv == nullptr || GetRowset()->Handle == 0 || IsRowsetReadonly)
	{
		e->Cancel = true;
		return;
	}
	DataGridViewRow ^row = e->Row;
	int nCount = row->Cells->Count;
	if(nCount == 0)
		return;
	Object ^obj = row->Cells[nCount - 1]->Value;
	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	int nRecordRowIndex = GetRecordRowIndex(row->Cells[0]->RowIndex);
	if(nRecordRowIndex == -1 || (unsigned short)nRecordRowIndex >= (unsigned short)sObtained)
		return;
	DBErrors->Clear();
	GetRowset()->Delete((short)nRecordRowIndex);
	GetAttachedClientSocket()->WaitAll();
	if(DBErrors->Count > 0)
	{
		row->ErrorText = DBErrors[0]->m_strErrorMsg;
		e->Cancel = true;
	}
}

void CAsynDBLiteEx::OnKeyDown(Object^ sender, KeyEventArgs ^e)
{
	if(e == nullptr || !IsRowsetOpened)
		return;
	e->Handled = true;
	e->SuppressKeyPress = true;
	switch (e->KeyCode)
	{
	case Keys::Home:
		FirstBatch();
		break;
	case Keys::Next:
		NextBatch();
		break;
	case Keys::Prior:
		if(IsRowsetScrollable)
		{
			PrevBatch();
		}
		break;
	case Keys::End:
		if(IsRowsetScrollable)
		{
			LastBatch();
		}
		break;
	default:
		e->Handled = false;
		e->SuppressKeyPress = false;
		break;
	}
}

void CAsynDBLite::DeleteRow(int nRowIndex)
{
	if(!IsRowsetOpened)
		throw gcnew Exception("Rowset not available!");
	if(IsRowsetReadonly)
		throw gcnew Exception("Rowset is readonly!");
	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	if(nRowIndex < 0 || nRowIndex >= (int)sObtained)
		throw gcnew Exception("Invalid row index!");
	m_pIURowset->Delete((short)nRowIndex);
}

void CAsynDBLiteEx::DeleteRow(DataGridViewRow ^row)
{
	if(row == nullptr)
		throw gcnew Exception(gcnew String("Must pass in a valid data grid view row object!"));
	if(m_dgv == nullptr)
		throw gcnew Exception(gcnew String("The object is not attached with a data grid view object!"));
	if(!IsRowsetOpened)
		throw gcnew Exception("Rowset not available!");
	if(IsRowsetReadonly)
		throw gcnew Exception("Rowset is readonly!");
	int nCols = row->Cells->Count;
	int nIndex = (int)row->Cells[nCols -1]->Value;

	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	if(nIndex < 0 || nIndex >= (int)((unsigned short)sObtained))	
		return;

	DeleteRow(nIndex);
}

void CAsynDBLite::Update(int nRowIndex, int nColIndex, Object ^objNewValue)
{
	if(!IsRowsetOpened)
		throw gcnew Exception("Rowset not available!");
	if(IsRowsetReadonly)
		throw gcnew Exception("Rowset is readonly!");
	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	if(nRowIndex < 0 || nRowIndex >= (int)sObtained)
		throw gcnew Exception("Invalid row index!");
	long lCols;
	m_pIURowset->GetCols(&lCols);
	if(nColIndex < 0 || nColIndex >= lCols)
		throw gcnew Exception("Invalid column index!");
	VARIANT_BOOL bWritable;
	HRESULT hr = m_pIURowset->IsWritable(nColIndex+1, &bWritable);
	if(bWritable == VARIANT_FALSE)
		return;
	CComVariant vtData;
	if(objNewValue != nullptr && objNewValue != DBNull::Value)
	{
		if(objNewValue->GetType() == Decimal::typeid)
		{
			short sDBType;
			hr = m_pIURowset->GetDataType(nColIndex+1, &sDBType);
			if(sDBType == sdVT_CY)
			{
				Decimal dec = (Decimal)objNewValue;
				vtData.cyVal.int64 = (__int64)dec*10000; //CY
				vtData.vt = VT_CY;
			}
		}
		else
		{
			Marshal::GetNativeVariantForObject(objNewValue, IntPtr(&vtData));
		}
	}
	else
	{
		VARIANT_BOOL bNullable;
		m_pIURowset->IsNullable(nColIndex+1, &bNullable);
		if(bNullable == VARIANT_FALSE)
			throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Not nullable for the column ") + (nColIndex + 1).ToString());
	}
	hr = m_pIURowset->SetData(0, nColIndex+1, vtData);
	if(hr != S_OK)
	{
		throw gcnew CAsynDBException(hr, gcnew String("Data type mismatch for the column ") + (nColIndex + 1).ToString());
	}
	hr = m_pIURowset->Update((short)nRowIndex);

}

void CAsynDBLiteEx::Update(DataGridViewCell ^dgvCell)
{
	if(dgvCell == nullptr)
		throw gcnew Exception(gcnew String("Must pass in a valid data grid view cell object!"));
	if(m_dgv == nullptr)
		throw gcnew Exception(gcnew String("The object is not attached with a data grid view object!"));
	int nCol = dgvCell->ColumnIndex;
	int nRowIndex = GetRecordRowIndex(dgvCell->RowIndex);
	if(nRowIndex == -1)
		return;
	short sObtained;
	m_pIURowset->GetRowsFetched(&sObtained);
	if(nRowIndex < (unsigned short)sObtained)
	{
		Update(nRowIndex, nCol, dgvCell->Value);
	}
}

void CAsynDBLiteEx::AddRecord(DataGridViewRow ^dgvRow)
{
	AddRecord(dgvRow, false);
}

void CAsynDBLiteEx::AddRecord(DataGridViewRow ^dgvRow, bool bNeedNewRecord)
{
	if(dgvRow == nullptr)
		throw gcnew Exception(gcnew String("Must pass in a valid data grid view row object!"));

	if(m_dgv == nullptr)
		throw gcnew Exception(gcnew String("The object is not attached with a data grid view object!"));
	int n;
	int nCount = dgvRow->Cells->Count;
	array<Object^> ^aRow = gcnew array<Object^>(nCount);
	for (n=0; n<nCount; n++)
	{
		aRow[n] = dgvRow->Cells[n]->Value;
	}
	AddRecord(aRow, bNeedNewRecord);
}

void CAsynDBLite::AddRecord(array<Object^> ^aRow)
{
	AddRecord(aRow, false);
}

void CAsynDBLite::AddRecord(array<Object^> ^aRow, bool bNeedNewRecord)
{
	if(aRow == nullptr)
		return;
	if(!IsRowsetOpened)
		throw gcnew Exception("Rowset not available!");
	if(IsRowsetReadonly)
		throw gcnew Exception("Rowset is readonly!");
	HRESULT hr;
	long lCols;
	short sDBType;
	VARIANT_BOOL bWritable;
	CComVariant vtData;
	hr = m_pIURowset->GetCols(&lCols);
	if(aRow->Length < lCols)
		throw gcnew Exception("Data array size is smaller than rowset column size!");
	long n;
	
	for(n=0; n<lCols; n++)
	{
		hr = m_pIURowset->IsWritable(n+1, &bWritable);
		if(bWritable == VARIANT_FALSE)
			continue;
		if(aRow[n] != nullptr && aRow[n] != DBNull::Value)
		{
			if(aRow[n]->GetType() == Decimal::typeid)
			{
				hr = m_pIURowset->GetDataType(n+1, &sDBType);
				if(sDBType == sdVT_CY)
				{
					Decimal dec = (Decimal)aRow[n];
					vtData.cyVal.int64 = (__int64)dec*10000; //CY
					vtData.vt = VT_CY;
				}
			}
			else
			{
				Marshal::GetNativeVariantForObject(aRow[n], IntPtr(&vtData));
			}
		}
		else
		{
			VARIANT_BOOL bNullable;
			m_pIURowset->IsNullable(n+1, &bNullable);
			if(bNullable == VARIANT_FALSE)
				throw gcnew CAsynDBException(E_UNEXPECTED, gcnew String("Not nullable for the column ") + (n + 1).ToString());
		}
		hr = m_pIURowset->SetData(0, n+1, vtData);
		if(hr != S_OK)
		{
			throw gcnew CAsynDBException(hr, gcnew String("Data type mismatches for the column ") + (n + 1).ToString());
		}
		vtData.Clear();
	}
	hr = m_pIURowset->Add(bNeedNewRecord ? VARIANT_TRUE : VARIANT_FALSE);
}

void CAsynDBLiteEx::RemoveDGVDelegates()
{
	if(m_dgv != nullptr)
	{
		m_dgv->UserDeletingRow -= gcnew DataGridViewRowCancelEventHandler(this, &CAsynDBLiteEx::OnUserDeletingRow);
		m_dgv->KeyDown -= gcnew KeyEventHandler(this, &CAsynDBLiteEx::OnKeyDown);
		m_dgv->CellClick -= gcnew DataGridViewCellEventHandler(this, &CAsynDBLiteEx::OnCellClick);
		m_dgv->DataError -= gcnew DataGridViewDataErrorEventHandler(this, &CAsynDBLiteEx::OnDataError); 
		m_dgv->CellValueChanged -= gcnew DataGridViewCellEventHandler(this, &CAsynDBLiteEx::OnCellValueChanged);
	}
}

void CAsynDBLiteEx::OnResultReturned(short sRequestID, CUQueue ^UQueue)
{
	switch(sRequestID)
	{
	case idRowsetOpenFromHandle:
	case idRowsetGetProviders:
	case idRowsetGetSchemaRowset:
	case idRowsetOpen:
		if(m_dgv != nullptr && IsRowsetOpened)
		{
			//append a hidden index to the end of data grid view to track rowset index no matter how you sort a grid view
			DataColumn ^dc = gcnew DataColumn(CurrentDataTable->TableName + "Index");
			dc->DataType = int::typeid;
			dc->AllowDBNull = false;
			dc->AutoIncrement = true;
			dc->AutoIncrementSeed = 0;
			dc->AutoIncrementStep = 1;
			CurrentDataTable->Columns->Add(dc);
			dc->ReadOnly = false;
			m_dgv->DataSource = CurrentDataTable;
			m_dgv->Columns[m_dgv->ColumnCount-1]->Visible = false;
		}
		break;
	case idRowsetStartFetchingBatch:
		m_bInternalChanging = true;
		break;
	case idRowsetSendSubBatch:
		if(!m_bAsynFetching && m_dgv != nullptr && m_dgv->DataSource != nullptr)
		{
			RemovePrevErrors();
		}
		break;
	case idRowsetGetRowsAt:
	case idRowsetGetBatchRecords:
	case idRowsetGetBatchRecordsEx:
	case idRowsetGetBatchRecordsLast:
	case idRowsetMoveFirst:
	case idRowsetMoveNext:
	case idRowsetMoveLast:
	case idRowsetMovePrev:
	case idRowsetAsynFetch:
		if(m_dgv != nullptr && IsRowsetOpened && m_dgv->DataSource != nullptr)
		{
			long nCol;
			long nCols;
			VARIANT_BOOL b;
			VARIANT_BOOL bReadOnly;
			m_pIURowset->GetCols(&nCols);
			m_pIURowset->IsReadOnly(&bReadOnly);
			for(nCol = 0; nCol<nCols; nCol++)
			{
				DataGridViewColumn ^dc = m_dgv->Columns[nCol];
				m_pIURowset->IsWritable(nCol + 1, &b);
				bool bRO = ((b == VARIANT_FALSE) || (bReadOnly != VARIANT_FALSE));
				if(bRO != dc->ReadOnly)
					dc->ReadOnly = bRO;
			}
			
			m_dgv->Columns[m_dgv->ColumnCount-1]->Visible = false;
			
			if(CurrentDataTable->Rows->Count > 0)
			{
				int nLastColumn = m_dgv->Columns->Count - 1;
				m_nStart = (unsigned int)((int)m_dgv[nLastColumn, 0]->Value);
			}
		}
		m_bInternalChanging = false;
		break;
	default:
		break;
	}
}

void CAsynDBLiteEx::RemovePrevErrors()
{
	int nCell;
	int nRow;
	unsigned short sSubBatch = (unsigned short)SubBatchSize;
	if(sSubBatch > GetRowset()->GetRowsFetched())
		sSubBatch = (unsigned short)GetRowset()->GetRowsFetched();
	int nRows = (unsigned short)sSubBatch;
	int nCells = m_dgv->Columns->Count - 1;
	for(nRow = nRows - 1; nRow >= 0; nRow--)
	{
		DataGridViewRow ^row = m_dgv->Rows[nRow];
		if(row->ErrorText->Length > 0)
			row->ErrorText = nullptr;
		for(nCell = 0; nCell < nCells; nCell++)
		{
			DataGridViewCell ^cell = row->Cells[nCell];
			if(cell->ErrorText->Length > 0)
			{
				cell->ErrorText = nullptr;
			}
		}
	}
	
}

CAsynDBLiteEx::CAsynDBLiteEx() 
	: m_bInternalChanging(false), m_nStart(0)
{
	
}

CAsynDBLiteEx::CAsynDBLiteEx(CClientSocket ^cs) 
	: CAsynDBLite(cs), m_bInternalChanging(false), m_nStart(0)
{

}

CAsynDBLiteEx::CAsynDBLiteEx(CClientSocket ^cs, IAsyncResultsHandler ^DefaultAsyncResultsHandler) 
	: CAsynDBLite(cs, DefaultAsyncResultsHandler), m_bInternalChanging(false), m_nStart(0)
{

}

CAsynDBLiteEx::~CAsynDBLiteEx()
{
	RemoveDGVDelegates();
	m_dgv = nullptr;
}

}
}
}

#endif