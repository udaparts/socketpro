using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Text;

namespace RouteZero
{
    class CAsyncRouteZero : CAsyncServiceHandler
    {
        public CAsyncRouteZero()
            : base(TEchoDConst.sidRouteSvs0)
        {
        }

        protected override void OnResultReturned(ushort sRequestId, CUQueue UQueue)
        {
            if (RouteeRequest)
            {
                switch (sRequestId)
                {
                    case TEchoDConst.idRouteFake0:
                        {
                            string s;
                            UQueue.Load(out s);
                            Console.WriteLine(s);
                        }
                        break;
                    case TEchoDConst.idRouteFake1:
                        {
                            string s;
                            UQueue.Load(out s);
                            s = "Zero * " + s;
                            Console.WriteLine(s);
                            UQueue.Save(s);
                        }
                        SendRouteeResult(UQueue);
                        break;
                    case TEchoDConst.idRouteComplex:
                        {
                            double d;
                            string s;
                            object simpleObj;
                            bool b;
                            UQueue.Load(out d).Load(out s).Load(out simpleObj).Load(out b);
                            Console.WriteLine("d = " + d + ", s = " + s + ", simbObject = " + CUQueue.ToString((byte[])simpleObj) + ", b = " + b);
                            SendRouteeResult(s, simpleObj);
                        }
                        break;
                    case TEchoDConst.idREcho1:
                        {
                            byte[] bytes;
                            UQueue.Load(out bytes);
                            string str = CUQueue.ToString(bytes);
                            Console.WriteLine(str);
                            str = "Zero + " + str;
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
