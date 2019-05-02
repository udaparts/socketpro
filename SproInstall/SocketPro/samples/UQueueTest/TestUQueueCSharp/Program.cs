using System;
using SocketProAdapter;
using System.Diagnostics;
using System.Collections.Generic;

namespace TestUQueue
{
    //[Serializable]
    public class CNoSerialization
    {
        public long Data;
        public string Str;
        public List<string> List;
    }

    [Serializable]
    public class CTestItem2
    {
        public DateTime m_dt;
        public long m_lData;
        public string m_strUID;
        public TimeSpan m_ts;
        public DateTimeOffset m_offset;
        public List<string> List = new List<string>();
    }

    public class CTestItem : IUSerializer
    {
        public void SaveTo(CUQueue UQueue)
        {
            int count;
            if (List == null)
                List = new List<string>();

            //Make sure both SaveTo and LoadFrom can match each other in sequence for all of members!
            UQueue.Push(m_lData);
            UQueue.Save(m_strUID);
            UQueue.Push(m_dt);
   
            count = List.Count;
            UQueue.Push(count);
            foreach (string str in List)
            {
                UQueue.Save(str);
            }

            UQueue.Push(m_ts);
            UQueue.Push(m_offset);
        }

        public int LoadFrom(CUQueue UQueue)
        {
            int count;
            string str;
            List.Clear();

            //Make sure both SaveTo and LoadFrom can match each other in sequence for all of members!

            int start = UQueue.GetSize();
            UQueue.Pop(out m_lData);
            UQueue.Load(out m_strUID);
            UQueue.Pop(out m_dt);

            UQueue.Pop(out count);
            while (count > 0)
            {
                UQueue.Load(out str);
                List.Add(str);
                --count;
            }

            UQueue.Pop(out m_ts);
            UQueue.Pop(out m_offset);

            return start - UQueue.GetSize();
        }

        public DateTime m_dt;
        public long m_lData;
        public string m_strUID;
        public TimeSpan m_ts;
        public DateTimeOffset m_offset;
        public List<string> List = new List<string>();
    }

    class Program
    {
        public void TestIUSerilizer()
        {
            CTestItem inItem = new CTestItem();
            CTestItem outOut = new CTestItem();
            using (CScopeUQueue su = new CScopeUQueue())
            {
                inItem.List.Add("test");
                inItem.List.Add("me");
                inItem.m_lData = 56781234567;
                inItem.m_strUID = "SoCketpro";
                inItem.m_dt = DateTime.Now;
                inItem.m_ts = new TimeSpan(1234567890);
                inItem.m_offset = new DateTimeOffset(5, 2, 4, 10, 50, 40, new TimeSpan());

                //use IUSerilizer
                su.Save(inItem);
                su.Load(out outOut);
                Debug.Assert(su.UQueue.Size == 0);
            }

            //Memory released back into memory pool for reuse
        }

        public void TestSerializable()
        {
            CTestItem2 inItem = new CTestItem2();
            CTestItem2 outOut = new CTestItem2();
            using (CScopeUQueue su = new CScopeUQueue())
            {
                inItem.List.Add("test");
                inItem.List.Add("me");
                inItem.m_lData = 56781234567;
                inItem.m_strUID = "SoCketpro";
                inItem.m_dt = DateTime.Now;
                inItem.m_ts = new TimeSpan(1234567890);
                inItem.m_offset = new DateTimeOffset(5, 2, 4, 10, 50, 40, new TimeSpan());
                
                //use MS native serialization
                su.Save(inItem);
                su.Load(out outOut);
                Debug.Assert(su.UQueue.Size == 0);
            }

            //Memory released back into memory pool for reuse
        }

        public void TestSimpleOnes()
        {
            int nOut = 0, nData = 1234;
            short sOut = 0, sData = 256;
            DateTime dtOut = default(DateTime);
            DateTime dt = DateTime.Now;
            string strOut = null, str = "MyTest";
            bool bOut = false, b = true;
            CUQueue UQueue = new CUQueue();
            UQueue.Push(nData);
            UQueue.Push(sData);
            UQueue.Push(dt);
            UQueue.Save(str);
            UQueue.Push(b);

            UQueue.Pop(out nOut);
            UQueue.Pop(out sOut);
            UQueue.Pop(out dtOut);
            UQueue.Load(out strOut);
            UQueue.Pop(out bOut);

            Debug.Assert(UQueue.GetSize() == 0);
        }

        public void TestNotSupport()
        {
            CNoSerialization nsOut, ns = new CNoSerialization();
            ns.Data = 12345;
            ns.Str = "Test";
            ns.List = new List<string>();
            ns.List.Add("Test");
            ns.List.Add("Me");

            try
            {
                using (CScopeUQueue su = new CScopeUQueue())
                {
                    su.Save(ns);
                    su.Load(out nsOut);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }

        static void Main(string[] args)
        {
            Program p = new Program();
            p.TestSimpleOnes();
            p.TestIUSerilizer();
            p.TestSerializable();
            p.TestNotSupport();
        }
    }
}
