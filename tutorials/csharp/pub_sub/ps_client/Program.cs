using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;

class Program
{
    private static string ToString(CMessageSender ms)
    {
        return string.Format("Sender attributes = (ip = {0}, port = {1}, self = {2}, service id = {3}, userid = {4})", ms.IpAddress, ms.Port, ms.SelfMessage, ms.SvsID, ms.UserId);
    }

    private static string ToString(uint[] groups)
    {
        int n = 0;
        string s = "[";
        foreach (uint id in groups)
        {
            if (n != 0)
                s += ", ";
            s += id;
            ++n;
        }
        s += "]";
        return s;
    }

    static void Main(string[] args)
    {
        Console.WriteLine("Input your user id ......");
        CConnectionContext cc = new CConnectionContext("localhost", 20901, Console.ReadLine(), "MyPassword", tagEncryptionMethod.TLSv1);
        
        //CA file is located at the directory ..\SocketProRoot\bin
        //CClientSocket.SSL.SetVerifyLocation("ca.cert.pem"); //linux

        //for windows platforms, you can use windows system store instead
        CClientSocket.SSL.SetVerifyLocation("root"); //or "my", "my@currentuser", "root@localmachine"

        using (CSocketPool<HelloWorld> spHw = new CSocketPool<HelloWorld>()) //true -- automatic reconnecting
        {
            spHw.DoSslServerAuthentication += (sender, cs) =>
            {
                int errCode;
                IUcert cert = cs.UCert;
                Console.WriteLine(cert.SessionInfo);
                string res = cert.Verify(out errCode);
                //do ssl server certificate authentication here
                return (errCode == 0); //true -- user id and password will be sent to server
            };

            //error handling ignored for code clarity
            bool ok = spHw.StartSocketPool(cc, 1, 1);
            HelloWorld hw = spHw.Seek(); //or HelloWorld hw = spHw.Lock();
            
            CClientSocket ClientSocket = hw.AttachedClientSocket;
            ClientSocket.Push.OnSubscribe += (cs, messageSender, groups) =>
            {
                Console.WriteLine("Subscribe for " + ToString(groups));
                Console.WriteLine(ToString(messageSender));
                Console.WriteLine();
            };

            ClientSocket.Push.OnUnsubscribe += (cs, messageSender, groups) =>
            {
                Console.WriteLine("Unsubscribe from " + ToString(groups));
                Console.WriteLine(ToString(messageSender));
                Console.WriteLine();
            };

            ClientSocket.Push.OnPublish += (cs, messageSender, groups, msg) =>
            {
                Console.WriteLine("Publish to " + ToString(groups));
                Console.WriteLine(ToString(messageSender));
                Console.WriteLine("message = " + msg);
                Console.WriteLine();
            };

            ClientSocket.Push.OnSendUserMessage += (cs, messageSender, msg) =>
            {
                Console.WriteLine("SendUserMessage");
                Console.WriteLine(ToString(messageSender));
                Console.WriteLine("message = " + msg);
                Console.WriteLine();
            };

            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hwConst.idSayHelloHelloWorld, "Jack", "Smith", (ar) =>
            {
                string ret;
                ar.Load(out ret);
                Console.WriteLine(ret);
            });

            uint[] chat_ids = { 1, 2 };
            ok = ClientSocket.Push.Publish("We are going to call the method Sleep", chat_ids);
            CAsyncServiceHandler.DAsyncResultHandler arh = null;
            ok = hw.SendRequest(hwConst.idSleepHelloWorld, (int)5000, arh);

            Console.WriteLine("Input a receiver for receiving my message ......");
            Console.WriteLine();
            ok = ClientSocket.Push.SendUserMessage("A message from " + cc.UserId, Console.ReadLine());
            ok = hw.WaitAll();

            Console.WriteLine("Press key ENTER to shutdown the demo application ......");
            Console.ReadLine();
        }
    }
}

