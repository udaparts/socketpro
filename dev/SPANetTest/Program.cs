using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Diagnostics;
using System.Data;


namespace SPANetTest
{
    [Serializable]
    public class CMySTest
    {
        public int Data;
        public string Str;
        public DateTime DT;
    }

    class Program
    {
        static void TestSerializable()
        {
            using (CScopeUQueue su = new CScopeUQueue())
            {
                int nOut;
                string strOut;
                CMySTest stOut;
                CMySTest st = new CMySTest();
                st.Data = 1234509;
                st.Str = "A test string";
                st.DT = DateTime.Now;
                su.Save(st.Data).Save(st).Save(st.Str);
                su.Load(out nOut).Load(out stOut).Load(out strOut);
                Debug.Assert(nOut == st.Data);
                Debug.Assert(strOut == st.Str);
                Debug.Assert(st.DT == stOut.DT);
            }
        }

        static void TestEchoSys(CConnectionContext cc)
        {
            using (CSocketPool<CEchoSys> spEchoSys = new CSocketPool<CEchoSys>())
            {
                bool ok = spEchoSys.StartSocketPool(cc, 1, 1);
                CEchoSys es = spEchoSys.Lock();
                if (es == null)
                {
                    CClientSocket cs = spEchoSys.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("Echo sys not connected");
                    return;
                }

                MyStruct ms = new MyStruct();
                ms.AInt = 2056789;
                ms.ABool = true;
                ms.WString = "This is test wchar_t string";
                byte[] astr = { 10, 50, 95, 24 };
                ms.AString = astr;
                MyStruct res = es.EchoMyStruct(ms);

                string sOut;
                long lOut;
                long lData = 1234567890987;
                CUQueue q = new CUQueue();
                q.Save("TestWSTRING").Save(lData);
                CUQueue qRes = es.EchoUQueue(q);
                qRes.Load(out sOut).Load(out lOut);
                object objRes = es.EchoComplex0(1234.56, "testSTR", "MyTestMe", true, out sOut);
                uint nData = es.ToTest("李宇宙透露的信息显示，当天中国领馆人员还去了。𠝹𠱓𠱸𠲖𠳏𠳕𠴕", out sOut);
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
                    CClientSocket cs = spEchoSys.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
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

                //Console.WriteLine("Input a line to continue (TestQueueByEchoSys).....");
                //string s = Console.ReadLine();

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
                    });

                    ok = es.SendRequest(TEchoDConst.idEchoComplex0CEchoSys, 1234.56, "testSTR", "MyTestMe", true, (ar) =>
                    {
                        ar.Load(out sOut).Load(out objRes);
                    });

