package loading_balance.client;

import SPA.*;
import SPA.ClientSide.*;
import loading_balance.piConst;

public class Program {

    public static void main(String[] args) {
        System.out.println("Client: load balancer address:");
        CConnectionContext cc = new CConnectionContext(new java.util.Scanner(System.in).nextLine(), 20901, "lb_client", "pwd_lb_client");
        try (CSocketPool<Pi> spPi = new CSocketPool<>(Pi.class, true)) //true -- automatic reconnecting
        {
            boolean ok = spPi.StartSocketPool(cc, 1);
            CClientSocket cs = spPi.getSockets()[0];

            //use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
            ok = cs.getClientQueue().StartQueue("pi_queue", 24 * 3600, false);
            cs.getClientQueue().setRoutingQueueIndex(true);

            Pi pi = spPi.getAsyncHandlers()[0];
            ok = pi.WaitAll(); //make sure all existing queued requests are processed before executing next requests

            double dPi[] = {0.0};
            int nDivision = 1000;
            int nNum = 10000000;
            double dStep = 1.0 / nNum / nDivision;
            int nReturns[] = {0};
            for (int n = 0; n < nDivision; ++n) {
                double dStart = (double) n / nDivision;
                ok = pi.SendRequest(piConst.idComputePi, new CScopeUQueue().Save(dStart).Save(dStep).Save(nNum), (ar) -> {
                    dPi[0] += ar.LoadDouble();
                    ++nReturns[0];
                });
            }
            ok = pi.WaitAll();
            System.out.println("Your pi = " + dPi[0] + ", returns = " + nReturns[0]);
            System.out.println("Press ENTER key to shutdown the demo application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
