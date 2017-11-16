
import SPA.*;
import SPA.UDB.*;
import SPA.ClientSide.*;
import java.util.concurrent.*;

public class Client_java {

    public static void main(String[] args) {
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        String host = in.nextLine();
        System.out.println("Sakila.payment filter: ");
        String filter = in.nextLine();
        CConnectionContext cc = new CConnectionContext(host, 20911, "SomeUserId", "A_Password_For_SomeUserId", tagEncryptionMethod.TLSv1);
        //CA file is located at the directory ../socketpro/bin
        CClientSocket.SSL.SetVerifyLocation("ca.cert.pem");
        CMasterPool<CWebAsyncHandler> master = new CMasterPool<>(CWebAsyncHandler.class, "");
        master.DoSslServerAuthentication = (pool, cs) -> {
            IUcert cert = cs.getUCert();
            RefObject<Integer> ret = new RefObject<>(0);
            String res = cert.Verify(ret);
            return (ret.Value == 0);
        };
        boolean ok = master.StartSocketPool(cc, 4, 1);
        if (!ok) {
            System.out.println("Failed in connecting to remote middle tier server, and press any key to close the application ......");
            in.nextLine();
            return;
        }
        CDataSet cache = master.getCache(); //accessing real-time update cache
        CWebAsyncHandler handler = master.Seek();
        handler.GetMasterSlaveConnectedSessions((index, m, s) -> {
            System.out.format("master connections: %d, slave connections: %d%n", m, s);
        });
        handler.QueryPaymentMaxMinAvgs(filter, (index, mma, res, errMsg) -> {
            if (res != 0) {
                System.out.format("QueryPaymentMaxMinAvgs error code: %d, error message: %s%n", res, errMsg);
            } else {
                System.out.format("QueryPaymentMaxMinAvgs max: %f, min: %f, avg: %f%n", mma.Max, mma.Min, mma.Avg);
            }
        });
        CDBVariantArray vData = new CDBVariantArray();
        vData.add(1); //Google company id
        vData.add("Ted Cruz");
        vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
        vData.add(1); //Google company id
        vData.add("Donald Trump");
        vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
        UFuture<Boolean> f = new UFuture<>();
        long call_index = handler.UploadEmployees(vData, (index, res, errMsg, vId) -> {
            if (res != 0) {
                System.out.format("UploadEmployees Error code = %d, error message = %s%n", res, errMsg);
            } else {
                vId.stream().forEach((id) -> {
                    System.out.println("Last id: " + id);
                });
            }
            f.set(true);
        }, (index) -> {
            System.out.println("Socket closed or request cancelled");
            f.set(true);
        });
        if (call_index != 0) {
            try {
                f.get(5000, TimeUnit.MILLISECONDS);
            } catch (InterruptedException | TimeoutException | ExecutionException err) {
                System.out.println("The above requests are not completed in 5 seconds");
            }
        } else {
            System.out.println("Socket already closed before sending request");
        }
        System.out.println("Press a key to test random returning ......");
        in.nextLine();
        CMaxMinAvg sum_mma = new CMaxMinAvg();
        long start = System.currentTimeMillis();
        RefObject<Integer> returned = new RefObject<>(0);
        handler = master.Seek(); //find a handler from a pool of sockets
        for (int n = 0; n < 10000; ++n) {
            if (handler.QueryPaymentMaxMinAvgs(filter, (index, mma, res, errMsg) -> {
                if (res != 0) {
                    System.out.format("QueryPaymentMaxMinAvgs call index: %d, error code: %d, error message: %s%n", index, res, errMsg);
                } else {
                    sum_mma.Avg += mma.Avg;
                    sum_mma.Max += mma.Max;
                    sum_mma.Min += mma.Min;
                    //System.out.println("QueryPaymentMaxMinAvgs call index = " + index);
                }
                returned.Value += 1;
            }) == 0) {
                break;
            }
        }
        handler.WaitAll();
        System.out.format("Time required: %d milliseconds for %d requests%n", System.currentTimeMillis() - start, returned.Value);
        System.out.format("QueryPaymentMaxMinAvgs sum_max: %f, sum_min: %f, sum_avg: %f%n", sum_mma.Max, sum_mma.Min, sum_mma.Avg);
        System.out.println("Press a key to test sequence returning ......");
        in.nextLine();
        CWebAsyncHandler.DRentalDateTimes rdt = (index, dates, res, errMsg) -> {
            if (res != 0) {
                System.out.format("GetRentalDateTimes call index: %d, error code: %d, error message: %s%n", index, res, errMsg);
            } else if (dates.rental_id == 0) {
                System.out.format("GetRentalDateTimes call index: %d rental_id=%d not available%n", index, dates.rental_id);
            } else {
                System.out.format("GetRentalDateTimes call index: %d rental_id=%d and dates (%s, %s, %s)%n", index, dates.rental_id, dates.Rental.toString(), dates.Return.toString(), dates.LastUpdate.toString());
            }
        };
        handler = master.Seek();
        for (int n = 0; n < 1000; ++n) {
            if (handler.GetRentalDateTimes(n + 1, rdt) == 0) {
                break;
            }
        }
        handler.WaitAll();
        System.out.println("Press a key to shutdown the demo application ......");
        in.nextLine();
    }
}
