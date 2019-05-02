using System;
using System.Data;
using Suntico.Server;
using SocketProAdapter.ServerSide;


namespace SServerTest
{
    class Program
    {
        static long ServerIndex = 0;
        static void Main(string[] args)
        {
            CSunticoServer MySocketProServer = new CSunticoServer();
            CSocketProServer.UseSSL("C:\\Program Files\\UDAParts\\SocketPro\\bin\\udacert.pfx", "mypassword", "udaparts", USOCKETLib.tagEncryptionMethod.MSTLSv1);
            bool ok = MySocketProServer.Run(20901);
            if (!ok)
                Console.WriteLine("Error code = " + CSocketProServer.LastSocketError.ToString());
            else
            {
                //the event will be raised from SocketPro worker thread
                MySocketProServer.Connected += new DConnected(MySocketProServer_Connected);

                //the event will be raised from SocketPro main thread
                MySocketProServer.Disconnected += new DDisconnected(MySocketProServer_Disconnected);
            }
            Console.WriteLine("Input a line quit to close the application ......");
            string str = Console.ReadLine();
            MySocketProServer.StopSocketProServer();
        }

        static void MySocketProServer_Disconnected(CSunticoPeer SunticoPeer)
        {
            Console.WriteLine("User id = {0} disconnected with index = {1}", SunticoPeer.UserID, SunticoPeer.ClientConnectionIndex);
        }

        static void MySocketProServer_Connected(CSunticoPeer SunticoPeer)
        {
            bool ok;
            DataSet ds = new DataSet();
            ds.ReadXmlSchema("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xsd");
            ds.ReadXml("C:\\chaohu\\suntico_consulting\\SComm\\Customer.xml");

            Console.WriteLine("User id = {0} connected with index = {1}", SunticoPeer.UserID, SunticoPeer.ClientConnectionIndex);

            CloudClientSharedForUnitTest.TestStructure ts = new CloudClientSharedForUnitTest.TestStructure()
            {
                n = 10,
                str = "ServerTest"
            };

            //the two events will be raised from SocketPro main thread
            SunticoPeer.OnClientStartTrans += SunticoPeer_OnClientStartTrans;
            SunticoPeer.OnClientEndTrans += SunticoPeer_OnClientEndTrans;

            //the five events will be raised from SocketPro worker thread
            SunticoPeer.OnClientDataReader += SunticoPeer_OnClientDataReader;
            SunticoPeer.OnClientDataSet += SunticoPeer_OnClientDataSet;
            SunticoPeer.OnClientStringObject += SunticoPeer_OnClientStringObject;
            SunticoPeer.OnGenericObject += SunticoPeer_OnGenericObject;
            SunticoPeer.OnClientDataTable += SunticoPeer_OnClientDataTable;

            string UserId = SunticoPeer.UserID;
            byte ClientConnIndex = SunticoPeer.ClientConnectionIndex;

            do
            {
                int[] group = { 1 };
                SunticoPeer.Push.Broadcast("test message", group);

                ok = SunticoPeer.SendBeginTrans(1);
                if (!ok)
                    break;
                ok = SunticoPeer.SendStringObject(Suntico.StringObjectType.Json, "{\"testc\" : 1}");
                if (!ok)
                    break;
                ok = SunticoPeer.SendGenericObject(1, ts);
                if (!ok)
                    break;
                ok = SunticoPeer.Send(ds, true) > 0; //directly use SocketProAdapter.ServerSide.CAdoClientPeer.Send
                if (!ok)
                    break;
                ok = SunticoPeer.SendEndTrans(delegate(long confirm)
                    {
                        //the event will be raised from SocketPro main thread
                        Console.WriteLine("Confirmed from client with number = " + confirm.ToString());
                    },
                2);
                if (!ok)
                    break;
            } while (false);
            if (!ok)
                Console.WriteLine("Client is offline");
        }

        static void SunticoPeer_OnClientDataTable(DataTable dt)
        {
            Console.WriteLine("Client sends a datatable with {0} columns inside", dt.Columns.Count);

            //do whatever you like here
        }

        static void SunticoPeer_OnGenericObject(long Clue, SocketProAdapter.CUQueue UQueue)
        {
            switch (Clue)
            {
                case 1:
                    {
                        CloudClientSharedForUnitTest.TestStructure ts;
                        int res = UQueue.Load(out ts);
                        Console.WriteLine("Client sends a generic object = '{0}'", ts.ToString());
                    }
                    break;
                default:
                    break;
            }
        }

        static void SunticoPeer_OnClientStringObject(Suntico.StringObjectType sot, string str)
        {
            Console.WriteLine("Client sends a string '{0} with object type = {1}", str, sot);
        }

        static void SunticoPeer_OnClientDataSet(DataSet ds)
        {
            Console.WriteLine("Client sends a dataset with name = {0} has {1} tables inside", ds.DataSetName, ds.Tables.Count);

            //do whatever you like here
        }

        static long SunticoPeer_OnClientEndTrans(long Clue)
        {
            Console.WriteLine("Client ends transaction with Clue = " + Clue.ToString());
            ++ServerIndex;
            return ServerIndex;
        }

        static long SunticoPeer_OnClientStartTrans(long Clue)
        {
            Console.WriteLine("Client starts transaction with Clue = " + Clue.ToString());
            ++ServerIndex;
            return ServerIndex;
        }

        static void SunticoPeer_OnClientDataReader(DataTable dt)
        {
            Console.WriteLine("Client sends a datareader with {0} columns inside", dt.Columns.Count);

            //do whatever you like here
        }
    }
}
