package pub_sub.client;

import hello_world.client.HelloWorld;
import SPA.ClientSide.*;

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
        //CConnectionContext cc = new CConnectionContext("localhost", 20901, scanner.nextLine(), "MyPassword");
        CConnectionContext cc = new CConnectionContext("localhost", 20901, scanner.nextLine(), "MyPassword", SPA.tagEncryptionMethod.TLSv1);

        //CA file is located at the directory ..\SocketProRoot\bin
        CClientSocket.SSL.SetVerifyLocation("ca.cert.pem"); //linux

        //for windows platforms, you can use windows system store instead
        //CClientSocket.SSL.SetVerifyLocation("my"); //or "root", "my@currentuser", "root@localmachine"
        CSocketPool<HelloWorld> spHw = new CSocketPool<>(HelloWorld.class); //true -- automatic reconnecting
        {
            spHw.DoSslServerAuthentication = new CSocketPool.DDoSslServerAuthentication() {
                @Override
                public boolean invoke(CSocketPool sender, CClientSocket cs) {
                    SPA.RefObject<Integer> errCode = new SPA.RefObject<>(0);
                    IUcert cert = cs.getUCert();
                    System.out.println(cert.SessionInfo);
                    String res = cert.Verify(errCode);

                    //do ssl server certificate authentication here
                    return (errCode.Value == 0); //true -- user id and password will be sent to server
                }
            };

            //error handling ignored for code clarity
            boolean ok = spHw.StartSocketPool(cc, 1, 1);
            HelloWorld hw = spHw.Seek(); //or HelloWorld hw = spHw.Lock();

            CClientSocket ClientSocket = hw.getAttachedClientSocket();
            ClientSocket.getPush().OnSubscribe = new DOnSubscribe() {
                @Override
                public void invoke(CClientSocket sender, CMessageSender messageSender, int[] groups) {
                    System.out.println("Subscribe for " + ToString(groups));
                    System.out.println(ToString(messageSender));
                    System.out.println();
                }
            };

            ClientSocket.getPush().OnUnsubscribe = new DOnUnsubscribe() {
                @Override
                public void invoke(CClientSocket sender, CMessageSender messageSender, int[] groups) {
                    System.out.println("Unsubscribe from " + ToString(groups));
                    System.out.println(ToString(messageSender));
                    System.out.println();
                }
            };

            ClientSocket.getPush().OnPublish = new DOnPublish() {
                @Override
                public void invoke(CClientSocket sender, CMessageSender messageSender, int[] groups, Object msg) {
                    System.out.println("Publish to " + ToString(groups));
                    System.out.println(ToString(messageSender));
                    System.out.println("message = " + msg);
                    System.out.println();
                }
            };

            ClientSocket.getPush().OnSendUserMessage = new DOnSendUserMessage() {
                @Override
                public void invoke(CClientSocket sender, CMessageSender messageSender, Object msg) {
                    System.out.println("SendUserMessage");
                    System.out.println(ToString(messageSender));
                    System.out.println("message = " + msg);
                    System.out.println();
                }
            };

            //asynchronously process multiple requests with inline batching for best network efficiency
            ok = hw.SendRequest(hello_world.hwConst.idSayHelloHelloWorld, new SPA.CScopeUQueue().Save("Jack").Save("Smith"), new CAsyncServiceHandler.DAsyncResultHandler() {
                @Override
                public void invoke(CAsyncResult ar) {
                    String ret = ar.LoadString();
                    System.out.println(ret);
                }
            });

            ok = ClientSocket.getPush().Publish("We are going to call the method Sleep", 1, 2);
            CAsyncServiceHandler.DAsyncResultHandler arh = null;
            ok = hw.SendRequest(hello_world.hwConst.idSleepHelloWorld, new SPA.CScopeUQueue().Save(5000), arh);

            System.out.println("Input a receiver for receiving my message ......");
            System.out.println();
            ok = ClientSocket.getPush().SendUserMessage("A message from " + cc.UserId, scanner.nextLine());
            ok = hw.WaitAll();

            System.out.println("Press key ENTER to shutdown the demo application ......");
            scanner.nextLine();
        }
    }
}
