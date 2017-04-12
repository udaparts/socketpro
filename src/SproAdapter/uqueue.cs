using System;
using System.IO;
#if WINCE
#else
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.Serialization.Formatters.Binary;
using System.Security;
#endif
using System.Runtime.Serialization;
using System.Runtime.InteropServices;
using System.Collections.Generic;

namespace SocketProAdapter
{
    /// <summary>
    /// database datetime with accuracy micro-second
    /// </summary>
    class UDateTime
    {
        const uint MICRO_SECONDS = 0xfffff; //20 bits
        /// <summary>
        /// database datetime with accuracy micro-second
        /// </summary>
        private ulong m_time;

        public ulong time
        {
            get
            {
                return m_time;
            }
        }

        public DateTime DateTime
        {
            get
            {
                return GetDateTime();
            }
        }

        private DateTime GetDateTime()
        {
            ulong dt = m_time;
            uint us = (uint)(dt & MICRO_SECONDS);
            dt >>= 20;
            int sec = (int)(dt & 0x3f);
            dt >>= 6;
            int min = (int)(dt & 0x3f);
            dt >>= 6;
            int hour = (int)(dt & 0x1f);
            dt >>= 5;
            int day = (int)(dt & 0x1f);
            dt >>= 5;
            int mon = (int)(dt & 0xf);
            dt >>= 4;
            int year = (int)dt;
            DateTime datetime;
            if (day == 0)
            {
                datetime = new DateTime(1, 1, 1, hour, min, sec, (int)(us / 1000));
            }
            else
            {
                datetime = new DateTime(year + 1900, mon + 1, day, hour, min, sec, (int)(us / 1000));
            }
            us %= 1000;
            us *= 10;
            return datetime.AddTicks(us);
        }

        public UDateTime()
        {
            m_time = 0;
        }

        public UDateTime(ulong time)
        {
            m_time = time;
        }

        public UDateTime(DateTime dt)
        {
            Set(dt);
        }

        public void Set(DateTime dt)
        {
            if (dt.Year < 1900)
            {
                throw new InvalidDataException("Datetime year can not be less than 1900");
            }
            m_time = (uint)((dt.Year - 1900) & 0x3ffff);
            m_time <<= 46; //18 bits for years
            ulong mid = (uint)((dt.Month - 1) & 0xf); //4 bits for month
            mid <<= 42;
            m_time += mid;
            mid = (uint)(dt.Day & 0x1f); //5 bits for day
            mid <<= 37;
            m_time += mid;
            mid = (uint)(dt.Hour & 0x1f); //5 bits for hour
            mid <<= 32;
            m_time += mid;
            mid = (uint)(dt.Minute & 0x3f); //6 bits for minute
            mid <<= 26;
            m_time += mid;
            mid = (uint)(dt.Second & 0x3f); //6 bits for second
            mid <<= 20;
            m_time += mid;
            uint us = (uint)dt.Millisecond * 1000;
            us += (uint)((dt.Ticks / 10) % 1000);
            m_time += us;
        }

        /// <summary>
        /// Call the method to get database datetime string
        /// </summary>
        /// <returns>a database datetime string</returns>
        public override string ToString()
        {
            bool micro = ((m_time & MICRO_SECONDS) > 0);
            bool has_time = (((m_time >> 20) & 0x1ffff) > 0);
            bool has_date = ((m_time >> 37) > 0);
            if (m_time == 0)
                return "";
            string format = "";
            if (has_date)
                format = "yyyy-MM-dd";
            if (has_time || micro)
            {
                if (format.Length > 0)
                    format += " ";
                format += "HH:mm:ss";
            }
            if (micro)
                format += ".ffffff";
            return GetDateTime().ToString(format);
        }
    }

    public interface IUSerializer
    {
        void LoadFrom(CUQueue UQueue);
        void SaveTo(CUQueue UQueue);
    }

    public interface IUPush
    {
        bool Publish(object Message, params uint[] Groups);
        bool Subscribe(params uint[] Groups);
        bool Unsubscribe();
        bool SendUserMessage(object Message, string UserId);
    }

    public interface IUPushEx : IUPush
    {
        bool Publish(byte[] Message, params uint[] Groups);
        bool SendUserMessage(string UserId, byte[] Message);
    }

    public class CUQueue
    {
        public const uint DEFAULT_BUFFER_SIZE = 4 * 1024;
        public const uint DEFAULT_MEMORY_BUFFER_BLOCK_SIZE = 4 * 1024;
        static List<MemoryStream> m_lstMemoryStream = new List<MemoryStream>();

        static MemoryStream LockMemoryStream()
        {
            MemoryStream ms = null;
            lock (m_lstMemoryStream)
            {
                if (m_lstMemoryStream.Count > 0)
                {
                    ms = m_lstMemoryStream[m_lstMemoryStream.Count - 1];
                    m_lstMemoryStream.RemoveAt(m_lstMemoryStream.Count - 1);
                    ms.SetLength(0);
                }
            }
            if (ms == null)
                ms = new MemoryStream();
            return ms;
        }

        static void ReleaseMemoryStream(MemoryStream ms)
        {
            if (ms == null)
                return;
            lock (m_lstMemoryStream)
            {
                m_lstMemoryStream.Add(ms);
            }
        }

        public static string ToString(sbyte[] astr, int len)
        {
            if (astr == null)
                return null;
            if (astr.Length == 0 || len == 0)
                return "";
            if (len == -1 || len > astr.Length)
                len = astr.Length;
            byte[] unsigned = (byte[])(Array)astr;
            return System.Text.Encoding.UTF8.GetString(unsigned, 0, len);
        }

        public static string ToString(sbyte[] astr)
        {
            return ToString(astr, -1);
        }

        public static string ToString(byte[] astr)
        {
            return ToString(astr, -1);
        }

        public static string ToString(byte[] astr, int len)
        {
            if (astr == null)
                return null;
            if (astr.Length == 0 || len == 0)
                return "";
            if (len == -1 || len > astr.Length)
                len = astr.Length;
            return System.Text.Encoding.UTF8.GetString(astr, 0, len);
        }

        //convert dotNet DateTime into native EPOCH time in ms
        public CUQueue Save(DateTime dt)
        {
            Save(new UDateTime(dt).time);
            return this;
        }

