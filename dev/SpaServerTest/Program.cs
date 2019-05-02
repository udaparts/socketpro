using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Text;
using System.Threading;

namespace SpaServerTest
{
    class Program
    {
        static void TestCUQueue()
        {
            string strOut;
            DateTime dtOut;
            long lOut;
            DateTime dtNow = DateTime.Now;
            CUQueue q = new CUQueue();
            long lData = 12345;
            string str = "testme";

            q.Save(dtNow).Save(lData).Save(str).Load(out dtOut).Save(dtNow).Load(out lOut).Load(out strOut).Load(out dtOut);
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
            TestCUQueue();

            using (CMySocketProServer MySocketProServer = new CMySocketProServer(4))
            {
                CMySocketProServer.QueueManager.WorkDirectory = GetWorkDirectory();

                //MySocketProServer.UseSSL("server.pem", "server.pem", "test");
                if (!MySocketProServer.Run(20901))
                    Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString() + ", errMsg = " + CSocketProServer.ErrorMessage);
                else
                {
                    bool ok = CSocketProServer.Router.SetRouting(TEchoDConst.sidRouteSvs0, tagRoutingAlgorithm.raAverage, TEchoDConst.sidRouteSvs1, tagRoutingAlgorithm.raAverage);
                    ok = MySocketProServer.SetR0AlphaRequest();
                    ok = MySocketProServer.SetR1AlphaRequest();

                    CSocketProServer.PushManager.AddAChatGroup(1, "GroupOne");
                    CSocketProServer.PushManager.AddAChatGroup(2, "R&D Department");
                    CSocketProServer.PushManager.AddAChatGroup(7, "HR Department");
                    CSocketProServer.PushManager.AddAChatGroup(16, "Management Department");

                    CSocketProServer.QueueManager.MessageQueuePassword = "MyPassword";
                    CMyPeer.m_mq = CSocketProServer.QueueManager.StartQueue("net_test_queue", 2000);
                    CMyPeer.m_mq.CancelQueuedMessages(1, 20);
                    CMyPeer.m_mq.RemoveByTTL();

                    DateTime dt = CMyPeer.m_mq.LastMessageTime;

                    ok = CMyPeer.m_mq.StartJob();
                    ulong index = CMyPeer.m_mq.Enqueue(TestMeConst.idDoIdle, (long)0, (int)54321, Encoding.ASCII.GetBytes("Message from temp queue 0"));
                    index = CMyPeer.m_mq.Enqueue(TestMeConst.idDoIdle, (long)12345, (int)234, Encoding.ASCII.GetBytes("Temp queue message 1"));
                    ok = CMyPeer.m_mq.AbortJob();
                    index = CMyPeer.m_mq.Enqueue(TestMeConst.idDoIdle, (long)1234578, (int)1234, Encoding.ASCII.GetBytes("Temp me queue message 2"));
                    index = CMyPeer.m_mq.Enqueue(TestMeConst.idDoIdle, (long)23412345, (int)2342, Encoding.ASCII.GetBytes("Temp queue message again 3"));

                    CServerQueue sq0 = CSocketProServer.QueueManager.StartQueue("sq0", 200 * 60);
                    CServerQueue sq1 = CSocketProServer.QueueManager.StartQueue("sq1", 200 * 60);
                    CServerQueue sq2 = CSocketProServer.QueueManager.StartQueue("sq2", 200 * 60);

                    IServerQueue[] asq = { sq0, sq1, sq2 };
                    ok = CMyPeer.m_mq.AppendTo(asq);
                    sq0.Dispose();
                    sq1.Dispose();
                    sq2.Dispose();
                }
                Console.WriteLine("Input a line to close the application ......");
                Console.ReadLine();
                MySocketProServer.StopSocketProServer();
            }
        }
    }
}
