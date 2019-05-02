#include "stdafx.h"
#include "RemotingAdoNetHelper.h"

using namespace System;
using namespace System::Data;
using namespace System::Collections;

#ifndef __NO_OPEN_MP__
	#include <omp.h>
#endif

namespace SocketProAdapter
{
	CAsyncAdoSerializationHelper::CAsyncAdoSerializationHelper()
	{
		m_bDataReader = false;
		m_qBit = gcnew CUQueue();
		m_qTemp = gcnew CUQueue();
		m_Buffer = nullptr;
		m_dts = nullptr;
		m_nBatchSize = 0;
		m_bLoadingDataTable = false;
		m_nAffected = 0;
		m_bDataSet = false;
		m_bParallel = false;
#ifndef __NO_OPEN_MP__
		m_lstUQueue = gcnew List<CUQueue^>();
#endif
	}

	CAsyncAdoSerializationHelper::CAsyncAdoSerializationHelper(CAsyncAdoSerializationHelper ^AsyncAdoSerializationHelper)
	{
		m_bParallel = false;
		m_bDataReader = false;
		m_qBit = gcnew CUQueue();
		m_qTemp = gcnew CUQueue();
		m_Buffer = nullptr;
		m_bLoadingDataTable = false;
		m_nAffected = 0;
		m_bDataSet = false;
#ifndef __NO_OPEN_MP__
		m_lstUQueue = gcnew List<CUQueue^>();
#endif
		if(AsyncAdoSerializationHelper != nullptr && AsyncAdoSerializationHelper->m_dts != nullptr)
		{
			int n;
			m_bLoadingDataTable = AsyncAdoSerializationHelper->m_bLoadingDataTable;
			m_dts = gcnew array<tagDataTypeSupported>(AsyncAdoSerializationHelper->m_dts->Length);
			for (n = 0; n < m_dts->Length; n++)
			{
				tagDataTypeSupported dts = AsyncAdoSerializationHelper->m_dts[n];
				m_dts[n] = dts;
			}
			m_nBatchSize = AsyncAdoSerializationHelper->m_nBatchSize;
		}
		else
		{
			m_dts = nullptr;
			m_nBatchSize = 0;
		}
	}

	CAsyncAdoSerializationHelper::CAsyncAdoSerializationHelper(IDataReader ^dr)
	{
		m_nAffected = 0;
		m_bDataReader = false;
		m_qBit = gcnew CUQueue();
		m_qTemp = gcnew CUQueue();
		m_Buffer = nullptr;
		m_bLoadingDataTable = false;
		m_bDataSet = false;
		m_bParallel = false;
#ifndef __NO_OPEN_MP__
		m_lstUQueue = gcnew List<CUQueue^>();
#endif
		if(dr == nullptr)
			m_dts = nullptr;
		else
		{
			int n;
			m_dts = gcnew array<tagDataTypeSupported>(dr->FieldCount);
			for (n = 0; n < dr->FieldCount; n++)
			{
				tagDataTypeSupported dts = GetDT(dr->GetFieldType(n));
				m_dts[n] = dts;
			}
		}
		m_nBatchSize = 0;
	}

	CAsyncAdoSerializationHelper::CAsyncAdoSerializationHelper(DataTable ^dt)
	{
		m_bDataReader = false;
		m_qBit = gcnew CUQueue();
		m_qTemp = gcnew CUQueue();
		m_Buffer = nullptr;
		m_bLoadingDataTable = false;
		m_nAffected = 0;
		m_bDataSet = false;
		m_bParallel = false;

#ifndef __NO_OPEN_MP__
		m_lstUQueue = gcnew List<CUQueue^>();
#endif
		if(dt == nullptr)
			m_dts = nullptr;
		else
		{
			int n;
			m_dts = gcnew array<tagDataTypeSupported>(dt->Columns->Count);
			for (n = 0; n < dt->Columns->Count; n++)
			{
				m_dts[n] = GetDT(dt->Columns[n]->DataType->FullName);
			}
		}
		m_nBatchSize = 0;
	}

	CAsyncAdoSerializationHelper::~CAsyncAdoSerializationHelper()
	{
		delete m_qBit;
		delete m_qTemp;
		m_Buffer = nullptr;
	}

	tagDataTypeSupported CAsyncAdoSerializationHelper::GetDT(Type ^type)
	{
		if (type == nullptr)
			return tagDataTypeSupported::dtUnknown;
		return GetDT(type->FullName);
	}

	Type ^CAsyncAdoSerializationHelper::GetType(tagDataTypeSupported dt)
	{
		switch (dt)
		{
			case tagDataTypeSupported::dtBoolean:
				return bool::typeid;
			case tagDataTypeSupported::dtByte:
				return System::Byte::typeid;
			case tagDataTypeSupported::dtBytes:
				return array<unsigned char>::typeid;
			case tagDataTypeSupported::dtChar:
				return System::Char::typeid;
			case tagDataTypeSupported::dtChars:
				return array<wchar_t>::typeid;
			case tagDataTypeSupported::dtDateTime:
				return DateTime::typeid;
			case tagDataTypeSupported::dtDecimal:
				return System::Decimal::typeid;
			case tagDataTypeSupported::dtDouble:
				return double::typeid;
			case tagDataTypeSupported::dtFloat:
				return float::typeid;
			case tagDataTypeSupported::dtGuid:
				return Guid::typeid;
			case tagDataTypeSupported::dtInt16:
				return short::typeid;
			case tagDataTypeSupported::dtInt32:
				return int::typeid;
			case tagDataTypeSupported::dtInt64:
				return System::Int64::typeid;
			case tagDataTypeSupported::dtUInt16:
				return System::UInt16::typeid;
			case tagDataTypeSupported::dtUInt32:
				return System::UInt32::typeid;
			case tagDataTypeSupported::dtUInt64:
				return System::UInt64::typeid;
			case tagDataTypeSupported::dtString:
				return System::String::typeid;
			case tagDataTypeSupported::dtValue:
				return System::Object::typeid;
			case tagDataTypeSupported::dtValues:
				return array<Object^>::typeid;
			case tagDataTypeSupported::dtNull:
				return DBNull::typeid;
			case tagDataTypeSupported::dtDateTimeOffset:
				return DateTimeOffset::typeid;
			case tagDataTypeSupported::dtTimeSpan:
				return TimeSpan::typeid;
			default:
				throw gcnew Exception("Unsupported data type!");
		}
	}

