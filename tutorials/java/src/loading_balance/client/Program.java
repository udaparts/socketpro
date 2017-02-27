package loading_balance.client;

import SPA.*;
import SPA.ClientSide.*;
import loading_balance.piConst;

public class Program {

    public static void main(String[] args) {
        boolean ok;
        CClientSocket.QueueConfigure.setMessageQueuePassword("MyPwdForMsgQueue");
        if (CUQueue.DEFAULT_OS == tagOperationSystem.osWin) {
            CClientSocket.QueueConfigure.setWorkDirectory("c:\\sp_test");
        } else {
            CClientSocket.QueueConfigure.setWorkDirectory("/home/yye/sp_test/");
        }
        CConnectionContext cc = new CConnectionContext("localhost", 20901, "lb_client", "pwd_lb_client");
        CSocketPool<Pi> spPi = new CSocketPool<>(Pi.class, true); //true -- automatic reconnecting
        {
            ok = spPi.StartSocketPool(cc, 1, 1);
            CClientSocket cs = spPi.getSockets()[0];

            //use persistent queue to ensure auto failure recovery and at-least-once or once-only delivery
            ok = cs.getClientQueue().StartQueue("pi_queue", 24 * 3600, (cs.getEncryptionMethod() == tagEncryptionMethod.TLSv1));
            cs.getClientQueue().setRoutingQueueIndex(true);

            Pi pi = spPi.getAsyncHandlers()[0];
            ok = pi.WaitAll(); //make sure all existing queued requests are processed before executing next requests

            final double[] dPi = {0.0};
            int nDivision = 1000;
            int nNum = 10000000;
            double dStep = 1.0 / nNum / nDivision;
            final int[] nReturns = {0};
            for (int n = 0; n < nDivision; ++n) {
                double dStart = (double) n / nDivision;
                ok = pi.SendRequest(piConst.idComputePi, new CScopeUQueue().Save(dStart).Save(dStep).Save(nNum), new CAsyncServiceHandler.DAsyncResultHandler() {
                    @Override
                    public void invoke(CAsyncResult ar) {
                        dPi[0] += ar.LoadDouble();
                        ++nReturns[0];
                    }
                });
            }
            ok = pi.WaitAll();
            System.out.println("Your pi = " + dPi[0] + ", returns = " + nReturns[0]);
            System.out.println("Press ENTER key to shutdown the demo application ......");
            new java.util.Scanner(System.in).nextLine();
        }
    }
}
