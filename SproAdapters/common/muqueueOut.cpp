// This is the main DLL file.

#include "stdafx.h"

#include "muqueue.h"
#include <vcclr.h>

#define VT_XML_OBJECT			(0xF00)
#define VT_DOTNET_OBJECT		(0xE00)
#define VT_USERIALIZER_OBJECT	(0xD00)

//#pragma warning(disable: 4947) // System::Runtime::InteropServices::FILETIME

namespace SocketProAdapter
{
	CUQueue::CUQueue()
	{
		m_pUQueue = new CInternalUQueue();
		m_ms = nullptr;
	}

	CUQueue::CUQueue(int nBlockSize)
	{
		if(nBlockSize <= 0)
			nBlockSize = 1024;
		m_pUQueue = new CInternalUQueue(128, nBlockSize);
		m_ms = nullptr;
	}
	
	CUQueue::~CUQueue()
	{
		if(m_pUQueue != NULL)
		{
			delete m_pUQueue;
		}
	}
	
	void CUQueue::SetHeadPosition()
	{
		m_pUQueue->SetHeadPosition();
	}
	
	void CUQueue::CleanTrack()
	{
		m_pUQueue->CleanTrack();
		if(m_ms != nullptr)
		{
			m_ms->SetLength(0);
			m_ms->Position = 0;
			array<BYTE>::Clear(m_ms->GetBuffer(), 0, m_ms->Capacity);
		}
	}

	void CUQueue::Empty()
	{
		m_pUQueue->Empty();
		if(m_ms != nullptr)
		{
			delete m_ms;
			m_ms = nullptr;
		}
	}

	int CUQueue::GetSize()
	{
		return m_pUQueue->GetSize();
	}

	void CUQueue::ReallocBuffer(int nSize)
	{
		if(nSize < 0)
			throw gcnew Exception(gcnew String("Bad input."));
		m_pUQueue->ReallocBuffer(nSize);
		if(m_pUQueue->GetBuffer() == NULL)
			throw gcnew Exception(gcnew String("Out of memory."));
	}

	void CUQueue::SetSize(int nSize)
	{
		if(nSize < 0)
			throw gcnew Exception(gcnew String("Bad input."));
		if((unsigned long)nSize > m_pUQueue->GetMaxSize())
		{
			m_pUQueue->ReallocBuffer(nSize);
			if(m_pUQueue->GetBuffer() == NULL)
				throw gcnew Exception(gcnew String("Out of memory."));
		}
		m_pUQueue->SetSize(nSize);
	}
	
	CInternalUQueue* CUQueue::GetInternalUQueue()
	{
		return m_pUQueue;
	}

	const unsigned char* const CUQueue::GetNativeBuffer(int nPos)
	{
		if(nPos < 0)
			throw gcnew Exception(gcnew String("Bad input."));
		return m_pUQueue->GetBuffer(nPos);
	}

	IntPtr CUQueue::GetBuffer()
	{
		return IntPtr((void*)m_pUQueue->GetBuffer());
	}

	IntPtr CUQueue::GetBuffer(int nPos)
	{
		if(nPos < 0)
			throw gcnew Exception(gcnew String("Bad input."));
		return IntPtr((void*)m_pUQueue->GetBuffer(nPos));
	}

	void CUQueue::Push(wchar_t wChar)
	{
		//m_pUQueue->Push((BYTE*)&wChar, sizeof(wChar));
		m_pUQueue->Push((const BYTE*)&wChar, 2);
	}
	
	void CUQueue::Push(bool bData)
	{
		//m_pUQueue->Push(&bData);
		m_pUQueue->Push((const BYTE*)&bData, 1);
	}

	void CUQueue::Push(BYTE bByte)
	{
		m_pUQueue->Push(&bByte, 1);
	}

	void CUQueue::Push(char cChar)
	{
		m_pUQueue->Push((const BYTE*)&cChar, 1);
	}
	
	void CUQueue::Push(short sData)
	{
		m_pUQueue->Push((const BYTE*)&sData, 2);
		//m_pUQueue->Push(&sData);
	}

	void CUQueue::Push(unsigned short usData)
	{
		m_pUQueue->Push((const BYTE*)&usData, 2);
	}

	void CUQueue::Push(int nData)
	{
		m_pUQueue->Push((const BYTE*)&nData, 4);
		//m_pUQueue->Push(&nData);
	}

	void CUQueue::Push(unsigned int unData)
	{
		m_pUQueue->Push((const BYTE*)&unData, 4);
		//m_pUQueue->Push(&unData);
	}

	void CUQueue::Push(__int64 lData)
	{
		m_pUQueue->Push((const BYTE*)&lData, 8);
		//m_pUQueue->Push(&lData);
	}

	void CUQueue::Push(unsigned __int64 ulData)
	{
		m_pUQueue->Push((const BYTE*)&ulData, 8);
		//m_pUQueue->Push(&ulData);
	}

	void CUQueue::Push(double dData)
	{
		m_pUQueue->Push((const BYTE*)&dData, 8);
		//m_pUQueue->Push(&dData);
	}

	void CUQueue::Push(float fData)
	{
		m_pUQueue->Push((const BYTE*)&fData, 4);
		//m_pUQueue->Push(&fData);
	}

	void CUQueue::Push(IUSerializer ^USerializer)
	{
		bool  b = (USerializer == nullptr);
		m_pUQueue->Push((unsigned char*)&b, sizeof(b));
		if(!b)
			USerializer->SaveTo(this);
	}

	void CUQueue::Push(String ^strData)
	{
		if(strData != nullptr && strData->Length != 0)
		{
			pin_ptr<const wchar_t> wch = PtrToStringChars( strData );
			m_pUQueue->Push((const unsigned char*)wch, strData->Length*sizeof(WCHAR));
		}
	}
/*
	CUQueue^ CUQueue::operator << (String ^str)
	{
		Push(str);
		return this;
	}

	CUQueue^ CUQueue::operator >> (String ^%str)
	{
		Pop(str);
		return this;
	}*/