	tagDataTypeSupported CAsyncAdoSerializationHelper::GetDT(System::String ^strType)
	{
		if (strType == "System.String")
		{
			return tagDataTypeSupported::dtString;
		}
		else if (strType == "System.Int32")
		{
			return tagDataTypeSupported::dtInt32;
		}
		else if (strType == "System.Single")
		{
			return tagDataTypeSupported::dtFloat;
		}
		else if (strType == "System.DateTime")
		{
			return tagDataTypeSupported::dtDateTime;
		}
		else if (strType == "System.Double")
		{
			return tagDataTypeSupported::dtDouble;
		}
		else if (strType == "System.Int16")
		{
			return tagDataTypeSupported::dtInt16;
		}
		else if (strType == "System.Boolean")
		{
			return tagDataTypeSupported::dtBoolean;
		}
		else if (strType == "System.Byte[]")
		{
			return tagDataTypeSupported::dtBytes;
		}
		else if (strType == "System.Decimal")
		{
			return tagDataTypeSupported::dtDecimal;
		}
		else if (strType == "System.Guid")
		{
			return tagDataTypeSupported::dtGuid;
		}
		else if (strType == "System.Int64")
		{
			return tagDataTypeSupported::dtInt64;
		}
		else if (strType == "System.UInt16")
		{
			return tagDataTypeSupported::dtUInt16;
		}
		else if (strType == "System.UInt32")
		{
			return tagDataTypeSupported::dtUInt32;
		}
		else if (strType == "System.UInt64")
		{
			return tagDataTypeSupported::dtUInt64;
		}
		else if (strType == "System.Object")
		{
			return tagDataTypeSupported::dtValue;
		}
		else if (strType == "System.Object[]")
		{
			return tagDataTypeSupported::dtValues;
		}
		else if (strType == "System.DBNull")
		{
			return tagDataTypeSupported::dtNull;
		}
		else if (strType == "System.Byte")
		{
			return tagDataTypeSupported::dtByte;
		}
		else if (strType == "System.Char")
		{
			return tagDataTypeSupported::dtChar;
		}
		else if (strType == "System.Char[]")
		{
			return tagDataTypeSupported::dtChars;
		}
		else if (strType == "System.UInt16[]")
		{
			return tagDataTypeSupported::dtChars;
		}
		else if (strType == "System.TimeSpan")
		{
			return tagDataTypeSupported::dtTimeSpan;
		}
		else if (strType == "System.DateTimeOffset")
		{
			return tagDataTypeSupported::dtDateTimeOffset;
		}
		else
		{
			throw gcnew Exception("Unsupported data type!");
		}
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, IDataReader ^dr)
	{
		int n;
		bool bNull;
		unsigned char b = 0;
		unsigned char bOne = 1;
		m_qBit->SetSize(0);
		m_qTemp->SetSize(0);
		int nLen = m_dts->Length;
		for (n = 0; n < nLen; n++)
		{
			bNull = dr->IsDBNull(n);
			if (bNull)
			{
				b += (unsigned char)(bOne << (unsigned char)(n % 8));
			}
			if ((n % 8) == 7)
			{
				m_qBit->Push(b);
				b = 0;
			}
			if (bNull)
				continue;
			switch (m_dts[n])
			{
				case tagDataTypeSupported::dtBoolean:
					m_qTemp->Push(dr->GetBoolean(n));
					break;
				case tagDataTypeSupported::dtByte:
					m_qTemp->Push(dr->GetByte(n));
					break;
				case tagDataTypeSupported::dtChar:
					m_qTemp->Push(dr->GetChar(n));
					break;
				case tagDataTypeSupported::dtDateTime:
					m_qTemp->Push(dr->GetDateTime(n));
					break;
				case tagDataTypeSupported::dtDecimal:
					m_qTemp->Push(dr->GetDecimal(n));
					break;
				case tagDataTypeSupported::dtDouble:
					m_qTemp->Push(dr->GetDouble(n));
					break;
				case tagDataTypeSupported::dtFloat:
					m_qTemp->Push(dr->GetFloat(n));
					break;
				case tagDataTypeSupported::dtGuid:
					m_qTemp->Push(dr->GetGuid(n));
					break;
				case tagDataTypeSupported::dtInt16:
					m_qTemp->Push(dr->GetInt16(n));
					break;
				case tagDataTypeSupported::dtInt32:
					m_qTemp->Push(dr->GetInt32(n));
					break;
				case tagDataTypeSupported::dtInt64:
					m_qTemp->Push(dr->GetInt64(n));
					break;
				case tagDataTypeSupported::dtString:
					{
						System::String ^str = dr->GetString(n);
						m_qTemp->Save(str);
					}
					break;
				case tagDataTypeSupported::dtBytes:
					{
						int nBytes = safe_cast<int>(dr->GetBytes(n, safe_cast<System::Int64>(0), nullptr, 0, 0));
						if (m_Buffer == nullptr || nBytes > m_Buffer->Length)
							m_Buffer = gcnew array<System::Byte>(nBytes + 1024);
						dr->GetBytes(n, safe_cast<System::Int64>(0), m_Buffer, 0, nBytes);
						m_qTemp->Push(nBytes);
						m_qTemp->Push(m_Buffer, nBytes);
					}
					break;
				case tagDataTypeSupported::dtUInt64:
				case tagDataTypeSupported::dtUInt32:
				case tagDataTypeSupported::dtUInt16:
				case tagDataTypeSupported::dtValue:
				case tagDataTypeSupported::dtValues:
				case tagDataTypeSupported::dtTimeSpan:
				case tagDataTypeSupported::dtDateTimeOffset:
					{
						System::Object ^obj = dr->GetValue(n);
						m_qTemp->Push(obj, false, false);
					}
					break;
				default:
					throw gcnew Exception("Unsupported data type for serialization!");
			}
		}
		if ((n % 8) != 0)
			m_qBit->Push(b);
		UQueue->Push(m_qBit->GetBuffer(), m_qBit->GetSize());
		UQueue->Push(m_qTemp->GetBuffer(), m_qTemp->GetSize());
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, DataRow ^dr)
	{
		int n;
		bool bNull;
		System::Byte b = 0;
		System::Byte bOne = 1;
		m_qBit->SetSize(0);
		m_qTemp->SetSize(0);
		bool bDeleted = (dr->RowState == DataRowState::Deleted);
		if(bDeleted)
			dr->RejectChanges();
		array<Object^> ^data = dr->ItemArray;
		int nLen = m_dts->Length;
		for (n = 0; n < nLen; n++)
		{
			Object ^myData = data[n];
			bNull = (myData == nullptr || myData == DBNull::Value);
			if (bNull)
			{
				b += (unsigned char)(bOne << (unsigned char)(n % 8));
			}
			if ((n % 8) == 7)
			{
				m_qBit->Push(b);
				b = 0;
			}
			if (bNull)
				continue;
			switch (m_dts[n])
			{
				case tagDataTypeSupported::dtBoolean:
					m_qTemp->Push(safe_cast<bool>(myData));
					break;
				case tagDataTypeSupported::dtByte:
					m_qTemp->Push(safe_cast<System::Byte>(myData));
					break;
				case tagDataTypeSupported::dtChar:
					m_qTemp->Push(safe_cast<System::Char>(myData));
					break;
				case tagDataTypeSupported::dtDateTime:
					m_qTemp->Push(safe_cast<DateTime>(myData));
					break;
				case tagDataTypeSupported::dtDecimal:
					m_qTemp->Push(safe_cast<System::Decimal>(myData));
					break;
				case tagDataTypeSupported::dtDouble:
					m_qTemp->Push(safe_cast<double>(myData));
					break;
				case tagDataTypeSupported::dtFloat:
					m_qTemp->Push(safe_cast<float>(myData));
					break;
				case tagDataTypeSupported::dtGuid:
					m_qTemp->Push(safe_cast<Guid>(myData));
					break;
				case tagDataTypeSupported::dtUInt16:
					m_qTemp->Push(safe_cast<System::UInt16>(myData));
					break;
				case tagDataTypeSupported::dtUInt32:
					m_qTemp->Push(safe_cast<System::UInt32>(myData));
					break;
				case tagDataTypeSupported::dtUInt64:
					m_qTemp->Push(safe_cast<System::UInt64>(myData));
					break;
				case tagDataTypeSupported::dtInt16:
					m_qTemp->Push(safe_cast<short>(myData));
					break;
				case tagDataTypeSupported::dtInt32:
					m_qTemp->Push(safe_cast<int>(myData));
					break;
				case tagDataTypeSupported::dtInt64:
					m_qTemp->Push(safe_cast<System::Int64>(myData));
					break;
				case tagDataTypeSupported::dtString:
					m_qTemp->Save(safe_cast<System::String^>(myData));
					break;
				case tagDataTypeSupported::dtValue:
				case tagDataTypeSupported::dtValues:
				case tagDataTypeSupported::dtChars:
				case tagDataTypeSupported::dtBytes:
				case tagDataTypeSupported::dtDateTimeOffset:
				case tagDataTypeSupported::dtTimeSpan:
					m_qTemp->Push(myData, false, false);
					break;
				default:
					throw gcnew Exception("Unsupported data type for serialization!");
			}
		}
		if ((n % 8) != 0)
			m_qBit->Push(b);
		UQueue->Push(m_qBit->GetBuffer(), m_qBit->GetSize());
		UQueue->Push(m_qTemp->GetBuffer(), m_qTemp->GetSize());
		if(bDeleted)
			dr->Delete();
		UQueue->Push((System::Byte)dr->RowState);
		UQueue->Push(dr->HasErrors);
		if (dr->HasErrors)
			UQueue->Save(dr->RowError);
	}

