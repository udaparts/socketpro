using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.ServiceModel;
using System.Reflection;

namespace ConsoleApplication6
{
    [ServiceContract]
    internal interface IRemote
    {
        [OperationContract]
        string Hello(string name);
    }

    [ServiceBehavior]
    internal class Remote : MarshalByRefObject, IRemote
    {
        public string Hello(string name)
        {
            //return string.Format("Hello, {0}!", name);
            return name;
        }
    }

    public class WCFRemPerf
    {
        private const int Iterations = 10000;

        public static void TestWcf()
        {
            var address = "net.pipe://localhost/test";

            var host = new ServiceHost(typeof(Remote));
            host.AddServiceEndpoint(typeof(IRemote), new NetNamedPipeBinding(NetNamedPipeSecurityMode.None), address);
            host.Open();

            var proxy = ChannelFactory<IRemote>.CreateChannel(new NetNamedPipeBinding(NetNamedPipeSecurityMode.None), new EndpointAddress(address));

            TestWcf(proxy, Iterations);
            TestWcf(proxy, Iterations);
            TestWcf(proxy, Iterations);
            TestWcf(proxy, Iterations);
            TestWcf(proxy, Iterations);

            Console.WriteLine("WCF done");

            host.Close();
        }

        private static void TestWcf(IRemote proxy, int iterations)
        {
            var start = DateTime.Now;

            for (var i = 0; i < iterations; i++)
            {
                proxy.Hello("MyEcho");
            }

            var stop = DateTime.Now;

            Console.WriteLine("Wcf: {0} ms.", (stop - start).TotalMilliseconds);
        }

        public static void TestRemoting()
        {
            var domain = AppDomain.CreateDomain("TestDomain");

            var proxy =
                (IRemote)
                domain.CreateInstanceFromAndUnwrap(Assembly.GetEntryAssembly().Location, "ConsoleApplication6.Remote");

            TestRemoting(proxy, Iterations);
            TestRemoting(proxy, Iterations);
            TestRemoting(proxy, Iterations);
            TestRemoting(proxy, Iterations);
            TestRemoting(proxy, Iterations);
            Console.WriteLine("Remoting done");
            Console.ReadKey();
        }

        private static void TestRemoting(IRemote proxy, int iterations)
        {
            var start = DateTime.Now;

            for (var i = 0; i < iterations; i++)
            {
                proxy.Hello("MyEcho");
            }

            var stop = DateTime.Now;

            Console.WriteLine("Remoting: {0} ms.", (stop - start).TotalMilliseconds);
        }
    }

}
