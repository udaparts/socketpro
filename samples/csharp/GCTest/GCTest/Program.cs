using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.Serialization.Formatters.Binary;
using System.Runtime.Serialization;
using SocketProAdapter;


namespace GCTest
{
    [Serializable]
    class TestSerialization
    {
        int n = 1;
        string s = "test";
    }

    class Program
    {
        public static int Test<T>(ICollection<T> obj, string name) 
            where T : new()
            {
                int length = -1;
                if (obj != null)
                    length = obj.Count;
                T t = new T();
                obj.Add(t);
                return length;
            }

        public static int Test<T>(Queue<T> obj, string name)
            where T : new()
        {
            int length = -1;
            if (obj != null)
                length = obj.Count;
            T t = default(T);
            obj.Enqueue(t);
            return length;
        }

        public static int Test<T, A>(IDictionary<T, A> obj, string name)
        {
            int length = -1;
            if (obj != null)
                length = obj.Count;
            T t = default(T);
            A a = default(A);
            obj.Add(t, a);
            return length;
        }

        public static int Test<T>(Stack<T> obj, string name)
        {
            int length = -1;
            if (obj != null)
                length = obj.Count;
            obj.Push(default(T));
            return length;
        }
      
        static void Main(string[] args)
        {
            string test = "MyTest";
            Stack<int> s = new Stack<int>();
            s.Push(3);
            s.Push(2);
            int[] mys = s.ToArray();

            LinkedList<int> lstLinkedInt = new LinkedList<int>();
            SortedDictionary<int, string> sdOut, sd = new SortedDictionary<int, string>();
            List<TestSerialization> arrTSOut, arrTS = new List<TestSerialization>(3);
            Dictionary<int, int> dic = new Dictionary<int, int>(5);
            Queue<int> q = new Queue<int>();
            q.Enqueue(1);

            int res = Test(arrTS, test);
            res = Test(s, test);
            res = Test(sd, test);
            res = Test(q, test);
            res = Test(dic, test);
            res = Test(lstLinkedInt, test);
            res = 0;

            Dictionary<string, int> dicOut;
            Dictionary<string, int> myDic = new Dictionary<string, int>();

            TestSerialization ts = new TestSerialization();
            arrTS.Add(ts);

            string k0 = "cye";
            string k1 = "ye";
            myDic.Add(k0, 1);
            myDic.Add(k1, 6);

            LinkedList<string> llOut;
            LinkedList<string> ll = new LinkedList<string>();
            ll.AddLast(k0);
            ll.AddLast(k1);
            q.Enqueue(234);
            Queue<int> qOut;
            Stack<string> ssOut, ss = new Stack<string>();
            ss.Push("test");
            ss.Push("kerui");
            sd.Add(2, "test");
            sd.Add(1, "again");

            using (CScopeUQueue su = new CScopeUQueue())
            {
                su.Save(arrTS);
                su.Save(ll);
                su.Save(myDic);
                su.Save(q);
                su.Save(sd);
                su.Save(ss);

                su.Load(out arrTSOut);
                su.Load(out llOut);
                su.Load(out dicOut);
                
                su.Load(out qOut); 
                su.Load(out sdOut);
                su.Load(out ssOut); 
            }

        }
    }
}
