package java_demo;

import SPA.ClientSide.*;
import SPA.UDB.*;
import java.util.concurrent.*;

/*
 //This is bad implementation for original SPA.ClientSide.CAsyncDBHandler.Open method!!!!
 public boolean Open(String strConnection, DResult handler, int flags, DDiscarded discarded) {
 String str = null;
 MyCallback<DResult> cb = new MyCallback<>(DB_CONSTS.idOpen, handler);
 CUQueue sb = CScopeUQueue.Lock();
 sb.Save(strConnection).Save(flags);
 synchronized (m_csOneSending) {
 //don't make m_csDB locked across calling SendRequest, which may lead to client dead-lock
 //in case a client asynchronously sends lots of requests without use of client side queue.
 synchronized (m_csDB) { //start lock here
 m_flags = flags;
 if (strConnection != null) {
 str = m_strConnection;
 m_strConnection = strConnection;
 }
 m_deqResult.add(cb);
 //cross SendRequest dead lock here
 if (SendRequest(DB_CONSTS.idOpen, sb, null, discarded)) {
 CScopeUQueue.Unlock(sb);
 return true;
 } else {
 m_deqResult.remove(cb);
 if (strConnection != null) {
 m_strConnection = str;
 }
 }
 } //end lock -- m_csDB
 }
 CScopeUQueue.Unlock(sb);
 return false;
 }
 */
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
        //send three sets of parameterised data in one shot for processing
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

    static final int m_cycle = 10;

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

    static UFuture<Boolean> DoFuture(CSocketPool<CSqlite> sp) {
        CSqlite sqlite = sp.Lock();
        UFuture<Boolean> f = new UFuture<>();
        if (sqlite == null) {
            synchronized (m_csConsole) {
                System.out.println("All sockets are disconnected from server");
            }
            f.set(false);
            return f;
        }
        boolean ok = false;
        do {
            if (!sqlite.BeginTrans(tagTransactionIsolation.tiReadCommited, (h, res, errMsg) -> {
                if (res != 0) {
                    synchronized (m_csConsole) {
                        System.out.println("BeginTrans: Error code=" + res + ", message=" + errMsg);
                    }
                }
            })) {
                break;
            }
            if (!sqlite.Execute("delete from EMPLOYEE;delete from COMPANY", (h, res, errMsg, affected, fail_ok, id) -> {
                if (res != 0) {
                    synchronized (m_csConsole) {
                        System.out.println("Execute_Delete: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                    }
                }
            })) {
                break;
            }
            if (!sqlite.Prepare("INSERT INTO COMPANY(ID,NAME)VALUES(?,?)")) {
                break;
            }
            CDBVariantArray vData = new CDBVariantArray();
            vData.add(1);
            vData.add("Google Inc.");
            vData.add(2);
            vData.add("Microsoft Inc.");
            //send two sets of parameterised data in one shot for processing
            if (!sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) -> {
                if (res != 0) {
                    synchronized (m_csConsole) {
                        System.out.println("INSERT COMPANY: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                    }
                }
            })) {
                break;
            }
            if (!sqlite.Prepare("INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)")) {
                break;
            }
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
            if (!sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) -> {
                if (res != 0) {
                    synchronized (m_csConsole) {
                        System.out.println("INSET EMPLOYEE: affected = " + affected + ", fails = " + (int) (fail_ok >> 32) + ", res = " + res + ", errMsg: " + errMsg);
                    }
                }
            })) {
                break;
            }
            if (!sqlite.EndTrans(tagRollbackPlan.rpDefault, (h, res, errMsg) -> {
                if (res != 0) {
                    synchronized (m_csConsole) {
                        System.out.println("EndTrans: Error code= " + res + ", message= " + errMsg);
                    }
                }
                f.set(true);
            }, (h, canceled) -> {
                synchronized (m_csConsole) {
                    System.out.println("EndTrans: " + (canceled ? "Request canceled" : "Socket closed"));
                }
                f.set(false);
            })) {
                break;
            }
            sp.Unlock(sqlite);
            ok = true;
        } while (false);
        if (!ok) {
            f.set(false);
            //Socket is closed at server side and the above locked handler is automatically unlocked
            synchronized (m_csConsole) {
                System.out.println("LastWait: Connection disconnected error code = " + sqlite.getAttachedClientSocket().getErrorCode() + ", message= " + sqlite.getAttachedClientSocket().getErrorMsg());
            }
        }
        return f;
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20901;
        cc.UserId = "usqlite_client_java";
        cc.Password = "pwd_for_usqlite";

        CSocketPool<CSqlite> spSqlite = new CSocketPool<>(CSqlite.class);
        //start socket pool having 1 worker thread which hosts two non-block socket
        boolean ok = spSqlite.StartSocketPool(cc, 2, 1);
        if (!ok) {
            System.out.println("No connection to sqlite server and press any key to close the demo ......");
            in.nextLine();
            return;
        }
        CSqlite sqlite = spSqlite.getAsyncHandlers()[0];

        //Use the above bad implementation to replace original SPA.ClientSide.CAsyncDBHandler.Open method
        //at file socketpro/src/jadpater/jspa/src/SPA/ClientSide/CAsyncDBHandler.java
        System.out.println("Doing Demo_Cross_Request_Dead_Lock ......");
        Demo_Cross_Request_Dead_Lock(sqlite);

        //create two tables, COMPANY and EMPLOYEE
        TestCreateTables(sqlite);
        ok = sqlite.WaitAll();

        System.out.println(sample_database + " created, opened and shared by two sessions");

        //make sure all other handlers/sockets to open the same database mysample.db
        CSqlite[] vSqlite = spSqlite.getAsyncHandlers();
        for (int n = 1;
                n < vSqlite.length;
                ++n) {
            vSqlite[n].Open(sample_database, (handler, res, errMsg) -> {
                if (res != 0) {
                    System.out.println("Open: res = " + res + ", errMsg: " + errMsg);
                }
            });
            ok = vSqlite[n].WaitAll();
        }

        ExecutorService executor = Executors.newFixedThreadPool(3);

        //execute manual transactions concurrently with transaction overlapping on the same session at client side
        FutureTask<String> f0 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
            return "";
        });
        FutureTask<String> f1 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
            return "";
        });
        FutureTask<String> f2 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
            return "";
        });
        executor.execute(f0);
        executor.execute(f1);
        executor.execute(f2);
        Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite);
        try {
            String s = f0.get();
            s = f1.get();
            s = f2.get();
        } catch (InterruptedException | ExecutionException err) {
            System.out.println(err.getLocalizedMessage());
        }
        System.out.println("Demo_Multiple_SendRequest_MultiThreaded_Wrong completed");
        System.out.println("");

        //execute manual transactions concurrently without transaction overlapping on the same session at client side by lock/unlock
        f0 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
            return "";
        });
        f1 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
            return "";
        });
        f2 = new FutureTask<>(() -> {
            Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
            return "";
        });
        executor.execute(f0);
        executor.execute(f1);
        executor.execute(f2);
        Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite);
        try {
            String s = f0.get();
            s = f1.get();
            s = f2.get();
        } catch (InterruptedException | ExecutionException err) {
            System.out.println(err.getLocalizedMessage());
        }
        System.out.println("Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock completed");
        System.out.println("");
        executor.shutdown();

        System.out.println("Demonstration of DoFuture .....");
        try {
            if (DoFuture(spSqlite).get(5000, TimeUnit.MILLISECONDS)) {
                System.out.println("All requests within the function DoFuture are completed");
            } else {
                System.out.println("The requests within the function DoFuture are canceled partially");
            }
        } catch (TimeoutException | InterruptedException | ExecutionException err) {
            System.out.println("The requests within the function DoFuture are not completed in 5 seconds");
        }
        System.out.println("");

        System.out.println("Press any key to close the application ......");
        in.nextLine();
    }
}
