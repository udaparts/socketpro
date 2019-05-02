using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Xml.Serialization;

namespace SocketProAdapter
{
	public interface IUSerializer
	{
		int LoadFrom(CUQueue UQueue);
		void SaveTo(CUQueue UQueue);
	}

    public interface IUPush
    {
        bool Broadcast(object Message, int[] Groups);
        bool Broadcast(byte[] Message, int[] Groups);
        bool Enter(int[] Groups);
        bool Exit();
        bool SendUserMessage(object Message, string UserId);
        bool SendUserMessage(string UserId, byte[] Message);
    }

	public class CClientSocketException : Exception
	{
		public CClientSocketException(int hr, string strMessage) : base(strMessage)
		{
			base.HResult = hr;
		}

		public new int HResult
		{
			get
			{
				return base.HResult;
			}
			set
			{
				base.HResult = value;
			}
		}
	}

	public class CSocketProServerException : CClientSocketException
	{
		public CSocketProServerException(int hr, string strMessage, int nSvsID, short sRequestID) : base(hr, strMessage)
		{
			m_nSvsID = nSvsID;
			m_sRequestID = sRequestID;
		}

		public CSocketProServerException(int hr, string strMessage) : base(hr, strMessage)
		{
			m_nSvsID = 0;
			m_sRequestID = 0;
		}
		public int		m_nSvsID;
		public short	m_sRequestID;
		internal const int TRANSFER_SERVER_EXCEPTION = (0x40000000);
		//#define E_FAIL                           _HRESULT_TYPEDEF_(0x80000008L)
		internal const int E_FAIL = (-0x7FFFFFF7);
	};

    public struct FILETIME
    {
        uint dwLowDateTime;
        uint dwHighDateTime;
    };

	public class CUQueue
	{
		[ StructLayout( LayoutKind.Sequential )]
		internal struct SystemTime 
		{
			public ushort wYear; 
			public ushort wMonth; 
			public ushort wDayOfWeek; 
			public ushort wDay; 
			public ushort wHour; 
			public ushort wMinute; 
			public ushort wSecond; 
			public ushort wMilliseconds;
		}
		
		[DllImport("oleaut32.dll")]
		private static extern int SystemTimeToVariantTime(out SystemTime sysTime, out double dTime);
		[DllImport("oleaut32.dll")]
		private static extern int VariantTimeToSystemTime(double dTime, out SystemTime sysTime);
		
		public void CleanTrack()
		{
			//????
		}

		public void Push(CSocketProServerException SocketProServerException)
		{
			if(SocketProServerException == null)
				throw new Exception("Can't serialize a null CSocketProServerException.");
			Push(SocketProServerException.HResult);
			if(SocketProServerException.HResult != 0)
			{
				Push(SocketProServerException.m_nSvsID);
				Push(SocketProServerException.m_sRequestID);
				Save(SocketProServerException.Message);
			}
		}

		public int Pop(out CSocketProServerException SocketProServerException)
		{
			int hr = 0;
			int nSvsID = 0;
			short sReqID = 0;
			string strMessage = null;
			int nGet = Pop(out hr);
			if(hr != 0)
			{
				nGet += Pop(out nSvsID);
				nGet += Pop(out sReqID);
				nGet += Load(out strMessage);
			}
			SocketProServerException = new CSocketProServerException(hr, strMessage, nSvsID, sReqID);
			return nGet;
		}

		public void Push(CUQueue UQqueue)
		{
			int nSize = -1;
			if(UQqueue != null)
			{
				nSize = UQqueue.GetSize();
				Push(nSize);
				Push(UQqueue.GetBuffer(), nSize);
			}
			else
			{
				Push(nSize);
			}
		}
		
		public int Discard(int nLen)
		{
			if(nLen <= 0)
				return 0;
			if(nLen > GetSize())
				nLen = GetSize();
			byte[]Buffer = new byte[nLen];
			return Pop(out Buffer, nLen);
		}

		public int Pop(out CUQueue UQueue)
		{
			int nSize = 0;
			int nGet = Pop(out nSize);
			if(nSize > 0)
			{
				if(nSize > GetSize())
				{
					SetSize(0);
					throw new Exception("Data de-serialization wrong");
				}
				UQueue = new CUQueue();
				byte[]Buffer = new byte[nSize];
				nGet += Pop(out Buffer, nSize);
				UQueue.Push(Buffer, nSize);
			}
            else
                UQueue = null;
			return nGet;
		}

		public CUQueue(int nInitSize)
		{
			m_bPushing = false;
			m_ms = new MemoryStream(nInitSize);
			m_bytes = new Byte[64];
		}
		public CUQueue()
		{
			m_bPushing = false;
			m_ms = new MemoryStream();
			m_bytes = new Byte[64];
		}
		public byte[] GetBuffer()
		{
			AdjustData();
//			m_ms.Capacity = (int)GetSize();
			return m_ms.GetBuffer();
		}
		
		public int GetSize()
		{
			if(m_bPushing)
				return (int)(m_ms.Length);
			return (int)(m_ms.Length - m_ms.Position);
		}
		public void SetSize(int nSize)
		{
			AdjustData();
			m_ms.SetLength(nSize);
		}
		public void Empty()
		{
			m_ms.SetLength(0);
			m_ms.Position = 0;
		}
		public void Push(bool bData)
		{
			byte []bytes = BitConverter.GetBytes(bData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 1);
			m_bPushing = true;
		}
		public void Push(byte bData)
		{
			byte []bytes = BitConverter.GetBytes(bData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 1);
			m_bPushing = true;
		}
		public void Push(sbyte sbData)
		{
			byte []bytes = BitConverter.GetBytes(sbData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 1);
			m_bPushing = true;
		}

