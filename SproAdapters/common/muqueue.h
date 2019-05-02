// SProAdapter.h

#pragma once

using namespace System;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Runtime::Serialization;
using namespace System::Runtime::Serialization::Formatters::Binary;
using namespace System::Runtime::Serialization::Formatters::Soap;
using namespace System::Collections::Generic;
using namespace System::Xml::Serialization;

namespace SocketProAdapter
{
/*
	[CLSCompliantAttribute(true)] 
	public ref struct CCriticalSection
	{
	public:
		CCriticalSection()
		{
			m_pCS = new CRITICAL_SECTION();
			::InitializeCriticalSection(m_pCS);
		}
		virtual ~CCriticalSection()
		{
			::DeleteCriticalSection(m_pCS);
			delete m_pCS;
		}

	public:
		void Lock()
		{
			::EnterCriticalSection(m_pCS);
		}
		void Unlock()
		{
			::LeaveCriticalSection(m_pCS);
		}
		bool TryLock()
		{
			return (::TryEnterCriticalSection(m_pCS) == 0) ? false : true;
		}

	private:
		LPCRITICAL_SECTION	m_pCS;
	};
*/
	[CLSCompliantAttribute(true)] 
	public ref class CClientSocketException : public Exception
	{
	public:
		CClientSocketException(int hr, String ^strMessage) : Exception(strMessage)
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
	[Serializable]
	public ref class CSocketProServerException : public CClientSocketException
	{
	public:
		CSocketProServerException(int hr, String ^strMessage, int nSvsID, short sRequestID)
			: CClientSocketException(hr, strMessage)
		{
			m_nSvsID = nSvsID;
			m_sRequestID = sRequestID;
		}

		CSocketProServerException(int hr, String ^strMessage)
			: CClientSocketException(hr, strMessage)
		{
			m_nSvsID = 0;
			m_sRequestID = 0;
		}

	public:
		int		m_nSvsID;
		short	m_sRequestID;
	};

	ref class CUQueue;
	[CLSCompliantAttribute(true)] 
	public interface class IUSerializer
	{
	public:
		int LoadFrom(CUQueue ^UQueue);
		void SaveTo(CUQueue ^UQueue);
	};

	#define MAKE_BALANCE " Make sure both serialization and de-serialization matches each of other in sequence."
	
	[CLSCompliantAttribute(true)] 
	public ref class CUQueue
	{
	public:
		CUQueue();
		CUQueue(int nBlockSize);
		virtual ~CUQueue();

	internal:
		bool m_bScope;

	public:
		/// <summary>
		/// Cleam all memory buffer except the buffer chunk packed with data. 
		/// Call this method to clean all of secret data left in memory foot print.
		/// </summary>
		void CleanTrack();

		/// <summary>
		/// Release all memores back into operation system. 
		/// </summary>
		void Empty();
		
		/// <summary>
		/// Returns the packed data size in byte.
		/// </summary>
		int GetSize();

		/// <summary>
		/// Set data size. If buffer is is less than expected size nSize, 
		/// the underlying buffer is reallocated for a larger buffer size.
		/// </summary>
		void SetSize(int nSize);

		/// <summary>
		/// Reallocate the buffer with a new specified size nSize. 
		/// </summary>
		void ReallocBuffer(int nSize);
		
		/// <summary>
		/// Set head position to 0 and move packed data in buffer as required. 
		/// </summary>
		void SetHeadPosition();
		
		/// <summary>
		/// Get a pointer to packed data in the underlying native memory.
		/// </summary>
		IntPtr GetBuffer();

		/// <summary>
		/// Get a pointer to packed data in the underlying native memory offset by nPos.
		/// </summary>
		IntPtr GetBuffer(int nPos);
		
		CInternalUQueue* GetInternalUQueue();
		const unsigned char* const GetNativeBuffer(int nPos);
		/*
			Pop and discard a number of bytes.

			!!!! Don't confuse the method with this method (int Pop(int %nData)) !!!!
		*/

		/// <summary>
		/// Discard a given number (nLen) of data. Don't get confused with the method CUQueue::Pop(int %nData)!
		/// The method returned the actual number of bytes discarded.
		/// </summary>
		int Discard(int nLen);
		
		property int BufferSize
		{
			int get()
			{
				return m_pUQueue->GetMaxSize();
			}
		}
		
		/// <summary>
		/// Same as the method GetSize()
		/// </summary>
		property int Size
		{
			int get()
			{
				return m_pUQueue->GetSize();
			}
		}
		
		property int BlockSize
		{
			int get()
			{
				return m_pUQueue->GetBlockSize();
			}
			void set(int nBlockSize)
			{
				if(nBlockSize < 64)
				{
					nBlockSize = 64;
				}
				m_pUQueue->SetBlockSize(nBlockSize);
			}
		}

		property int HeadPosition
		{
			int get()
			{
				return m_pUQueue->GetHeadPosition();
			}
		}
		
		void Push(bool bData);
		void Push(BYTE bByte);
		void Push(wchar_t wChar);
		void Push(char sbData);
		void Push(short sData);
		void Push(unsigned short usData);
		void Push(int nData);
		void Push(unsigned int unData);
		void Push(__int64 lData);
		void Push(unsigned __int64 ulData);
		void Push(double dData);
		void Push(float fData);
		void Push(Decimal decData);
		void Push(TimeSpan ts);

		/*
			Note that System::Runtime::InteropServices::FILETIME is not supported
		*/
		void Push(System::Runtime::InteropServices::ComTypes::FILETIME ftData);
		void Push(Guid guidData);
		int Pop([Out]bool %bData);
		int Pop([Out]BYTE %bByte);
		
		/// <summary>
		/// Retrieve all data from the memory queue. 
		/// The method returns the data size obtained actually in byte.
		/// </summary>
		int Pop([Out]array<BYTE> ^% aByte);

		/// <summary>
		/// Retrieve a specified size (nLen) of data in byte from the memory queue.
		/// The method returns the data size obtained actually in byte.
		/// </summary>
		int Pop([Out]array<BYTE> ^% aByte, int nLen);
		int Pop([Out]wchar_t %wChar);
		int Pop([Out]char %sbData);
		int Pop([Out]short %sData);
		int Pop([Out]unsigned short %usData);
		int Pop([Out]int %nData);
		int Pop([Out]unsigned int %unData);
		int Pop([Out]__int64 %lData);
		int Pop([Out]unsigned __int64 %ulData);
		int Pop([Out]double %dData);
		int Pop([Out]float %fData);
		int Pop([Out]Decimal %decData);
		int Pop([Out]DateTime %dtData);
		int Pop([Out]Guid %guidData);
		int Pop([Out]TimeSpan %ts);
		int Pop([Out]DateTimeOffset %offset);
		/*
			Note that System::Runtime::InteropServices::FILETIME is not supported
		*/
		int Pop([Out]System::Runtime::InteropServices::ComTypes::FILETIME %ftData);


		/*
			Pay attention to the difference between Save/Load and Push/Pop.

			Saving or loading a string is involved with an int indicating the length of a string in byte ahead.
			Pushing or poping a string is NOT inolved with an int ahead
		*/
		/// <summary>
		/// Save a string into the memory queue without saving string length.
		/// Keep eyes on difference between this method and the method Save.
		/// </summary>
		void Push(String ^strData);
		void Push(array<BYTE> ^aData, int nLen, int nIndexStarted);
		void Push(array<BYTE> ^aData, int nLen);
		void Push(array<BYTE> ^aData);
		void Push(DateTime dtData);
		void Push(CUQueue ^UQueue);
		void Push(DateTimeOffset offset);
		void Push(CSocketProServerException ^SocketProServerException);
		
		/*
			Push or pop an obejct. 

			If push or pop an object that contains a non-primitive type of data,
			CUQueue tries to use MS dotNet serializer automatically with binary format
		*/
		/// <summary>
		/// Retrieve a .NET object from the memory queue.
		/// Note that .NET object may be compatible with many native VARIANTs.
		/// </summary>
		int Pop([Out]Object ^%obData);
		
		/// <summary>
		/// Retrieve a CUQueue from main memory queue.
		/// </summary>
		int Pop([Out]CUQueue ^%UQueue);
		
		/// <summary>
		/// Retrieve a CSocketProServerException from the memory queue. 
		/// </summary>
		int Pop([Out]CSocketProServerException ^%SocketProServerException);

		/// <summary>
		/// Write a .NET object into the memory queue with highest compatibility with native VARIANTs.
		/// Note that .NET object may be compatible with many native VARIANTs.
		/// </summary>
		void Push(Object ^obData);

		/// <summary>
		/// Write a .NET object into the memory queue. 
		/// If bToNative is set to true and obData contains a DateTime, the DateTime will be converted into VARIANT DATE.
		/// Note that .NET object may be compatible with many native VARIANTs.
		/// </summary>
		void Push(Object ^obData, bool bToNative);

		/// <summary>
		/// Write a .NET object into the memory queue. 
		/// If bToNative is set to true and obData contains a DateTime, the DateTime will be converted into VARIANT DATE.
		/// If both bToNative and bDecToCY are true, obData will be converted into VARIANT Currency if obData contains a Decimal data. 
		/// Note that .NET object may be compatible with many native VARIANTs.
		/// </summary>
		void Push(Object ^obData, bool bToNative, bool bDecToCY);
		
		/*
			If you use the interface IUSerializer to serialize or de-serialize an object,
			it'll significantly improve the performance over serialization with dotNet serializer.

			However, it requires a little more code only
		*/
		
		/// <summary>
		/// Write a .NET object into the memory queue through the interface IUSerializer. 
		/// Note that packing an object is much faster through IUSerializer than .NET serializer.
		/// </summary>
		void Push(IUSerializer ^USerializer);

		/// <summary>
		/// Retrieve a .NET object from the memory queue through the interface IUSerializer. 
		/// Note that retrieving an object is much faster through IUSerializer than .NET serializer.
		/// </summary>
		generic<typename T> where T : IUSerializer, gcnew()
		int Pop([Out]T %USerializer)
		{
			bool b;
			int nLen = m_pUQueue->Pop((unsigned char*)&b, sizeof(b));
			do
			{
				if(b)
				{
					Object ^obj = nullptr;
					USerializer = (T)obj;
					break;
				}
				if(USerializer == nullptr)
					USerializer = gcnew T();
				nLen += USerializer->LoadFrom(this);
			}while(false);
			return nLen;
		}

		/*
			Pay attention to the difference between Save/Load and Push/Pop.

			Saving or loading a string is involved with an int indicating the length of a string in byte ahead.
			Pushing or poping a string is NOT inolved with an int ahead
		*/
		/// <summary>
		/// Retrieve a string from the memory queue. 
		/// The method returns the number of bytes retrieved.
		/// </summary>
		int Pop([Out]String ^%strData);

		/// <summary>
		/// Retrieve a specified length of string from the memory queue in byte. 
		/// The method returns the actual number of bytes retrieved.
		/// Pay attention to the difference between this method and the method Load.
		/// </summary>
		int Pop([Out]String ^%strData, int nBytes);
		
		
		/*
			Fast push or pop an array of primitive data, values and structures.
		*/
		void Push(IntPtr ptr, int nLen);
		int Pop([Out]IntPtr ptr, int nLen);
		
		
		/*
			Push or pop an object through MS dotNet serializer.

			It is simple to use for complex data type. However, it is much slower. 
		*/

		/// <summary>
		/// Write an object into memory queue through standard .NET binary serializer. 
		/// </summary>
		generic<typename T>
		void Serialize(T obj);

		/// <summary>
		/// Write an object into memory queue through standard .NET serializer (binary or soap). 
		/// </summary>
		generic<typename T>
		void Serialize(T obj, bool bSoapFormat);

		/// <summary>
		/// Write an object into memory queue through a specified serializer format formatter in either binary or soap. 
		/// </summary>
		generic<typename T>
		void Serialize(T obj, bool bSoapFormat, IFormatter ^formatter);
		
		/// <summary>
		/// Write an object into memory queue through XML serialization.
		/// </summary>
		generic<typename T>
		void XmlSerialize(T obj);

		/// <summary>
		/// Retrieve an object from memory queue through XML serialization. 
		/// </summary>
		generic<typename T>
		int XmlDeserialize(T %obj);

		/// <summary>
		/// Retrieve an object from memory queue through standard .NET binary serializer. 
		/// </summary>
		generic < typename T >
		int Deserialize([Out]T %obj);

		/// <summary>
		/// Retrieve an object from memory queue through through standard .NET serializer (binary or soap).  
		/// </summary>
		generic < typename T >
		int Deserialize([Out]T %obj, bool bSoapFormat);

		/// <summary>
		/// Retrieve an object from memory queue through a specified serializer format formatter in either binary or soap. 
		/// </summary>
		generic < typename T >
		int Deserialize([Out]T %obj, bool bSoapFormat, IFormatter ^formatter);
		
		/*
			Pay attention to the difference between Save/Load and Push/Pop.

			Saving or loading a string is involved with an int indicating the length of a string in byte ahead.
			Pushing or poping a string is NOT inolved with an int ahead
		*/

		/// <summary>
		/// Write an integer data first that indicates the string length in byte. 
		/// Afterwards, write the string (strData) into the memory queue. 
		/// Note if the string data is null, the integer data will be -1 or 0xFFFFFFFF.
		/// Keep eyes on the difference between this method and the method Push for string.
		/// </summary>
		void Save(String ^strData);

		/// <summary>
		/// Retrieve an integer data first that indicates the coming string length in byte first. 
		/// Afterwards, retrieve the string (str) from the memory queue according to the integer data. 
		/// Keep eyes on the difference between this method and the method Pop for string.
		/// </summary>
		int Load([Out]String ^%str);
		
		/*
			These methods are useful if you like to communicate with native code.
		*/
		int PopCY([Out]Decimal %cyMoney);
		int PopVariantBool([Out]bool %b);
		int PopSystemTime([Out]DateTime %dt);
		int PopVariantDate([Out]DateTime %dt);
//		DateTime^ PopSystemTime();
//		DateTime^ PopVariantDate();
		void PushCY(Decimal cyMoney);
		void PushVariantBool(bool b);
		void PushSystemTime(DateTime dt);
		void PushVariantDate(DateTime dt);

    /*
    The following not working in pair for load and save!!!

    template <typename T>
    void Save(Generic::IList<T> ^obj)
    {
      if(obj == nullptr)
        throw gcnew Exception("Can not serialize a null collection object");
      Push(obj->Count);
      for each(T t in obj)
      {
        Save(t);
      }
    }

    template <typename T>
    int Load([Out]Generic::IList<T> %obj)
    {
      int n;
      int count;
      obj.Clear();
      int start = GetSize();
      Pop(count);
      for(n=0; n<count; ++n)
      {
        T t;
        Load(t);
        obj.Add(t);
      }
      return (start - GetSize());
    }

    template <typename K, typename V>
    void Save(Generic::Dictionary<K, V> ^obj)
    {
      if(obj == nullptr)
        throw gcnew Exception("Can not serialize a null dictionary object");
      Push(obj->Count);
      for each(K k in obj->Keys)
      {
        Save(k);
        Save(obj[k]);
      }
    }
		
    template <typename K, typename V>
    int Load([Out]Generic::Dictionary<K, V> %obj)
    {
      int n;
      int count;
      obj.Clear();
      int start = GetSize();
      Pop(count);
      for(n=0; n<count; ++n)
      {
        K k;
        V v;
        Load(k);
        Load(v);
        obj.Add(k, v);
      }
      return (start - GetSize());
    }

    generic <typename K, typename V>
    void Save(Generic::SortedDictionary<K, V> ^obj)
    {
      if(obj == nullptr)
        throw gcnew Exception("Can not serialize a null dictionary object");
      Push(obj->Count);
      for each(K k in obj->Keys)
      {
        Save(k);
        Save(obj[k]);
      }
    }

    generic <typename K, typename V>
    int Load([Out]Generic::SortedDictionary<K, V> %obj)
    {
      int n;
      int count;
      obj.Clear();
      int start = GetSize();
      Pop(count);
      for(n=0; n<count; ++n)
      {
        K k;
        V v;
        Load(k);
        Load(v);
        obj.Add(k, v);
      }
      return (start - GetSize());
    }

    generic <typename T>
    void Save(Generic::Queue<T> ^obj)
    {
      if(obj == nullptr)
        throw gcnew Exception("Can not serialize a null queue object");
      Push(obj->Count);
      for each(T t in obj)
      {
        Save(t);
      }
    }

    generic <typename T>
    int Load([Out]Generic::Queue<T> %obj)
    {
      int n;
      int count;
      obj.Clear();
      int start = GetSize();
      Pop(count);
      for(n=0; n<count; ++n)
      {
        T t;
        Load(t);
        obj.Enqueue(t);
      }
      return (start - GetSize());
    }

    generic <typename T>
    void Save(Generic::Stack<T> ^obj)
    {
      int n;
      if(obj == nullptr)
        throw gcnew Exception("Can not serialize a null stack object");
      int count = obj->Count;
      Push(count);
      array<T> ^arr = obj->ToArray();
      for(n=count-1; n>=0; --n)
      {
        Save(arr[n]);
      }
    }

    generic <typename T>
    int Load([Out]Generic::Stack<T> %obj)
    {
      int n;
      int count;
      obj.Clear();
      int start = GetSize();
      Pop(count);
      for(n=0; n<count; ++n)
      {
        T t;
        Load(t);
        obj.Push(t);
      }
      return (start - GetSize());
    }*/


		generic < typename T >
		void Save(T data)
		{
			Type ^t = T::typeid;
			do
			{
				if(t == String::typeid)
				{
					Save((String^)data);
					break;
				}
				if(t->IsPrimitive)
				{
					Push(IntPtr(&data), (int)sizeof(T));
					break;
				}
				if(t == DateTime::typeid)
				{
					Push((DateTime)data);
					break;
				}
				if(t == DateTimeOffset::typeid)
				{
					Push((DateTimeOffset)data);
					break;
				}
				if(t == TimeSpan::typeid)
				{
					Push((TimeSpan)data);
					break;
				}
				if(t == Object::typeid)
				{
					Push((Object^)data);
					break;
				}
				if(t == Guid::typeid)
				{
					Push((Guid)data);
					break;
				}
				if(t == Decimal::typeid)
				{
					Push((Decimal)data);
					break;
				}
				if(t == CUQueue::typeid)
				{
					Push((CUQueue^)data);
					break;
				}
				if(t == CSocketProServerException::typeid)
				{
					Push((CSocketProServerException^)data);
					break;
				}
				if(t == System::Runtime::InteropServices::ComTypes::FILETIME::typeid)
				{
					Push((System::Runtime::InteropServices::ComTypes::FILETIME)data);
					break;
				}

				Type ^type= t->GetInterface("SocketProAdapter.IUSerializer");
				if(type != nullptr)
				{
					Push((IUSerializer ^)data);
					/*if(data == nullptr)
						data = (T)System::Activator::CreateInstance(t);
					IUSerializer ^USerializer = (IUSerializer ^)data;
					USerializer->SaveTo(this);*/
					break;
				}

				if(t->IsSerializable)
				{
					Serialize(data);
					break;
				}

				throw gcnew System::InvalidOperationException("Serialization not supported for data type " 
					+ t->FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer, or serializable at least.");
			}while(false);
		}

		generic <typename T>
		int Load([Out]T %data)
		{
			int len = 0;
			Type ^type = T::typeid;
			do
			{
				if(type == String::typeid)
				{
					String ^str = nullptr;
					len = Load(str);
					data = (T)str;
					break;
				}

				if(type->IsPrimitive)
				{
					T t;
					len = Pop(IntPtr(&t), (int)sizeof(T));
					data = t;
					break;
				}

				if(type == DateTime::typeid)
				{
					DateTime dt;
					len = Pop(dt);
					data = (T)dt;
					break;
				}

				if(type == DateTimeOffset::typeid)
				{
					DateTimeOffset offset;
					len = Pop(offset);
					data = (T)offset;
					break;
				}

				if(type == TimeSpan::typeid)
				{
					TimeSpan ts;
					len = Pop(ts);
					data = (T)ts;
					break;
				}

				if(type == Decimal::typeid)
				{
					Decimal dec;
					len = Pop(dec);
					data = (T)dec;
					break;
				}

				if(type == Guid::typeid)
				{
					Guid guid;
					len = Pop(guid);
					data = (T)guid;
					break;
				}

				if(type == Object::typeid)
				{
					Object ^obj;
					len = Pop(obj);
					data = (T)obj;
					break;
				}

				Type ^t= type->GetInterface("SocketProAdapter.IUSerializer");
				if(t != nullptr)
				{
					bool b;
					len = m_pUQueue->Pop((unsigned char*)&b, sizeof(b));
					if(b)
					{
						Object ^obj = nullptr;
						data = (T)obj;
						break;
					}
					if(data == nullptr)
						data = (T)System::Activator::CreateInstance(type);
					IUSerializer ^p = (IUSerializer^)data;
					len += p->LoadFrom(this);
					break;
				}

				if(type->IsSerializable)
				{
					len = Deserialize(data);
					break;
				}

				if(type == System::Runtime::InteropServices::ComTypes::FILETIME::typeid)
				{
					System::Runtime::InteropServices::ComTypes::FILETIME ft;
					len = Pop(ft);
					data = (T)ft;
					break;
				}

				if(t == CUQueue::typeid)
				{
					CUQueue ^q = (CUQueue^) data;
					len = Pop(q);
					data = (T)q;
					break;
				}

				if(t == CSocketProServerException::typeid)
				{
					CSocketProServerException ^ex;
					len = Pop(ex);
					data = (T)ex;
					break;
				}
				throw gcnew System::InvalidOperationException("Deserialization not supported for data type " 
					+ t->FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer, or serializable at least.");
			}while(false);
			return len;
		}
		
	private:
		template <class tDataType>
		int PopMData(tDataType %tData)
		{
			pin_ptr<tDataType> p = &tData;
			unsigned long nGet = m_pUQueue->Pop((BYTE*)p, (unsigned long)sizeof(tDataType), 0);
			if(nGet < sizeof(tDataType))
			{
				throw gcnew Exception(gcnew String("Invalid data found during de-serialization.") + MAKE_BALANCE);
			}
			return nGet;
		}

		template<class tType>
		int PopMDataArray(array<tType> ^%pArray)
		{
			ATLASSERT(pArray != nullptr);
			unsigned long nLen = (unsigned long)(pArray->Length * sizeof(tType));
			pin_ptr<tType> p = &pArray[0];
			unsigned long nGet = m_pUQueue->Pop((BYTE*)p, nLen);
			if(nGet < nLen)
			{
				throw gcnew Exception(gcnew String("Invalid data array found during de-serialization.") + MAKE_BALANCE);
			}
			return nLen;
		}

		template<class tType>
		void PushMDataArray(array<tType> ^pArray)
		{
			ATLASSERT(pArray != nullptr);
			int nLen = pArray->Length;
			m_pUQueue->Push((BYTE*)&nLen, sizeof(nLen));
			pin_ptr<tType> p = &pArray[0];
			m_pUQueue->Push((BYTE*)p, (unsigned long)(nLen*sizeof(tType)));
		}
		MemoryStream	^m_ms;
	internal:
		CInternalUQueue	*m_pUQueue;
	};
	
	/// <summary>
	/// A class for reusing a pool of recycled memory queues.
	/// </summary>
	[CLSCompliantAttribute(true)] 
	public ref class CScopeUQueue
	{
	public:
		CScopeUQueue();
		virtual ~CScopeUQueue();

	public:
		/// <summary>
		/// Delete all of recycled memory queues
		/// </summary>
		static void DestroyUQueuePool();

		/// <summary>
		/// Clean all of recycled memory queues without deleting them.
		/// </summary>
		static void CleanUQueuePool();
		
		/// <summary>
		/// Query the memory consumed in bytes, which excludes the using memory.
		/// </summary>
		static __int64 GetMemoryConsumed();

		/// <summary>
		/// Get a memory queue from pool.
		/// </summary>
		static CUQueue^ Lock();

		/// <summary>
		/// Put a memory queue into pool for reuse.
		/// </summary>
		static void Unlock(CUQueue ^UQueue);

		/// <summary>
		/// Internal real memory buffer
		/// </summary>
		property CUQueue^ UQueue
		{
			CUQueue^ get()
			{
				return m_UQueue;
			}
		}

    /*
    Not working correctly in pair for Save and Load!!!

    template <typename T>
    void Save(Generic::IList<T> ^obj)
    {
      m_UQueue->Save<T>(obj);
    }

    template <typename T>
    int Load([Out]Generic::IList<T> %obj)
    {
      return m_UQueue->Load<T>(obj);
    }

    template <typename K, typename V>
    void Save(Generic::Dictionary<K, V> ^obj)
    {
      m_UQueue->Save<K, V>(obj);
    }

    template <typename K, typename V>
    int Load([Out]Generic::Dictionary<K, V> %obj)
    {
      return m_UQueue->Load<K, V>(obj);
    }

    generic <typename K, typename V>
    void Save(Generic::SortedDictionary<K, V> ^obj)
    {
      m_UQueue->Save<K, V>(obj);
    }

    generic <typename K, typename V>
    int Load([Out]Generic::SortedDictionary<K, V> %obj)
    {
      return m_UQueue->Load<K, V>(obj);
    }

    generic <typename T>
    void Save(Generic::Queue<T> ^obj)
    {
      m_UQueue->Save<T>(obj);
    }

    generic <typename T>
    int Load([Out]Generic::Queue<T> %obj)
    {
      return m_UQueue->Load<T>(obj);
    }

    generic <typename T>
    void Save(Generic::Stack<T> ^obj)
    {
      m_UQueue->Save<T>(obj);
    }

    generic <typename T>
    int Load([Out]Generic::Stack<T> %obj)
    {
      return m_UQueue->Load<T>(obj);
    }*/
		
		/// <summary>
		/// Save a data into internal memory queue.
		/// </summary>
		generic < typename T >
		void Save(T data)
		{
			m_UQueue->Save(data);
		}
		
		/// <summary>
		/// Save a string into internal memory queue.
		/// </summary>
		void Save(String ^str)
		{
			m_UQueue->Save(str);
		}

    /// <summary>
    /// Load a data from internal memory queue.
    /// </summary>
    generic <typename T>
    int Load([Out]T %data)
    {
      return m_UQueue->Load(data);
    }

    /// <summary>
    /// Load a string from internal memory queue.
    /// </summary>
    int Load([Out]String ^%str)
    {
      return m_UQueue->Load(str);
    }
		
	internal:
		CUQueue  ^m_UQueue;
		/*CUQueue^ Detach()
		{
			CUQueue ^q = m_UQueue;
			m_UQueue = nullptr;
			return q;
		}*/

	private:
		/*CUQueue^ Lock();
		void Unlock(CUQueue ^UQueue);*/
		static System::Collections::Generic::Stack<CUQueue^> ^m_sQueue = gcnew System::Collections::Generic::Stack<CUQueue^>;
	};
}
