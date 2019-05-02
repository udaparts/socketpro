using System;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Data;

namespace cetest
{
    class Program
    {
        static void TestHelloWorld(CConnectionContext cc)
        {
            using (CSocketPool<CHwAsyncHandler> hwPool = new CSocketPool<CHwAsyncHandler>())
            {
                bool ok = hwPool.StartSocketPool(cc, 1, 1);
                CHwAsyncHandler hw = hwPool.Lock();
                if (hw == null)
                {
                    Console.WriteLine("Hello world service not connected");
                    return;
                }
                ok = hw.AttachedClientSocket.ClientQueue.StartQueue("hwqueue", 24 * 3600);
                if (!ok)
                    Console.WriteLine("Failed in creating queue with status = " + hw.AttachedClientSocket.ClientQueue.QueueStatus);
                try
                {
                    string str = hw.SayHelloFromClient("UDAParts", 123);
                    Console.WriteLine(str);
                }
                catch (Exception err)
                {
                    Console.WriteLine("ErrMsg = " + err.Message + ", stack = " + err.StackTrace);
                }
                hwPool.ShutdownPool();
            }
        }

        static void TestAdo(CConnectionContext cc)
        {
            using (CSocketPool<RAdo> spAdo = new CSocketPool<RAdo>())
            {
                bool ok = spAdo.StartSocketPool(cc, 1, 1);
                RAdo ado = spAdo.Lock();
                if (ado == null)
                {
                    Console.WriteLine("RAdo not connected");
                    return;
                }
                ok = ado.AttachedClientSocket.ClientQueue.StartQueue("adoqueue", 24 * 3600);
                if (!ok)
                    Console.WriteLine("Failed in creating queue with status = " + ado.AttachedClientSocket.ClientQueue.QueueStatus);
                DataSet ds = ado.GetDataSet("select * from Person.Address", "select * from Purchasing.Vendor");
                if (ds != null)
                    Console.WriteLine("Tables = " + ds.Tables.Count);
                ok = ado.SendDataSet(ds);
                ok = ado.WaitAll();
                DataTable dt = ado.GetDataReader("select * from Person.Person");
                if (dt != null)
                    Console.WriteLine("Columns = " + dt.Columns.Count + ", rows = " + dt.Rows.Count);
                ok = ado.SendDataTable(dt);
            }
        }

        static void TestEcho(CConnectionContext cc)
        {
            using (CSocketPool<CTOne> sp = new CSocketPool<CTOne>())
            {
                sbyte[] astr;
                string wstr;
                sp.DoSslServerAuthentication += (sender, cs) =>
                {
                    Console.WriteLine("Issue = " + cs.UCert.Issuer);
                    Console.WriteLine("Session info = " + cs.UCert.SessionInfo);
                    return true;
                };
                bool ok = sp.StartSocketPool(cc, 1, 1);
                CTOne tone = sp.Lock();
                if (!ok || tone == null)
                {
                    Console.WriteLine("Failed in connecting to remote server");
                    return;
                }
              
                Console.WriteLine(tone.Echo("Test CE echo"));
                Console.WriteLine("QueryCount() = " + tone.QueryCount());
                Console.WriteLine("QueryGlobalCount() = " + tone.QueryGlobalCount());
                Console.WriteLine("QueryGlobalFastCount() = " + tone.QueryGlobalFastCount());
                sbyte[] data = { (sbyte)'A', (sbyte)'B', (sbyte)'C' };
                MyStruct ms = new MyStruct();
                ms.ABool = true;
                ms.AInt = 98765432;
                ms.AString = new byte[] { (byte)12, (byte)23, (byte)249 };
                ms.WString = "test";
                bool ret = tone.EchoEx(data, " = abc", ms, out astr, out wstr);
                Console.WriteLine("Ret = " + ret + ", wstr = " + wstr);
                sp.ShutdownPool();
            }
        }

