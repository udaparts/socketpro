
import SPA.*;
import SPA.ClientSide.*;
import SPA.UDB.*;
import java.util.*;

public class Test_java {

    @SuppressWarnings("unchecked")
    public static void main(String[] args) {
        java.util.Scanner in = new java.util.Scanner(System.in);
        //load settings from some configuration file
        try {
            CSpConfig sc = SpManager.SetConfig(true, "C:\\cyetest\\socketpro\\samples\\stream_system\\sp_config.json");
            CYourServer.Master = (CSqlMasterPool<CMysql>) SpManager.GetPool("masterdb");
            CYourServer.Slave = (CSqlMasterPool<CMysql>.CSlavePool) SpManager.GetPool("slavedb0");
            CYourServer.FrontCachedTables.add("sakila.actor");
            CYourServer.FrontCachedTables.add("sakila.language");
            CYourServer.FrontCachedTables.add("sakila.country");
            CYourServer server = new CYourServer(2);
            CDataSet cache = CYourServer.Master.getCache();
            ArrayList<Pair<String, String>> v0 = cache.getDBTablePair();
            if (v0.isEmpty()) {
                System.out.println("There is no table cached");
            } else {
                System.out.println("Table cached:");
                v0.stream().forEach((p) -> {
                    System.out.format("DB name = %s, table name = %s%n", p.first, p.second);
                });
                HashMap<Integer, CDBColumnInfo> keys = cache.FindKeys(v0.get(0).first, v0.get(0).second);
                keys.forEach((k, v) -> {
                    System.out.format("Key ordinal = %d, key column name = %s%n", k, v.DisplayName);
                });
            }

            CTable tbl = new CTable();
            int res = cache.Find("sakila", "actor", 0, CTable.Operator.less, 12, tbl);
            res = cache.Between("sakila", "actor", 0, 1, 12, tbl);

            CDBVariantArray v = new CDBVariantArray();
            v.add(1);
            v.add(10);
            v.add(100);
            res = cache.In("sakila", "actor", 0, v, tbl);
            res = cache.NotIn("sakila", "actor", 0, v, tbl);
            res = 0;
            CYourServer.CreateTestDB();
            System.out.println("Starting middle tier server ......");
            if (CUQueue.DEFAULT_OS == tagOperationSystem.osWin) {
                server.UseSSL("C:\\cyetest\\socketpro\\bin\\intermediate.pfx", "", "mypassword");
            } else {
                server.UseSSL("intermediate.cert.pem", "intermediate.key.pem", "mypassword");
            }
            if (!server.Run(20911)) {
                System.out.println("Error happens with error message = " + SPA.ServerSide.CSocketProServer.getErrorMessage());
            }
        } catch (Exception err) {
            System.out.println(err.toString());
        }
        System.out.println("Press any key to shut down the application ......");
        in.nextLine();
    }
}