                    ok = es.SendRequest(TEchoDConst.idEchoComplex0CEchoSys + 1, sEchoTest, (ar) =>
                    {
                        ar.Load(out sOut).Load(out nData);
                    });
                    ok = es.AttachedClientSocket.ClientQueue.EndJob();
                }
                ok = es.WaitAll();
                //Console.WriteLine("Queue test completed, and input a line to stop");
                //tring str = Console.ReadLine();
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
					CClientSocket cs = spAdo.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("RAdo not connected");
                    return;
                }
                DataSet ds = ado.GetDataSet("select * from Person.Address", "select * from Purchasing.Vendor");
                ok = ado.SendDataSet(ds);
                ok = ado.WaitAll();
                DataTable dt = ado.GetDataReader("select * from Person.Person");
                ok = ado.SendDataTable(dt);
                dt = null;
            }
        }

        static void TestEchoObject(CConnectionContext cc)
        {
            using (CSocketPool<CEchoObject> spEchoObject = new CSocketPool<CEchoObject>())
            {
                bool ok = spEchoObject.StartSocketPool(cc, 1, 1);
                CEchoObject eo = spEchoObject.Lock();
                if (eo == null)
                {
					CClientSocket cs = spEchoObject.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("Echo object not connected");
                    return;
                }

                object res;
                object obj = null;
                res = eo.EchoEmpty(obj);
                Debug.Assert(res == null);

                obj = true;
                res = eo.EchoBool(obj);
                Debug.Assert((bool)res == (bool)obj);

                obj = (sbyte)110;
                res = eo.EchoInt8(obj);
                Debug.Assert((sbyte)res == (sbyte)obj);

                obj = (byte)212;
                res = eo.EchoUInt8(obj);
                Debug.Assert((byte)res == (byte)obj);

                obj = (short)-12345;
                res = eo.EchoInt16(obj);
                Debug.Assert((short)res == (short)obj);

                obj = (ushort)65000;
                res = eo.EchoUInt16(obj);
                Debug.Assert((ushort)res == (ushort)obj);

                obj = (int)-12345678;
                res = eo.EchoInt32(obj);
                Debug.Assert((int)res == (int)obj);

                obj = (uint)912345678;
                res = eo.EchoUInt32(obj);
                Debug.Assert((uint)res == (uint)obj);

                obj = (long)-123456789123;
                res = eo.EchoInt64(obj);
                Debug.Assert((long)res == (long)obj);

                obj = (ulong)912345678345678;
                res = eo.EchoUInt64(obj);
                Debug.Assert((ulong)res == (ulong)obj);

                obj = 123.56f;
                res = eo.EchoFloat(obj);
                Debug.Assert((float)res == (float)obj);

                obj = 6123.5676;
                res = eo.EchoDouble(obj);
                Debug.Assert((double)res == (double)obj);

                obj = "test me";
                res = eo.EchoString(obj);
                Debug.Assert((string)res == (string)obj);

                obj = 12345.67m;
                res = eo.EchoDecimal(obj);
                Debug.Assert((decimal)res == (decimal)obj);

                Guid guid = Guid.NewGuid();
                res = eo.EchoUUID(guid);
                //it is ok for Guid itself, but exception for boxed Guid
                //Debug.Assert(res == null);

                bool[] arrBool = { true, false };
                res = eo.EchoBoolArray(arrBool);

                decimal[] arrDec = { 10.24m, 2456.7891m };
                res = eo.EchoDecimalArray(arrDec);

                byte[] arrByte = { 213, 123, 45 };
                res = eo.EchoUInt8Array(arrByte);

                ushort[] arrUShort = { 21300, 12223, 45000 };
                res = eo.EchoUInt16Array(arrUShort);

                short[] arrShort = { -123, -2, 21732 };
                res = eo.EchoInt16Array(arrShort);

                uint[] arrUInt = { 21300000, 12223, 450009023 };
                res = eo.EchoUInt32Array(arrUInt);

                int[] arrInt = { -123, -2, 21732 };
                res = eo.EchoInt32Array(arrInt);

                long[] arrLong = { -123, -2, 21732 };
                res = eo.EchoInt64Array(arrLong);

                ulong[] arrULong = { 21300000, 12223, 450009023 };
                res = eo.EchoUInt64Array(arrULong);

                string[] arrString = { "", "test0", "againme", null };
                res = eo.EchoStringArray(arrString);

                DateTime dt = DateTime.Now;
                res = eo.EchoDateTime(dt);

                DateTime[] dtArray = new DateTime[3];
                dtArray[0] = DateTime.Now;
                dtArray[1] = dt.AddMilliseconds(-10);
                dtArray[2] = dt.AddMilliseconds(20);
                res = eo.EchoDateTimeArray(dtArray);
            }
        }

        static void TestEchoBasic(CConnectionContext cc)
        {
            CUQueue q = new CUQueue();
            using (CSocketPool<CEchoBasic> spEchoBasic = new CSocketPool<CEchoBasic>())
            {
                bool ok = spEchoBasic.StartSocketPool(cc, 1, 1);
                CEchoBasic eb = spEchoBasic.Lock();
                if (eb == null)
                {
					CClientSocket cs = spEchoBasic.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("Echo basic not connected");
                    return;
                }

                ok = eb.AttachedClientSocket.ClientQueue.StartQueue("cyenet_queue", 30 * 24 * 3600);
                string userId = eb.AttachedClientSocket.UID;

                uint[] groups = { 1, 2, 7 };

                eb.AttachedClientSocket.Push.Subscribe(1, 2, 7);
                eb.AttachedClientSocket.Push.Publish("Test Message from Push", 1, 2, 7);
                eb.AttachedClientSocket.Push.SendUserMessage("AMessageToUser", "SocketPro");
                q.Save(1234567890123).Save("Test me");
                eb.AttachedClientSocket.Push.SendUserMessage("SocketPro", q.GetBuffer());
                eb.AttachedClientSocket.Push.Publish(q.GetBuffer(), groups);

                eb.AttachedClientSocket.Push.Unsubscribe();

                string uid = eb.AttachedClientSocket.UID;
                uint port;
                string peerName = eb.AttachedClientSocket.GetPeerName(out port);

                ok = eb.EchoBool(ok);
                Debug.Assert(ok);

                string sInput = "李宇宙透露的信息显示，当天中国领馆人员还去了。𠝹𠱓𠱸𠲖𠳏𠳕𠴕";
                string sOut = eb.EchoString(sInput);
                Debug.Assert(sInput == sOut);

                decimal decInput = (decimal)27.865;
                decimal dec = eb.EchoDecimal(decInput);
                Debug.Assert(dec == decInput);

                double d = eb.EchoDouble(23.7654321);
                Debug.Assert(d == 23.7654321);

                float f = eb.EchoFloat(234.567f);
                Debug.Assert(f == 234.567f);

                Guid guidInput = System.Guid.NewGuid();
                Guid guidOut = eb.EchoGuid(guidInput);
                Debug.Assert(guidOut == guidInput);

                short s = 23456;
                short sRes = eb.EchoInt16(s);
                Debug.Assert(sRes == s);

                int nInput = -23456789;
                int nRes = eb.EchoInt32(nInput);
                Debug.Assert(nRes == nInput);

                long lInput = -123456789098756;
                long lRes = eb.EchoInt64(lInput);
                Debug.Assert(lRes == lInput);

                ushort us = 64321;
                ushort usOut = eb.EchoUInt16(us);
                Debug.Assert(usOut == us);

                uint un = 4000000561;
                uint unOut = eb.EchoUInt32(un);
                Debug.Assert(un == unOut);

                ulong ulInput = ulong.MaxValue - 200;
                ulong ulOut = eb.EchoUInt64(ulInput);
                Debug.Assert(ulInput == ulOut);

                sbyte sbInput = -23;
                sbyte sbOut = eb.EchoInt8(sbInput);
                Debug.Assert(sbInput == sbOut);

                byte bInput = 224;
                byte bOut = eb.EchoUInt8(bInput);
                Debug.Assert(bInput == bOut);

                char wc = '露';
                char wcOut = eb.EchoWChar(wc);
                Debug.Assert(wc == wcOut);

                sInput = "This is a test for ASCII string";
                byte[] bytes = Encoding.ASCII.GetBytes(sInput);
                sbyte[] sbytes = eb.EchoAString(bytes);
                sOut = CUQueue.ToString(sbytes);
                Debug.Assert(sInput == sOut);

                DateTime dtNow = DateTime.Now;
                DateTime dtOut = eb.EchoDateTime(dtNow);
                Debug.Assert(dtNow.Millisecond == dtOut.Millisecond && dtNow.Year == dtOut.Year);

            }
        }

        static void TestHelloWorldInParallel(CConnectionContext cc)
        {
            using (CSocketPool<CHwAsyncHandler> hwPool = new CSocketPool<CHwAsyncHandler>())
            {
                object cs = new object();
                bool ok = hwPool.StartSocketPool(cc, 1, 3);
                CHwAsyncHandler hw0 = hwPool.Lock();
                if (hw0 == null)
                {
					CClientSocket s = hwPool.Sockets[0];
                    Console.WriteLine("ErrorCode = " + s.ErrorCode + ", error message = " + s.ErrorMsg);
                    Console.WriteLine("Hello world service not connected");
                    return;
                }
                CHwAsyncHandler hw1 = hwPool.Lock();
                CHwAsyncHandler hw2 = hwPool.Lock();
                hw0.SendRequest(HwConst.idSayHello, "Hello world 0", (CAsyncResult ar) =>
                {
                    string res; ar.Load(out res);
                    lock (cs) Console.WriteLine(res);
                });
                hw1.SendRequest(HwConst.idSayHello, "Hello world 1", (CAsyncResult ar) =>
                {
                    string res; ar.Load(out res);
                    lock (cs) Console.WriteLine(res);
                });
                hw2.SendRequest(HwConst.idSayHello, "Hello world 2", (CAsyncResult ar) =>
                {
                    string res; ar.Load(out res);
                    lock (cs) Console.WriteLine(res);
                });
                hw0.WaitAll();
                hw1.WaitAll();
                hw2.WaitAll();
            }
        }

        static void TestHelloWorld(CConnectionContext cc)
        {
            using (CSocketPool<CHwAsyncHandler> hwPool = new CSocketPool<CHwAsyncHandler>())
            {
                bool ok = hwPool.StartSocketPool(cc, 1, 1);
                CHwAsyncHandler hw = hwPool.Lock();
                if (hw == null)
                {
					CClientSocket cs = hwPool.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("Hello world service not connected");
                    return;
                }
                Console.WriteLine(hw.SayHelloFromClient("UDAParts", 123));
            }
        }

        static void DoPerfStudy(CConnectionContext cc, uint cycles)
        {
			System.Threading.Thread.Sleep(100);
            using (CSocketPool<CPerfStudy> perf = new CSocketPool<CPerfStudy>())
            {
                string str = "";
                bool ok = perf.StartSocketPool(cc, 1, 1);
				CPerfStudy ps = perf.Lock();
                if (ps == null)
                {
					CClientSocket cs = perf.Sockets[0];
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
                    Console.WriteLine("Performance study service not connected");
                    return;
                }
                
                System.Diagnostics.Stopwatch sw = new Stopwatch();
                sw.Start();
                for (uint n = 0; n < cycles; ++n)
                {
                    str = ps.DoEcho("MyEcho");
                }
                sw.Stop();
                Console.WriteLine("DoEcho/Time in ms = " + sw.ElapsedMilliseconds.ToString());

                sw.Reset();
                sw.Start();
                for (uint n = 0; n < cycles; ++n)
                {
                    str = ps.DoSlowEcho("MyEcho");
                }
                sw.Stop();
                Console.WriteLine("DoSlowEcho/Time in ms = " + sw.ElapsedMilliseconds.ToString());
            }
        }

        static void TestAppend()
        {
            int n = 0;
            string clientQueueName = "TestAppend";
            List<IClientQueue> vQueue = new List<IClientQueue>();
            CConnectionContext cc = new CConnectionContext("127.0.0.1", uint.MaxValue, "SocketPro", "PassOne");
            CSocketPool<CHwAsyncHandler> pool = new CSocketPool<CHwAsyncHandler>();
            bool ok = pool.StartSocketPool(cc, 3, 1);
            foreach (CClientSocket s in pool.Sockets)
            {
                ok = s.ClientQueue.StartQueue(clientQueueName + n.ToString(), 3600 * 1000, true);
                ++n;
                vQueue.Add(s.ClientQueue);
            }

            CHwAsyncHandler src = null;
            CSocketPool<CHwAsyncHandler> srcPool = new CSocketPool<CHwAsyncHandler>();
            ok = srcPool.StartSocketPool(cc, 1, 1);
            IClientQueue cq = null;
            foreach (CHwAsyncHandler h in srcPool.AsyncHandlers)
            {
                src = h;
                cq = h.AttachedClientSocket.ClientQueue;
                ok = cq.StartQueue(clientQueueName + "_src", 3600 * 1000, false);
                break;
            }

            src.SayHelloFromClient("John Smith", 0);
            src.SayHelloFromClient("Hallen Clinton", 1);

            ok = cq.AppendTo(vQueue.ToArray());
        }

        static void TestServerQueue(CConnectionContext cc)
        {
            int n;
            CAsyncServiceHandler.DAsyncResultHandler ash = null;
            const int CYCLES = 100000;
            using (CSocketPool<CServerQueue> sqPool = new CSocketPool<CServerQueue>())
            {
                bool ok = sqPool.StartSocketPool(cc, 1, 1);
                if (ok)
                {
                    CServerQueue sq = sqPool.Lock();
                    DateTime dtPrev = DateTime.Now;
                    for (n = 0; n < CYCLES; ++n)
                    {
                        ok = sq.SendRequest(SQueueConst.idEnqueueCServerQueue, "Effect of reduced power consumption", "Hello ", n, ash);
                        if (sq.AttachedClientSocket.BytesInSendingBuffer > 60 * 1460)
                        {
                            if (!sq.WaitAll())
                                Console.WriteLine("Error = " + sq.AttachedClientSocket.ErrorCode + ", error message = " + sq.AttachedClientSocket.ErrorMsg);
                            /*
                            else
                                Console.WriteLine("Enqueue time = " + (DateTime.Now - dtPrev).TotalMilliseconds + ", quit = 0");
                            */
                        }
                    }
                    if (!sq.WaitAll())
                        Console.WriteLine("Error = " + sq.AttachedClientSocket.ErrorCode + ", error message = " + sq.AttachedClientSocket.ErrorMsg);
                    Console.WriteLine("Enqueue time = " + (DateTime.Now - dtPrev).TotalMilliseconds + ", quit = 0");
                    sq.SendRequest(SQueueConst.idQueryTimes, (ar) =>
                    {
                        long lSwitchTime;
                        long lEnqueueTime;
                        ar.Load(out lSwitchTime).Load(out lEnqueueTime);
                        Console.WriteLine("Time for switch = " + lSwitchTime + ", time for enqueue = " + lEnqueueTime);
                    });
                    ok = sq.WaitAll();
                    //n = Console.Read();
                }
            }
        }

        static string GetWorkDirectory()
        {
            switch (System.Environment.OSVersion.Platform)
            {
                case PlatformID.Win32NT:
                case PlatformID.Win32S:
                case PlatformID.Win32Windows:
                    return "c:\\cyetest\\";
                case PlatformID.WinCE:
                    return "";
                default:
                    return "/home/yye/cyetest/";
            }
        }

        static void Main(string[] args)
        {
            CClientSocket.QueueConfigure.WorkDirectory = GetWorkDirectory();
            TestSerializable();
            CClientSocket.QueueConfigure.MessageQueuePassword = "PasswordForMessageQueue";
            CClientSocket.SSL.SetVerifyLocation("ca.pem");
            CClientSocket.SSL.CertificateVerify += (preverified, depth, errCode, errMessage, ci) => {
                return true;
            };

            //TestAppend();
            //ConsoleApplication6.WCFRemPerf.TestWcf();
            //ConsoleApplication6.WCFRemPerf.TestRemoting();
            CConnectionContext cc = new CConnectionContext();
            cc.EncrytionMethod = tagEncryptionMethod.TLSv1;
            cc.Port = 20901;
            cc.UserId = ".NetUserId";
            cc.Password = "NetSecret";
            Console.WriteLine("Input a host .....");
            cc.Host = Console.ReadLine();

            TestServerQueue(cc);
            TestHelloWorld(cc);
            TestEchoBasic(cc);
            TestEchoObject(cc);
            TestEchoSys(cc);
            TestQueueByEchoSys(cc);
            TestMySysHandler(cc);
            DoPerfStudy(cc, 10000);
            TestAdo(cc);
        }

        static void TestMySysHandler(CConnectionContext cc)
        {
            long totalCycle = 10;
            using (CSocketPool<CMySvsHandler> mshPool = new CSocketPool<CMySvsHandler>())
            {
                bool ok = mshPool.StartSocketPool(cc, 1, 2);
                CMySvsHandler msh = mshPool.Lock();
                if (msh == null)
                {
					CClientSocket cs = mshPool.AsyncHandlers[0].AttachedClientSocket;
                    Console.WriteLine("ErrorCode = " + cs.ErrorCode + ", error message = " + cs.ErrorMsg);
					Console.WriteLine("No connection to CMySvsHandler");
                    return;
                }

                msh.AttachedClientSocket.BaseRequestProcessed += (sender, reqId) =>
                {
                    Console.WriteLine("Base request id = " + reqId.ToString());
                };

                byte[] sample = { 1, 2, 3, 4, 5, 6, 7 };
                var handlers = mshPool.AsyncHandlers;

                for (long n = 0; n < totalCycle; ++n)
                {
                    foreach (CMySvsHandler sh in handlers)
                    {
                        ok = sh.SendRequest(TestMeConst.idSleep, (uint)0, delegate(CAsyncResult ar)
                        {

                        });

                        ok = sh.SendRequest(TestMeConst.idBadRequest, (uint)123456, "This is a input", delegate(CAsyncResult ar)
                        {

                        });

                        ok = sh.SendRequest(TestMeConst.idDequeue, 100, delegate(CAsyncResult ar)
                        {

                        });

                        ok = sh.SendRequest(TestMeConst.idEcho, sample, delegate(CAsyncResult ar)
                        {

                        });
                    }

                    if (n != 0 && (n % 5) == 1)
                    {
                        foreach (CMySvsHandler sh in handlers)
                        {
                            sh.WaitAll();
                        }
                    }
                }
            }
        }
    }
}
