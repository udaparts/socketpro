package loading_balance.worker;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext("127.0.0.1", 20901, "lb_worker", "pwdForlb_worker");
        try (CSocketPool<PiWorker> spPi = new CSocketPool<>(PiWorker.class, true)) //true -- automatic reconnecting
        {
            if (!spPi.StartSocketPool(cc, 1)) {
                System.out.println("No connection at starting time ......");
            }
            System.out.println("Press key ENTER to shut down the application ......");
            String s = new java.util.Scanner(System.in).nextLine();
        }
    }
}