	void CAsyncAdoSerializationHelper::PushHeader(CUQueue ^UQueue, DataTable ^dt, bool bNeedParentRelations, bool bNeedChildRelations)
	{
		int n;
		int nSize;
		if (dt == nullptr)
			return;
		//UQueue->SetSize(0);
		m_dts = gcnew array<tagDataTypeSupported>(dt->Columns->Count);
		UQueue->Push(dt->Rows->Count);
		System::Byte bData = 0;
		if (bNeedParentRelations)
			bData += 2;
		if (bNeedChildRelations)
			bData += 4;
		UQueue->Push(bData);
		UQueue->Save(dt->TableName);
		Push(UQueue, dt->Columns);
		for (n = 0; n < dt->Columns->Count; n++)
		{
			m_dts[n] = GetDT(dt->Columns[n]->DataType->FullName);
		}
		UQueue->Save(dt->DisplayExpression);
		UQueue->Push(dt->MinimumCapacity);
		UQueue->Save(dt->Namespace);
		UQueue->Save(dt->Prefix);
		nSize = dt->PrimaryKey->Length;
		UQueue->Push(nSize);
		for (n = 0; n < nSize; n++)
		{
			UQueue->Push(dt->PrimaryKey[n]->Ordinal);
		}
		if (bNeedParentRelations)
		{
			Push(UQueue, dt->ParentRelations);
		}
		if (bNeedChildRelations)
		{
			Push(UQueue, dt->ChildRelations);
		}
	}

	int CAsyncAdoSerializationHelper::CountNames(ArrayList ^al, System::String ^strColName)
	{
		int n;
		int nCount = 0;
		int nSize = al->Count;
		for (n = 0; n < nSize; n++)
		{
			System::String ^str = safe_cast<System::String^>(al[n]);
			if (str != nullptr && System::String::Compare(str, strColName, true) == 0)
			{
				++nCount;
			}
		}
		return nCount;
	}

	void CAsyncAdoSerializationHelper::PushHeader(CUQueue ^UQueue, IDataReader ^dr)
	{
		int n;
		int nCount;
		ArrayList ^al = gcnew ArrayList();
		//UQueue->SetSize(0);
		UQueue->Push(dr->FieldCount);
		UQueue->Push(dr->RecordsAffected);
		DataTable ^dtSchema = dr->GetSchemaTable();
		int nColumnName = dtSchema->Columns->IndexOf("ColumnName");
		int nColumnSize = dtSchema->Columns->IndexOf("ColumnSize");
		int nDataType = dtSchema->Columns->IndexOf("DataType");
		int nIsLong = dtSchema->Columns->IndexOf("IsLong");
		int nAllowDBNull = dtSchema->Columns->IndexOf("AllowDBNull");
		int nIsReadOnly = dtSchema->Columns->IndexOf("IsReadOnly");
		int nIsRowVersion = dtSchema->Columns->IndexOf("IsRowVersion");
		int nIsUnique = dtSchema->Columns->IndexOf("IsUnique");
		int nIsKey = dtSchema->Columns->IndexOf("IsKey"); //IsKeyColumn
		int nIsAutoIncrement = dtSchema->Columns->IndexOf("IsAutoIncrement");
		m_dts = gcnew array<tagDataTypeSupported>(dr->FieldCount);
		for (n = 0; n < dr->FieldCount; n++)
		{
			tagDataTypeSupported dts = GetDT(dr->GetFieldType(n));
			m_dts[n] = dts;
			UQueue->Push(safe_cast<short>(dts));
		}
		for each (DataRow ^row in dtSchema->Rows)
		{
			int nData = 0;
			System::String ^str = nullptr;
			if (nIsAutoIncrement != -1)
			{
				if (row[nIsAutoIncrement]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsAutoIncrement);
			}
	
			if (nIsKey != -1)
			{
				if (row[nIsKey]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsKey);
			}
	
			if (nAllowDBNull != -1)
			{
				if (row[nAllowDBNull]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbAllowDBNull);
			}
	
			if (nIsReadOnly != -1)
			{
				if (row[nIsReadOnly]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsReadOnly);
			}
	
			if (nIsRowVersion != -1)
			{
				if (row[nIsRowVersion]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsRowVersion);
			}
	
			if (nIsUnique != -1)
			{
				if (row[nIsUnique]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsUnique);
			}
	
			if (nIsLong != -1)
			{
				if (row[nIsLong]->Equals(true))
					nData |= safe_cast<int>(tagColumnBit::cbIsLong);
			}
	
			UQueue->Push(nData);
	
			nData = 0;
			if (nColumnSize != -1)
			{
				nData = safe_cast<int>(row[nColumnSize]);
			}
			UQueue->Push(nData);
	
			if (nColumnName != -1)
			{
				str = safe_cast<System::String^>(row[nColumnName]);
				nCount = CountNames(al, str);
				if (nCount > 0)
					str += nCount.ToString();
				al->Add(str);
			}
			UQueue->Save(str);
		}
	}

	void CAsyncAdoSerializationHelper::PushHeader(CUQueue ^UQueue, DataSet ^ds)
	{
		System::Byte bData = 0;
		//UQueue->SetSize(0);
		if (ds->CaseSensitive)
			bData += 2;
		if (ds->EnforceConstraints)
			bData += 4;
		UQueue->Push(ds->Tables->Count);
		UQueue->Push(bData);
		UQueue->Save(ds->DataSetName);
		UQueue->Save(ds->Namespace);
		UQueue->Save(ds->Prefix);
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, DataColumn ^dc)
	{
		bool bNull = (dc == nullptr);
		UQueue->Push(bNull);
		if (bNull)
			return;
		System::Byte bData = 0;
		if (dc->AllowDBNull)
			bData += safe_cast<System::Byte>(tagColumnBit::cbAllowDBNull);
		if (dc->AutoIncrement)
			bData += safe_cast<System::Byte>(tagColumnBit::cbIsAutoIncrement);
		if (dc->ReadOnly)
			bData += safe_cast<System::Byte>(tagColumnBit::cbIsReadOnly);
		if (dc->Unique)
			bData += safe_cast<System::Byte>(tagColumnBit::cbIsUnique);
		UQueue->Push(bData);
		UQueue->Push(dc->AutoIncrementSeed);
		UQueue->Push(dc->AutoIncrementStep);
		UQueue->Save(dc->Caption);
		UQueue->Push(safe_cast<System::Byte>(dc->ColumnMapping));
		UQueue->Save(dc->ColumnName);
		UQueue->Push(safe_cast<short>(GetDT(dc->DataType->FullName)));
		UQueue->Push(dc->DefaultValue, false, false);
		UQueue->Save(dc->Expression);
		UQueue->Push(dc->MaxLength);
		UQueue->Save(dc->Namespace);
		UQueue->Save(dc->Prefix);
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, DataColumnCollection ^Cols)
	{
		bool bNull = (Cols == nullptr);
		UQueue->Push(bNull);
		if (!bNull)
		{
			int n;
			int nLen = 0;
			if (Cols != nullptr && Cols->Count > 0)
				nLen = Cols->Count;
			UQueue->Push(nLen);
			for (n = 0; n < nLen; n++)
			{
				Push(UQueue, Cols[n]);
			}
		}
	}

