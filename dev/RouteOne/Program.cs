using System;
using System.Collections.Generic;
using System.Text;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using RouteOne;
using System.Linq;

namespace RouteOne
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
            CClientSocket.QueueConfigure.WorkDirectory = GetWorkDirectory();
            CClientSocket.QueueConfigure.MessageQueuePassword = "MyPwdForMsgQueue";
            Console.WriteLine("Routor host address .....");
            string host = Console.ReadLine();
            CConnectionContext cc = new CConnectionContext(host, 20901, "RouteeUserId", "RPassword");
            using (CSocketPool<CAsyncRouteOne> aroPool = new CSocketPool<CAsyncRouteOne>())
            {
                ok = aroPool.StartSocketPool(cc, 1, 1);
                CAsyncRouteOne aro = aroPool.AsyncHandlers[0];

                aro.AttachedClientSocket.BaseRequestProcessed += (sender, bid) =>
                {
                    switch (bid)
                    {
                        case tagBaseRequestID.idRouteeChanged:
                            Console.WriteLine("Routee changed with count = " + sender.RouteeCount);
                            break;
                        default:
                            Console.WriteLine("Base request id = " + bid.ToString());
                            break;
                    }
                };
                ok = aro.AttachedClientSocket.ClientQueue.StartQueue("RouteOne", 720 * 3600);
                ok = aro.SendRequest(TEchoDConst.idCheckRouteeServiceId, (ar) =>
                {
                    uint svsId;
                    ar.Load(out svsId);
                    svsId = 0;
                });
                ok = aro.WaitAll();
                Console.WriteLine("Wait a line to contine or quit (Q) .....");
                string str = Console.ReadLine();
                while (ok && str != "Q" && aro.AttachedClientSocket.RouteeCount > 0)
                {
                    string msg = "--- 1One ---";
                    byte[] bytes = ASCIIEncoding.ASCII.GetBytes(msg);

                    //ok = aro.StartBatching();
                    ok = aro.SendRequest(TEchoDConst.idREcho1, bytes, (ar) =>
                    {
                        byte[] astr;
                        ar.Load(out astr);
                        string strReturn = CUQueue.ToString(astr);
                        Console.WriteLine(strReturn + ", routee count = " + aro.AttachedClientSocket.RouteeCount);
                    });

                    ok = aro.SendRequest(TEchoDConst.idRouteComplex, 27.542, msg, (object)bytes, true, (ar) =>
                    {
                        msg = null;
                        object obj;
                        ar.Load(out msg).Load(out obj);
                        Console.WriteLine("&&& string = " + msg + ", object = " + CUQueue.ToString((byte[])obj) + " &&&");
                        obj = null;
                    });
                    //ok = aro.CommitBatching(false);

                    ok = aro.WaitAll();
                    Console.WriteLine("Wait a line to contine or quit (Q) .....");
                    str = Console.ReadLine();
                }
            }
        }
    }
}
