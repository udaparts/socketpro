package pub_sub.client;

import hello_world.client.HelloWorld;
import SPA.ClientSide.*;
import SPA.tagOperationSystem;

public class Program {

    private static String ToString(CMessageSender ms) {
        return String.format("Sender attributes = (ip = %s, port = %d, self = %b, service id = %d, userid = %s)", ms.IpAddress, ms.Port, ms.SelfMessage, ms.SvsID, ms.UserId);
    }

    private static String ToString(int[] groups) {
        String s = "[";
        if (groups != null) {
            int n = 0;
            for (int id : groups) {
                if (n != 0) {
                    s += ", ";
                }
                s += id;
                ++n;
            }
        }
        s += "]";
        return s;
    }

    public static void main(String[] args) {
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        System.out.println("Input your user id ......");
        CConnectionContext cc = new CConnectionContext("localhost", 20901, scanner.nextLine(), "MyPassword", SPA.tagEncryptionMethod.TLSv1);

        if (SPA.CUQueue.DEFAULT_OS != tagOperationSystem.osWin) {
            //CA file is located at the directory ../SocketProRoot/bin
            CClientSocket.SSL.SetVerifyLocation("ca.cert.pem"); //linux
        } else {
            //for windows platforms, you can use windows system store instead
            //"ca.cert.pem" already loaded into "root", "my@currentuser", "root@localmachine"
        }
        try (CSocketPool<HelloWorld> spHw = new CSocketPool<>(HelloWorld.class)) //true -- automatic reconnecting
        {
            spHw.DoSslServerAuthentication = (sender, cs) -> {
                SPA.RefObject<Integer> errCode = new SPA.RefObject<>(0);
                IUcert cert = cs.getUCert();
                System.out.println(cert.SessionInfo);
                String res = cert.Verify(errCode);

                //do ssl server certificate authentication here
                
                //true -- user id and password will be sent to server
                return (errCode.Value == 0);
            };

            //error handling ignored for code clarity
            boolean ok = spHw.StartSocketPool(cc, 1);
            HelloWorld hw = spHw.Seek(); //or HelloWorld hw = spHw.Lock();

            CClientSocket cs = hw.getSocket();
            cs.getPush().OnSubscribe = (sender, messageSender, groups) -> {
                System.out.println("Subscribe for " + ToString(groups));
                System.out.println(ToString(messageSender));
                System.out.println();
            };

            cs.getPush().OnUnsubscribe = (sender, messageSender, groups) -> {
                System.out.println("Unsubscribe from " + ToString(groups));
                System.out.println(ToString(messageSender));
                System.out.println();
            };

            cs.getPush().OnPublish = (sender, messageSender, groups, msg) -> {
                System.out.println("Publish to " + ToString(groups));
                System.out.println(ToString(messageSender));
                System.out.println("message = " + msg);
                System.out.println();
            };

            cs.getPush().OnSendUserMessage = (sender, messageSender, msg) -> {
                System.out.println("SendUserMessage");
                System.out.println(ToString(messageSender));
                System.out.println("message = " + msg);
                System.out.println();
            };

            //streaming multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hello_world.hwConst.idSayHello, new SPA.CScopeUQueue().Save("Jack").Save("Smith"), (ar) -> {
                String ret = ar.LoadString();
                System.out.println(ret);
            });

            ok = cs.getPush().Publish("We are going to call the method Sleep", 1, 2);
            ok = hw.SendRequest(hello_world.hwConst.idSleep, new SPA.CScopeUQueue().Save(5000), null);

            System.out.println("Input a receiver for receiving my message ......");
            System.out.println();
            ok = cs.getPush().SendUserMessage("A message from " + cc.UserId, scanner.nextLine());

            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
        }
    }
}