	void CUQueue::Push(array<BYTE> ^aData, int nLen, int nIndex)
	{
		if(aData != nullptr && nLen != 0)
		{
			if(nLen < 0 || nIndex < 0 || nIndex > aData->Length -1)
				throw gcnew Exception(gcnew String("Bad input."));
			if(nLen > aData->Length - nIndex)
				nLen = aData->Length - nIndex;
			if(nLen != 0)
			{
				pin_ptr<BYTE> wch = &aData[nIndex];
				m_pUQueue->Push(wch, (unsigned long)nLen);
			}
		}
	}

	void CUQueue::Push(array<BYTE> ^aData, int nLen)
	{
		if(aData != nullptr && nLen != 0)
		{
			if(nLen < 0)
				throw gcnew Exception(gcnew String("Bad input."));
			if(nLen > aData->Length)
				nLen = aData->Length;
			pin_ptr<BYTE> wch = &aData[0];
			m_pUQueue->Push(wch, (unsigned long)nLen);
		}
	}
	
	void CUQueue::Push(array<BYTE> ^aData)
	{
		if(aData != nullptr && aData->Length != 0)
		{
			pin_ptr<BYTE> wch = &aData[0];
			m_pUQueue->Push(wch, aData->Length);
		}
	}

	void CUQueue::Push(CSocketProServerException ^SocketProServerException)
	{
		if(SocketProServerException == nullptr)
			throw gcnew Exception(gcnew String("Can't serialize a null CSocketProServerException."));
		Push(SocketProServerException->HResult);
		if(SocketProServerException->HResult != S_OK)
		{
			Push(SocketProServerException->m_nSvsID);
			Push(SocketProServerException->m_sRequestID);
			Save(SocketProServerException->Message);
		}
	}
	
	void CUQueue::Push(CUQueue ^UQueue)
	{
		ULONG ulSize = UQUEUE_SIZE_NULL;
		if(UQueue != nullptr)
		{
			ulSize = (ULONG)UQueue->GetSize();
			m_pUQueue->Push(&ulSize);
			m_pUQueue->Push(UQueue->m_pUQueue->GetBuffer(), ulSize);
		}
		else
		{
			m_pUQueue->Push(&ulSize);
		}
	}

	void CUQueue::Push(DateTime dtData)
	{
		pin_ptr<DateTime> ptr = &dtData;
		m_pUQueue->Push((BYTE*)ptr, (unsigned long)sizeof(dtData));
	}

	void CUQueue::Push(Decimal decData)
	{
		pin_ptr<Decimal> ptr = &decData;
		m_pUQueue->Push((BYTE*)ptr, (unsigned long)sizeof(decData));
	}

	void CUQueue::Push(System::Runtime::InteropServices::ComTypes::FILETIME ftData)
	{
		pin_ptr<System::Runtime::InteropServices::ComTypes::FILETIME> ptr = &ftData;
		m_pUQueue->Push((BYTE*)ptr, (unsigned long)sizeof(ftData));
	}
	
	void CUQueue::Push(Guid guidData)
	{
		pin_ptr<Guid> ptr = &guidData;
		m_pUQueue->Push((const BYTE*)ptr, (unsigned long)sizeof(guidData));
	}

	void CUQueue::PushCY(Decimal cyMoney)
	{
		__int64 lMoney = (__int64)(cyMoney*10000);
		m_pUQueue->Push((BYTE*)&lMoney, sizeof(lMoney));
	}

	void CUQueue::PushVariantBool(bool b)
	{
		unsigned short usData = b ? 0xFFFF : 0;
		m_pUQueue->Push((BYTE*)&usData, sizeof(usData));
	}

	void CUQueue::PushSystemTime(DateTime dt)
	{
		SYSTEMTIME stData;
		stData.wYear = (WORD)dt.Year;
		stData.wMonth = (WORD)dt.Month;
		stData.wDay = (WORD)dt.Day;
		stData.wHour = (WORD)dt.Hour;
		stData.wMinute = (WORD)dt.Minute;
		stData.wSecond = (WORD)dt.Second;
		stData.wMilliseconds = (WORD)dt.Millisecond;
		m_pUQueue->Push(&stData);
	}

	void CUQueue::PushVariantDate(DateTime dt)
	{
		double dDT;
		SYSTEMTIME stData;
		stData.wYear = (WORD)dt.Year;
		stData.wMonth = (WORD)dt.Month;
		stData.wDay = (WORD)dt.Day;
		stData.wHour = (WORD)dt.Hour;
		stData.wMinute = (WORD)dt.Minute;
		stData.wSecond = (WORD)dt.Second;
		stData.wMilliseconds = (WORD)dt.Millisecond;
		::SystemTimeToVariantTime(&stData, &dDT);
		m_pUQueue->Push((BYTE*)&dDT, sizeof(dDT));
	}

	void CUQueue::Push(Object ^obData)
	{
		//Beginning with SocketPro version 4.3.0.1, 
		//convert dotNet DateTime into native variant date
		Push(obData, true, false);

		//old 
		//Push(obData, false, false);
	}

	void CUQueue::Push(Object ^obData, bool bToNative)
	{
		Push(obData, bToNative, false);
	}