	void CAsyncAdoSerializationHelper::PushTableColNamesOnly(CUQueue ^UQueue, array<DataColumn^> ^dcs)
	{
		UQueue->Push(dcs->Length);
		for each (DataColumn ^dc in dcs)
		{
			UQueue->Save(dc->Table->TableName);
			UQueue->Save(dc->Ordinal);
		}
	}

	int CAsyncAdoSerializationHelper::PopTableColNamesOnly(CUQueue ^UQueue, array<DataColumn^> ^%dcs)
	{
		int n;
		int count;
		int ordinal;
		String ^tableName;
		int start = UQueue->GetSize();
		UQueue->Pop(count);
		dcs = gcnew array<DataColumn^>(count);
		for(n=0; n<count; ++n)
		{
			UQueue->Load(tableName);
			UQueue->Pop(ordinal);
			dcs[n] = CurrentDataSet->Tables[tableName]->Columns[ordinal];
		}
		return (start - UQueue->GetSize());

	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, array<DataColumn^> ^dcs)
	{
		if (dcs == nullptr)
		{
			UQueue->Push(safe_cast<int>(-1));
		}
		else
		{
			UQueue->Push(dcs->Length);
			for each (DataColumn ^dc in dcs)
			{
				Push(UQueue, dc);
			}
		}
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, ForeignKeyConstraint ^fkc)
	{
		bool bNull = (fkc == nullptr);
		UQueue->Push(bNull);
		if (!bNull)
		{
			Push(UQueue, fkc->Columns);
			Push(UQueue, fkc->RelatedColumns);
			UQueue->Save(fkc->ConstraintName);
			UQueue->Push(safe_cast<System::Byte>(fkc->DeleteRule));
			UQueue->Push(safe_cast<System::Byte>(fkc->AcceptRejectRule));
			UQueue->Push(safe_cast<System::Byte>(fkc->UpdateRule));
		}
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, UniqueConstraint ^uc)
	{
		bool bNull = (uc == nullptr);
		UQueue->Push(bNull);
		if (!bNull)
		{
			Push(UQueue, uc->Columns);
			UQueue->Save(uc->ConstraintName);
			UQueue->Push(uc->IsPrimaryKey);
		}
	}

	void CAsyncAdoSerializationHelper::Push(CUQueue ^UQueue, DataRelationCollection ^drc)
	{
		UQueue->Push(drc->Count);
		for each (DataRelation ^dr in drc)
		{
			PushTableColNamesOnly(UQueue, dr->ChildColumns);
			UQueue->Push(dr->Nested);
			UQueue->Save(dr->RelationName);
			PushTableColNamesOnly(UQueue, dr->ParentColumns);
		}
	}

	DataRelationCollection ^CAsyncAdoSerializationHelper::LoadDataSetRelations(CUQueue ^UQueue)
	{
		DataRelationCollection ^drc = nullptr;
		if (UQueue->GetSize() > 0)
		{
			CurrentDataSet->Relations->Clear();
			drc = CurrentDataSet->Relations;
			Pop(UQueue, drc);
		}
		return drc;
	}

	int CAsyncAdoSerializationHelper::LoadRows(CUQueue ^UQueue)
	{
		int nGet = 0;
		if(m_bLoadingDataTable)
		{
			nGet = LoadDataTableRows(UQueue);
		}
		else
		{
			nGet = LoadDataReaderRecords(UQueue);
		}
		return nGet;
	}

	int CAsyncAdoSerializationHelper::LoadDataTableRows(CUQueue ^UQueue)
	{
		if (UQueue->GetSize() == 0)
			return 0;
		DataTable ^dt = m_dt;
#ifndef __NO_OPEN_MP__
		if(m_bParallel && m_nBatchSize > 0)
		{
			if(!(UQueue->GetSize() <= m_nBatchSize && m_lstUQueue->Count == 0))
			{
				if(m_lstUQueue->Count == (Environment::ProcessorCount - 1))
				{
					m_lstUQueue->Add(UQueue);
				}
				else
				{
					CUQueue ^temp = LockUQueue();
					temp->SetSize(0);
					temp->Push(UQueue->GetBuffer(), UQueue->GetSize());
					UQueue->SetSize(0);
					m_lstUQueue->Add(temp);
				}
				int nGet = 0;
				if(m_lstUQueue->Count == Environment::ProcessorCount)
				{
					int n;
					nGet = LoadDataTableRows(m_lstUQueue);
					for(n=0; n<(Environment::ProcessorCount - 1); n++)
					{
						UnlockUQueue(m_lstUQueue[n]);
					}
					m_lstUQueue->Clear();
				}
				return nGet;
			}
		}
#endif
		if (dt == nullptr)
			throw gcnew Exception("Must pass in an valid DataTable object!");
		bool b;
		DataRowState drs;
		DataRow ^dr;
		int nSize = 0;
		array<Object^> ^aData = nullptr;
		//dt->BeginLoadData();
		while (UQueue->GetSize() > 0)
		{
			Pop(UQueue, aData, drs);
			switch(drs)
			{
			case DataRowState::Added:
				dr = dt->Rows->Add(aData);
				break;
			case DataRowState::Modified:
				dr = dt->Rows->Add(aData);
				dr->AcceptChanges();
				dr->SetModified();		
				break;
			case DataRowState::Deleted:
				dr = dt->Rows->Add(aData);
				dr->AcceptChanges();
				dr->Delete();
				break;
			case DataRowState::Unchanged:
				dr = dt->Rows->Add(aData);
				dr->AcceptChanges();
				break;
			default: //DataRowState.Detached
				throw gcnew Exception("Wrong DataRow state!");
			}
			UQueue->Pop(b);
			if (b)
			{
				String ^str = nullptr;
				UQueue->Load(str);
				dr->RowError = str;
			}
			++nSize;
		}
		//dt->EndLoadData();
		return nSize;
	}

#ifndef __NO_OPEN_MP__
	void CAsyncAdoSerializationHelper::StartLoadingInParallel()
	{
		if(Environment::ProcessorCount > 1)
		{
			m_bParallel = true;
		}
	}

	int CAsyncAdoSerializationHelper::EndLoadingInParallel()
	{
		int nGet = 0;
		DataTable ^dt = m_dt;
		if(m_bParallel)
		{
			if(m_lstUQueue->Count > 0)
			{
				int n;
				if(dt != nullptr)
				{
					if(m_bLoadingDataTable)
						nGet = LoadDataTableRows(m_lstUQueue);
					else
						nGet = LoadDataReaderRecords(m_lstUQueue);
				}
				int nSize = m_lstUQueue->Count;
				for(n= 0; n<nSize; n++)
				{
					UnlockUQueue(m_lstUQueue[n]);
				}
				m_lstUQueue->Clear();
			}
			m_bParallel = false;
		}
		return nGet;
	}

