using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Text;

namespace RouteOne
{
    class CAsyncRouteOne : CAsyncServiceHandler
    {
        public CAsyncRouteOne()
            : base(TEchoDConst.sidRouteSvs1)
        {

        }

        protected override void OnResultReturned(ushort sRequestId, CUQueue UQueue)
        {
            if (RouteeRequest)
            {
                switch (sRequestId)
                {
                    case TEchoDConst.idRouteFake1:
                        {
                            string s;
                            UQueue.Load(out s);
                            Console.WriteLine(s);
                        }
                        break;
                    case TEchoDConst.idRouteFake0:
                        {
                            string s;
                            UQueue.Load(out s);
                            s = "One * " + s;
                            Console.WriteLine(s);
                            UQueue.Save(s);
                        }
                        SendRouteeResult(UQueue);
                        break;
                    case TEchoDConst.idRouteStruct:
                        {
                            MyStruct ms;
                            UQueue.Load(out ms);
                            ms.ABool = (!ms.ABool);
                            ms.AInt += 1;
                            ms.WString = "One / " + ms.WString;
                            Console.WriteLine(ms.ToString());
                            SendRouteeResult(ms);
                        }
                        break;
                    case TEchoDConst.idREcho0:
                        {
                            byte[] bytes;
                            UQueue.Load(out bytes);
                            string str = CUQueue.ToString(bytes);
                            Console.WriteLine(str);
                            str = "One + " + str;
                            bytes = ASCIIEncoding.ASCII.GetBytes(str);
                            SendRouteeResult(bytes);
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
