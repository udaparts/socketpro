package loading_balance.client;

import SPA.*;
import SPA.ClientSide.*;
import loading_balance.piConst;
import java.util.*;

public class Program {

    public static void main(String[] args) {
        System.out.println("Client: load balancer address:");
        CConnectionContext cc = new CConnectionContext(new Scanner(System.in).nextLine(), 20901, "lb_client", "pwd_lb_client");
        try (CSocketPool<Pi> spPi = new CSocketPool<>(Pi.class))
        {
            spPi.setQueueName("lbqname");
            if (!spPi.StartSocketPool(cc, 1)) {
                System.out.println("No connection to " + cc.Host);
                System.out.println("Press ENTER key to kill the demo ......");
                return;
            }
            Pi pi = spPi.SeekByQueue();
            int nDivision = 1000, nNum = 10000000;
            double dStart, dPi = 0, dStep = 1.0 / nNum / nDivision;
            List<UFuture<CScopeUQueue>> list = new ArrayList<>();
            try (CScopeUQueue sb = new CScopeUQueue()) {
                try {
                    for (int n = 0; n < nDivision; ++n) {
                        dStart = (double) n / nDivision;
                        list.add(pi.sendRequest(piConst.idComputePi, sb.Save(dStart).Save(dStep).Save(nNum)));
                        sb.getUQueue().SetSize(0); //reset buffer
                    }
                } catch (CSocketError ex) {
                    System.out.println(ex);
                }
            }
            for (UFuture<CScopeUQueue> f : list) {
                try {
                    CScopeUQueue sb = f.get();
                    dPi += sb.LoadDouble();
                    System.out.println("dStart: " + sb.LoadDouble());
                } catch (CSocketError | CServerError ex) {
                    System.out.println(ex);
                } catch (Exception ex) {
                    //bad parameter, CUQueue de-serilization exception
                    System.out.println("Unexpected: " + ex.getMessage());
                }
            }
            System.out.println("pi: " + dPi + ", returns: " + list.size());
            System.out.println("Press ENTER key to kill the demo ......");
            new Scanner(System.in).nextLine();
        }
    }
}