        static void TestQueueByEchoSys(CConnectionContext cc)
        {
            int n;
            string sEchoTest = "SocketPro -- a package of revolutionary software components written from batching, asynchrony and parallel computation with many unique and critical features. These features assist you to quickly develop high speed and scalable distributed applications on Windows and smart devices as well as web browsers.Tutorial One -- The first tutorial is a hello project to support five simple requests. One of them is a slow request processed with a worker thread at the server side. The tutorial tells you how to code step-by-step at both server and client sides based on the classes of SocketProAdapter and its sub namespaces. The tutorial leads you to do two experiments that require you to use the well known tool Telnet for connecting from a client to a remote SocketPro server. The tutorial tells you SocketPro threads management at server side. It tells you what variables should be synchronized with a critical section or monitor. For client development, the tutorial is focused on how to use SocketProAdapter at client side, how to turn on/off online compressing, how to batch requests, how to do asynchrony computation, how to do synchrony computation, and how to switch between the two computation models. This tutorial leads you to do an particular experiment, freezing and de-freezing Window GUIs at run time.";
            const int TEST_CYCLES = 10;
            using (CSocketPool<CEchoSys> spEchoSys = new CSocketPool<CEchoSys>())
            {
                bool ok = spEchoSys.StartSocketPool(cc, 2, 1);
                CEchoSys es = spEchoSys.Lock();
                if (es == null)
                {
                    Console.WriteLine("Echo sys not connected");
                    return;
                }
                CEchoSys es1 = spEchoSys.Lock();
                es.AttachedClientSocket.BaseRequestProcessed += (sender, reqId) =>
                {
                    Console.WriteLine("Request Id = " + reqId.ToString());
                };

                ok = es.AttachedClientSocket.ClientQueue.StartQueue("echosys", (uint)30 * 24 * 3600 * 1000);
                ok = es1.AttachedClientSocket.ClientQueue.StartQueue("echosys", (uint)30 * 24 * 3600 * 1000);

                CEchoSys es2 = spEchoSys.SeekByQueue("echosys");

                Console.WriteLine("Input a line to continue (TestQueueByEchoSys).....");
                string s = Console.ReadLine();

                MyStruct ms = new MyStruct();
                ms.AInt = 2056789;
                ms.ABool = true;
                ms.WString = "This is test wchar_t string";
                byte[] astr = { 10, 50, 95, 24 };
                ms.AString = astr;
                MyStruct res = es.EchoMyStruct(ms);

                object objRes;
                string sOut;
                long lOut;
                uint nData;
                long lData = 1234567890987;
                CUQueue q = new CUQueue();
                q.Save("TestWSTRING").Save(lData);
                CUQueue qRes = es.EchoUQueue(q);
                if (qRes != null)
                {
                    qRes.Load(out sOut).Load(out lOut);
                }

                for (n = 0; n < TEST_CYCLES; ++n)
                {
                    ok = es.AttachedClientSocket.ClientQueue.StartJob();
                    ok = es.SendRequest(TEchoDConst.idEchoMyStructCEchoSys, ms, (ar) =>
                    {
                        ar.Load(out res);
                        Console.WriteLine("ms = " + ((res == null) ? "null" : res.ToString()));
                    });

                    ok = es.SendRequest(TEchoDConst.idEchoComplex0CEchoSys, 1234.56, "testSTR", "MyTestMe", true, (ar) =>
                    {
                        ar.Load(out sOut).Load(out objRes);
                        Console.WriteLine("sOut = " + sOut + ", objRes = " + objRes.ToString());
                    });

                    ok = es.SendRequest(TEchoDConst.idEchoComplex0CEchoSys + 1, sEchoTest, (ar) =>
                    {
                        ar.Load(out sOut).Load(out nData);
                        Console.WriteLine("sOut = " + sOut + ", nData = " + nData);
                    });
                    ok = es.AttachedClientSocket.ClientQueue.EndJob();
                }
                ok = es.WaitAll();
                Console.WriteLine("Queue test completed, and input a line to stop");
                string str = Console.ReadLine();
            }
        }

        static void Main(string[] args)
        {
            CClientSocket.QueueConfigure.MessageQueuePassword = "MyQueuePassword";
            CConnectionContext cc = new CConnectionContext("192.168.1.109", 20901, "SocketPro", "Password", tagEncryptionMethod.TLSv1);
            TestHelloWorld(cc);
            TestEcho(cc);
            TestAdo(cc);
            TestQueueByEchoSys(cc);
        }
    }
}