        //convert EPOCH date in ms into dotNet DateTime
        public CUQueue Load(out DateTime dt)
        {
            ulong diff;
            Load(out diff);
            dt = new UDateTime(diff).DateTime;
            return this;
        }

        public void CleanTrack()
        {
            //????
        }

        public CUQueue Save(CUQueue UQqueue)
        {
            uint nSize = uint.MaxValue;
            if (UQqueue != null)
            {
                nSize = UQqueue.GetSize();
                Save(nSize);
                Push(UQqueue.IntenalBuffer, UQqueue.HeadPosition, nSize);
            }
            else
            {
                Save(nSize);
            }
            return this;
        }

        public uint Discard(uint len)
        {
            if (len > m_len)
                len = m_len;
            m_len -= len;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += len;
            return len;
        }

        public CUQueue Load(out CUQueue UQueue)
        {
            uint nSize;
            Load(out nSize);
            if (nSize != uint.MaxValue)
            {
                if (nSize > GetSize())
                {
                    SetSize(0);
                    throw new InvalidOperationException("Data de-serialization wrong");
                }
                UQueue = new CUQueue();
                UQueue.Push(m_bytes, m_position, nSize);
                Discard(nSize);
            }
            else
                UQueue = null;
            return this;
        }

        private tagOperationSystem m_os = Defines.OperationSystem;
        private bool m_bEndian = false;

        public tagOperationSystem OS
        {
            get
            {
                return m_os;
            }
            set
            {
                m_os = value;
            }
        }

        public bool Endian
        {
            get
            {
                return m_bEndian;
            }
            set
            {
                m_bEndian = value;
            }
        }

        public CUQueue()
        {
            m_bytes = new Byte[DEFAULT_BUFFER_SIZE];
            m_blockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
        }

        public CUQueue(uint InitialSize)
        {
            m_bytes = new Byte[InitialSize];
            m_blockSize = DEFAULT_MEMORY_BUFFER_BLOCK_SIZE;
        }

        public CUQueue(uint InitialSize, uint BlockSize)
        {
            m_bytes = new Byte[InitialSize];
            m_blockSize = BlockSize;
        }

        /// <summary>
        /// Get a new array of bytes containing the packed data.
        /// </summary>
        /// <returns>An array of bytes</returns>
        public byte[] GetBuffer()
        {
            return GetBuffer(0);
        }

        /// <summary>
        /// Get a new array of bytes containing the packed data.
        /// </summary>
        /// <param name="offset">An integer for position offset which defaults to zero</param>
        /// <returns>An array of bytes</returns>
        public byte[] GetBuffer(uint offset)
        {
            if (offset > m_len)
                offset = m_len;
            byte[] bytes = new byte[m_len - offset];
            Buffer.BlockCopy(m_bytes, (int)(m_position + offset), bytes, 0, (int)(m_len - offset));
            return bytes;
        }

        /// <summary>
        /// A property for internal buffer which contains actual data in binary format. It is more efficient than the method GetBuffer, but you must pay attention to the property HeadPosition.
        /// </summary>
        public byte[] IntenalBuffer
        {
            get
            {
                return m_bytes;
            }
        }

        public uint GetSize()
        {
            return m_len;
        }

        public void SetSize(uint size)
        {
            if (m_bytes == null && size > 0)
                throw new InvalidOperationException("Buffer is empty");
            if (size == 0)
            {
                m_position = 0;
                m_len = 0;
                return;
            }
            if (size > (uint)(m_bytes.Length - m_position))
            {
                throw new InvalidOperationException("Bad new size");
            }
            m_len = size;
        }

        public void Empty()
        {
            m_bytes = null;
            m_position = 0;
            m_len = 0;
        }

        public unsafe CUQueue Save(bool bData)
        {
            return Append(&bData, sizeof(bool));
        }

        public unsafe CUQueue Save(byte bData)
        {
            return Append(&bData, sizeof(byte));
        }

        public unsafe CUQueue Save(sbyte sbData)
        {
            return Append(&sbData, sizeof(sbyte));
        }

        public unsafe CUQueue Save(short sData)
        {
            return Append(&sData, sizeof(short));
        }

        public unsafe CUQueue Save(ushort usData)
        {
            return Append(&usData, sizeof(ushort));
        }

        public unsafe CUQueue Save(int nData)
        {
            return Append(&nData, sizeof(int));
        }

        public unsafe CUQueue Save(uint unData)
        {
            return Append(&unData, sizeof(uint));
        }

        public unsafe CUQueue Save(ulong ulData)
        {
            return Append(&ulData, sizeof(ulong));
        }

        public unsafe CUQueue Save(long lData)
        {
            return Append(&lData, sizeof(long));
        }
#if WINCE

#else
        [DllImport("msvcrt.dll", EntryPoint = "memcpy", CallingConvention = CallingConvention.Cdecl, SetLastError = false), SuppressUnmanagedCodeSecurity]
        internal static unsafe extern void* CopyMemory(void* dest, void* src, ulong count);
#endif
        private unsafe CUQueue Append(void* pData, uint len)
        {
            if (TailSize < (uint)len)
            {
                uint addedSize = (((uint)len - TailSize) / m_blockSize + 1) * m_blockSize;
                Realloc(MaxBufferSize + addedSize);
            }
#if WINCE
            //add extra copy here. Use byte[] instead of MemoryStream in the future
            Marshal.Copy((System.IntPtr)pData, m_bytes, (int)(m_position + m_len), (int)len);
#else
            fixed (byte* des = m_bytes)
            {
                CopyMemory(des + m_position + m_len, pData, len);
            }
#endif
            m_len += (uint)len;
            return this;
        }

        public unsafe CUQueue Save(double dData)
        {
            return Append(&dData, sizeof(double));
        }

        public unsafe CUQueue Save(float fData)
        {
            return Append(&fData, sizeof(float));
        }

        public unsafe CUQueue Save(char wChar)
        {
            return Append(&wChar, sizeof(char));
        }

        public unsafe CUQueue Push(IntPtr p, uint size)
        {
            return Append((void*)p, size);
        }

