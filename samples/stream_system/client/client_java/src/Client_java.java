
import SPA.*;
import SPA.ClientSide.*;
import java.util.*;

public class Client_java {

    public static void main(String[] args) {
        System.out.println("Remote middle tier host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        String host = in.nextLine();
        System.out.println("Sakila.payment filter: ");
        String filter = in.nextLine();
        CConnectionContext cc = new CConnectionContext(host, 20911, "SomeUserId", "A_Password_For_SomeUserId", tagEncryptionMethod.TLSv1);
        if (CUQueue.DEFAULT_OS != tagOperationSystem.osWin) {
            //CA file is located at the directory ../socketpro/bin
            CClientSocket.SSL.SetVerifyLocation("ca.cert.pem");
        }
        try (CMasterPool<CWebAsyncHandler> master = new CMasterPool<>(CWebAsyncHandler.class, "")) {
            master.DoSslServerAuthentication = (pool, cs) -> {
                IUcert cert = cs.getUCert();
                RefObject<Integer> ret = new RefObject<>(0);
                String res = cert.Verify(ret);
                return (ret.Value == 0);
            };
            //master.setQueueName("mcqueue");
            boolean ok = master.StartSocketPool(cc, 1);
            if (!ok) {
                System.out.println("No connection to remote middle tier server, and press any key to close the demo ......");
                in.nextLine();
                return;
            }
            //accessing real-time update cache
            CDataSet cache = master.getCache();
            CWebAsyncHandler handler = master.Seek();
            SPA.UDB.CDBVariantArray vData = new SPA.UDB.CDBVariantArray();
            vData.add(1); //Google company id
            vData.add("Ted Cruz");
            vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
            vData.add(1); //Google company id
            vData.add("Donald Trump");
            vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
            vData.add(2); //Microsoft company id
            vData.add("Hillary Clinton");
            vData.add(new java.sql.Timestamp(System.currentTimeMillis()));
            CUQueue sb = new CUQueue();
            try {
                UFuture<CScopeUQueue> fms = handler.sendRequest(Consts.idGetMasterSlaveConnectedSessions);
                sb.Save(filter);
                UFuture<CScopeUQueue> fmma = handler.sendRequest(Consts.idQueryMaxMinAvgs, sb);
                sb.SetSize(0);
                vData.SaveTo(sb);
                UFuture<CScopeUQueue> fue = handler.sendRequest(Consts.idUploadEmployees, sb);
                sb.SetSize(0);
                CScopeUQueue rb = fms.get();
                System.out.format("master connections: %d, slave connections: %d%n", rb.LoadInt(), rb.LoadInt());
                rb = fmma.get();
                int res = rb.LoadInt();
                String errMsg = rb.LoadString();
                CMaxMinAvg mma = rb.Load(CMaxMinAvg.class);
                if (res != 0) {
                    System.out.format("QueryPaymentMaxMinAvgs error code: %d, message: %s%n", res, errMsg);
                } else {
                    System.out.format("QueryPaymentMaxMinAvgs max: %f, min: %f, avg: %f%n", mma.Max, mma.Min, mma.Avg);
                }
                rb = fue.get(5, java.util.concurrent.TimeUnit.SECONDS);
                res = rb.LoadInt();
                errMsg = rb.LoadString();
                CLongArray vId = rb.Load(CLongArray.class);
                if (res != 0) {
                    System.out.format("UploadEmployees Error code: %d, message: %s%n", res, errMsg);
                } else {
                    vId.stream().forEach((id) -> {
                        System.out.println("Last id: " + id);
                    });
                }
                System.out.println("Press ENTER key to test requests parallel processing and fault tolerance at server side ......");
                in.nextLine();
                Deque<UFuture<CScopeUQueue>> qF = new ArrayDeque<>();
                sb.Save(filter);
                CMaxMinAvg sum_mma = new CMaxMinAvg();
                long start = System.currentTimeMillis();
                for (int n = 0; n < 10000; ++n) {
                    qF.add(handler.sendRequest(Consts.idQueryMaxMinAvgs, sb));
                }
                int returns = qF.size();
                while (qF.size() > 0) {
                    rb = qF.removeFirst().get();
                    res = rb.LoadInt();
                    errMsg = rb.LoadString();
                    if (res != 0) {
                        System.out.format("QueryPaymentMaxMinAvgs call error code: %d, message: %s%n", res, errMsg);
                    } else {
                        mma = rb.Load(CMaxMinAvg.class);
                        sum_mma.Avg += mma.Avg;
                        sum_mma.Max += mma.Max;
                        sum_mma.Min += mma.Min;
                    }
                }
                System.out.format("Time required: %d milliseconds for %d requests%n", System.currentTimeMillis() - start, returns);
                System.out.format("QueryPaymentMaxMinAvgs sum_max: %f, sum_min: %f, sum_avg: %f%n", sum_mma.Max, sum_mma.Min, sum_mma.Avg);
                System.out.println("Press ENTER key to test requests server parallel processing, fault tolerance and sequence returning ......");
                in.nextLine();
                for (long n = 0; n < 16000; ++n) {
                    sb.SetSize(0);
                    sb.Save(n + 1);
                    qF.add(handler.sendRequest(Consts.idGetRentalDateTimes, sb));
                }
                long prev_rental_id = 0;
                System.out.println("GetRentalDateTimes:");
                while (qF.size() > 0) {
                    rb = qF.removeFirst().get();
                    CRentalDateTimes dates = rb.Load(CRentalDateTimes.class);
                    res = rb.LoadInt();
                    errMsg = rb.LoadString();
                    if (res != 0) {
                        System.out.format("\terror code: %d, message: %s%n", res, errMsg);
                    } else if (dates.LastUpdate.getTime() <= 0 && dates.Rental.getTime() <= 0 && dates.Return.getTime() <= 0) {
                        System.out.format("\trental_id: %d not available%n", dates.rental_id);
                    } else {
                        if (0 == prev_rental_id || dates.rental_id == prev_rental_id + 1) {
                            //System.out.format("\trental_id: %d and dates (%s, %s, %s)%n", dates.rental_id, dates.Rental.toString(), dates.Return.toString(), dates.LastUpdate.toString());
                        } else {
                            System.out.println("\t****** returned out of order ******");
                        }
                    }
                    prev_rental_id = dates.rental_id;
                }
            } catch (java.util.concurrent.TimeoutException ex) {
                System.out.println("The request UploadEmployees not completed in 5 seconds");
            } catch (CServerError | CSocketError ex) {
                System.out.println(ex);
            } catch (Exception ex) {
                System.out.println(ex);
            }
            System.out.println("Press a key to shutdown the demo application ......");
            in.nextLine();
        }
    }
}
