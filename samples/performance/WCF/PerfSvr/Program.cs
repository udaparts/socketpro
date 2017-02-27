using System;
using System.ServiceModel;
using DefMyCalls;

namespace PerfSvr
{
    class Program
    {
        static void Main(string[] args)
        {
            // Create a ServiceHost for the CalculatorService type.
            using (ServiceHost serviceHost = new ServiceHost(typeof(DefMyCalls.CMyCallsImpl)))
            {
                // Open the ServiceHost to create listeners and start listening for messages.
                serviceHost.Open();

                // The service can now be accessed.
                Console.WriteLine("The service is ready.");
                Console.WriteLine("Press <ENTER> to terminate service.");
                Console.ReadLine();
                serviceHost.Close();
            }

        }
    }
}