        /// <summary>
        /// Save a string into this memory buffer without regarding its length. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="str">a string</param>
        /// <returns>a reference to this memory buffer</returns>
        public unsafe CUQueue Push(string str)
        {
            if (str != null && str.Length > 0)
            {
                fixed (char* c = str)
                {
                    Append(c, ((uint)(str.Length)) * sizeof(char));
                }
            }
            return this;
        }

        /// <summary>
        /// Save an array of bytes into this memory buffer. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="bytes">an array of bytes</param>
        /// <param name="offset">offset length in byte</param>
        /// <param name="len">length in byte</param>
        /// <returns>a reference to this memory buffer</returns>
        public unsafe CUQueue Push(byte[] bytes, uint offset, uint len)
        {
            if (bytes == null || len == 0)
                return this;
            if (offset + len > (uint)bytes.Length)
                throw new InvalidDataException("Offset and length are bad");
            fixed (byte* p = bytes)
            {
                Append(p + offset, len);
            }
            return this;
        }

        /// <summary>
        /// Save an array of bytes into this memory buffer. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="bytes">an array of bytes</param>
        /// <param name="len">length in byte</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Push(byte[] bytes, uint len)
        {
            return Push(bytes, 0, len);
        }

        /// <summary>
        /// Save an array of bytes into this memory buffer. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="bytes">an array of bytes</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Push(byte[] bytes)
        {
            if (bytes == null)
                return this;
            return Push(bytes, (uint)bytes.Length);
        }

        /// <summary>
        /// Save an array of ANSCII chars into this memory buffer. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="bytes">an array of ANSCII chars</param>
        /// <param name="len">length in byte</param>
        /// <returns>a reference to this memory buffer</returns>
        public unsafe CUQueue Push(sbyte[] bytes, uint len)
        {
            if (bytes == null || len == 0)
                return this;
            if (len > (uint)bytes.Length)
                len = (uint)bytes.Length;
            fixed (sbyte* p = bytes)
            {
                Append(p, len);
            }
            return this;
        }

        /// <summary>
        /// Save an array of ANSCII chars into this memory buffer. In general, don't use this method directly but the method Save instead
        /// </summary>
        /// <param name="bytes">an array of ANSCII chars</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Push(sbyte[] bytes)
        {
            if (bytes == null)
                return this;
            return Push(bytes, (uint)bytes.Length);
        }

        //convert dotNet bool into variant bool
        public CUQueue SaveVariantBool(bool b)
        {
            ushort bV = (ushort)(b ? 0xFFFF : 0x0000);
            return Save(bV);
        }

        //convert dotNet decimal data into native Currency
        public CUQueue SaveCY(decimal cyMoney)
        {
            long lMoney = (long)(cyMoney * 10000);
            return Save(lMoney);
        }

        public CUQueue Save(TimeSpan dt)
        {
            long ticks = dt.Ticks;
            return Save(ticks);
        }

        public unsafe CUQueue Save(Guid guid)
        {
            return Append(&guid, 16); //sizeof(Guid)
        }

        public unsafe CUQueue Save(decimal decData)
        {
            return Append(&decData, sizeof(decimal));
        }

        /// <summary>
        /// Save data into this memory buffer. In general, don't use this method directly but the method Serialize or Save instead
        /// </summary>
        /// <param name="USerializer">an interface to an object</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Push(IUSerializer USerializer)
        {
            USerializer.SaveTo(this);
            return this;
        }
#if WINCE
#else
        public CUQueue Serialize<T>(T obj) where T : new()
        {
            bool bNull = (obj == null) ? true : false;
            Save(bNull);
            if (bNull)
                return this;
            IFormatter formatter = new BinaryFormatter();
            MemoryStream ms = LockMemoryStream();
            formatter.Serialize(ms, obj);
            byte[] bytes = ms.ToArray();
            ReleaseMemoryStream(ms);
            return Push(bytes);
        }