        public int XmlDeserialize<T>(out T obj)
        {
            int lDataLen = 0;
            bool bNull = false;
            if (m_bPushing)
            {
                m_ms.Seek(0, SeekOrigin.Begin);
                m_bPushing = false;
            }
            if (GetSize() < sizeof(bool))
                throw new Exception("No valid data available");
            uint nLen = (uint)GetSize();
            Pop(out bNull);
            if (m_ms.Position == m_ms.Length)
                Empty();
            if (GetSize() < sizeof(int))
                throw new Exception("No valid data available");
            Pop(out lDataLen);
            if (m_ms.Position == m_ms.Length)
                Empty();
            if (GetSize() < lDataLen)
                throw new Exception("No valid data available");
            XmlSerializer xml = new XmlSerializer(typeof(T));
            if (m_temp == null)
                m_temp = new MemoryStream();
            else
            {
                m_temp.Position = 0;
                m_temp.SetLength(0);
            }
            byte[] bytes = null;
            Pop(out bytes, lDataLen);
            if (m_ms.Position == m_ms.Length)
                Empty();
            m_temp.Write(bytes, 0, lDataLen);
            m_temp.Position = 0;
            obj = (T)xml.Deserialize(m_temp);
            return (int)(nLen - (uint)GetSize());
        }

