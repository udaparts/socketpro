
import SPA.*;
import SPA.UDB.*;
import java.util.*;

public class Test_java {

    public static void main(String[] args) {
        java.util.Scanner in = new java.util.Scanner(System.in);
        //load settings from some configuration file
        CConfig config = CConfig.getConfig();
        if (config.m_vccSlave.isEmpty() || config.m_slave_threads == 0 || config.m_sessions_per_host == 0 || config.m_nMasterSessions == 0) {
            System.out.println("Wrong settings for remote MySQL master and slave servers, and press any key to stop the server ......");
            in.nextLine();
            return;
        }
        CYourServer server = new CYourServer(config.m_main_threads);
        //start two socket pools, master and slave
        CYourServer.StartMySQLPools();

        CDataSet cache = CYourServer.Master.getCache();

        ArrayList<Pair<String, String>> v0 = CYourServer.Master.getCache().getDBTablePair();
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
        int res = cache.Find("main", "actor", 0, CTable.Operator.less, 12, tbl);
        res = cache.Between("main", "actor", 0, 1, 12, tbl);

        CDBVariantArray v = new CDBVariantArray();
        v.add(1);
        v.add(10);
        v.add(100);
        res = cache.In("main", "actor", 0, v, tbl);
        res = cache.NotIn("main", "actor", 0, v, tbl);
        res = 0;
        CYourServer.CreateTestDB();
        System.out.println("Starting middle tier server ......");
        if (config.m_store_or_pfx.endsWith(".pfx")) {
            //windows .pfx + password
            server.UseSSL(config.m_store_or_pfx, "", config.m_password_or_subject);
        } else if (config.m_key.endsWith(".pem")) {
            server.UseSSL(config.m_cert, config.m_key, config.m_password_or_subject);
        } else {
            //load cert and private key from windows system cert store
            server.UseSSL(config.m_store_or_pfx/*"my"*/, config.m_password_or_subject, "");
        }
        if (!server.Run(config.m_nPort, 32, !config.m_bNoIpV6)) {
            System.out.println("Error happens with error message = " + SPA.ServerSide.CSocketProServer.getErrorMessage());
        }
        System.out.println("Press any key to shut down the application ......");
        in.nextLine();
    }
}
