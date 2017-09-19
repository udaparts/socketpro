package java_demo;

import SPA.ClientSide.*;
import SPA.UDB.*;

public class Java_demo {

    static final String sample_database = "mysample.db";

    static void Demo_Cross_Request_Dead_Lock(CSqlite sqlite) {
        boolean ok;
        int count = 1000000;
        //uncomment the following call to remove potential cross-request dead lock
        //ok = sqlite.getAttachedClientSocket().getClientQueue().StartQueue("cross_locking_0", 3600);
        do {
            ok = sqlite.Open(sample_database, (handler, res, errMsg) -> {
                if (res != 0) {
                    System.out.println("Open: res = " + res + ", errMsg: " + errMsg);
                }
            });
            --count;
        } while (count > 0 && ok);
    }

    static void TestCreateTables(CSqlite sqlite) {
        String sql = "CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL, NAME CHAR(64) NOT NULL)";
        boolean ok = sqlite.Execute(sql, (handler, res, errMsg, affected, fail_ok, id) -> {
            if (res != 0) {
                System.out.println("affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", oks = " + (int) fail_ok + ", res = " + res + ", errMsg: " + errMsg);
            }
        });
        sql = "CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
        ok = sqlite.Execute(sql, (handler, res, errMsg, affected, fail_ok, id) -> {
            if (res != 0) {
                System.out.println("affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", oks = " + (int) fail_ok + ", res = " + res + ", errMsg: " + errMsg);
            }
        });
    }

    static final Object m_csConsole = new Object();

    static void StreamSQLsWithManualTransaction(CSqlite sqlite) {
        boolean ok = sqlite.BeginTrans(tagTransactionIsolation.tiReadCommited, (h, res, errMsg) -> {
            if (res != 0) {
                synchronized (m_csConsole) {
                    System.out.println("BeginTrans: Error code=" + res + ", message=" + errMsg);
                }
            }
        });
        ok = sqlite.Execute("delete from EMPLOYEE;delete from COMPANY", (h, res, errMsg, affected, fail_ok, id) -> {
            if (res != 0) {
                synchronized (m_csConsole) {
                    System.out.println("Execute_Delete: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                }
            }
        });
        ok = sqlite.Prepare("INSERT INTO COMPANY(ID,NAME)VALUES(?,?)");
        CDBVariantArray vData = new CDBVariantArray();
        vData.add(1);
        vData.add("Google Inc.");
        vData.add(2);
        vData.add("Microsoft Inc.");
        //send two sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) -> {
            if (res != 0) {
                synchronized (m_csConsole) {
                    System.out.println("INSERT COMPANY: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                }
            }
        });
        ok = sqlite.Prepare("INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)");
        vData.clear();
        vData.add(1);
        vData.add(1); //google company id
        vData.add("Ted Cruz");
        vData.add(new java.util.Date());
        vData.add(2);
        vData.add(1); //google company id
        vData.add("Donald Trump");
        vData.add(new java.util.Date());
        vData.add(3);
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        //send three sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) -> {
            if (res != 0) {
                synchronized (m_csConsole) {
                    System.out.println("INSET EMPLOYEE: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                }
            }
        });
        sqlite.EndTrans(tagRollbackPlan.rpDefault, (h, res, errMsg) -> {
            if (res != 0) {
                synchronized (m_csConsole) {
                    System.out.println("EndTrans: Error code=" + res + ", message=" + errMsg);
                }
            }
        });
    }

    static final int m_cycle = 100;

    static void Demo_Multiple_SendRequest_MultiThreaded_Wrong(CSocketPool<CSqlite> spSqlite) {
        int cycle = m_cycle;
        while (cycle > 0) {
            //Seek an async handler on the min number of requests queued in memory and its associated socket connection
            CSqlite sqlite = spSqlite.Seek();
            //synchronized(sqlite) { //uncomment this call to remove potential batch request overlap
            StreamSQLsWithManualTransaction(sqlite);
            //}
            --cycle;
        }
        for (CSqlite s : spSqlite.getAsyncHandlers()) {
            s.WaitAll();
        }
    }

    static void Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(CSocketPool<CSqlite> spSqlite) {
        int cycle = m_cycle;
        while (cycle > 0) {
            //Take an async handler infinitely from socket pool for sending multiple requests from current thread
            CSqlite sqlite = spSqlite.Lock();
            StreamSQLsWithManualTransaction(sqlite);
            //Put back a previously locked async handler to pool for reuse.
            spSqlite.Unlock(sqlite);
            --cycle;
        }
        for (CSqlite s : spSqlite.getAsyncHandlers()) {
            s.WaitAll();
        }
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        cc.Host = new java.util.Scanner(System.in).next();
        cc.Port = 20901;
        cc.UserId = "usqlite_client_java";
        cc.Password = "pwd_for_usqlite";

        CSocketPool<CSqlite> spSqlite = new CSocketPool<>(CSqlite.class);
        boolean ok = spSqlite.StartSocketPool(cc, 1, 2);
        if (!ok) {
            System.out.println("No connection to sqlite server and press any key to close the demo ......");
            new java.util.Scanner(System.in).nextLine();
            return;
        }
        CSqlite sqlite = spSqlite.getAsyncHandlers()[0];
        //Use the above bad implementation to replace original SPA.ClientSide.CAsyncDBHandler.Open method
        //at file socketpro/src/jadpater/jspa/src/SPA/ClientSide/CAsyncDBHandler.java
        System.out.println("Doing Demo_Cross_Request_Dead_Lock ......");
        Demo_Cross_Request_Dead_Lock(sqlite);
        TestCreateTables(sqlite);
        ok = sqlite.WaitAll();
        System.out.println(sample_database + " created, opened and shared by two sessions");
        CSqlite[] vSqlite = spSqlite.getAsyncHandlers();
        for (int n = 1; n < vSqlite.length; ++n) {
            //make sure all other handlers/sockets to open the same database mysample.db
            vSqlite[n].Open(sample_database, (handler, res, errMsg) -> {
                if (res != 0) {
                    System.out.println("Open: res = " + res + ", errMsg: " + errMsg);
                }
            });
            ok = vSqlite[n].WaitAll();
        }
        //execute manual transactions concurrently with transaction overlapping on the same session at client side
        Runnable t0 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
        };
        Runnable t1 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
        };
        Runnable t2 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
        };
        Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
        t0.run();
        t1.run();
        t2.run();
        System.out.println("Demo_Multiple_SendRequest_MultiThreaded_Wrong completed");
        System.out.println("");

        //execute manual transactions concurrently without transaction overlapping on the same session at client side by lock/unlock
        Runnable t3 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
        };
        Runnable t4 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
        };
        Runnable t5 = () -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
        };
        Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
        t0.run();
        t1.run();
        t2.run();
        System.out.println("Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock completed");
        System.out.println("");
        System.out.println("Press any key to close the application ......");
        new java.util.Scanner(System.in).nextLine();
    }
}