        public void XmlSerialize<T>(T obj)
        {
            bool bNull = (obj == null);
            Push(bNull);
            if (bNull)
                return;
            XmlSerializer xml = new XmlSerializer(typeof(T));
            m_ms.Seek(0, SeekOrigin.End);
            if (m_temp == null)
                m_temp = new MemoryStream();
            else
            {
                m_temp.Position = 0;
                m_temp.SetLength(0);
            }
            xml.Serialize(m_temp, obj);
            int nLen = (int)m_temp.Length;
            Push(nLen);
            m_ms.Seek(0, SeekOrigin.End);
            m_temp.Position = 0;
            m_ms.Write(m_temp.GetBuffer(), 0, nLen);
            m_bPushing = true;
        }
		public void Push(short sData)
		{
			byte []bytes = BitConverter.GetBytes(sData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 2);
			m_bPushing = true;
		}
		public void Push(ushort usData)
		{
			byte []bytes = BitConverter.GetBytes(usData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 2);
			m_bPushing = true;
		}
		public void Push(int nData)
		{
			byte []bytes = BitConverter.GetBytes(nData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 4);
			m_bPushing = true;
		}
		public void Push(uint unData)
		{
			byte []bytes = BitConverter.GetBytes(unData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 4);
			m_bPushing = true;
		}
		unsafe public void Push(ulong ulData)
		{
			byte *pData = (byte*)&ulData;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(ulong));
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(m_bytes, 0, sizeof(ulong));
			m_bPushing = true;
		}
		unsafe public void Push(long lData)
		{
			byte *pData = (byte*)&lData;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(long));
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(m_bytes, 0, sizeof(long));
			m_bPushing = true;
		}
		unsafe public void Push(double dData)
		{
			byte *pData = (byte*)&dData;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(double));
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(m_bytes, 0, sizeof(double));
			m_bPushing = true;
		}
		public void Push(float fData)
		{
			byte []bytes = BitConverter.GetBytes(fData);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 4);
			m_bPushing = true;
		}
		public void Push(char wChar)
		{
			byte []bytes = BitConverter.GetBytes(wChar);
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, 2);
			m_bPushing = true;
		}
		public void Push(string strData)
		{
			byte []bytes = null;
			if(strData != null && strData.Length>0)
				bytes = System.Text.Encoding.Unicode.GetBytes(strData);
			else
				return;
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(bytes, 0, bytes.Length);
			m_bPushing = true;
		}
		public void Push(byte []Bytes, int nLen)
		{
			if(Bytes == null || nLen == 0)
				return;
			if(nLen>Bytes.Length)
				nLen = Bytes.Length;
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(Bytes, 0, nLen);
			m_bPushing = true;
		}
		public void Push(byte []Bytes)
		{
			if(Bytes == null)
				return;
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			m_ms.Write(Bytes, 0, Bytes.Length);
			m_bPushing = true;
		}
		
		unsafe public void PushSystemTime(DateTime dt)
		{
			SystemTime st = new SystemTime();
			st.wYear = (ushort)dt.Year;
			st.wMonth = (ushort)dt.Month;
			st.wDayOfWeek = (ushort)dt.DayOfWeek;
			st.wDay = (ushort)dt.Day;
			st.wHour = (ushort)dt.Hour;
			st.wMinute = (ushort)dt.Minute;
			st.wSecond = (ushort)dt.Second;
			st.wMilliseconds = (ushort)dt.Millisecond;
			byte *pData = (byte*)&st;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(SystemTime));
			Push(m_bytes, sizeof(SystemTime));
		}

		//convert dotNet bool into variant bool
		public void PushVariantBool(bool b)
		{
			ushort bV = (ushort)(b ? 0xFFFF : 0x0000);
			Push(bV);
		}

		//convert dotNet DateTime into native variant time
		public void PushVariantDate(DateTime dt)
		{
			double dDT = 0;
			SystemTime st = new SystemTime();
			st.wYear = (ushort)dt.Year;
			st.wMonth = (ushort)dt.Month;
			st.wDayOfWeek = (ushort)dt.DayOfWeek;
			st.wDay = (ushort)dt.Day;
			st.wHour = (ushort)dt.Hour;
			st.wMinute = (ushort)dt.Minute;
			st.wSecond = (ushort)dt.Second;
			st.wMilliseconds = (ushort)dt.Millisecond;
			SystemTimeToVariantTime(out st, out dDT);
			Push(dDT);
		}
		
		//convert dotNet decimal data into native Currency
		public void PushCY(decimal cyMoney)
		{
			long lMoney = (long)(cyMoney*10000);
			Push(lMoney);
		}

        unsafe public void Push(FILETIME ft)
		{
			byte *pData = (byte*)&ft;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(FILETIME));
			Push(m_bytes, sizeof(FILETIME));
		}

		unsafe public void Push(DateTime dt)
		{
			byte *pData = (byte*)&dt;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(DateTime));
			Push(m_bytes, sizeof(DateTime));
		}

        public void Push(TimeSpan dt)
        {
            long ticks = dt.Ticks;
            Push(ticks);
        }

		unsafe public void Push(Guid guid)
		{
			byte *pData = (byte*)&guid;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(Guid));
			Push(m_bytes, sizeof(Guid));
		}
		unsafe public void Push(decimal decData)
		{
			byte *pData = (byte*)&decData;
			Marshal.Copy((System.IntPtr)pData, m_bytes, 0, sizeof(decimal));
			Push(m_bytes, sizeof(decimal));
		}
		
		public void Push(IUSerializer USerializer)
		{
            bool bNull = (USerializer == null);
            Push(bNull);
			if(!bNull)
			{
				USerializer.SaveTo(this);
			}
		}

        public void Save<T>(T data)
        {
			if(typeof(T) ==  typeof(string))
			{
                string str = data as string;
				Save(str);
			}
            else if (data is int)
            {
                object obj = data;
                Push((int)obj);
            }
            else if (data is uint)
            {
                object obj = data;
                Push((uint)obj);
            }
            else if (data is float)
            {
                object obj = data;
                Push((float)obj);
            }
            else if (data is double)
            {
                object obj = data;
                Push((double)obj);
            }
            else if (data is long)
            {
                object obj = data;
                Push((long)obj);
            }
            else if (data is bool)
            {
                object obj = data;
                Push((bool)obj);
            }
            else if (data is DateTime)
            {
                object obj = data;
                Push((DateTime)obj);
            }
            else if (data is TimeSpan)
            {
                object obj = data;
                Push((TimeSpan)obj);
            }
            else if (data is ulong)
            {
                object obj = data;
                Push((ulong)obj);
            }
            else if (data is Guid)
            {
                object g = data;
                Push((Guid)g);
            }
            else if (data is decimal)
            {
                object d = data;
                Push((decimal)d);
            }
            else if (typeof(T) == typeof(object))
            {
                Push((object)data);
            }
            else if (data is short)
            {
                object obj = data;
                Push((short)obj);
            }
            else if (data is ushort)
            {
                object obj = data;
                Push((ushort)obj);
            }
            else if (data is char)
            {
                object obj = data;
                Push((char)obj);
            }
            else if (data is sbyte)
            {
                object obj = data;
                Push((sbyte)obj);
            }
            else if (data is byte)
            {
                object obj = data;
                Push((byte)obj);
            }
            else if (typeof(T) == typeof(CUQueue))
			{
                CUQueue q = data as CUQueue;
				Push(q);
			}
            else if (typeof(T) == typeof(CSocketProServerException))
			{
				Push(data as CSocketProServerException);
			}
            else if (hasUSerializer(typeof(T)))
            {
                IUSerializer serializer = data as IUSerializer;
                Push(serializer);
            }
            else if (data is FILETIME)
			{
                object obj = data;
				Push((FILETIME)obj);
			}
            else 
                throw new System.InvalidOperationException("Serialization not supported for data type " + typeof(T).FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer.");
        }

		public void Save(string strData)
		{
			int nLen;
			if(strData == null)
			{
				nLen = -1;
				Push(nLen);
			}
			else
			{
				nLen = strData.Length*2;
				Push(nLen);
				Push(strData);
			}
		}
/*
		public void Serialize(object obj)
		{
			Serialize(obj, false, null);
		}
		public void Serialize(object obj, bool bSoapFormat)
		{
			Serialize(obj, bSoapFormat, null);
		}
		public void Serialize(object obj, bool bSoapFormat, IFormatter formatter)
		{
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			if(formatter == null)
			{
				if(bSoapFormat)
				{
					formatter = new SoapFormatter();
				}
				else
				{
					formatter = new BinaryFormatter();
				}
			}
			formatter.Serialize(m_ms, obj);
			m_bPushing = true;
		}*/
		public void Push(object obData)
		{
			//Beginning with SocketPro version 4.3.0.1, 
			//convert dotNet DateTime into native variant date
			Push(obData, true, false);

			//old 
			//Push(obData, false, false);
		}
		public void Push(object obData, bool bToNative)
		{
			Push(obData, bToNative, false);
		}
		public void Push(object obData, bool bToNative, bool bDecToCY)
		{
			ushort vt = 0;
			AdjustData();
			m_ms.Seek(0, SeekOrigin.End);
			if(obData == null)
			{
				vt = (ushort)tagSockDataType.sdVT_EMPTY;
				Push(vt);
				return;
			}
            else if (obData == DBNull.Value)
            {
                vt = (ushort)tagSockDataType.sdVT_NULL;
                Push(vt);
                return;
            }
            else if (obData is IUSerializer)
            {
                vt = (ushort)tagSockDataType.sdVT_USERIALIZER_OBJECT;
                Push(vt);
                Push((IUSerializer)obData);
                return;
            }
            else if (obData is TimeSpan)
            {
                vt = (ushort)tagSockDataType.sdVT_TIMESPAN;
                Push(vt);
                Push(((TimeSpan)obData).Ticks);
                return;
            }
			if(obData.GetType().IsArray)
			{
				switch(obData.GetType().FullName)
				{
					case "System.String[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_BSTR|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						string []strArray = (string[])obData;
						uint nSize = (uint)strArray.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							string str = strArray[nIndex];
							if(str == null)
							{
								uint unLen = 0xFFFFFFFF;
								Push(unLen);
							}
							else
							{
								uint unLen = (uint)(str.Length*2);
								Push(unLen);
								Push(str);
							}
						}
					}
						break;
					case "SocketProAdapter.FILETIME[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_FILETIME|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						FILETIME []aF = (FILETIME[])obData;
						uint nSize = (uint)aF.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							Push(aF[nIndex]);
						}
					}
						break;
					case "System.Single[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_R4|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						float []aF = (float[])obData;
						uint nSize = (uint)aF.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							Push(aF[nIndex]);
						}
					}
						break;
					case "System.DateTime[]":
					{
						int nIndex;
						vt = ((ushort)tagSockDataType.sdVT_DATE|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						DateTime []aDT = (DateTime[])obData;
						uint nSize = (uint)aDT.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							if(bToNative)
							{
								PushVariantDate(aDT[nIndex]);
							}
							else
							{
								Push(aDT[nIndex]);
							}
						}
					}
						break;
					case "System.Double[]":
					{
						int nIndex;
						vt = ((ushort)tagSockDataType.sdVT_R8|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						double []aD = (double[])obData;
						uint nSize = (uint)aD.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							Push(aD[nIndex]);
						}
					}
						break;
					case "System.Decimal[]":
					{
						int nIndex;
						if(bDecToCY && bToNative)
						{
							vt = ((ushort)tagSockDataType.sdVT_CY|(ushort)tagSockDataType.sdVT_ARRAY);
						}
						else
						{
							vt = ((ushort)tagSockDataType.sdVT_DECIMAL|(ushort)tagSockDataType.sdVT_ARRAY);
						}
						Push(vt);
						decimal []aD = (decimal[])obData;
						uint nSize = (uint)aD.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							if(bDecToCY && bToNative)
							{
								PushCY(aD[nIndex]);
							}
							else
							{
								Push(aD[nIndex]);
							}
						}
					}
						break;
					case "System.Boolean[]":
					{
						int nIndex;
						vt = ((ushort)tagSockDataType.sdVT_BOOL|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						bool []bData = (bool[])obData;
						uint nSize = (uint)bData.Length;
						Push(nSize);
						for(nIndex=0; nIndex<nSize; nIndex++)
						{
							ushort bBool = bData[nIndex] ? (ushort)0xFFFF : (ushort)0x0000;
							Push(bBool);
						}
					}
						break;
					case "System.Byte[]":
					{
						vt = ((ushort)tagSockDataType.sdVT_UI1|(ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						byte []bytes = (byte[])obData;
						uint nSize = (uint)bytes.Length;
						Push(nSize);
						Push(bytes);
					}
						break;
					case "System.SByte[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_I1 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						sbyte []sbytes = (sbyte[])obData;
						uint nSize = (uint)sbytes.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(sbytes[nIndex]);
						}
					}
						break;
					case "System.UInt64[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_UI8 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						ulong []ulData = (ulong[])obData;
						uint nSize = (uint)ulData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(ulData[nIndex]);
						}
					}
						break;
					case "System.Int64[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_I8 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						long []lData = (long[])obData;
						uint nSize = (uint)lData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(lData[nIndex]);
						}
					}
						break;
					case "System.UInt32[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_UI4 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						uint []unData = (uint[])obData;
						uint nSize = (uint)unData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(unData[nIndex]);
						}
					}
						break;
					case "System.Int32[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_I4 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						int []nData = (int[])obData;
						uint nSize = (uint)nData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(nData[nIndex]);
						}
					}
						break;
					case "System.Char[]":
                    {
                        uint nIndex;
                        vt = ((ushort)tagSockDataType.sdVT_UI2 | (ushort)tagSockDataType.sdVT_ARRAY);
                        Push(vt);
                        char[] usData = (char[])obData;
                        uint nSize = (uint)usData.Length;
                        Push(nSize);
                        for (nIndex = 0; nIndex < nSize; nIndex++)
                        {
                            Push(usData[nIndex]);
                        }
                    }
						break;
					case "System.UInt16[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_UI2 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						ushort []usData = (ushort[])obData;
						uint nSize = (uint)usData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(usData[nIndex]);
						}
					}
						break;
					case "System.Int16[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_I2 | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						short []sData = (short[])obData;
						uint nSize = (uint)sData.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(sData[nIndex]);
						}
					}
						break;
					case "System.Guid[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_CLSID | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						Guid []guidA = (Guid[])obData;
						uint nSize = (uint)guidA.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(guidA[nIndex]);
						}
					}
						break;
                    case "System.DBNull[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagSockDataType.sdVT_NULL | (ushort)tagSockDataType.sdVT_ARRAY);
                            Push(vt);
                            DBNull[] dbNulls = (DBNull[])obData;
                            uint nSize = (uint)dbNulls.Length;
                            Push(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Push((short)tagSockDataType.sdVT_NULL);
                            }
                        }
                        break;
                    case "System.TimeSpan[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagSockDataType.sdVT_TIMESPAN | (ushort)tagSockDataType.sdVT_ARRAY);
                            Push(vt);
                            TimeSpan[] dbNulls = (TimeSpan[])obData;
                            uint nSize = (uint)dbNulls.Length;
                            Push(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Push(dbNulls[nIndex].Ticks);
                            }
                        }
                        break;
					case "System.Object[]":
					{
						uint nIndex;
						vt = ((ushort)tagSockDataType.sdVT_VARIANT | (ushort)tagSockDataType.sdVT_ARRAY);
						Push(vt);
						object []obA = (object[])obData;
						uint nSize = (uint)obA.Length;
						Push(nSize);
						for(nIndex = 0; nIndex<nSize; nIndex++)
						{
							Push(obA[nIndex], bToNative, bDecToCY);
						}
					}
						break;
					default:
						break;
				}
			}
			else
			{
				switch(obData.GetType().FullName)
				{
					case "System.DBNull":
						vt = (ushort)tagSockDataType.sdVT_EMPTY;
						Push(vt);
						break;
					case "System.String":
					{
						vt = (ushort)tagSockDataType.sdVT_BSTR;
						Push(vt);
						string str = (string)obData;
						if(str == null)
						{
							uint unLen = 0xFFFFFFFF;
							Push(unLen);
						}
						else
						{
							uint unLen = (uint)(str.Length*2);
							Push(unLen);
							Push(str);
						}
					}
						break;
					case "System.Single":
						vt = (ushort)tagSockDataType.sdVT_R4;
						Push(vt);
						Push((float)obData);
						break;
                    case "SocketProAdapter.FILETIME":
						vt = (ushort)tagSockDataType.sdVT_FILETIME;
						Push(vt);
						Push((FILETIME)obData);
						break;
					case "System.DateTime":
						vt = (ushort)tagSockDataType.sdVT_DATE;
						Push(vt);
						if(bToNative)
						{
							PushVariantDate((DateTime)obData);
						}
						else
						{
							Push((DateTime)obData);
						}
						break;
					case "System.Double":
						vt = (ushort)tagSockDataType.sdVT_R8;
						Push(vt);
						Push((double)obData);
						break;
					case "System.Decimal":
						if(bDecToCY && bToNative)
						{
							vt = (ushort)tagSockDataType.sdVT_CY;
							Push(vt);
							PushCY((decimal)obData);
						}
						else
						{
							vt = (ushort)tagSockDataType.sdVT_DECIMAL;
							Push(vt);
							Push((decimal)obData);
						}
						break;
					case "System.Boolean":
						vt = (ushort)tagSockDataType.sdVT_BOOL;
						Push(vt);
					{
						bool bData = (bool)obData;
						ushort bBool = bData ? (ushort)0xFFFF : (ushort)0x0000;
						Push(bBool);
					}
						break;
					case "System.Byte":
						vt = (ushort)tagSockDataType.sdVT_UI1;
						Push(vt);
						Push((byte)obData);
						break;
					case "System.SByte":
						vt = (ushort)tagSockDataType.sdVT_I1;
						Push(vt);
						Push((sbyte)obData);
						break;
					case "System.UInt64":
						vt = (ushort)tagSockDataType.sdVT_UI8;
						Push(vt);
						Push((ulong)obData);
						break;
					case "System.Int64":
						vt = (ushort)tagSockDataType.sdVT_I8;
						Push(vt);
						Push((long)obData);
						break;
					case "System.UInt32":
						vt = (ushort)tagSockDataType.sdVT_UI4;
						Push(vt);
						Push((uint)obData);
						break;
					case "System.Int32":
						vt = (ushort)tagSockDataType.sdVT_I4;
						Push(vt);
						Push((int)obData);
						break;
					case "System.Guid":
						vt = (ushort)tagSockDataType.sdVT_CLSID;
						Push(vt);
						Push((Guid)obData);
						break;
					case "System.Char":
                        vt = (ushort)tagSockDataType.sdVT_UI2;
                        Push(vt);
                        Push((char)obData);
                        break;
					case "System.UInt16":
						vt = (ushort)tagSockDataType.sdVT_UI2;
						Push(vt);
						Push((ushort)obData);
						break;
					case "System.Int16":
						vt = (ushort)tagSockDataType.sdVT_I2;
						Push(vt);
						Push((short)obData);
						break;
					default:
                        throw new Exception("Unsupported serialization");
//						Serialize(obData);
//						break;
				}
			}
			m_bPushing = true;
		}
		public int Pop(out byte []Bytes, int nLen)
		{
			if(nLen <= 0 || GetSize() == 0)
			{
                Bytes = null;
				return 0;
			}
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(nLen > GetSize())
				nLen = GetSize();
			
			Bytes = new byte[nLen];

			m_ms.Read(Bytes, 0, (int)nLen);
			if(m_ms.Position == m_ms.Length)
				Empty();
			return nLen;
		}

		public int Pop(out byte []Bytes)
		{
			if(GetSize() == 0)
			{
                Bytes = null;
				return 0;
			}
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nBytes;
			nBytes = GetSize();
			Bytes = new byte[nBytes];
			m_ms.Read(Bytes, 0, (int)nBytes);
			if(m_ms.Position == m_ms.Length)
				Empty();
			return nBytes;
		}
		
		public int Pop(out byte bByte)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 1);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==1)
			{
				bByte = m_bytes[0];
				return 1;
			}
            else
                bByte = 0;
			return 0;
		}

		public int Pop(out string strData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nBytes = ((int)m_ms.Length/2)*2;
			if(nBytes > m_bytes.Length)
				m_bytes = new Byte[nBytes];
			m_ms.Read(m_bytes, 0, nBytes);
			if(m_ms.Position == m_ms.Length)
				Empty();
			strData = System.Text.Encoding.Unicode.GetString(m_bytes, 0, nBytes);
			return nBytes;
		}
		public int Pop(out string strData, int nBytes)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(nBytes>m_ms.Length)
			{
				nBytes = (int)m_ms.Length;
				nBytes = (nBytes/2)*2;
			}
			if(nBytes > m_bytes.Length)
				m_bytes = new Byte[nBytes];
			m_ms.Read(m_bytes, 0, nBytes);
			if(m_ms.Position == m_ms.Length)
				Empty();
			strData = System.Text.Encoding.Unicode.GetString(m_bytes, 0, nBytes);
			return nBytes;
		}

        private bool hasUSerializer(Type type)
        {
            Type[] types = type.GetInterfaces();
            foreach (Type t in types)
            {
                if (t.FullName == "SocketProAdapter.IUSerializer")
                    return true;
            }
            return false;
        }

        public int Load<T>(out T data)
        {
            data = default(T);
            int nStart = GetSize();
            if (typeof(T) == typeof(string))
            {
                string str;
                Load(out str);
                data = (T)((object) str);
            }
            else if (data is int)
            {
                int n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is uint)
            {
                uint n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is bool)
            {
                bool n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is short)
            {
                short n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is ushort)
            {
                ushort n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is float)
            {
                float n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is double)
            {
                double n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is long)
            {
                long n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is char)
            {
                char n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is sbyte)
            {
                sbyte n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is byte)
            {
                byte n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is ulong)
            {
                ulong n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is DateTime)
            {
                DateTime n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is TimeSpan)
            {
                TimeSpan ts;
                Pop(out ts);
                data = (T)((object)ts);
            }
            else if (typeof(T) == typeof(object))
            {
                object n;
                Pop(out n);
                data = (T)(n);
            }
            else if (data is Guid)
            {
                Guid n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is decimal)
            {
                decimal n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (typeof(T) == typeof(CUQueue))
            {
                CUQueue n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (typeof(T) == typeof(CSocketProServerException))
            {
                CSocketProServerException n = null;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (data is FILETIME)
            {
                FILETIME n;
                Pop(out n);
                data = (T)((object)n);
            }
            else if (hasUSerializer(typeof(T)))
            {
                bool bNull;
                Pop(out bNull);
                if (!bNull)
                {
                    IUSerializer uobj = (IUSerializer)(System.Activator.CreateInstance(typeof(T)));
                    uobj.LoadFrom(this);
                    data = (T)((object)uobj);
                }
            }
            else
                throw new System.InvalidOperationException("Serialization not supported for data type " + typeof(T).FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer.");
            return (nStart - GetSize());
        }

		public int Load(out string str)
		{
			uint unLen = 0;
			int nGet = Pop(out unLen);
			if(nGet < 4)
				throw new Exception("No valid data available");
			if(unLen != 0xFFFFFFFF && unLen > GetSize())
			{
				SetSize(0);
				throw new Exception("Invalid data found");
			}

			if(unLen != 0xFFFFFFFF)
			{
				Pop(out str, (int)unLen);
			}
			else
			{
                str = null;
				return nGet;
			}
			return ((int)unLen + 4);
		}
		
        
		public int Pop<T>(out T USerializer) where T : IUSerializer, new()
		{
            bool bNull = false;
            USerializer = default(T);
            int len = Pop(out bNull);
            if (!bNull)
            {
                if (USerializer == null)
                    USerializer = new T();
                len += USerializer.LoadFrom(this);
            }
                
            return len;
		}

/*		public int Deserialize(out object obj)
		{
			return Deserialize(out obj, false, null);
		}
		public int Deserialize(out object obj, bool bSoapFormat)
		{
			return Deserialize(out obj, bSoapFormat, null);
		}
		public int Deserialize(out object obj, bool bSoapFormat, IFormatter formatter)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			else if(m_ms.Position != 0)
			{
				AdjustData();
			}
			long lStart = m_ms.Length;
			if(formatter == null)
			{
				if(bSoapFormat)
				{
					formatter = new SoapFormatter();
				}
				else
				{
					formatter = new BinaryFormatter();
				}
			}
			obj = formatter.Deserialize(m_ms);
			if(m_ms.Position == m_ms.Length)
				Empty();
			else
			{
				AdjustData();
			}
			return (int)(lStart - m_ms.Length);
		}*/
		
		public int Pop(out object obData)
		{
			ushort uArray = (ushort)tagSockDataType.sdVT_ARRAY;
			ushort vt = 0;
			
			int lLen = Pop(out vt);
            if (vt == (ushort)tagSockDataType.sdVT_EMPTY)
            {
                obData = null;
                return lLen;
            }
            else if (vt == (ushort)tagSockDataType.sdVT_NULL)
            {
                obData = DBNull.Value;
                return lLen;
            }
			uArray = (ushort)(vt & uArray);
			if ( uArray == (ushort)tagSockDataType.sdVT_ARRAY)
			{
				uint unSize = 0;
				uint un;
				ushort usArray = (ushort)tagSockDataType.sdVT_ARRAY;
				vt = (ushort)(vt &~ usArray);
				lLen += Pop(out unSize);
				switch((tagSockDataType)vt)
				{
					case tagSockDataType.sdVT_BOOL:
					{
						ushort vtBool = 0;
						bool []aBools = new bool[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out vtBool);
							aBools[un] = (vtBool == 0) ? false : true;
						}
						obData = aBools;
					}
						break;
					case tagSockDataType.sdVT_DECIMAL:
					{
						decimal []aDecimals = new decimal[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aDecimals[un]);
						}
						obData = aDecimals;
					}
						break;
					case tagSockDataType.sdVT_I1:
					{
						sbyte []aSbytes = new sbyte[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aSbytes[un]);
						}
						obData = aSbytes;
					}
						break;
					case tagSockDataType.sdVT_I2:
					{
						short []aShorts = new short[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aShorts[un]);
						}
						obData = aShorts;
					}
						break;
					case tagSockDataType.sdVT_I4:
					{
						int []aInts = new int[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aInts[un]);
						}
						obData = aInts;
					}
						break;
					case tagSockDataType.sdVT_I8:
					{
						long []aLongs = new long[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aLongs[un]);
						}
						obData = aLongs;
					}
						break;
					case tagSockDataType.sdVT_R4:
					{
						float []aFloats = new float[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aFloats[un]);
						}
						obData = aFloats;
					}
						break;
					case tagSockDataType.sdVT_DATE:
					{
						DateTime []aDateTimes = new DateTime[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += PopVariantDate(out aDateTimes[un]);
						}
						obData = aDateTimes;
					}
						break;
					case tagSockDataType.sdVT_R8:
					{
						double []aDoubles = new double[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aDoubles[un]);
						}
						obData = aDoubles;
					}
						break;
					case tagSockDataType.sdVT_UI1:
					{
						byte []bytes = null;
						lLen += Pop(out bytes, (int)unSize);
						obData = bytes;
					}
						break;
					case tagSockDataType.sdVT_UI2:
					{
						ushort []aUshorts = new ushort[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aUshorts[un]);
						}
						obData = aUshorts;
					}
						break;
					case tagSockDataType.sdVT_FILETIME:
					{
						FILETIME []aFileTimes = new FILETIME[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aFileTimes[un]);
						}
						obData = aFileTimes;
					}
						break;
					case tagSockDataType.sdVT_UI4:
					{
						uint []aUints = new uint[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aUints[un]);
						}
						obData = aUints;
					}
						break;
					case tagSockDataType.sdVT_UI8:
					{
						ulong []aULongs = new ulong[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aULongs[un]);
						}
						obData = aULongs;
					}
						break;
					case tagSockDataType.sdVT_VARIANT:
					{
						object []aObjects = new object[unSize];
						for(un = 0; un<unSize; un++)
						{
						{
							object obj = null;
							lLen += Pop(out obj);
							aObjects[un] = obj;
						}
						}
						obData = aObjects;
					}
						break;
					case tagSockDataType.sdVT_CY:
					{
						long lCY = 0;
						decimal []aDecimals = new decimal[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out lCY);
							aDecimals[un] = lCY;
							aDecimals[un] /= 10000;
						}
						obData = aDecimals;
					}
						break;
					case tagSockDataType.sdVT_CLSID:
					{
						Guid []aGuids = new Guid[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out aGuids[un]);
						}
						obData = aGuids;
					}
						break;
                    case tagSockDataType.sdVT_TIMESPAN:
                        {
                            long ticks;
                            TimeSpan[] aGuids = new TimeSpan[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                lLen += Pop(out ticks);
                                aGuids[un] = new TimeSpan(ticks);
                            }
                            obData = aGuids;
                        }
                        break;
					case tagSockDataType.sdVT_BSTR:
					{
						uint uLen = 0;
						string []aStr = new string[unSize];
						for(un = 0; un<unSize; un++)
						{
							lLen += Pop(out uLen);
							if(uLen != 0xFFFFFFFF)
							{
								lLen += Pop(out aStr[un], (int)uLen);
							}
						}
						obData = aStr;
					}
						break;
					default:
                        throw new Exception("Unsupported serialization");