	int CAsyncAdoSerializationHelper::LoadDataTableRows(List<CUQueue^> ^lstUQueue)
	{
		int d;
		int nGet = 0;
		if(lstUQueue == nullptr || lstUQueue->Count == 0)
			return 0;
		DataTable ^dt = m_dt;
		if(dt == nullptr || dt->Columns->Count == 0)
			throw gcnew Exception("Must pass in an valid DataTable object!");
		int nSize = lstUQueue->Count;
		List<CPDataRowContext^> ^aContext = gcnew List<CPDataRowContext^>();
		for(d=0; d<nSize; d++)
		{
			CPDataRowContext ^context = gcnew CPDataRowContext();
			context->m_UQueue = lstUQueue[d];
			context->m_lstRow = gcnew List<array<Object^>^>();
			context->m_lstState = gcnew List<DataRowState>();
			context->m_lstError = gcnew List<String^>();
			aContext->Add(context);
		}
		omp_set_dynamic(0);
		omp_set_num_threads(Environment::ProcessorCount);
		#pragma omp parallel
		{
			int n;
			int nLen = 0;
			#pragma omp for
			for(n=0; n<nSize; ++n)
			{
				CUQueue ^UQueue = aContext[n]->m_UQueue;
				while (UQueue->GetSize() > 0)
				{
					DataRowState	drs;
					array<Object^> ^aData = nullptr;
					String ^strError = nullptr;
					nLen += PopDataTableRow(UQueue, aData, drs, strError);
					aContext[n]->m_lstRow->Add(aData);
					aContext[n]->m_lstState->Add(drs);
					aContext[n]->m_lstError->Add(strError);
				}
			}
			#pragma omp barrier
			{
				nGet += nLen;
			}
		}
		Object ^obj;
		DataRowState drs;
		DataRow ^dr;
		nGet = 0;
//		dt->BeginLoadData();
		for(d=0; d<nSize; d++)
		{
			int j;
			CPDataRowContext ^context = aContext[d];
			List<array<Object^>^> ^lstRow = context->m_lstRow;
			List<DataRowState> ^lstDRS = context->m_lstState;
			List<String^> ^lstError = context->m_lstError;
			int nCount = lstRow->Count;
			for(j=0; j<nCount; j++)
			{
				dr = dt->Rows->Add(lstRow[j]);
				dr->RowError = lstError[j];
				drs = lstDRS[j];
				switch(drs)
				{
				case DataRowState::Added:
					break;
				case DataRowState::Modified:
					dr->AcceptChanges();
					obj = dr[0];
					dr[0] = obj;
					break;
				case DataRowState::Deleted:
					dr->AcceptChanges();
					dr->Delete();
					break;
				case DataRowState::Unchanged:
					dr->AcceptChanges();
					break;
				default: //DataRowState.Detached
					throw gcnew Exception("Wrong DataRow state!");
				}
				nGet++;
			}
		}
//		dt->EndLoadData();
		return nGet;
	}

	int CAsyncAdoSerializationHelper::LoadDataReaderRecords(List<CUQueue^> ^lstUQueue)
	{
		int d;
		int nGet = 0;
		if(lstUQueue == nullptr || lstUQueue->Count == 0)
			return 0;
		DataTable ^dt = m_dt;
		if(dt == nullptr || dt->Columns->Count == 0)
			throw gcnew Exception("Must pass in an valid DataTable object!");
		List<CPRecordContext^> ^aContext = gcnew List<CPRecordContext^>();
		int nSize = lstUQueue->Count;
		for(d=0; d<nSize; d++)
		{
			CPRecordContext ^context = gcnew CPRecordContext();
			context->m_UQueue = lstUQueue[d];
			if(context == nullptr || context->m_UQueue == nullptr)
				throw gcnew Exception("Data row context must contain a valid CUQueue instance!");
			context->m_lstRow = gcnew List<array<Object^>^>();
			aContext->Add(context);
		}
		omp_set_dynamic(0);
		omp_set_num_threads(Environment::ProcessorCount);
		#pragma omp parallel
		{
			int n;
			int nLen = 0;
			#pragma omp for
			for(n=0; n<nSize; ++n)
			{
				CUQueue ^UQueue = aContext[n]->m_UQueue;
				while (UQueue->GetSize() > 0)
				{
					array<Object^> ^row = nullptr;
					nLen += PopDataRecord(UQueue, row);
					aContext[n]->m_lstRow->Add(row);
				}
			}
			#pragma omp barrier
			{
				nGet += nLen;
			}
		}
		nGet = 0;
//		dt->BeginLoadData();
		for(d=0; d<nSize; d++)
		{
			CPRecordContext ^context = aContext[d];
			int j;
			int nCount = context->m_lstRow->Count;
			for(j=0; j<nCount; j++)
			{
				array<Object^> ^row = context->m_lstRow[j];
				dt->Rows->Add(row);
				nGet++;
			}
		}
//		dt->EndLoadData();
		return nGet;
	}
	
	CUQueue ^CAsyncAdoSerializationHelper::LockUQueue()
	{
		CUQueue ^UQueue = nullptr;
		{
			CAutoLock AutoLock(&g_cs.m_sec);
			if(m_sMQ->Count > 0)
				UQueue = m_sMQ->Pop();
		}
		if(UQueue == nullptr)
		{
			UQueue = gcnew CUQueue();
		}
		return UQueue;
	}

	void CAsyncAdoSerializationHelper::UnlockUQueue(CUQueue ^UQueue)
	{
		if(UQueue != nullptr)
		{
			CAutoLock AutoLock(&g_cs.m_sec);
			m_sMQ->Push(UQueue);
		}
	}