	void CUQueue::Push(Object ^obData, bool bToNative, bool bDecToCY)
	{
		unsigned short vt = VT_EMPTY;
		if(obData == nullptr)
		{
			m_pUQueue->Push(&vt);
			return;
		}
		else if(obData == DBNull::Value)
		{
			vt = VT_NULL;
			m_pUQueue->Push(&vt);
			return;
		}
		Type ^t = obData->GetType();
		if(t->IsArray)
		{
			vt = VT_ARRAY;
			if(t->FullName == "System.Byte[]")
			{
				vt |= VT_UI1;
				m_pUQueue->Push(&vt);
				array<BYTE> ^pByte = (array<BYTE> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.String[]")
			{
				int nIndex;
				vt |= VT_BSTR;
				m_pUQueue->Push(&vt);
				array<String^> ^aStr = (array<String^> ^)obData;
				int nSize = aStr->Length;
				Push(nSize);
				for(nIndex = 0; nIndex<nSize; nIndex++)
				{
					Save(aStr[nIndex]);
				}
			}
			else if(t->FullName == "System.UInt16[]")
			{
				vt |= VT_UI2;
				m_pUQueue->Push(&vt);
				array<unsigned short> ^pByte = (array<unsigned short> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Char[]")
			{
				vt |= VT_UI2;
				m_pUQueue->Push(&vt);
				array<wchar_t> ^pByte = (array<wchar_t> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Int16[]")
			{
				vt |= VT_I2;
				m_pUQueue->Push(&vt);
				array<short> ^pByte = (array<short> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Int32[]")
			{
				vt |= VT_I4;
				m_pUQueue->Push(&vt);
				array<long> ^pByte = (array<long> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Single[]")
			{
				vt |= VT_R4;
				m_pUQueue->Push(&vt);
				array<float> ^pByte = (array<float> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Double[]")
			{
				vt |= VT_R8;
				m_pUQueue->Push(&vt);
				array<double> ^pByte = (array<double> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Boolean[]")
			{
				int nIndex;
				vt |= VT_BOOL;
				m_pUQueue->Push(&vt);
				array<bool> ^aBool = (array<bool> ^)obData;
				int nSize = aBool->Length;
				Push(nSize);
				for(nIndex = 0; nIndex<nSize; nIndex++)
				{
					PushVariantBool(aBool[nIndex]);
				}
			}
			else if(t->FullName == "System.SByte[]")
			{
				vt |= VT_I1;
				m_pUQueue->Push(&vt);
				array<char> ^pByte = (array<char> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Runtime.ComTypes.InteropServices.FILETIME[]")
			{
				vt |= VT_FILETIME;
				m_pUQueue->Push(&vt);
				array<System::Runtime::InteropServices::ComTypes::FILETIME> ^pByte = (array<System::Runtime::InteropServices::ComTypes::FILETIME> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Int64[]")
			{
				vt |= VT_I8;
				m_pUQueue->Push(&vt);
				array<__int64> ^pByte = (array<__int64> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.UInt64[]")
			{
				vt |= VT_UI8;
				m_pUQueue->Push(&vt);
				array<unsigned __int64> ^pByte = (array<unsigned __int64> ^)obData;
				PushMDataArray(pByte);
			}
			
			else if(t->FullName == "System.UInt32[]")
			{
				vt |= VT_UI4;
				m_pUQueue->Push(&vt);
				array<unsigned long> ^pByte = (array<unsigned long> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Guid[]")
			{
				vt |= VT_CLSID;
				m_pUQueue->Push(&vt);
				array<Guid> ^pByte = (array<Guid> ^)obData;
				PushMDataArray(pByte);
			}
			else if(t->FullName == "System.Object[]")
			{
				int nIndex;
				vt |= VT_VARIANT;
				m_pUQueue->Push(&vt);
				array<Object^> ^obA = (array<Object^> ^)obData;
				int nSize = obA->Length;
				Push(nSize);
				for(nIndex = 0; nIndex<nSize; nIndex++)
				{
					Push(obA[nIndex], bToNative, bDecToCY);
				}
			}
			else if(t->FullName == "System.DateTime[]")
			{
				vt |= VT_DATE;
				m_pUQueue->Push(&vt);
				array<DateTime> ^aDate = (array<DateTime> ^)obData;
				if(bToNative)
				{
					int nIndex;
					int nSize = aDate->Length;
					Push(nSize);
					for(nIndex = 0; nIndex<nSize; nIndex++)
					{
						PushVariantDate(aDate[nIndex]);
					}
				}
				else
				{
					PushMDataArray(aDate);
				}
			}
			else if(t->FullName == "System.Decimal[]")
			{
				if(bDecToCY && bToNative)
				{
					vt |= VT_CY;
				}
				else
				{
					vt |= VT_DECIMAL;
				}
				m_pUQueue->Push(&vt);
				array<Decimal> ^aDec = (array<Decimal> ^)obData;
				if(bDecToCY && bToNative)
				{
					int nIndex;
					int nSize = aDec->Length;
					Push(nSize);
					for(nIndex = 0; nIndex<nSize; nIndex++)
					{
						PushCY(aDec[nIndex]);
					}
				}
				else
				{
					PushMDataArray(aDec);
				}
			}
			else if(t->FullName == "System.DBNull[]")
			{
				int nIndex;
				vt |= VT_NULL;
				m_pUQueue->Push(&vt);
				array<DBNull^> ^aNull = (array<DBNull^> ^)obData;
				int nSize = aNull->Length;
				Push(nSize);
				for(nIndex = 0; nIndex<nSize; nIndex++)
				{
					Push((unsigned short)VT_NULL);
				}
			}
			else
			{
				int nIndex;
				CUQueue ^UQueue = gcnew CUQueue();
				array<Object ^> ^aObj = (array<Object ^> ^)obData;
				int nSize = aObj->Length;
				for(nIndex = 0; nIndex<nSize; nIndex++)
				{
					UQueue->Serialize(aObj[nIndex]);
				}
				vt |= VT_DOTNET_OBJECT;
				m_pUQueue->Push(&vt);
				Push(nSize);
				m_pUQueue->Push(UQueue->m_pUQueue->GetBuffer(), UQueue->m_pUQueue->GetSize());
				UQueue->Empty();
			}
		}
		else
		{
			if(t->IsPrimitive)
			{
				if(t == int::typeid)
				{
					vt = VT_I4;
					Push(vt);
					Push((int)obData);
				}
				else if(t == __int64::typeid)
				{
					vt = VT_I8;
					Push(vt);
					Push((__int64)obData);
				}
				else if(t == double::typeid)
				{
					vt = VT_R8;
					m_pUQueue->Push(&vt);
					Push((double)obData);
				}
				else if(t == float::typeid)
				{
					vt = VT_R4;
					m_pUQueue->Push(&vt);
					Push((float)obData);
				}
				else if(t == Byte::typeid)
				{
					vt = VT_UI1;
					Push(vt);
					Push((BYTE)obData);
				}
				else if(t == bool::typeid)
				{
					vt = VT_BOOL;
					Push(vt);
					bool bData = (bool)obData;
					unsigned short bBool = bData ? 0xFFFF : 0x0000;
					Push(bBool);
				}
				else if(t == short::typeid)
				{
					vt = VT_I2;
					Push(vt);
					Push((short)obData);
				}
				else if(t == unsigned int::typeid)
				{
					vt = VT_UI4;
					Push(vt);
					Push((unsigned int)obData);
				}
				else if(t == unsigned __int64::typeid)
				{
					vt = VT_UI8;
					Push(vt);
					Push((unsigned __int64)obData);
				}
				else if(t == char::typeid)
				{
					vt = VT_I1;
					Push(vt);
					Push((char)obData);
				}
				else if(t == wchar_t::typeid)
				{
					vt = VT_UI2;
					Push(vt);
					Push((wchar_t)obData);
				}
				else if(t == unsigned short::typeid)
				{
					vt = VT_UI2;
					Push(vt);
					Push((unsigned short)obData);
				}
				else if(t == DBNull::typeid)
				{
					vt = VT_NULL;
					m_pUQueue->Push(&vt);
				}
				else
				{
					throw gcnew Exception(gcnew String("Not implemented."));
				}
			}
			else
			{
				if(t == String::typeid)
				{
					vt = VT_BSTR;
					m_pUQueue->Push(&vt);
					String ^str = (String^)obData;
					if(str == nullptr)
					{
						unsigned long unLen = 0xFFFFFFFF;
						m_pUQueue->Push(&unLen);
					}
					else
					{
						unsigned long unLen = (unsigned long)(str->Length*sizeof(wchar_t));
						m_pUQueue->Push(&unLen);
						Push(str);
					}
				}
				else if(t == Decimal::typeid)
				{
					if(bDecToCY && bToNative)
					{
						vt = VT_CY;
						Push(vt);
						PushCY((Decimal)obData);
					}
					else
					{
						vt = VT_DECIMAL;
						Push(vt);
						Push((Decimal)obData);
					}
				}
				else if(t == DateTime::typeid)
				{
					vt = VT_DATE;
					m_pUQueue->Push(&vt);
					if(bToNative)
					{
						PushVariantDate((DateTime)obData);
					}
					else
					{
						Push((DateTime)obData);
					}
				}
				else if(t == Guid::typeid)
				{
					vt = VT_CLSID;
					Push(vt);
					Push((Guid)obData);
				}
				else if(t == System::Runtime::InteropServices::ComTypes::FILETIME::typeid)
				{
					vt = VT_FILETIME;
					Push(vt);
					Push((System::Runtime::InteropServices::ComTypes::FILETIME)obData);
				}
				else
				{
					IUSerializer ^us= dynamic_cast<IUSerializer^>(obData);
					if(us != nullptr)
					{
						vt =  VT_USERIALIZER_OBJECT;
						m_pUQueue->Push(&vt);
						us->SaveTo(this);
					}
					else
					{
						vt = VT_DOTNET_OBJECT;
						m_pUQueue->Push(&vt);
						Serialize(obData);
					}
				}
			}
		}
	}
	
	void CUQueue::Push(IntPtr ptr, int nLen)
	{
		if(nLen < 0)
			throw gcnew Exception(gcnew String("Bad input"));
		pin_ptr<BYTE> pData = (BYTE*)ptr.ToPointer();
		if(pData != nullptr && nLen > 0)
		{
			m_pUQueue->Push((const BYTE*)pData, nLen);
		}
	}

	int CUQueue::Pop(IntPtr ptr, int nLen)
	{
		if(nLen < 0)
			throw gcnew Exception(gcnew String("Bad input"));
		pin_ptr<BYTE> pData = (BYTE*)ptr.ToPointer();
		if(nLen == 0 || pData == nullptr)
			return 0;
		return m_pUQueue->Pop(pData, nLen);
	}

	generic<typename T>
	void CUQueue::XmlSerialize(T obj)
	{
		bool bNull = (obj == nullptr) ? true : false;
		Push(bNull);
		if(bNull)
			return;
		if(m_ms == nullptr)
			m_ms = gcnew MemoryStream();
		else
		{
			m_ms->SetLength(0);
			m_ms->Position = 0;
		}
		XmlSerializer ^serializer = gcnew XmlSerializer(T::typeid);
		serializer->Serialize(m_ms, obj);
		int nLen = (int)m_ms->Length;
		Push((int)nLen);
		Push(m_ms->GetBuffer(), (int)m_ms->Length);
	}

	generic<typename T>
	int CUQueue::XmlDeserialize(T %obj)
	{
		int nLen;
		bool bNull;
		if(m_pUQueue->GetSize() < sizeof(bool))
			throw gcnew Exception(gcnew String("No valid data available."));
		unsigned long ulStart = m_pUQueue->GetSize();
		Pop(bNull);
		if(bNull)
			return sizeof(bNull);
		if(m_pUQueue->GetSize() < sizeof(int))
			throw gcnew Exception(gcnew String("No valid data available."));
		m_pUQueue->Pop(&nLen);
		if((unsigned long)nLen > m_pUQueue->GetSize())
			throw gcnew Exception(gcnew String("No valid data available."));
		if(m_ms == nullptr)
			m_ms = gcnew MemoryStream();
		else
		{
			m_ms->SetLength(0);
			m_ms->Position = 0;
		}
		array<BYTE> ^aBytes = nullptr;
		Pop(aBytes, nLen);
		m_ms->Write(aBytes, 0, nLen);
		if(aBytes != nullptr)
		{
			delete aBytes;
		}
		m_ms->Position = 0;
		XmlSerializer ^serializer = gcnew XmlSerializer(T::typeid);
		obj = (T)serializer->Deserialize(m_ms);
		return (int)(ulStart - m_pUQueue->GetSize());
	}
	
	generic<typename T>
	void CUQueue::Serialize(T obj)
	{
		Serialize(obj, false, nullptr);
	}
	
	generic<typename T>
	void CUQueue::Serialize(T obj, bool bSoapFormat)
	{
		Serialize(obj, bSoapFormat, nullptr);
	}

	generic<typename T>
	void CUQueue::Serialize(T obj, bool bSoapFormat, IFormatter ^formatter)
	{
		bool bNull = (obj == nullptr) ? true : false;
		Push(bNull);
		if(bNull)
			return;
		if(formatter == nullptr)
		{
			if(bSoapFormat)
			{
				formatter = gcnew SoapFormatter();
			}
			else
			{
				formatter = gcnew BinaryFormatter();
			}
		}
		if(m_ms == nullptr)
		{
			m_ms = gcnew MemoryStream();
		}
		else
		{
			m_ms->SetLength(0);
			m_ms->Position = 0;
		}
		
		formatter->Serialize(m_ms, obj);
		Push(m_ms->GetBuffer(), (int)m_ms->Length);
		
	}
	
	void CUQueue::Save(String ^strData)
	{
		int nLen;
		if(strData == nullptr)
		{
			nLen = -1;
			m_pUQueue->Push((const BYTE*)&nLen, sizeof(nLen));
		}
		else
		{
			nLen = strData->Length*2;
			m_pUQueue->Push((const BYTE*)&nLen, sizeof(nLen));
			Push(strData);
		}
	}

	int CUQueue::Discard(int nLen)
	{
		return m_pUQueue->Pop((unsigned long)nLen);
	}
	
	int CUQueue::Pop(BYTE %bByte)
	{
		return PopMData(bByte);
	}

	int CUQueue::Pop(IUSerializer ^%USerializer)
	{
		bool b;
		int nLen = m_pUQueue->Pop((unsigned char*)&b, sizeof(b));
		do
		{
			if(b)
			{
				USerializer = nullptr;
				break;
			}
			if(USerializer == nullptr)
				throw gcnew System::InvalidOperationException("Must pass in an object implemented with the interface IUSerializer");
			nLen += USerializer->LoadFrom(this);
		}while(false);
		return nLen;
	}

	int CUQueue::Pop(CSocketProServerException ^%SocketProServerException)
	{
		int hr;
		ULONG nSvsID = 0;
		unsigned short sReqID = 0;
		String ^strMessage = nullptr;
		int nGet = Pop(hr);
		if(hr != S_OK)
		{
			nGet += m_pUQueue->Pop(&nSvsID);
			nGet += m_pUQueue->Pop(&sReqID);
			nGet += Load(strMessage);
		}
		SocketProServerException = gcnew CSocketProServerException(hr, strMessage, nSvsID, sReqID);
		return nGet;
	}

	int CUQueue::Pop(CUQueue ^%UQueue)
	{
		ULONG ulSize;
		int nGet = m_pUQueue->Pop(&ulSize);
		if(ulSize != UQUEUE_SIZE_NULL)
		{
			if(ulSize > m_pUQueue->GetSize())
			{
				nGet += (int)m_pUQueue->GetSize();
				m_pUQueue->SetSize(0);
				throw gcnew Exception(gcnew String("No valid data available."));
			}
			if(UQueue == nullptr)
			{
				UQueue = gcnew CUQueue();
			}
			UQueue->m_pUQueue->Push(m_pUQueue->GetBuffer(), ulSize);
			m_pUQueue->Pop(ulSize);
			nGet += (int)ulSize;
		}
		return nGet;
	}

	int CUQueue::Pop(array<BYTE> ^% aByte)
	{
		if(m_pUQueue->GetSize() == 0)
		{
			aByte = nullptr;
			return 0;
		}
		unsigned int nSize = 0;
		if(aByte != nullptr && aByte->Length > 0)
		{
			nSize = aByte->Length;
			if(nSize > m_pUQueue->GetSize())
				nSize = m_pUQueue->GetSize();
		}
		else
		{
			nSize = m_pUQueue->GetSize();
			aByte = gcnew array<BYTE>(nSize);
		}
		pin_ptr<BYTE> pByte = &aByte[0];
		m_pUQueue->Pop(pByte, nSize);
		return nSize;
	}

	int CUQueue::Pop(array<BYTE> ^% aByte, int nLen)
	{
		if(nLen < 0)
			throw gcnew Exception(gcnew String("Bad input."));
		unsigned int nSize = m_pUQueue->GetSize();
		if(nSize == 0 || nLen == 0)
		{
			aByte = nullptr;
			return 0;
		}
		if((unsigned int)nLen > nSize)
			nLen = (unsigned int)nSize;
		if(aByte == nullptr || aByte->Length < nLen)
		{
			aByte = gcnew array<BYTE>(nLen);
		}
		pin_ptr<BYTE> pByte = &aByte[0];
		m_pUQueue->Pop(pByte, nLen);
		return nLen;
	}

	int CUQueue::Pop(bool %bData)
	{
		return PopMData(bData);
	}

	int CUQueue::Pop(wchar_t %wChar)
	{
		return PopMData(wChar);
	}

	int CUQueue::Pop(char %sbData)
	{
		return PopMData(sbData);
	}

	int CUQueue::Pop(short %sData)
	{
		return PopMData(sData);
	}

	int CUQueue::Pop(unsigned short %usData)
	{
		return PopMData(usData);
	}

	int CUQueue::Pop(int %nData)
	{
		return PopMData(nData);
	}

	int CUQueue::Pop(unsigned int %unData)
	{
		return PopMData(unData);
	}

	int CUQueue::Pop(__int64 %lData)
	{
		return PopMData(lData);
	}

	int CUQueue::Pop(unsigned __int64 %ulData)
	{
		return PopMData(ulData);
	}

	int CUQueue::Pop(double %dData)
	{
		return PopMData(dData);
	}

	int CUQueue::Pop(float %fData)
	{
		return PopMData(fData);
	}

	/*int CUQueue::PopFloat([Out]float %pData)
	{
		return PopMData(pData);
	}*/

	int CUQueue::Pop(Decimal %decData)
	{
		return PopMData(decData);
	}

	int CUQueue::Pop(DateTime %dtData)
	{
		return PopMData(dtData);
	}

	int CUQueue::Pop(System::Runtime::InteropServices::ComTypes::FILETIME %ftData)
	{
		return PopMData(ftData);
	}

	int CUQueue::Pop(Guid %guidData)
	{
		return PopMData(guidData);
	}

	int CUQueue::Pop(String ^%strData)
	{
		ULONG ulLen = (m_pUQueue->GetSize()/2)*2;
		strData = nullptr;
		if(ulLen != 0)
		{
			strData = gcnew String((wchar_t*)m_pUQueue->GetBuffer(), 0, ulLen/sizeof(wchar_t));
			m_pUQueue->Pop(ulLen);
		}
		return (int)ulLen;
	}

	int CUQueue::Pop(String ^%strData, int nBytes)
	{
		if(nBytes < 0 || (nBytes%sizeof(wchar_t)) != 0)
			throw gcnew Exception(gcnew String("Bad input."));
		strData = nullptr;
		ULONG ulLen = (m_pUQueue->GetSize()/2)*2;
		if((unsigned long)nBytes > ulLen)
			nBytes = ulLen;
		if(nBytes != 0)
		{
			strData = gcnew String((wchar_t*)m_pUQueue->GetBuffer(), 0, nBytes/sizeof(wchar_t));
			m_pUQueue->Pop(nBytes);
		}
		return (int)nBytes;
	}

	int CUQueue::PopCY(Decimal %cyMoney)
	{
		__int64 lData;
		int nGet = m_pUQueue->Pop(&lData);
		if(nGet < sizeof(lData))
			throw gcnew Exception(gcnew String("No valid data available."));
		cyMoney = lData;
		cyMoney /= 10000;
		return nGet;
	}

	int CUQueue::PopVariantBool(bool %b)
	{
		unsigned short usData;
		int nGet = m_pUQueue->Pop(&usData);
		if(nGet < sizeof(usData))
			throw gcnew Exception(gcnew String("No valid data available."));
		b = (usData > 0) ? true : false;
		return nGet;
	}

	int CUQueue::PopSystemTime(DateTime %dt)
	{
		SYSTEMTIME sysTime;
		if(m_pUQueue->Pop(&sysTime) < sizeof(sysTime))
			throw gcnew Exception(gcnew String("No valid data available."));
		dt = DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
		return sizeof(sysTime);
	}
/*
	DateTime^ CUQueue::PopSystemTime()
	{
		SYSTEMTIME sysTime;
		if(m_pUQueue->Pop(&sysTime) < sizeof(sysTime))
			throw gcnew Exception(gcnew String("No valid data available."));
		return gcnew DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	}

	DateTime^ CUQueue::PopVariantDate()
	{
		double dDT;
		if(m_pUQueue->Pop(&dDT) < sizeof(dDT))
			throw gcnew Exception(gcnew String("No valid data available."));
		SYSTEMTIME sysTime;
		::VariantTimeToSystemTime(dDT, &sysTime);
		return gcnew DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
	}*/

	int CUQueue::PopVariantDate(DateTime %dt)
	{
		double dDT;
		if(m_pUQueue->Pop(&dDT) < sizeof(dDT))
			throw gcnew Exception(gcnew String("No valid data available."));
		SYSTEMTIME sysTime;
		::VariantTimeToSystemTime(dDT, &sysTime);
		dt = DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
		return sizeof(dDT);
	}
	int CUQueue::Pop(Object ^%obData)
	{
		bool bBad = false;
		unsigned short vt = 0;
		int lLen = m_pUQueue->Pop(&vt);
		if(lLen < sizeof(vt))
			throw gcnew Exception(gcnew String("No valid data available."));
		if(vt == VT_EMPTY)
		{
			obData = nullptr;
			return lLen;
		}
		else if(vt == VT_NULL)
		{
			obData = DBNull::Value;
			return lLen;
		}
		if((vt & VT_ARRAY) == VT_ARRAY)
		{
			ULONG ulSize = 0;
			if(m_pUQueue->GetSize() < sizeof(ULONG))
			{
				obData = nullptr;
				bBad = true;
			}
			lLen += m_pUQueue->Pop(&ulSize);
			if(bBad || ulSize == 0 || ulSize > m_pUQueue->GetSize())
			{
				m_pUQueue->SetSize(0);
			}
			else
			{
				switch(vt)
				{
				case (VT_I1|VT_ARRAY):
					{
						array<char> ^pData = gcnew array<char> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_UI1|VT_ARRAY):
					{
						array<BYTE> ^pData = gcnew array<BYTE> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_I2|VT_ARRAY):
					{
						array<short> ^pData = gcnew array<short> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_UI2|VT_ARRAY):
					{
						array<unsigned short> ^pData = gcnew array<unsigned short> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_INT|VT_ARRAY):
				case (VT_I4|VT_ARRAY):
					{
						array<int> ^pData = gcnew array<int> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_UINT|VT_ARRAY):
				case (VT_UI4|VT_ARRAY):
					{
						array<unsigned int> ^pData = gcnew array<unsigned int> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_I8|VT_ARRAY):
					{
						array<__int64> ^pData = gcnew array<__int64> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_UI8|VT_ARRAY):
					{
						array<unsigned __int64> ^pData = gcnew array<unsigned __int64> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_R4|VT_ARRAY):
					{
						array<float> ^pData = gcnew array<float> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_R8|VT_ARRAY):
					{
						array<double> ^pData = gcnew array<double> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_DATE|VT_ARRAY):
					{
						ULONG ul;
						array<DateTime> ^pData = gcnew array<DateTime> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += PopVariantDate(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_CY|VT_ARRAY):
					{
						ULONG ul;
						array<Decimal> ^pData = gcnew array<Decimal> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += PopCY(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_CLSID|VT_ARRAY):
					{
						array<Guid> ^pData = gcnew array<Guid> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_FILETIME|VT_ARRAY):
					{
						array<System::Runtime::InteropServices::ComTypes::FILETIME> ^pData = gcnew array<System::Runtime::InteropServices::ComTypes::FILETIME> (ulSize);
						lLen += PopMDataArray(pData);
						obData = pData;
					}
					break;
				case (VT_BSTR|VT_ARRAY):
					{
						ULONG ul;
						array<String^> ^pData = gcnew array<String^> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += Load(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_BOOL|VT_ARRAY):
					{
						ULONG ul;
						array<bool> ^pData = gcnew array<bool> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += PopVariantBool(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_VARIANT|VT_ARRAY):
					{
						ULONG ul;
						array<Object^> ^pData = gcnew array<Object^> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += Pop(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_DOTNET_OBJECT|VT_ARRAY):
					{
						ULONG ul;
						array<Object^> ^pData = gcnew array<Object^> (ulSize);
						for(ul=0; ul<ulSize; ul++)
						{
							lLen += Deserialize(pData[ul]);
						}
						obData = pData;
					}
					break;
				case (VT_USERIALIZER_OBJECT|VT_ARRAY):
				default:
					obData = nullptr;
					throw gcnew Exception(gcnew String("Unknown data type."));
					break;
				}
			}
		}
		else
		{
			switch(vt)
			{
			case VT_I1:
				{
					char cData;
					if(m_pUQueue->GetSize() < sizeof(char))
						bBad = true;
					lLen += m_pUQueue->Pop(&cData);
					obData = cData;
				}
				break;
			case VT_UI1:
				{
					BYTE bData;
					if(m_pUQueue->GetSize() < sizeof(bData))
						bBad = true;
					lLen += m_pUQueue->Pop(&bData, sizeof(bData));
					obData = bData;
				}
				break;
			case VT_I2:
				{
					short sData;
					if(m_pUQueue->GetSize() < sizeof(short))
						bBad = true;
					lLen += m_pUQueue->Pop(&sData);
					obData = sData;
				}
				break;
			case VT_UI2:
				{
					unsigned short usData;
					if(m_pUQueue->GetSize() < sizeof(unsigned short))
						bBad = true;
					lLen += m_pUQueue->Pop(&usData);
					obData = usData;
				}
				break;
			case VT_INT:
			case VT_I4:
				{
					long lData;
					if(m_pUQueue->GetSize() < sizeof(long))
						bBad = true;
					lLen += m_pUQueue->Pop(&lData);
					obData = lData;
				}
				break;
			case VT_UINT:
			case VT_UI4:
				{
					unsigned long ulData;
					if(m_pUQueue->GetSize() < sizeof(unsigned long))
						bBad = true;
					lLen += m_pUQueue->Pop(&ulData);
					obData = ulData;
				}
				break;
			case VT_I8:
				{
					__int64 llData;
					if(m_pUQueue->GetSize() < sizeof(__int64))
					{
						bBad = true;
					}
					lLen += m_pUQueue->Pop(&llData);
					obData = llData;
				}
				break;
			case VT_UI8:
				{
					unsigned __int64 ullData;
					if(m_pUQueue->GetSize() < sizeof(unsigned __int64))
						bBad = true;
					lLen += m_pUQueue->Pop(&ullData);
					obData = ullData;
				}
				break;
			case VT_R4:
				{
					float fData;
					if(m_pUQueue->GetSize() < sizeof(float))
					{
						bBad = true;
					}
					lLen += m_pUQueue->Pop(&fData);
					obData = fData;
				}
				break;
			case VT_R8:
				{
					double dData;
					if(m_pUQueue->GetSize() < sizeof(double))
						bBad = true;
					lLen += m_pUQueue->Pop(&dData);
					obData = dData;
				}
				break;
			case VT_CLSID:
				{
					Guid guid;
					if(m_pUQueue->GetSize() < sizeof(Guid))
					{
						bBad = true;
					}
					lLen += m_pUQueue->Pop(&guid);
					obData = guid;
				}
				break;
			case VT_BOOL:
				{
					VARIANT_BOOL bData;
					if(m_pUQueue->GetSize() < sizeof(bData))
						bBad = true;
					lLen += m_pUQueue->Pop(&bData);
					obData = (bData == VARIANT_FALSE) ? false : true;
				}
				break;
			case VT_DECIMAL:
				{
					Decimal dec;
					if(m_pUQueue->GetSize() < sizeof(dec))
					{
						bBad = true;
					}
					lLen += m_pUQueue->Pop(&dec);
					obData = dec;
				}
				break;
			case VT_DATE:
				{
					DateTime dt = DateTime();
					if(m_pUQueue->GetSize() < sizeof(DATE))
					{
						bBad = true;
					}
					lLen += PopVariantDate(dt);
					obData = dt;
				}
				break;
			case VT_CY:
				{
					Decimal dec;
					if(m_pUQueue->GetSize() < sizeof(CURRENCY))
					{
						bBad = true;
					}
					lLen += PopCY(dec);
					obData = dec;
				}
				break;
			case VT_FILETIME:
				{
					System::Runtime::InteropServices::ComTypes::FILETIME ft;
					if(m_pUQueue->GetSize() < sizeof(System::Runtime::InteropServices::ComTypes::FILETIME))
					{
						bBad = true;
					}
					lLen += Pop(ft);
					obData = ft;
				}
				break;
			case VT_BSTR:
				{
					if(m_pUQueue->GetSize() < sizeof(unsigned long))
					{
						bBad = true;
					}
					String ^str = nullptr;
					lLen += Load(str);
					obData = str;
				}
				break;
			case VT_VARIANT:
				Pop(obData);
				break;
			case VT_DOTNET_OBJECT:
				Deserialize(obData);
				break;
			case VT_USERIALIZER_OBJECT:
				if(obData != nullptr)
				{
					IUSerializer ^us= dynamic_cast<IUSerializer^>(obData);
					if(us != nullptr)
					{
						lLen += us->LoadFrom(this);
						break;
					}
				}
			default:
				throw gcnew Exception(gcnew String("Unknown data type."));
				break;
			}
		}
		if(bBad)
		{
			throw gcnew Exception(gcnew String("No valid data available."));
		}
		return lLen;
	}
	generic < typename T >
	int CUQueue::Deserialize(T %obj)
	{
		return Deserialize(obj, false, nullptr);
	}
	generic < typename T >
	int CUQueue::Deserialize(T %obj, bool bSoapFormat)
	{
		return Deserialize(obj, bSoapFormat, nullptr);
	}
	generic < typename T >
	int CUQueue::Deserialize(T %obj, bool bSoapFormat, IFormatter ^formatter)
	{
		bool bNull;
		if(m_pUQueue->GetSize() < sizeof(bool))
			throw gcnew Exception(gcnew String("No valid data available."));
		unsigned long nLen = m_pUQueue->GetSize();
		Pop(bNull);
		if(bNull)
		{
			return sizeof(bNull);
		}
		unsigned long ulLen = m_pUQueue->GetSize();
		if(ulLen == 0)
		{
			Object ^myNull = nullptr;
			obj = (T)myNull;
			return 0;
		}
		if(formatter == nullptr)
		{
			if(bSoapFormat)
			{
				formatter = gcnew SoapFormatter();
			}
			else
			{
				formatter = gcnew BinaryFormatter();
			}
		}
		if(m_ms == nullptr)
		{
			m_ms = gcnew MemoryStream();
		}
		else
		{
			m_ms->SetLength(0);
			m_ms->Position = 0;
		}
		array<BYTE> ^aBytes = nullptr;
		Pop(aBytes);
		m_ms->Write(aBytes, 0, aBytes->Length);
		if(aBytes != nullptr)
		{
			delete aBytes;
		}
		m_ms->Position = 0;
		obj = (T)formatter->Deserialize(m_ms);
		ulLen = (unsigned long)(m_ms->Length - m_ms->Position);
		if(ulLen != 0)
		{
			Push(m_ms->GetBuffer(), ulLen, (int)m_ms->Position);
		}
		return (int)(nLen - m_pUQueue->GetSize());
	}

	int CUQueue::Load(String ^%str)
	{
		str = nullptr;
		unsigned int unLen;
		int nGet = m_pUQueue->Pop(&unLen);
		if(nGet < sizeof(unLen))
			throw gcnew Exception(gcnew String("No valid data available."));
		if(unLen != 0xFFFFFFFF && unLen > m_pUQueue->GetSize())
		{
			m_pUQueue->SetSize(0);
			throw gcnew Exception(gcnew String("Invalid data found."));
		}

		if(unLen != 0xFFFFFFFF)
		{
			str = gcnew String((wchar_t*)m_pUQueue->GetBuffer(), 0, unLen/sizeof(wchar_t));
			m_pUQueue->Pop(unLen);
		}
		else
		{
			return nGet;
		}
		return ((int)unLen + sizeof(unLen));
	}

	CScopeUQueue::CScopeUQueue() : m_UQueue(Lock())
	{
	}
	CScopeUQueue::~CScopeUQueue()
	{
		Unlock(m_UQueue);
	}

	CUQueue^ CScopeUQueue::Lock()
	{
		CUQueue ^p;
		g_cs.Lock();
		if(m_sQueue->Count > 0)
			p = m_sQueue->Pop();
		else
			p = gcnew CUQueue();
		g_cs.Unlock();
		return p;
	}

	void CScopeUQueue::Unlock(CUQueue ^UQueue)
	{
		if(UQueue != nullptr)
		{
			UQueue->SetSize(0);
			g_cs.Lock();
			m_sQueue->Push(UQueue);
			g_cs.Unlock();
		}
	}

	void CScopeUQueue::DestroyUQueuePool()
	{
		CAutoLock	AutoLock(&g_cs.m_sec);
		while(m_sQueue->Count > 0)
		{
			delete (m_sQueue->Pop());
		}
	}

	__int64 CScopeUQueue::GetMemoryConsumed()
	{
		CUQueue ^p;
		__int64 lSize = 0;
		System::Collections::Generic::Stack<CUQueue^> ^sQueue = gcnew System::Collections::Generic::Stack<CUQueue^>;
		CAutoLock	AutoLock(&g_cs.m_sec);
		while(m_sQueue->Count > 0)
		{
			p = m_sQueue->Pop();
			lSize += p->BufferSize;
			sQueue->Push(p);
		}
		m_sQueue = sQueue;
		return lSize;
	}

	void CScopeUQueue::CleanUQueuePool()
	{
		CUQueue ^p;
		System::Collections::Generic::Stack<CUQueue^> ^sQueue = gcnew System::Collections::Generic::Stack<CUQueue^>;
		CAutoLock	AutoLock(&g_cs.m_sec);
		while(m_sQueue->Count > 0)
		{
			p = m_sQueue->Pop();
			p->CleanTrack();
			sQueue->Push(p);
		}
		m_sQueue = sQueue;
	}
}