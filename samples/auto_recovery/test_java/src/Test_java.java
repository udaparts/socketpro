
import SPA.ClientSide.*;

public class Test_java {

    public static void main(String[] args) {
        final int sessions_per_host = 2;
        final int cycles = 10000;
        String[] vHost = {"localhost", "ws-yye-1"};
        java.util.Scanner scanner = new java.util.Scanner(System.in);
        CSocketPool<CSqlite> sp = new CSocketPool<CSqlite>(CSqlite.class);
        sp.setQueueName("ar_java");
        CConnectionContext[][] ppCc = new CConnectionContext[1][vHost.length * sessions_per_host]; //one thread enough
        for (int n = 0; n < vHost.length; ++n) {
            for (int j = 0; j < sessions_per_host; ++j) {
                ppCc[0][n * sessions_per_host + j] = new CConnectionContext(vHost[n], 20901, "AClientUserId", "Mypassword");
            }
        }
        boolean ok = sp.StartSocketPool(ppCc);
        if (!ok) {
            System.out.println("There is no connection and press any key to close the application ......");
            scanner.nextLine();
            return;
        }
        String sql = "SELECT max(amount), min(amount), avg(amount) FROM payment";
        System.out.println("Input a filter for payment_id");
        String filter = scanner.nextLine();
        if (filter.length() > 0) {
            sql += (" WHERE " + filter);
        }
        CSqlite[] v = sp.getAsyncHandlers();
        for (CSqlite h : v) {
            ok = h.Open("sakila.db", (handler, res, errMsg) -> {
                if (res != 0) {
                    System.out.println("Error code: " + res + ", error message: " + errMsg);
                }
            });
        }

        class CContext {

            public int returned = 0;
            public double dmax = 0.0, dmin = 0.0, davg = 0.0;
            public SPA.UDB.CDBVariantArray row = new SPA.UDB.CDBVariantArray();
        }
        final CContext context = new CContext();

        CAsyncDBHandler.DExecuteResult er = (h, res, errMsg, affected, fail_ok, lastId) -> {
            if (res != 0) {
                System.out.println("Error code: " + res + ", error message: " + errMsg);
            } else {
                context.dmax += (double) context.row.get(0);
                context.dmin += (double) context.row.get(1);
                context.davg += (double) context.row.get(2);
            }
            context.returned += 1;
        };
        CAsyncDBHandler.DRows r = (h, vData) -> {
            context.row.clear();
            context.row.addAll(vData);
        };
        CSqlite sqlite = sp.Seek();
        ok = sqlite.Execute(sql, er, r);
        ok = sqlite.WaitAll();

        System.out.println("Press key ENTER to shutdown the demo application ......");
        scanner.nextLine();
    }

}