//						Deserialize(out obData);
//						break;
				}
			}
			else
			{							  
				switch((tagSockDataType)vt)
				{
					case tagSockDataType.sdVT_EMPTY:
						obData = null;
						break;
					case tagSockDataType.sdVT_BSTR:
					{
						uint unSize = 0;
						lLen += Pop(out unSize);
						if(lLen == 0)
							obData = "";
						else
						{
	
							string str = null;
							lLen += Pop(out str, (int)unSize);
							obData = str;
						}
					}
						break;
					case tagSockDataType.sdVT_BOOL:
					{
						ushort vtBool = 0;
						lLen += Pop(out vtBool);
						if(vtBool > 0)
							obData = true;
						else
							obData = false;
					}
						break;
					case tagSockDataType.sdVT_FILETIME:
					{
						FILETIME ft = new FILETIME();
						lLen += Pop(out ft);
						obData = ft;
					}
						break;
					case tagSockDataType.sdVT_CY:
					{
						long lCY = 0;
						decimal decData;
						lLen += Pop(out lCY);
						decData = lCY;
						decData /= 10000;
						obData = decData;
					}
						break;
					case tagSockDataType.sdVT_DECIMAL:
					{
						decimal decData = 0;
						lLen += Pop(out decData);
						obData = decData;
					}
						break;
					case tagSockDataType.sdVT_I1:
					{
						sbyte sData = 0;
						lLen += Pop(out sData);
						obData = sData;
					}
						break;
					case tagSockDataType.sdVT_I2:
					{
						short sData = 0;
						lLen += Pop(out sData);
						obData = sData;
					}
						break;
					case tagSockDataType.sdVT_I4:
					{
						int nData = 0;
						lLen += Pop(out nData);
						obData = nData;
					}
						break;
					case tagSockDataType.sdVT_I8:
					{
						long lData = 0;
						lLen += Pop(out lData);
						obData = lData;
					}
						break;
					case tagSockDataType.sdVT_R4:
					{
						float fData = 0;
						lLen += Pop(out fData);
						obData = fData;
					}
						break;
					case tagSockDataType.sdVT_DATE:
					{
						DateTime dt = new DateTime();
						lLen += PopVariantDate(out dt);
						obData = dt;
					}
						break;
					case tagSockDataType.sdVT_R8:
					{
						double dData = 0;
						lLen += Pop(out dData);
						obData = dData;
					}
						break;
					case tagSockDataType.sdVT_UI1:
					{
						byte bData = 0;
						lLen += Pop(out bData);
						obData = bData;
					}
						break;
					case tagSockDataType.sdVT_UI2:
					{
						ushort usData = 0;
						lLen += Pop(out usData);
						obData = usData;
					}
						break;
					case tagSockDataType.sdVT_UI4:
					{
						uint unData = 0;
						lLen += Pop(out unData);
						obData = unData;
					}
						break;
					case tagSockDataType.sdVT_UI8:
					{
						ulong ulData = 0;
						lLen += Pop(out ulData);
						obData = ulData;
					}
						break;
					case tagSockDataType.sdVT_CLSID:
					{
						Guid guid = new Guid();
						lLen += Pop(out guid);
						obData = guid;
					}
						break;
                    case tagSockDataType.sdVT_TIMESPAN:
                        {
                            long ticks;
                            lLen += Pop(out ticks);
                            obData = new TimeSpan(ticks);
                        }
                        break;
					default:
                        throw new Exception("Unsupported serialization");
				}
			}
			return lLen;
		}
		public int Pop(out Guid guid)
		{
			byte []bytes = new byte[16];
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(m_ms.Length<16)
			{
				m_ms.Read(bytes, 0, (int)m_ms.Length);
                guid = default(Guid);
				return 0;
			}
			int nGet = m_ms.Read(bytes, 0, 16);
			if(m_ms.Position == m_ms.Length)
				Empty();
			guid = new Guid(bytes);
			return nGet;
		}
		unsafe public int Pop(out decimal decData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(m_ms.Length<16)
			{
				m_ms.Read(m_bytes, 0, (int)m_ms.Length);
                decData = default(decimal);
				return 0;
			}
			decimal decTemp;
			int nGet = m_ms.Read(m_bytes, 0, 16);
			if(m_ms.Position == m_ms.Length)
				Empty();
			byte *pDec = (byte*)&decTemp;
			Marshal.Copy(m_bytes, 0, (System.IntPtr)pDec, 16);
			decData = decTemp;
			return nGet;
		}
		public int Pop(out bool bData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 1);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==1)
			{
				bData = (m_bytes[0] > 0) ? true : false;
				return 1;
			}
            else
                bData = default(bool);
			return 0;
		}
		public int Pop(out sbyte aChar)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 1);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==1)
			{
				aChar = (sbyte)(m_bytes[0]);
				return 1;
			}
            else
                aChar = default(sbyte);
			return 0;
		}
		public int Pop(out char wChar)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 2);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==2)
			{
				wChar = BitConverter.ToChar(m_bytes, 0);
				return 2;
			}
            else
                wChar = default(char);
			return 0;
		}
		public int Pop(out ushort usData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 2);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==2)
			{
				usData = BitConverter.ToUInt16(m_bytes, 0);
				return 2;
			}
            else
                usData = default(ushort);
			return 0;
		}
		public int Pop(out short sData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 2);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==2)
			{
				sData = BitConverter.ToInt16(m_bytes, 0);
				return 2;
			}
            else
                sData = default(short);
			return 0;
		}
		
		//convert native variant bool into dotnet bool
		public int PopVariantBool(out bool b)
		{
			ushort usB = 0;
			int l = Pop(out usB);
			b = (usB > 0);
			return l;
		}

		//convert native variant date into dotNet DateTime
		public int PopVariantDate(out DateTime dt)
		{
			SystemTime st = new SystemTime();
			double dDT = 0;
			int nGet = Pop(out dDT);
			VariantTimeToSystemTime(dDT, out st);
			dt = new DateTime(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			return nGet;
		}
		
		//dotNet has no data type currency, convert native currency into decimal
		public int PopCY(out decimal cyMoney)
		{
			long lMoney = 0;
			int lGet = Pop(out lMoney);
			cyMoney = lMoney;
			cyMoney /= 10000;
			return lGet;
		}
		
		unsafe public int PopSystemTime(out DateTime dt)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(m_ms.Length<sizeof(SystemTime))
			{
				m_ms.Read(m_bytes, 0, (int)m_ms.Length);
                dt = default(DateTime);
				return 0;
			}
			SystemTime sysTime = new SystemTime();
			int nGet = m_ms.Read(m_bytes, 0, sizeof(SystemTime));
			if(m_ms.Position == m_ms.Length)
				Empty();
			byte *pDec = (byte*)&sysTime;
			Marshal.Copy(m_bytes, 0, (System.IntPtr)pDec, sizeof(FILETIME));
			dt = new DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
			return nGet;
		}

		unsafe public int Pop(out FILETIME ft)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(m_ms.Length<sizeof(FILETIME))
			{
				m_ms.Read(m_bytes, 0, (int)m_ms.Length);
                ft = default(FILETIME);
				return 0;
			}
			FILETIME fTime = new FILETIME();
			int nGet = m_ms.Read(m_bytes, 0, sizeof(FILETIME));
			if(m_ms.Position == m_ms.Length)
				Empty();
			byte *pDec = (byte*)&fTime;
			Marshal.Copy(m_bytes, 0, (System.IntPtr)pDec, sizeof(FILETIME));
			ft = fTime;
			return nGet;
		}

        public int Pop(out TimeSpan ts)
        {
            long ticks;
            int size = Pop(out ticks);
            ts = new TimeSpan(ticks);
            return size;
        }

		unsafe public int Pop(out DateTime dt)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			if(m_ms.Length<sizeof(DateTime))
			{
				m_ms.Read(m_bytes, 0, (int)m_ms.Length);
                dt = default(DateTime);
				return 0;
			}
			DateTime dtTime = new DateTime();
			int nGet = m_ms.Read(m_bytes, 0, sizeof(DateTime));
			if(m_ms.Position == m_ms.Length)
				Empty();
			byte *pDec = (byte*)&dtTime;
			Marshal.Copy(m_bytes, 0, (System.IntPtr)pDec, sizeof(DateTime));
			dt = dtTime;
			return nGet;
		}

		public int Pop(out float fData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 4);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==4)
			{
				fData = BitConverter.ToSingle(m_bytes, 0);
				return 4;
			}
            else
                fData = default(float);
			return 0;
		}
		public int Pop(out int nData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 4);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==4)
			{
				nData = BitConverter.ToInt32(m_bytes, 0);
				return 4;
			}
            else
                nData = default(int);
			return 0;
		}
		public int Pop(out uint unData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 4);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==4)
			{
				unData = BitConverter.ToUInt32(m_bytes, 0);
				return 4;
			}
            else
                unData = default(uint);
			return 0;
		}
		public int Pop(out ulong ulData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 8);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==8)
			{
				ulData = BitConverter.ToUInt64(m_bytes, 0);
				return 8;
			}
            else
                ulData = default(ulong);
			return 0;
		}
		public int Pop(out double dData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 8);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==8)
			{
				dData = BitConverter.ToDouble(m_bytes, 0);
				return 8;
			}
            else
                dData = default(double);
			return 0;
		}
		public int Pop(out long lData)
		{
			if(m_bPushing)
			{
				m_ms.Seek(0, SeekOrigin.Begin);
				m_bPushing = false;
			}
			int nGet = m_ms.Read(m_bytes, 0, 8);
			if(m_ms.Position == m_ms.Length)
				Empty();
			if(nGet==8)
			{
				lData = BitConverter.ToInt64(m_bytes, 0);
				return 8;
			}
            else
                lData = default(long);
			return 0;
		}
		unsafe public void AdjustData()
		{
			if(m_ms.Position > 0)
			{
				if(m_bPushing)
				{
					m_ms.Position = 0;
				}
				else
				{
					if(m_ms.Length == m_ms.Position)
					{
						m_ms.SetLength(0);
						m_ms.Position = 0;
					}
					else
					{
						uint uiLen = (uint)(m_ms.Length - m_ms.Position);
						byte []bytes = new byte[uiLen];
						m_ms.Read(bytes, 0, (int)uiLen);
						m_ms.SetLength(0);
						m_ms.Write(bytes, 0, (int)uiLen);
						m_ms.Position = 0;
					}
				}
			}
		}
        public int Size 
        {
            get
            {
                return GetSize();
            }
        }

        public int HeadPosition 
        {
            get
            {
                return (int)m_ms.Position;
            }
        }
        public int BufferSize 
        {
            get
            {
                int nLen = m_ms.Capacity;
                if (m_bytes != null)
                    nLen += m_bytes.Length;
                if (m_temp != null)
                    nLen += m_temp.Capacity;
                return nLen;
            }
        }

		private MemoryStream	m_ms;
		private bool	m_bPushing;
		private byte[]	m_bytes;
        private MemoryStream m_temp;
	}
}