        public CUQueue Deserialize<T>(out T obj) where T : new()
        {
            bool bNull;
            Load(out bNull);
            if (bNull)
            {
                object objNull = null;
                obj = (T)objNull;
                return this;
            }
            IFormatter formatter = new BinaryFormatter();
            MemoryStream ms = LockMemoryStream();
            ms.Write(m_bytes, (int)m_position, (int)m_len);
            ms.Seek(0, SeekOrigin.Begin);
            obj = (T)formatter.Deserialize(ms);
            m_position = 0;
            m_len = (uint)(ms.Length - ms.Position);
            ms.Read(m_bytes, 0, (int)m_len);
            ReleaseMemoryStream(ms);
            return this;
        }
#endif
        public CUQueue Save<T>(T data)
        {
            //typeof equal is very fast and generic (http://blogs.msdn.com/b/vancem/archive/2006/10/01/779503.aspx).
            object obj = data;
            Type t = typeof(T);
            if (t == typeof(string))
            {
                string str = data as string;
                Save(str);
            }
            else if (t == typeof(int))
            {
                Save((int)obj);
            }
            else if (t == typeof(uint))
            {
                Save((uint)obj);
            }
            else if (t == typeof(float))
            {
                Save((float)obj);
            }
            else if (t == typeof(double))
            {
                Save((double)obj);
            }
            else if (t == typeof(long))
            {
                Save((long)obj);
            }
            else if (t == typeof(bool))
            {
                Save((bool)obj);
            }
            else if (t == typeof(byte[]))
            {
                byte[] str = data as byte[];
                Save(str);
            }
            else if (t == typeof(DateTime))
            {
                Save((DateTime)obj);
            }
            else if (t == typeof(TimeSpan))
            {
                Save((TimeSpan)obj);
            }
            else if (t == typeof(ulong))
            {
                Save((ulong)obj);
            }
            else if (t == typeof(Guid))
            {
                Save((Guid)obj);
            }
            else if (t == typeof(decimal))
            {
                Save((decimal)obj);
            }
            else if (typeof(T) == typeof(object))
            {
                Save(obj);
            }
            else if (t == typeof(short))
            {
                Save((short)obj);
            }
            else if (t == typeof(ushort))
            {
                Save((ushort)obj);
            }
            else if (t == typeof(char))
            {
                Save((char)obj);
            }
            else if (t == typeof(sbyte))
            {
                Save((sbyte)obj);
            }
            else if (t == typeof(byte))
            {
                Save((byte)obj);
            }
            else if (typeof(T) == typeof(CUQueue))
            {
                CUQueue q = data as CUQueue;
                Save(q);
            }
            else if (t == typeof(sbyte[]))
            {
                sbyte[] str = data as sbyte[];
                Save(str);
            }
            else if (hasUSerializer(typeof(T)))
            {
                IUSerializer serializer = data as IUSerializer;
                Push(serializer);
            }
#if WINCE
#else
            else if (t.IsSerializable)
            {
                Serialize((object)data);
            }
#endif
            else
                throw new InvalidOperationException("Serialization not supported for data type " + typeof(T).FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer.");
            return this;
        }

        public CUQueue Save(sbyte[] str)
        {
            uint nLen;
            if (str == null)
            {
                nLen = uint.MaxValue;
                Save(nLen);
            }
            else
            {
                nLen = (uint)str.Length;
                Save(nLen).Push(str);
            }
            return this;
        }

        public CUQueue Save(byte[] bytes)
        {
            uint nLen;
            if (bytes == null)
            {
                nLen = uint.MaxValue;
                Save(nLen);
            }
            else
            {
                nLen = (uint)bytes.Length;
                Save(nLen).Push(bytes);
            }
            return this;
        }

        public CUQueue Save(string strData)
        {
            uint nLen;
            if (strData == null)
            {
                nLen = uint.MaxValue;
                Save(nLen);
            }
            else
            {
                nLen = ((uint)strData.Length) * 2;
                Save(nLen).Push(strData);
            }
            return this;
        }

        public CUQueue Save(object obData)
        {
            Save(obData, true, false);
            return this;
        }

        public CUQueue Save(object obData, bool bToNative)
        {
            Save(obData, bToNative, false);
            return this;
        }

        public CUQueue Save(object obData, bool bToNative, bool bDecToCY)
        {
            ushort vt = 0;
            if (obData == DBNull.Value || obData == null)
            {
                vt = (ushort)tagVariantDataType.sdVT_NULL;
                Save(vt);
                return this;
            }
            else if (obData is IUSerializer)
            {
                vt = (ushort)tagVariantDataType.sdVT_USERIALIZER_OBJECT;
                Save(vt).Push((IUSerializer)obData);
                return this;
            }
            else if (obData is TimeSpan)
            {
                vt = (ushort)tagVariantDataType.sdVT_TIMESPAN;
                Save(vt).Save(((TimeSpan)obData).Ticks);
                return this;
            }
            if (obData.GetType().IsArray)
            {
                switch (obData.GetType().FullName)
                {
                    case "System.String[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_BSTR | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            string[] strArray = (string[])obData;
                            uint nSize = (uint)strArray.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                string str = strArray[nIndex];
                                if (str == null)
                                {
                                    uint unLen = uint.MaxValue;
                                    Save(unLen);
                                }
                                else
                                {
                                    uint unLen = (uint)(str.Length * 2);
                                    Save(unLen);
                                    Push(str);
                                }
                            }
                        }
                        break;
                    case "System.Single[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_R4 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            float[] aF = (float[])obData;
                            uint nSize = (uint)aF.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(aF[nIndex]);
                            }
                        }
                        break;
                    case "System.DateTime[]":
                        {
                            int nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_DATE | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            DateTime[] aDT = (DateTime[])obData;
                            uint nSize = (uint)aDT.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(aDT[nIndex]);
                            }
                        }
                        break;
                    case "System.Double[]":
                        {
                            int nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_R8 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            double[] aD = (double[])obData;
                            uint nSize = (uint)aD.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(aD[nIndex]);
                            }
                        }
                        break;
                    case "System.Decimal[]":
                        {
                            int nIndex;
                            if (bDecToCY && bToNative)
                            {
                                vt = ((ushort)tagVariantDataType.sdVT_CY | (ushort)tagVariantDataType.sdVT_ARRAY);
                            }
                            else
                            {
                                vt = ((ushort)tagVariantDataType.sdVT_DECIMAL | (ushort)tagVariantDataType.sdVT_ARRAY);
                            }
                            Save(vt);
                            decimal[] aD = (decimal[])obData;
                            uint nSize = (uint)aD.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                if (bDecToCY && bToNative)
                                {
                                    SaveCY(aD[nIndex]);
                                }
                                else
                                {
                                    Save(aD[nIndex]);
                                }
                            }
                        }
                        break;
                    case "System.Boolean[]":
                        {
                            int nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_BOOL | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            bool[] bData = (bool[])obData;
                            uint nSize = (uint)bData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                ushort bBool = bData[nIndex] ? (ushort)0xFFFF : (ushort)0x0000;
                                Save(bBool);
                            }
                        }
                        break;
                    case "System.Byte[]":
                        {
                            vt = ((ushort)tagVariantDataType.sdVT_UI1 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt).Save((byte[])obData);
                        }
                        break;
                    case "System.SByte[]":
                        {
                            vt = ((ushort)tagVariantDataType.sdVT_I1 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt).Save((sbyte[])obData);
                        }
                        break;
                    case "System.UInt64[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_UI8 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            ulong[] ulData = (ulong[])obData;
                            uint nSize = (uint)ulData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(ulData[nIndex]);
                            }
                        }
                        break;
                    case "System.Int64[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_I8 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            long[] lData = (long[])obData;
                            uint nSize = (uint)lData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(lData[nIndex]);
                            }
                        }
                        break;
                    case "System.UInt32[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_UINT | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            uint[] unData = (uint[])obData;
                            uint nSize = (uint)unData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(unData[nIndex]);
                            }
                        }
                        break;
                    case "System.Int32[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_INT | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            int[] nData = (int[])obData;
                            uint nSize = (uint)nData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(nData[nIndex]);
                            }
                        }
                        break;
                    case "System.Char[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_UI2 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            char[] usData = (char[])obData;
                            uint nSize = (uint)usData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(usData[nIndex]);
                            }
                        }
                        break;
                    case "System.UInt16[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_UI2 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            ushort[] usData = (ushort[])obData;
                            uint nSize = (uint)usData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(usData[nIndex]);
                            }
                        }
                        break;
                    case "System.Int16[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_I2 | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            short[] sData = (short[])obData;
                            uint nSize = (uint)sData.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(sData[nIndex]);
                            }
                        }
                        break;
                    case "System.Guid[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_CLSID | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            Guid[] guidA = (Guid[])obData;
                            uint nSize = (uint)guidA.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(guidA[nIndex]);
                            }
                        }
                        break;
                    case "System.DBNull[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_NULL | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            DBNull[] dbNulls = (DBNull[])obData;
                            uint nSize = (uint)dbNulls.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save((short)tagVariantDataType.sdVT_NULL);
                            }
                        }
                        break;
                    case "System.TimeSpan[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_TIMESPAN | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            TimeSpan[] dbNulls = (TimeSpan[])obData;
                            uint nSize = (uint)dbNulls.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(dbNulls[nIndex].Ticks);
                            }
                        }
                        break;
                    case "System.Object[]":
                        {
                            uint nIndex;
                            vt = ((ushort)tagVariantDataType.sdVT_VARIANT | (ushort)tagVariantDataType.sdVT_ARRAY);
                            Save(vt);
                            object[] obA = (object[])obData;
                            uint nSize = (uint)obA.Length;
                            Save(nSize);
                            for (nIndex = 0; nIndex < nSize; nIndex++)
                            {
                                Save(obA[nIndex], bToNative, bDecToCY);
                            }
                        }
                        break;
                    default:
                        throw new InvalidOperationException("Data type not supported");
                }
            }
            else
            {
                switch (obData.GetType().FullName)
                {
                    case "System.Object":
                        vt = (ushort)tagVariantDataType.sdVT_EMPTY;
                        Save(vt);
                        break;
                    case "System.String":
                        {
                            vt = (ushort)tagVariantDataType.sdVT_BSTR;
                            Save(vt).Save((string)obData);
                        }
                        break;
                    case "System.Single":
                        vt = (ushort)tagVariantDataType.sdVT_R4;
                        Save(vt).Save((float)obData);
                        break;
                    case "System.DateTime":
                        vt = (ushort)tagVariantDataType.sdVT_DATE;
                        Save(vt).Save((DateTime)obData);
                        break;
                    case "System.Double":
                        vt = (ushort)tagVariantDataType.sdVT_R8;
                        Save(vt).Save((double)obData);
                        break;
                    case "System.Decimal":
                        if (bDecToCY && bToNative)
                        {
                            vt = (ushort)tagVariantDataType.sdVT_CY;
                            Save(vt).SaveCY((decimal)obData);
                        }
                        else
                        {
                            vt = (ushort)tagVariantDataType.sdVT_DECIMAL;
                            Save(vt).Save((decimal)obData);
                        }
                        break;
                    case "System.Boolean":
                        vt = (ushort)tagVariantDataType.sdVT_BOOL;
                        Save(vt);
                        {
                            bool bData = (bool)obData;
                            ushort bBool = bData ? (ushort)0xFFFF : (ushort)0x0000;
                            Save(bBool);
                        }
                        break;
                    case "System.Byte":
                        vt = (ushort)tagVariantDataType.sdVT_UI1;
                        Save(vt).Save((byte)obData);
                        break;
                    case "System.SByte":
                        vt = (ushort)tagVariantDataType.sdVT_I1;
                        Save(vt).Save((sbyte)obData);
                        break;
                    case "System.UInt64":
                        vt = (ushort)tagVariantDataType.sdVT_UI8;
                        Save(vt).Save((ulong)obData);
                        break;
                    case "System.Int64":
                        vt = (ushort)tagVariantDataType.sdVT_I8;
                        Save(vt).Save((long)obData);
                        break;
                    case "System.UInt32":
                        vt = (ushort)tagVariantDataType.sdVT_UI4;
                        Save(vt).Save((uint)obData);
                        break;
                    case "System.Int32":
                        vt = (ushort)tagVariantDataType.sdVT_I4;
                        Save(vt).Save((int)obData);
                        break;
                    case "System.Guid":
                        vt = (ushort)tagVariantDataType.sdVT_CLSID;
                        Save(vt).Save((Guid)obData);
                        break;
                    case "System.Char":
                        vt = (ushort)tagVariantDataType.sdVT_UI2;
                        Save(vt).Save((char)obData);
                        break;
                    case "System.UInt16":
                        vt = (ushort)tagVariantDataType.sdVT_UI2;
                        Save(vt).Save((ushort)obData);
                        break;
                    case "System.Int16":
                        vt = (ushort)tagVariantDataType.sdVT_I2;
                        Save(vt).Save((short)obData);
                        break;
                    default:
                        vt = (ushort)tagVariantDataType.sdVT_BSTR;
                        Save(vt).Save(obData.ToString());
                        break;
                }
            }
            return this;
        }

        /// <summary>
        /// Read an array of bytes from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="Bytes">an array of bytes receiving data</param>
        /// <param name="nLen">expected length in bytes</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out byte[] Bytes, uint nLen)
        {
            if (nLen > m_len)
                nLen = m_len;
            if (m_len == 0)
            {
                Bytes = new byte[0];
                return this;
            }
            Bytes = new byte[nLen];
            Buffer.BlockCopy(m_bytes, (int)m_position, Bytes, 0, (int)nLen);
            m_len -= nLen;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += nLen;
            return this;
        }

        public CUQueue Pop(uint nLen, ref byte[] Bytes)
        {
            if (nLen > m_len)
                nLen = m_len;
            if (m_len == 0)
            {
                Bytes = new byte[0];
                return this;
            }
            if (Bytes != null && (uint)Bytes.Length == nLen)
            {
            }
            else
            {
                Bytes = new byte[nLen];
            }
            Buffer.BlockCopy(m_bytes, (int)m_position, Bytes, 0, (int)nLen);
            m_len -= nLen;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += nLen;
            return this;
        }

        /// <summary>
        /// Read an array of ANSCII chars from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="sBytes">an array of ANSCII bytes receiving data</param>
        /// <param name="nLen">expected length in bytes</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out sbyte[] sBytes, uint nLen)
        {
            if (nLen > m_len)
                nLen = m_len;
            if (m_len == 0)
            {
                sBytes = new sbyte[0];
                return this;
            }
            sBytes = new sbyte[nLen];
            Buffer.BlockCopy(m_bytes, (int)m_position, sBytes, 0, (int)nLen);
            m_len -= nLen;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += nLen;
            return this;
        }

        public CUQueue Pop(uint nLen, ref sbyte[] sBytes)
        {
            if (nLen > m_len)
                nLen = m_len;
            if (m_len == 0)
            {
                sBytes = new sbyte[0];
                return this;
            }
            if (sBytes != null && (uint)sBytes.Length == nLen)
            {
            }
            else
            {
                sBytes = new sbyte[nLen];
            }
            Buffer.BlockCopy(m_bytes, (int)m_position, sBytes, 0, (int)nLen);
            m_len -= nLen;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += nLen;
            return this;
        }

        /// <summary>
        /// Read an array of bytes from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="Bytes">an array of bytes receiving data</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out byte[] Bytes)
        {
            return Pop(out Bytes, m_len);
        }

        /// <summary>
        /// Read an array of ANSCII bytes from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="sBytes">an array of ANSCII receiving data</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out sbyte[] sBytes)
        {
            return Pop(out sBytes, m_len);
        }

        public unsafe CUQueue Load(out byte bByte)
        {
            byte b;
            CopyTo(&b, sizeof(byte));
            bByte = b;
            return this;
        }

        /// <summary>
        /// Read string out from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="strData">a string receiving data</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out string strData)
        {
            uint nBytes = (m_len / sizeof(char)) * sizeof(char);
            return Pop(out strData, nBytes);
        }

        /// <summary>
        /// Read string out from this memory buffer. In general, don't use this method but the method Load instead
        /// </summary>
        /// <param name="strData">a string receiving data</param>
        /// <param name="nBytes">a given number of bytes</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop(out string strData, uint nBytes)
        {
            if ((nBytes % sizeof(char)) > 0)
                throw new ArgumentException("Bad input length for receiving a string");
            if (nBytes > m_len)
                nBytes = m_len;
            strData = System.Text.Encoding.Unicode.GetString(m_bytes, (int)m_position, (int)nBytes);
            m_len -= nBytes;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += nBytes;
            return this;
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

        public CUQueue Load<T>(out T data)
        {
            //typeof equal is very fast and generic (http://blogs.msdn.com/b/vancem/archive/2006/10/01/779503.aspx).
            Type type = typeof(T);
            data = default(T);
            if (type == typeof(string))
            {
                string str;
                Load(out str);
                data = (T)((object)str);
            }
            else if (type == typeof(int))
            {
                int n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(uint))
            {
                uint n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(bool))
            {
                bool n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(short))
            {
                short n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(ushort))
            {
                ushort n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(float))
            {
                float n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(double))
            {
                double n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(long))
            {
                long n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(char))
            {
                char n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(sbyte))
            {
                sbyte n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(byte))
            {
                byte n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(byte[]))
            {
                byte[] str;
                Load(out str);
                data = (T)((object)str);
            }
            else if (type == typeof(ulong))
            {
                ulong n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(DateTime))
            {
                DateTime n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(TimeSpan))
            {
                TimeSpan ts;
                Load(out ts);
                data = (T)((object)ts);
            }
            else if (type == typeof(object))
            {
                object n;
                Load(out n);
                data = (T)(n);
            }
            else if (type == typeof(Guid))
            {
                Guid n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(decimal))
            {
                decimal n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(CUQueue))
            {
                CUQueue n;
                Load(out n);
                data = (T)((object)n);
            }
            else if (type == typeof(sbyte[]))
            {
                sbyte[] str;
                Load(out str);
                data = (T)((object)str);
            }
            else if (hasUSerializer(typeof(T)))
            {
                IUSerializer uobj = (IUSerializer)(System.Activator.CreateInstance(typeof(T)));
                uobj.LoadFrom(this);
                data = (T)((object)uobj);
            }
#if WINCE
#else
            else if (typeof(T).IsSerializable)
            {
                object obj;
                Deserialize(out obj);
                data = (T)obj;
            }
#endif
            else
                throw new InvalidOperationException("Serialization not supported for data type " + typeof(T).FullName + ". To support serialization, your class should be implemented with SocketProAdapter.IUSerializer");
            return this;
        }

        public CUQueue Load(out string str)
        {
            uint len;
            Load(out len);
            if (len == uint.MaxValue)
                str = null;
            else if ((len % sizeof(char)) > 0 || len > GetSize())
            {
                SetSize(0);
                throw new InvalidOperationException("Invalid data found for retrieving a string");
            }
            else
            {
                Pop(out str, len);
            }
            return this;
        }

        public CUQueue Load(out sbyte[] str)
        {
            uint len;
            Load(out len);
            if (len == uint.MaxValue)
                str = null;
            else if (len > GetSize())
            {
                SetSize(0);
                throw new InvalidOperationException("Invalid data found");
            }
            else
            {
                Pop(out str, len);
            }
            return this;
        }

        public CUQueue Load(out byte[] str)
        {
            uint len;
            Load(out len);
            if (len == uint.MaxValue)
                str = null;
            else if (len > GetSize())
            {
                SetSize(0);
                throw new InvalidOperationException("Invalid data found");
            }
            else
            {
                Pop(out str, len);
            }
            return this;
        }

        /// <summary>
        /// Read an object implemented with the interface USerializer out from this memory buffer. In general, don't use this method but the method Deserialize or Load instead
        /// </summary>
        /// <typeparam name="T">a type of classes implemented with the interface USerializer</typeparam>
        /// <param name="USerializer">an object implemented with the interface USerializer</param>
        /// <returns>a reference to this memory buffer</returns>
        public CUQueue Pop<T>(out T USerializer) where T : IUSerializer, new()
        {
            USerializer = new T();
            USerializer.LoadFrom(this);
            return this;
        }

        public CUQueue Load(out object obData)
        {
            ushort vt;
            return Load(out obData, out vt);
        }

        private const ushort usArray = (ushort)tagVariantDataType.sdVT_ARRAY;

        public CUQueue Load(out object obData, out ushort vt)
        {
            Load(out vt);
            if (vt == (ushort)tagVariantDataType.sdVT_EMPTY)
            {
                obData = new object();
                return this;
            }
            else if (vt == (ushort)tagVariantDataType.sdVT_NULL)
            {
                obData = null;
                return this;
            }
            bool isArray = ((vt & usArray) == usArray);
            if (isArray)
            {
                uint un;
                uint unSize;
                Load(out unSize);
                switch ((tagVariantDataType)(vt & ~usArray))
                {
                    case tagVariantDataType.sdVT_BOOL:
                        {
                            ushort vtBool;
                            bool[] aBools = new bool[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out vtBool);
                                aBools[un] = (vtBool == 0) ? false : true;
                            }
                            obData = aBools;
                        }
                        break;
                    case tagVariantDataType.sdVT_DECIMAL:
                        {
                            decimal[] aDecimals = new decimal[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aDecimals[un]);
                            }
                            obData = aDecimals;
                        }
                        break;
                    case tagVariantDataType.sdVT_I1:
                        {
                            if (m_len < unSize)
                            {
                                Empty();
                                throw new InvalidOperationException("Invalid data found");
                            }
                            obData = System.Text.Encoding.UTF8.GetString(m_bytes, (int)m_position, (int)unSize);
                            Discard(unSize);
                        }
                        break;
                    case tagVariantDataType.sdVT_I2:
                        {
                            short[] aShorts = new short[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aShorts[un]);
                            }
                            obData = aShorts;
                        }
                        break;

                    case tagVariantDataType.sdVT_I4:
                    case tagVariantDataType.sdVT_INT:
                        {
                            int[] aInts = new int[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aInts[un]);
                            }
                            obData = aInts;
                        }
                        break;
                    case tagVariantDataType.sdVT_I8:
                        {
                            long[] aLongs = new long[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aLongs[un]);
                            }
                            obData = aLongs;
                        }
                        break;
                    case tagVariantDataType.sdVT_R4:
                        {
                            float[] aFloats = new float[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aFloats[un]);
                            }
                            obData = aFloats;
                        }
                        break;
                    case tagVariantDataType.sdVT_DATE:
                        {
                            DateTime[] aDateTimes = new DateTime[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aDateTimes[un]);
                            }
                            obData = aDateTimes;
                        }
                        break;
                    case tagVariantDataType.sdVT_R8:
                        {
                            double[] aDoubles = new double[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aDoubles[un]);
                            }
                            obData = aDoubles;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI1:
                        {
                            byte[] bytes;
                            Pop(out bytes, unSize);
                            obData = bytes;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI2:
                        {
                            ushort[] aUshorts = new ushort[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aUshorts[un]);
                            }
                            obData = aUshorts;
                        }
                        break;
                    case tagVariantDataType.sdVT_UINT:
                    case tagVariantDataType.sdVT_UI4:
                        {
                            uint[] aUints = new uint[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aUints[un]);
                            }
                            obData = aUints;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI8:
                        {
                            ulong[] aULongs = new ulong[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aULongs[un]);
                            }
                            obData = aULongs;
                        }
                        break;
                    case tagVariantDataType.sdVT_VARIANT:
                        {
                            object[] aObjects = new object[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                {
                                    object obj = null;
                                    Load(out obj);
                                    aObjects[un] = obj;
                                }
                            }
                            obData = aObjects;
                        }
                        break;
                    case tagVariantDataType.sdVT_CY:
                        {
                            long lCY;
                            decimal[] aDecimals = new decimal[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out lCY);
                                aDecimals[un] = lCY;
                                aDecimals[un] /= 10000;
                            }
                            obData = aDecimals;
                        }
                        break;
                    case tagVariantDataType.sdVT_CLSID:
                        {
                            Guid[] aGuids = new Guid[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out aGuids[un]);
                            }
                            obData = aGuids;
                        }
                        break;
                    case tagVariantDataType.sdVT_TIMESPAN:
                        {
                            long ticks;
                            TimeSpan[] aGuids = new TimeSpan[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out ticks);
                                aGuids[un] = new TimeSpan(ticks);
                            }
                            obData = aGuids;
                        }
                        break;
                    case tagVariantDataType.sdVT_BSTR:
                        {
                            uint uLen;
                            string[] aStr = new string[unSize];
                            for (un = 0; un < unSize; un++)
                            {
                                Load(out uLen);
                                if (uLen != uint.MaxValue)
                                {
                                    Pop(out aStr[un], uLen);
                                }
                            }
                            obData = aStr;
                        }
                        break;
                    default:
                        throw new InvalidOperationException("Unsupported serialization");
                }
            }
            else
            {
                switch ((tagVariantDataType)vt)
                {
                    case tagVariantDataType.sdVT_EMPTY:
                        obData = null;
                        break;
                    case tagVariantDataType.sdVT_BSTR:
                        {
                            string str;
                            Load(out str);
                            obData = str;
                        }
                        break;
                    case tagVariantDataType.sdVT_BOOL:
                        {
                            ushort vtBool;
                            Load(out vtBool);
                            if (vtBool > 0)
                                obData = true;
                            else
                                obData = false;
                        }
                        break;
                    case tagVariantDataType.sdVT_CY:
                        {
                            long lCY;
                            decimal decData;
                            Load(out lCY);
                            decData = lCY;
                            decData /= 10000;
                            obData = decData;
                        }
                        break;
                    case tagVariantDataType.sdVT_DECIMAL:
                        {
                            decimal decData;
                            Load(out decData);
                            obData = decData;
                        }
                        break;
                    case tagVariantDataType.sdVT_I1:
                        {
                            sbyte sData;
                            Load(out sData);
                            obData = sData;
                        }
                        break;
                    case tagVariantDataType.sdVT_I2:
                        {
                            short sData;
                            Load(out sData);
                            obData = sData;
                        }
                        break;
                    case tagVariantDataType.sdVT_I4:
                        {
                            int nData;
                            Load(out nData);
                            obData = nData;
                        }
                        break;
                    case tagVariantDataType.sdVT_I8:
                        {
                            long lData;
                            Load(out lData);
                            obData = lData;
                        }
                        break;
                    case tagVariantDataType.sdVT_R4:
                        {
                            float fData;
                            Load(out fData);
                            obData = fData;
                        }
                        break;
                    case tagVariantDataType.sdVT_DATE:
                        {
                            DateTime dt;
                            Load(out dt);
                            obData = dt;
                        }
                        break;
                    case tagVariantDataType.sdVT_R8:
                        {
                            double dData;
                            Load(out dData);
                            obData = dData;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI1:
                        {
                            byte bData;
                            Load(out bData);
                            obData = bData;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI2:
                        {
                            ushort usData;
                            Load(out usData);
                            obData = usData;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI4:
                        {
                            uint unData;
                            Load(out unData);
                            obData = unData;
                        }
                        break;
                    case tagVariantDataType.sdVT_UI8:
                        {
                            ulong ulData;
                            Load(out ulData);
                            obData = ulData;
                        }
                        break;
                    case tagVariantDataType.sdVT_CLSID:
                        {
                            Guid guid;
                            Load(out guid);
                            obData = guid;
                        }
                        break;
                    case tagVariantDataType.sdVT_TIMESPAN:
                        {
                            long ticks;
                            Load(out ticks);
                            obData = new TimeSpan(ticks);
                        }
                        break;
                    default:
                        throw new InvalidOperationException("Unsupported serialization");
                }
            }
            return this;
        }

        private unsafe void CopyTo(void* data, uint len)
        {
            if (m_len < len)
            {
                Empty();
                throw new InvalidOperationException("Invalid data found");
            }
#if WINCE
            Marshal.Copy(m_bytes, (int)m_position, (IntPtr)data, (int)len);
#else
            fixed (byte* source = m_bytes)
            {
                CopyMemory(data, source + m_position, len);
            }
#endif
            m_len -= (uint)len;
            if (m_len == 0)
                m_position = 0;
            else
                m_position += (uint)len;
        }

        public unsafe CUQueue Load(out Guid guid)
        {
            Guid g;
            CopyTo(&g, 16); //sizeof(Guid)
            guid = g;
            return this;
        }

        internal unsafe CUQueue LoadDBGuid(out object guid)
        {
            ushort vt;
            Load(out vt);
            if (vt != (ushort)(tagVariantDataType.sdVT_UI1 | tagVariantDataType.sdVT_ARRAY))
                throw new InvalidOperationException("Invalid data found");
            uint len;
            Load(out len);
            if (len != sizeof(Guid))
                throw new InvalidOperationException("Invalid data found");
            Guid g;
            CopyTo(&g, 16); //sizeof(Guid)
            guid = g;
            return this;
        }

        public unsafe CUQueue Load(out decimal decData)
        {
            decimal d;
            CopyTo(&d, sizeof(decimal));
            decData = d;
            return this;
        }

        public unsafe CUQueue Load(out bool bData)
        {
            bool b;
            CopyTo(&b, sizeof(bool));
            bData = b;
            return this;
        }

        public unsafe CUQueue Load(out sbyte aChar)
        {
            sbyte s;
            CopyTo(&s, sizeof(sbyte));
            aChar = s;
            return this;
        }

        public unsafe CUQueue Load(out char wChar)
        {
            char c;
            CopyTo(&c, sizeof(char));
            wChar = c;
            return this;
        }

        public unsafe CUQueue Load(out ushort usData)
        {
            ushort n;
            CopyTo(&n, sizeof(ushort));
            usData = n;
            return this;
        }

        public unsafe CUQueue Load(out short sData)
        {
            short s;
            CopyTo(&s, sizeof(short));
            sData = s;
            return this;
        }

        //convert native variant bool into dotnet bool
        public CUQueue LoadVariantBool(out bool b)
        {
            ushort usB;
            Load(out usB);
            b = (usB > 0);
            return this;
        }

        //dotNet has no data type currency, convert native currency into decimal
        public CUQueue LoadCY(out decimal cyMoney)
        {
            long lMoney;
            Load(out lMoney);
            cyMoney = lMoney;
            cyMoney /= 10000;
            return this;
        }
        public CUQueue Load(out TimeSpan ts)
        {
            long ticks;
            Load(out ticks);
            ts = new TimeSpan(ticks);
            return this;
        }

        public unsafe CUQueue Load(out float fData)
        {
            float f;
            CopyTo(&f, sizeof(float));
            fData = f;
            return this;
        }

        public unsafe CUQueue Load(out int nData)
        {
            int n;
            CopyTo(&n, sizeof(int));
            nData = n;
            return this;
        }

        public unsafe CUQueue Load(out uint unData)
        {
            uint n;
            CopyTo(&n, sizeof(uint));
            unData = n;
            return this;
        }

        public unsafe CUQueue Load(out ulong ulData)
        {
            ulong n;
            CopyTo(&n, sizeof(ulong));
            ulData = n;
            return this;
        }

        public unsafe CUQueue Load(out double dData)
        {
            double d;
            CopyTo(&d, sizeof(double));
            dData = d;
            return this;
        }

        public void Realloc(uint newMaxSize)
        {
            if (newMaxSize == 0)
                newMaxSize = DEFAULT_BUFFER_SIZE;
            byte[] buffer = new byte[newMaxSize];
            if (m_len > 0)
            {
                m_len = (m_len > newMaxSize) ? newMaxSize : m_len;
                Buffer.BlockCopy(m_bytes, (int)m_position, buffer, 0, (int)m_len);
            }
            m_bytes = buffer;
            m_position = 0;
        }

        public uint MaxBufferSize
        {
            get
            {
                if (m_bytes == null)
                    return 0;
                return (uint)m_bytes.Length;
            }
        }

        public unsafe CUQueue Load(out long lData)
        {
            long n;
            CopyTo(&n, sizeof(long));
            lData = n;
            return this;
        }

        public uint Size
        {
            get
            {
                return m_len;
            }
        }

        /// <summary>
        /// An integer value for next popping position
        /// </summary>
        public uint HeadPosition
        {
            get
            {
                return m_position;
            }
        }

        /// <summary>
        /// An integer value for unused bytes
        /// </summary>
        public uint TailSize
        {
            get
            {
                if (m_bytes == null)
                    return 0;
                return (uint)(m_bytes.Length - m_position - m_len);
            }
        }

        private uint m_position = 0;
        private uint m_len = 0;
        internal byte[] m_bytes;
        private uint m_blockSize;
    }
}
