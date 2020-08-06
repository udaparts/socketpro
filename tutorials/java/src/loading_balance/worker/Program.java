package loading_balance.worker;

import SPA.ClientSide.*;

public class Program {

    public static void main(String[] args) {
        System.out.println("Worker: load balancer address:");
        CConnectionContext cc = new CConnectionContext(new java.util.Scanner(System.in).nextLine(), 20901, "lb_worker", "pwdForlb_worker");
        try (CSocketPool<PiWorker> spPi = new CSocketPool<>(PiWorker.class, true)) //true -- automatic reconnecting
        {
            if (!spPi.StartSocketPool(cc, 1, Runtime.getRuntime().availableProcessors())) {
                System.out.println("No connection at starting time ......");
            }
            System.out.println("Press key ENTER to shut down the application ......");
            String s = new java.util.Scanner(System.in).nextLine();
        }
    }
}
