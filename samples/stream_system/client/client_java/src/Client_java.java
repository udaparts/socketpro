
import SPA.*;
import SPA.UDB.*;
import SPA.ClientSide.*;
import java.util.concurrent.*;

public class Client_java {

    public static void main(String[] args) {
        System.out.println("Remote middle tier host: ");
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
        boolean ok = master.StartSocketPool(cc, 1, 1);
        if (!ok) {
            System.out.println("Failed in connecting to remote middle tier server, and press any key to close the application ......");
            in.nextLine();
            return;
        }
        CDataSet cache = master.getCache(); //accessing real-time update cache
        CWebAsyncHandler handler = master.Seek();
        ok = handler.GetMasterSlaveConnectedSessions((m, s) -> {
            System.out.format("master connections: %d, slave connections: %d%n", m, s);
        });
        ok = handler.QueryPaymentMaxMinAvgs(filter, (mma, res, errMsg) -> {
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
        ok = handler.UploadEmployees(vData, (res, errMsg, vId) -> {
            if (res != 0) {
                System.out.format("UploadEmployees Error code = %d, error message = %s%n", res, errMsg);
            } else {
                vId.stream().forEach((id) -> {
                    System.out.println("Last id: " + id);
                });
            }
            f.set(true);
        }, (h, canceled) -> {
            if (canceled) {
                System.out.println("Request canceled");
            } else {
                System.out.println("Socket closed");
            }
            f.set(true);
        });
        if (ok) {
            try {
                f.get(5000, TimeUnit.MILLISECONDS);
            } catch (InterruptedException | TimeoutException | ExecutionException err) {
                System.out.println("The above requests are not completed in 5 seconds");
            }
        } else {
            System.out.println("Socket already closed before sending request");
        }
        System.out.println("Press ENTER key to test requests parallel processing and fault tolerance at server side ......");
        in.nextLine();
        CMaxMinAvg sum_mma = new CMaxMinAvg();
        long start = System.currentTimeMillis();
        RefObject<Integer> returned = new RefObject<>(0);
        for (int n = 0; n < 10000; ++n) {
            if (!handler.QueryPaymentMaxMinAvgs(filter, (mma, res, errMsg) -> {
                if (res != 0) {
                    System.out.format("QueryPaymentMaxMinAvgs call error code: %d, error message: %s%n", res, errMsg);
                } else {
                    sum_mma.Avg += mma.Avg;
                    sum_mma.Max += mma.Max;
                    sum_mma.Min += mma.Min;
                }
                returned.Value += 1;
            })) {
                break;
            }
        }
        if (!handler.WaitAll()) {
            System.out.println("Socket closed");
        }
        System.out.format("Time required: %d milliseconds for %d requests%n", System.currentTimeMillis() - start, returned.Value);
        System.out.format("QueryPaymentMaxMinAvgs sum_max: %f, sum_min: %f, sum_avg: %f%n", sum_mma.Max, sum_mma.Min, sum_mma.Avg);
        System.out.println("Press ENTER key to test requests server parallel processing, fault tolerance and sequence returning ......");
        in.nextLine();
        SPA.RefObject<Long> prev_rental_id = new SPA.RefObject<>((long) 0);
        CWebAsyncHandler.DRentalDateTimes rdt = (dates, res, errMsg) -> {
            if (res != 0) {
                System.out.format("GetRentalDateTimes error code: %d, error message: %s%n", res, errMsg);
                prev_rental_id.Value = (long) 0;
            } else if (dates.rental_id == 0) {
                System.out.format("GetRentalDateTimes rental_id=%d not available%n", dates.rental_id);
                prev_rental_id.Value = (long) 0;
            } else {
                if (0 == prev_rental_id.Value || dates.rental_id == prev_rental_id.Value + 1) {
                    System.out.format("GetRentalDateTimes rental_id=%d and dates (%s, %s, %s)%n", dates.rental_id, dates.Rental.toString(), dates.Return.toString(), dates.LastUpdate.toString());
                } else {
                    System.out.println("****** GetRentalDateTimes returned out of order ******");
                }
                prev_rental_id.Value = dates.rental_id;
            }
        };
        //all requests should be returned in sequence (max rental_id = 16049)
        for (int n = 0; n < 1000; ++n) {
            if (!handler.GetRentalDateTimes(n + 1, rdt)) {
                break;
            }
        }
        if (!handler.WaitAll()) {
            System.out.println("Socket closed");
        }
        System.out.println("Press a key to shutdown the demo application ......");
        in.nextLine();
    }
}
