using System;
using System.Collections.Generic;
using System.Text;
using Suntico;
using Suntico.Client;
using SocketProAdapter;
using System.Data;

namespace SClientTest
{
    class Program
    {
        static CClientPoint client = null;
        static CCloudConnectionContext[] clouds = new CCloudConnectionContext[2];

        [MTAThread]
        static void Main(string[] args)
        {
            //set one master server and one or more backup cloud servers
            clouds[0] = new CCloudConnectionContext()
            {
                IpAddress = "localhost",
                Port = 20901,
                UserId = "SocketPro",
                Password = "PassOne",
                Secure = false,
                Zip = false
            };

            clouds[1] = new CCloudConnectionContext()
            {
                IpAddress = "127.0.0.1",
                Port = 20901,
                UserId = "SocketPro",
                Password = "PassOne",
                Secure = false,
                Zip = true
            };

            client = new CClientPoint(clouds);

            //set event handlers
            client.Failover += client_Failover;
            client.ChannelsClosed += client_ChannelsClosed;
            client.ChannelsOpened += client_CloudConnected;
            client.CloudMessage.OnGenericObject += new DGenericObject(CloudMessage_OnGenericObject);
            client.CloudMessage.OnEndTrans += new DEndTrans(CloudMessage_OnEndTrans);
            client.CloudMessage.OnStartTrans += new DStartTrans(CloudMessage_OnStartTrans);
            client.CloudMessage.OnGeneralMessage += new DGeneralMessage(CloudMessage_OnGeneralMessage);
            client.CloudMessage.OnStringObject += new DStringObject(CloudMessage_OnStringObject);
            client.CloudMessage.OnDataSet += new DDataSet(CloudMessage_OnDataSet);

            //initialize connecting
            client.Start();
            Console.WriteLine("Press any key to close the application ......");
            Console.Read();
            client.Dispose();
        }

        static void client_CloudConnected()
        {
            bool ok;
            CloudClientSharedForUnitTest.TestStructure ts = new CloudClientSharedForUnitTest.TestStructure()
            {
                n = 10,
                str = "ClientTest"
            };

            DataSet ds = new DataSet("MyDataSet");
            ds.ReadXmlSchema("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xsd");
            ds.ReadXml("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xml");

            DataTable dt = ds.Tables[0];
            ICustomerMessage cm = client.CustomerMessage;

            do
            {
                ok = (client.Start() == Suntico.ConnectionStatus.Opened);
                if (!ok)
                {
                    Console.WriteLine("Can not open channels to remote Suntico cloud servers");
                    break;
                }

                //start socket connections (channels)
                Suntico.ConnectionStatus cs = cm.BeginTrans(22,
                    delegate(long res)
                    {
                        Console.WriteLine("A remote Suntico cloud server just started a transaction with res = " + res.ToString());
                    }
                    , 30000
                );

                if (cs == Suntico.ConnectionStatus.Closed)
                {
                    Console.WriteLine("Remote Suntico cloud servers not available!");
                    break;
                }

                cm.Send("{\"AInt\":12345,\"AStr\":\"test\",\"ABool\":true}",
                    Suntico.StringObjectType.Json,
                    delegate()
                    {
                        Console.WriteLine("A remote Suntico cloud processed JSON text string");
                    }
                );

                cm.Send(1, ts, delegate()
                {
                    Console.WriteLine("A remote Suntico cloud processed a customer object");
                }
                );

                //You can send ADO.NET objects through load balancing now.
                cm.Send(ds);

                cs = cm.Commit(1,
                    delegate(long res)
                    {
                        Console.WriteLine("A remote Suntico cloud server just committed a transaction with res = " + res.ToString());
                    }
                );

                if (cs == Suntico.ConnectionStatus.Closed)
                {
                    Console.WriteLine("Remote Suntico cloud servers not available!");
                    break;
                }

                ok = cm.Wait(); //optional call for converting async into sync
                if (!ok)
                    Console.WriteLine("Remote Suntico cloud servers do not process transaction in 30 seconds!");

                if (client.Channels == 0)
                    Console.WriteLine("Remote Suntico cloud servers are closed!");

            } while (false);

            Console.WriteLine("Fails = " + client.Fails.ToString());
        }

        static void CloudMessage_OnDataSet(System.Data.DataSet ds)
        {
            Console.WriteLine("DataSet with name = {0} has {1} tables inside", ds.DataSetName, ds.Tables.Count);
        }

        static void CloudMessage_OnStringObject(Suntico.StringObjectType sot, string str)
        {
            Console.WriteLine("StringObject {0} from Suntico cloud server with type = " + sot.ToString(), str);
        }

        static void CloudMessage_OnGeneralMessage(string msg, int Group, int ServiceId)
        {
            Console.WriteLine("General message = " + msg + " from " + Group.ToString());
        }

        static void CloudMessage_OnStartTrans(long Clue)
        {
            Clue = 0;
        }

        static void CloudMessage_OnEndTrans(long Clue)
        {
            Console.WriteLine("Suntico ends a transaction with clue = " + Clue.ToString());
        }

        static void CloudMessage_OnGenericObject(long Clue, SocketProAdapter.CUQueue Queue)
        {
            switch (Clue)
            {
                case 1:
                    {
                        CloudClientSharedForUnitTest.TestStructure objFromCloudServer;
                        Queue.Load(out objFromCloudServer);
                        Console.WriteLine(objFromCloudServer.ToString());
                    }
                    break;
                default:
                    break;
            }
        }

        static void client_ChannelsClosed()
        {
            Console.WriteLine("Oops, all of channels are closed now!");
        }

        static void client_Failover()
        {
            Console.WriteLine("A remote Suntico cloud server is closed with {0} remaining channels", client.Channels);
        }
    }
}
