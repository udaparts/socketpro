using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using RouteZero;
using System.Linq;

namespace RouteZero
{
    class Program
    {
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
            bool ok;
            uint index = 0;
            Console.WriteLine("Routor host address .....");
            string host = Console.ReadLine();
            CClientSocket.QueueConfigure.WorkDirectory = GetWorkDirectory();
            CClientSocket.QueueConfigure.MessageQueuePassword = "MyPwdForMsgQueue";
            CConnectionContext cc = new CConnectionContext(host, 20901, "RouteeUserId", "RPassword");

            using (CSocketPool<CAsyncRouteZero> arzPool = new CSocketPool<CAsyncRouteZero>())
            {
                ok = arzPool.StartSocketPool(cc, 1, 1);
                CAsyncRouteZero arz = arzPool.AsyncHandlers[0];
                arz.AttachedClientSocket.BaseRequestProcessed += (sender, bid) =>
                {
                    switch (bid)
                    {
                        case tagBaseRequestID.idRouteeChanged:
                            Console.WriteLine("Routee changed with count = " + arz.AttachedClientSocket.RouteeCount);
                            break;
                        default:
                            Console.WriteLine("Base request id = " + bid.ToString());
                            break;
                    }
                };
                ok = arz.AttachedClientSocket.ClientQueue.StartQueue("RouteZero", 720 * 3600);
                ok = arz.SendRequest(TEchoDConst.idRoutorClientCount, (ar) =>
                {
                    uint count;
                    ar.Load(out count);
                    count = 0;
                });
                ok = arz.WaitAll();
                Console.WriteLine("Wait a number to contine or quit (Q) .....");
                string str = Console.ReadLine();
                while (ok && str != "Q" && arz.AttachedClientSocket.RouteeCount > 0)
                {
                    string msg = "+++ 0 Test +++";
                    byte[] bytes = ASCIIEncoding.ASCII.GetBytes(msg);
                    //ok = arz.StartBatching();
                    ok = arz.SendRequest(TEchoDConst.idREcho0, bytes, (ar) =>
                    {
                        byte[] astr;
                        ar.Load(out astr);
                        string strReturn = CUQueue.ToString(astr);
                        Console.WriteLine(strReturn + ", routee count = " + arz.AttachedClientSocket.RouteeCount);
                    });

                    MyStruct ms = new MyStruct();
                    ms.ABool = true;
                    ms.AInt = index;
                    ms.AString = bytes;
                    ms.WString = msg;

                    ok = arz.SendRequest(TEchoDConst.idRouteStruct, ms, (ar) =>
                    {
                        ar.Load(out ms);
                        Console.WriteLine(arz.ToString());
                    });

                    ++index;

                    //ok = arz.CommitBatching(false);
                    ok = arz.WaitAll();
                    Console.WriteLine("Wait a line to continue (Q to end) .....");
                    str = Console.ReadLine();
                }
            }
        }
    }
}
