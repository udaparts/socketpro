
//#define USE_SQLCLIENT

using System;
using System.Data;
using DefMyCallsInterface;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Channels;
using System.Runtime.Remoting.Channels.Tcp;

namespace PerfSvr
{
    class Program
    {
        static void Main(string[] args)
        {
            CMyCallsImpl demoSvr = new CMyCallsImpl();
            TcpChannel tcp = new TcpChannel(21910);
            System.Runtime.Remoting.Channels.ChannelServices.RegisterChannel(tcp, false);
            RemotingConfiguration.RegisterWellKnownServiceType(typeof(DefMyCallsInterface.CMyCallsImpl), "MyCalls", WellKnownObjectMode.SingleCall);
            Console.WriteLine("Server started at port 21910");
            DataTable myTest = demoSvr.OpenRowset("select * from shippers");
            if(myTest != null)
                Console.WriteLine("Database connection established");
            Console.WriteLine("Press the key <ENTER> to terminate the application ......");
            Console.ReadLine();
        }
    }
}