	int CAsyncAdoSerializationHelper::PopDataTableRow(CUQueue ^UQueue, array<Object^> ^%aData, DataRowState %drs, String ^%strError)
	{
		int n;
		strError = nullptr;
		unsigned char bData = 0;
		unsigned char bOne = 1;
		CInternalUQueue *pUQueue = UQueue->m_pUQueue;
		String ^str = nullptr;
		int nSize = UQueue->GetSize();
		int nLen = m_dts->Length;
		if(aData == nullptr || aData->Length != nLen)
			aData = gcnew array<Object^>(nLen);
		int nBits = m_dts->Length / 8 + (((m_dts->Length % 8) != 0) ? 1 : 0);
		array<BYTE> ^aBit = gcnew array<BYTE>(nBits);
		UQueue->Pop(aBit, nBits);
		for (n = 0; n < nLen; n++)
		{
			if ((n % 8) == 0)
				bData = aBit[n / 8];
			if ((bData & (bOne << (unsigned char)(n % 8))) != 0)
			{
				aData[n] = DBNull::Value;
			}
			else
			{
				switch (m_dts[n])
				{
					case tagDataTypeSupported::dtBoolean:
						{
							bool myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtByte:
						{
							unsigned char myData;
							pUQueue->Pop(&myData, 1);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtChar:
						{
							wchar_t myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(myData));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDateTime:
						{
							DateTime myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDecimal:
						{
							System::Decimal myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDouble:
						{
							double myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtFloat:
						{
							float myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtGuid:
						{
							Guid myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt16:
						{
							unsigned short myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(myData));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt32:
						{
							unsigned int myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt64:
						{
							System::UInt64 myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt16:
						{
							short myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt32:
						{
							int myData;
							pUQueue->Pop(&myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt64:
						{
							System::Int64 myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtString:
						{
							System::String ^myData;
							UQueue->Load(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtValue:
					case tagDataTypeSupported::dtValues:
					case tagDataTypeSupported::dtChars:
					case tagDataTypeSupported::dtBytes:
					case tagDataTypeSupported::dtTimeSpan:
					case tagDataTypeSupported::dtDateTimeOffset:
						UQueue->Pop(aData[n]);
						break;
					default:
						throw gcnew Exception("Unsupported data type for serialization!");
				}
			}
		}
		pUQueue->Pop(&bData, 1);
		drs = (DataRowState)bData;
		bool b;
		UQueue->Pop(b);
		if (b)
		{
			UQueue->Load(strError);
		}
		return (nSize - UQueue->GetSize());
	}
#endif

	int CAsyncAdoSerializationHelper::LoadDataReaderRecords(CUQueue ^UQueue)
	{
		if (UQueue->GetSize() == 0)
			return 0;
		DataTable ^dt = m_dt;
#ifndef __NO_OPEN_MP__
		if(m_bParallel && m_nBatchSize > 0)
		{
			if(!(UQueue->GetSize() <= m_nBatchSize && m_lstUQueue->Count == 0))
			{
				if(m_lstUQueue->Count == (Environment::ProcessorCount - 1))
				{
					m_lstUQueue->Add(UQueue);
				}
				else
				{
					CUQueue ^temp = LockUQueue();
					temp->SetSize(0);
					temp->Push(UQueue->GetBuffer(), UQueue->GetSize());
					UQueue->SetSize(0);
					m_lstUQueue->Add(temp);
				}
				int nGet = 0;
				if(m_lstUQueue->Count == Environment::ProcessorCount)
				{
					int n;
					nGet = LoadDataReaderRecords(m_lstUQueue);
					for(n=0; n<(Environment::ProcessorCount - 1); n++)
					{
						UnlockUQueue(m_lstUQueue[n]);
					}
					m_lstUQueue->Clear();
				}
				return nGet;
			}
		}
#endif
		if (dt == nullptr)
			throw gcnew Exception("Must pass in an valid DataTable object!");
		int nSize = 0;
		array<System::Object^> ^aData = nullptr;
		//dt->BeginLoadData();
		while (UQueue->GetSize() > 0)
		{
			PopDataRecord(UQueue, aData);
			dt->Rows->Add(aData);
			++nSize;
		}
		//dt->EndLoadData();
		return nSize;
	}

	int CAsyncAdoSerializationHelper::PopDataRecord(CUQueue ^UQueue, array<System::Object^> ^%aData)
	{
		int n;
		BYTE bData = 0;
		BYTE bOne = 1;
		CInternalUQueue *pUQueue = UQueue->m_pUQueue;
		int nSize = UQueue->GetSize();
		int nLen = m_dts->Length;
		if (aData == nullptr || aData->Length != nLen)
			aData = gcnew array<Object^>(nLen);
		int nBits = m_dts->Length / 8 + (((m_dts->Length % 8) != 0) ? 1 : 0);
		array<BYTE> ^aBit = gcnew array<BYTE>(nBits);
		UQueue->Pop(aBit, nBits);
		for (n = 0; n < nLen; n++)
		{
			if ((n % 8) == 0)
				bData = aBit[n / 8];
			if ((bData & (bOne << (unsigned char)(n % 8))) != 0)
			{
				aData[n] = DBNull::Value;
			}
			else
			{
				switch (m_dts[n])
				{
					case tagDataTypeSupported::dtBoolean:
						{
							bool myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(bool));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtByte:
						{
							unsigned char myData;
							pUQueue->Pop(&myData, sizeof(myData));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtChar:
						{
							wchar_t myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(wchar_t));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDateTime:
						{
							DateTime myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDecimal:
						{
							System::Decimal myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDouble:
						{
							double myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(double));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtFloat:
						{
							float myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(float));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtGuid:
						{
							Guid myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt16:
						{
							short myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(myData));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt32:
						{
							int myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(int));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt64:
						{
							__int64 myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(__int64));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtString:
						{
							System::String ^myData;
							UQueue->Load(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtBytes:
						{
							int nBytes;
							UQueue->Pop(nBytes);
							array<System::Byte> ^buffer = gcnew array<System::Byte>(nBytes);
							UQueue->Pop(buffer);
							aData[n] = buffer;
						}
						break;
					case tagDataTypeSupported::dtUInt64:
					case tagDataTypeSupported::dtUInt32:
					case tagDataTypeSupported::dtUInt16:
					case tagDataTypeSupported::dtValue:
					case tagDataTypeSupported::dtValues:
					case tagDataTypeSupported::dtTimeSpan:
					case tagDataTypeSupported::dtDateTimeOffset:
						UQueue->Pop(aData[n]);
						break;
					default:
						throw gcnew Exception("Unsupported data type for serialization!");
				}
			}
		}
		return (nSize - UQueue->GetSize());
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, array<Object^> ^%aData, DataRowState %drs)
	{
		int n;
		unsigned char bData = 0;
		unsigned char bOne = 1;
		CInternalUQueue *pUQueue = UQueue->m_pUQueue;
		int nSize = UQueue->GetSize();
		int nLen = m_dts->Length;
		if(aData == nullptr)
			aData = gcnew array<Object^>(nLen);
		int nBits = m_dts->Length / 8 + (((m_dts->Length % 8) != 0) ? 1 : 0);
		array<BYTE> ^aBit = gcnew array<BYTE>(nBits);
		UQueue->Pop(aBit, nBits);
		for (n = 0; n < nLen; n++)
		{
			if ((n % 8) == 0)
				bData = aBit[n / 8];
			if ((bData & (bOne << (unsigned char)(n % 8))) != 0)
			{
				aData[n] = DBNull::Value;
			}
			else
			{
				switch (m_dts[n])
				{
					case tagDataTypeSupported::dtBoolean:
						{
							bool myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(bool));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtByte:
						{
							unsigned char myData;
							pUQueue->Pop(&myData, 1);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtChar:
						{
							wchar_t myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(wchar_t));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDateTime:
						{
							DateTime myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDecimal:
						{
							System::Decimal myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtDouble:
						{
							double myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(double));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtFloat:
						{
							float myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(float));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtGuid:
						{
							Guid myData;
							UQueue->Pop(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt16:
						{
							unsigned short myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(unsigned short));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt32:
						{
							unsigned int myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(unsigned int));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtUInt64:
						{
							unsigned __int64 llData;
							pUQueue->Pop((unsigned char*)&llData, sizeof(unsigned __int64));
							aData[n] = llData;
						}
						break;
					case tagDataTypeSupported::dtInt16:
						{
							short myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(short));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt32:
						{
							int myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(int));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtInt64:
						{
							__int64 myData;
							pUQueue->Pop((unsigned char*)&myData, sizeof(__int64));
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtString:
						{
							System::String ^myData;
							UQueue->Load(myData);
							aData[n] = myData;
						}
						break;
					case tagDataTypeSupported::dtValue:
					case tagDataTypeSupported::dtValues:
					case tagDataTypeSupported::dtChars:
					case tagDataTypeSupported::dtBytes:
					case tagDataTypeSupported::dtTimeSpan:
					case tagDataTypeSupported::dtDateTimeOffset:
						UQueue->Pop(aData[n]);
						break;
					default:
						throw gcnew Exception("Unsupported data type for serialization!");
				}
			}
		}
		pUQueue->Pop(&bData, 1);
		drs = (DataRowState)bData;
		return (nSize - UQueue->GetSize());
	}

	DataSet^ CAsyncAdoSerializationHelper::LoadDataSetHeader(CUQueue ^UQueue)
	{
		int nSize;
		if (UQueue->GetSize() == 0)
			return nullptr;
		System::String ^str = nullptr;
		System::Byte bData;
		DataSet ^ds = gcnew DataSet();
		m_ds = ds;
		nSize = UQueue->GetSize();
		UQueue->Pop(m_nAffected);
		UQueue->Pop(bData);
		ds->CaseSensitive = ((bData & 2) == 2);
		ds->EnforceConstraints = ((bData & 4) == 4);
		UQueue->Load(str);
		ds->DataSetName = str;
		UQueue->Load(str);
		ds->Namespace = str;
		UQueue->Load(str);
		ds->Prefix = str;
		return ds;
	}

	DataTable^ CAsyncAdoSerializationHelper::LoadDataTableHeader(CUQueue ^UQueue)
	{
		int n;
		int nData;
		int nOrdinal;
		unsigned char bData;
		bool bNeedChildRelations;
		bool bNeedbParentRelations;
		if (UQueue->GetSize() == 0)
			return nullptr;
		System::String ^str = nullptr;
		DataTable ^dt;
		m_dt = dt = gcnew DataTable();
		int nSize = UQueue->GetSize();
		UQueue->Pop(m_nAffected);
		UQueue->Pop(bData);
		bNeedChildRelations = ((bData & 2) == 2);
		bNeedbParentRelations = ((bData & 4) == 4);
		DataColumnCollection ^dcc = dt->Columns;
		UQueue->Load(str);
		dt->TableName = str;
		Pop(UQueue, dcc);
		int nCount = dcc->Count;
		m_dts = gcnew array<tagDataTypeSupported>(nCount);
		for (n = 0; n < nCount; n++)
		{
			m_dts[n] = GetDT(dcc[n]->DataType->FullName);
		}
		UQueue->Load(str);
		dt->DisplayExpression = str;
		UQueue->Pop(nData);
		dt->MinimumCapacity = nData;
		UQueue->Load(str);
		dt->Namespace = str;
		UQueue->Load(str);
		dt->Prefix = str;
		UQueue->Pop(nData);
		array<DataColumn^>^ pk = gcnew array<DataColumn^>(nData);
		for (n = 0; n < nData; n++)
		{
			UQueue->Pop(nOrdinal);
			DataColumn ^dc = dt->Columns[nOrdinal];
			pk[n] = dc;
		}
		dt->PrimaryKey = pk;
		if (bNeedbParentRelations)
		{
			DataRelationCollection ^drc = dt->ParentRelations;
			Pop(UQueue, drc);
		}
		if (bNeedChildRelations)
		{
			DataRelationCollection ^drc = dt->ChildRelations;
			Pop(UQueue, drc);
		}
		if(UQueue->GetSize() >= sizeof(int))
		{
			UQueue->Pop(m_nBatchSize);
		}
		else
		{
			m_nBatchSize = 0;
		}
		return dt;
	}

	DataTable ^CAsyncAdoSerializationHelper::LoadDataReaderHeader(CUQueue ^UQueue)
	{
		int nData;
		short sData;
		int nFieldCount;
		int n;
		if (UQueue->GetSize() == 0)
			return nullptr;
		DataTable ^dt = gcnew DataTable();
		m_dt = dt;
		System::String ^str = nullptr;
		m_bLoadingDataTable = false;
		UQueue->Pop(nFieldCount);
		UQueue->Pop(m_nAffected);
		m_dts = gcnew array<tagDataTypeSupported>(nFieldCount);
		for (n = 0; n < nFieldCount; n++)
		{
			UQueue->Pop(sData);
			m_dts[n] = (tagDataTypeSupported)(sData);
		}
		m_qTemp->SetSize(0);
		for (n = 0; n < nFieldCount; n++)
		{
			UQueue->Pop(nData);
			DataColumn ^dc = gcnew DataColumn();
			dc->DataType = GetType(m_dts[n]);
			dc->AllowDBNull = ((nData & safe_cast<int>(tagColumnBit::cbAllowDBNull)) == safe_cast<int>(tagColumnBit::cbAllowDBNull));
			dc->AutoIncrement = ((nData & safe_cast<int>(tagColumnBit::cbIsAutoIncrement)) == safe_cast<int>(tagColumnBit::cbIsAutoIncrement));
			dc->ReadOnly = ((nData & safe_cast<int>(tagColumnBit::cbIsReadOnly)) == safe_cast<int>(tagColumnBit::cbIsReadOnly));
			dc->Unique = ((nData & safe_cast<int>(tagColumnBit::cbIsUnique)) == safe_cast<int>(tagColumnBit::cbIsUnique));
			bool cbIsLong = ((nData & safe_cast<int>(tagColumnBit::cbIsLong)) == safe_cast<int>(tagColumnBit::cbIsLong));
			if ((nData & safe_cast<int>(tagColumnBit::cbIsKey)) == safe_cast<int>(tagColumnBit::cbIsKey))
			{
				m_qTemp->Push(n);
			}
			UQueue->Pop(nData);
			if (!cbIsLong && nData > 0 && (m_dts[n] == tagDataTypeSupported::dtString || m_dts[n] == tagDataTypeSupported::dtChars))
			{
				dc->MaxLength = nData; //ColumnSize
			}
			UQueue->Load(str);
			dc->ColumnName = str;
			dt->Columns->Add(dc);
		}
	
		if (m_qTemp->GetSize() > 0)
		{
			int nIndex = 0;
			array<DataColumn^> ^dcs = gcnew array<DataColumn^>(m_qTemp->GetSize() / 4);
			while (m_qTemp->GetSize() > 0)
			{
				m_qTemp->Pop(nData);
				DataColumn ^dc = dt->Columns[nData];
				dcs[nIndex] = dc;
				++nIndex;
			}
			dt->PrimaryKey = dcs;
		}
		
		if(UQueue->GetSize() >= sizeof(int))
		{
			UQueue->Pop(m_nBatchSize);
		}
		else
		{
			m_nBatchSize = 0;
		}
		return dt;
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, DataColumn ^%dc)
	{
		bool bNull;
		int nLen = UQueue->GetSize();
		UQueue->Pop(bNull);
		if (bNull)
			dc = nullptr;
		else
		{
			int nData;
			System::Object ^ob;
			short sData;
			System::String ^str;
			System::Int64 lData;
			System::Byte bData;
			UQueue->Pop(bData);
			if (dc == nullptr)
				dc = gcnew DataColumn();
			dc->AllowDBNull = ((bData & safe_cast<int>(tagColumnBit::cbAllowDBNull)) == safe_cast<int>(tagColumnBit::cbAllowDBNull));
			dc->AutoIncrement = ((bData & safe_cast<int>(tagColumnBit::cbIsAutoIncrement)) == safe_cast<int>(tagColumnBit::cbIsAutoIncrement));
			dc->ReadOnly = ((bData & safe_cast<int>(tagColumnBit::cbIsReadOnly)) == safe_cast<int>(tagColumnBit::cbIsReadOnly));
			dc->Unique = ((bData & safe_cast<int>(tagColumnBit::cbIsUnique)) == safe_cast<int>(tagColumnBit::cbIsUnique));
			UQueue->Pop(lData);
			dc->AutoIncrementSeed = lData;
			UQueue->Pop(lData);
			dc->AutoIncrementStep = lData;
			UQueue->Load(str);
			dc->Caption = str;
			UQueue->Pop(bData);
			dc->ColumnMapping = safe_cast<MappingType>(bData);
			UQueue->Load(str);
			dc->ColumnName = str;
			UQueue->Pop(sData);
			dc->DataType = GetType((tagDataTypeSupported)(sData));
			UQueue->Pop(ob);
			dc->DefaultValue = ob;
			UQueue->Load(str);
			dc->Expression = str;
			UQueue->Pop(nData);
			dc->MaxLength = nData;
			UQueue->Load(str);
			dc->Namespace = str;
			UQueue->Load(str);
			dc->Prefix = str;
		}
		return (nLen - UQueue->GetSize());
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, DataColumnCollection ^%Cols)
	{
		bool bNull;
		int nSize = UQueue->GetSize();
		UQueue->Pop(bNull);
		if (bNull)
			Cols = nullptr;
		else
		{
			int n;
			int nLen;
			DataColumn ^dc = nullptr;
			Cols->Clear();
			UQueue->Pop(nLen);
			for (n = 0; n < nLen; n++)
			{
				Pop(UQueue, dc);
				Cols->Add(dc);
				dc = nullptr;
			}
		}
		return (nSize - UQueue->GetSize());
	}

	void CAsyncAdoSerializationHelper::AddAAU()
	{
		int n, nCount;
		if (m_dt == nullptr || m_dtBackup == nullptr)
			return;
		if (m_dt->Columns->Count != m_dtBackup->Columns->Count)
			throw gcnew Exception("Bad operation for table column mis-matching!");
		m_dt->EndLoadData(); //can't put this call after the below calls

		nCount = m_dt->Columns->Count;
		for (n = 0; n < nCount; n++)
		{
			DataColumn ^dc = m_dt->Columns[n];
			dc->AllowDBNull = m_dtBackup->Columns[n]->AllowDBNull;
			dc->AutoIncrement = m_dtBackup->Columns[n]->AutoIncrement;
			dc->Unique = m_dtBackup->Columns[n]->Unique;
			dc->ReadOnly = m_dtBackup->Columns[n]->ReadOnly;
		}

    array<DataColumn^> ^keys = m_dtBackup->PrimaryKey;
    nCount = keys->Length;
    for(n=0; n<nCount; ++n)
    {
      keys[n] = m_dt->Columns[keys[n]->Ordinal];
    }
    m_dt->PrimaryKey = keys;

	}

	void CAsyncAdoSerializationHelper::RemoveAAU()
	{
		if (m_dt == nullptr)
			return;
		int n, nSize;
    m_dt->PrimaryKey = nullptr;
		nSize = m_dt->Columns->Count;
		for(n=0; n<nSize; n++)
		{
			DataColumn ^p = m_dt->Columns[n];
			
			//The property AllowDBNull has an important role on performance
			p->AllowDBNull = true;
			p->AutoIncrement = false;
			
      p->Unique = false;
			p->ReadOnly = false;
		}
		m_dt->BeginLoadData();
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, array<DataColumn^> ^%dcs)
	{
		int n;
		int nSize;
		int nLen = UQueue->GetSize();
		UQueue->Pop(nSize);
		if (nSize == -1)
		{
			dcs = nullptr;
		}
		else
		{
			if (dcs == nullptr || dcs->Length != nSize)
				dcs = gcnew array<DataColumn^>(nSize);
			for (n = 0; n < nSize; n++)
			{
				Pop(UQueue, dcs[n]);
			}
		}
		return (nLen - UQueue->GetSize());
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, ForeignKeyConstraint ^%fkc)
	{
		bool b;
		int nSize = UQueue->GetSize();
		UQueue->Pop(b);
		if (b) //null
		{
			fkc = nullptr;
		}
		else
		{
			System::Byte bData = 0;
			System::String ^str = nullptr;
			array<DataColumn^> ^dcsChild = nullptr;
			Pop(UQueue, dcsChild);
			array<DataColumn^> ^dcsParent = nullptr;
			Pop(UQueue, dcsParent);
			fkc = gcnew ForeignKeyConstraint(dcsParent, dcsChild);
			UQueue->Load(str);
			fkc->ConstraintName = str;
			UQueue->Pop(bData);
			fkc->AcceptRejectRule = safe_cast<AcceptRejectRule>(bData);
	
			UQueue->Pop(bData);
			fkc->UpdateRule = safe_cast<Rule>(bData);
	
			UQueue->Pop(bData);
			fkc->DeleteRule = safe_cast<Rule>(bData);
		}
		return (nSize - UQueue->GetSize());
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, UniqueConstraint ^%uc)
	{
		bool bNull;
		int nSize = UQueue->GetSize();
		UQueue->Pop(bNull);
		if (!bNull)
		{
			System::String ^str = nullptr;
			array<DataColumn^> ^dcs = nullptr;
			bool b = false;
			Pop(UQueue, dcs);
			UQueue->Load(str);
			UQueue->Pop(b);
			uc = gcnew UniqueConstraint(str, dcs, b);
		}
		return nSize - UQueue->GetSize();
	}

	int CAsyncAdoSerializationHelper::Pop(CUQueue ^UQueue, DataRelationCollection ^%drc)
	{
		int n;
		System::String ^str;
		bool b;
		int nData;
		int nSize = UQueue->GetSize();
		UQueue->Pop(nData);
		drc->Clear();
		for (n = 0; n < nData; n++)
		{
			array<DataColumn^> ^dcsChild = nullptr;
			PopTableColNamesOnly(UQueue, dcsChild);
	
			UQueue->Pop(b);
			UQueue->Load(str);
	
			array<DataColumn^> ^dcsParent = nullptr;
			PopTableColNamesOnly(UQueue, dcsParent);
	
			DataRelation ^dr = gcnew DataRelation(str, dcsParent, dcsChild);
			dr->Nested = b;
			drc->Add(dr);
		}
		return (nSize - UQueue->GetSize());
	}

	void CAsyncAdoSerializationHelper::FinalizeRecords()
	{
		if(m_dt != nullptr)
		{
			m_dt->EndLoadData();
			m_dt->BeginLoadData();
		}
	}

	void CAsyncAdoSerializationHelper::Load(short sRequestID, CUQueue ^UQueue)
	{
		switch(sRequestID)
		{
		case CAsyncAdoSerializationHelper::idDataSetHeaderArrive:
            if (UQueue->GetSize() > 0)
			{
				m_bDataSet = true;
                LoadDataSetHeader(UQueue);
            }
            break;
		case CAsyncAdoSerializationHelper::idDataReaderHeaderArrive:
			if (UQueue->GetSize() > 0)
			{
				m_bDataReader = true;
				
				LoadDataReaderHeader(UQueue);
				m_dtBackup = m_dt->Clone();
				
				//for better performance
				RemoveAAU();
			}
#ifndef __NO_OPEN_MP__
			StartLoadingInParallel();
#endif
			break;
		case CAsyncAdoSerializationHelper::idDataTableHeaderArrive:
			if (UQueue->GetSize() > 0)
			{
				m_bLoadingDataTable = true;
				LoadDataTableHeader(UQueue);
				if(m_bDataSet)
					m_ds->Tables->Add(m_dt);
				m_dtBackup = m_dt->Clone();    
				//for better performance
				RemoveAAU();
			}
#ifndef __NO_OPEN_MP__
			StartLoadingInParallel();
#endif
			break;
		case CAsyncAdoSerializationHelper::idDataTableRowsArrive:
		case CAsyncAdoSerializationHelper::idDataReaderRecordsArrive:
			if (UQueue->GetSize() > 0)
				LoadRows(UQueue);
			break;
		case CAsyncAdoSerializationHelper::idEndDataReader:
			m_bDataReader = false;
			if(m_dt != nullptr)
				m_dt->AcceptChanges();
			AddAAU(); //reset datatable

#ifndef __NO_OPEN_MP__
			EndLoadingInParallel();
#endif
			break;
		case CAsyncAdoSerializationHelper::idEndDataTable:
			m_bLoadingDataTable = false;
			AddAAU(); //reset datatable
#ifndef __NO_OPEN_MP__
			EndLoadingInParallel();
#endif
			break;
		case CAsyncAdoSerializationHelper::idEndDataSet:
			if(UQueue->GetSize() > 0)
			{
				DataRelationCollection ^drc = LoadDataSetRelations(UQueue);
				m_bDataSet = false;
				/*if (drc != nullptr)
				{
					int n;
					int nSize = drc->Count;
					for(n=0; n<nSize; n++)
					{
						DataRelation ^dr = drc[n];
						m_ds->Relations->Add(dr);
					}
				}*/
			}
            break;
		default:
			break;
		}
	}

}