using System;
using System.Collections.Generic;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using SocketProAdapter.UDB;
using System.Threading.Tasks;

class Program
{
    /*
    //This is bad implementation for original SocketProAdapter.ClientSide.CAsyncDBHandler.Open method!!!!
    public virtual bool Open(string strConnection, DResult handler, uint flags) {
        string s = null;
        lock (m_csDB) {
            m_flags = flags;
            if (strConnection != null) {
                s = m_strConnection;
                m_strConnection = strConnection;
            }
            //self cross-SendRequest dead-locking here !!!!
            if (SendRequest(idOpen, strConnection, flags, (ar) => {
                int res, ms; string errMsg;
                ar.Load(out res).Load(out errMsg).Load(out ms);
                lock (m_csDB) { //self dead-lock !!!!
                    CleanRowset(); m_dbErrCode = res; m_lastReqId = idOpen;
                    if (res == 0) {
                        m_strConnection = errMsg; errMsg = "";
                    }
                    else
                        m_strConnection = "";
                    m_dbErrMsg = errMsg; m_ms = (tagManagementSystem)ms;
                    m_parameters = 0; m_indexProc = 0; m_output = 0;
                }
                if (handler != null)
                    handler(this, res, errMsg);
            })) {
                return true;
            }
            if (strConnection != null)
                m_strConnection = s;
        }
        return false;
    }
    */

    const string sample_database = "mysample.db";
    static void Demo_Cross_Locking_Dead_Lock(CSqlite sqlite)
    {
        uint count = 1000000;
        do
        {
            bool ok = sqlite.Open(sample_database, (handler, res, errMsg) =>
            {
                if (res != 0) Console.WriteLine("Open: res = {0}, errMsg: {1}", res, errMsg);
            });
            --count;
        } while (count > 0);
    }

    static void TestCreateTables(CSqlite sqlite)
    {
        string create_table = "CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL, NAME CHAR(64) NOT NULL)";
        bool ok = sqlite.Execute(create_table, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        });
        create_table = "CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))";
        ok = sqlite.Execute(create_table, (handler, res, errMsg, affected, fail_ok, id) =>
        {
            Console.WriteLine("affected = {0}, fails = {1}, oks = {2}, res = {3}, errMsg: {4}, last insert id = {5}", affected, (uint)(fail_ok >> 32), (uint)fail_ok, res, errMsg, id);
        });
    }

    static object m_csConsole = new object();
    static void StreamSQLsWithManualTransaction(CSqlite sqlite) {
        bool ok = sqlite.BeginTrans(tagTransactionIsolation.tiReadCommited, (h, res, errMsg) => {
            lock(m_csConsole)
                if (res != 0) Console.WriteLine("BeginTrans: Error code={0}, message={1}", res, errMsg);
        });
        ok = sqlite.Execute("delete from EMPLOYEE;delete from COMPANY", (h, res, errMsg, affected, fail_ok, id) => {
            lock (m_csConsole)
                if (res != 0) Console.WriteLine("Execute_Delete: affected={0}, fails={1}, res={2}, errMsg={3}", affected, (uint)(fail_ok >> 32), res, errMsg);
        });
        ok = sqlite.Prepare("INSERT INTO COMPANY(ID,NAME)VALUES(?,?)");
        CDBVariantArray vData = new CDBVariantArray();
        vData.Add(1); vData.Add("Google Inc.");
        vData.Add(2); vData.Add("Microsoft Inc.");
        //send two sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) => {
            lock (m_csConsole)
                if (res != 0) Console.WriteLine("Execute_Delete: affected={0}, fails={1}, res={2}, errMsg={3}", affected, (uint)(fail_ok >> 32), res, errMsg);
        });
        ok = sqlite.Prepare("INSERT INTO EMPLOYEE(EMPLOYEEID,CompanyId,name,JoinDate)VALUES(?,?,?,?)");
        vData.Clear();
        vData.Add(1); vData.Add(1); /*google company id*/ vData.Add("Ted Cruz"); vData.Add(DateTime.Now);
        vData.Add(2); vData.Add(1); /*google company id*/ vData.Add("Donald Trump"); vData.Add(DateTime.Now);
        vData.Add(3); vData.Add(2); /*Microsoft company id*/ vData.Add("Hillary Clinton"); vData.Add(DateTime.Now);
        //send three sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, (h, res, errMsg, affected, fail_ok, id) => {
            lock (m_csConsole)
                if (res != 0) Console.WriteLine("Execute_Delete: affected={0}, fails={1}, res={2}, errMsg={3}", affected, (uint)(fail_ok >> 32), res, errMsg);
        });
        sqlite.EndTrans(tagRollbackPlan.rpDefault, (h, res, errMsg) => {
            lock (m_csConsole)
                if (res != 0) Console.WriteLine("EndTrans: Error code={0}, message={1}", res, errMsg);
        });
    }

    const uint m_cycle = 100;
    static void Demo_Multiple_SendRequest_MultiThreaded_Wrong(object sp) {
        uint cycle = m_cycle; CSocketPool<CSqlite> spSqlite = (CSocketPool<CSqlite>)sp;
        while (cycle > 0) {
            //Seek an async handler on the min number of requests queued in memory and its associated socket connection
            CSqlite sqlite = spSqlite.Seek();
            StreamSQLsWithManualTransaction(sqlite);
            --cycle;
        }
        foreach (CSqlite s in spSqlite.AsyncHandlers) {
            s.WaitAll();
        }
    }
    static void Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(object sp) {
        uint cycle = m_cycle; CSocketPool<CSqlite> spSqlite = (CSocketPool<CSqlite>)sp;
        while (cycle > 0) {
            //Take an async handler infinitely from socket pool for sending multiple requests from current thread
            CSqlite sqlite = spSqlite.Lock();
            StreamSQLsWithManualTransaction(sqlite);
            //Put back a previously locked async handler to pool for reuse.
            spSqlite.Unlock(sqlite);
            --cycle;
        }
        foreach (CSqlite s in spSqlite.AsyncHandlers) {
            s.WaitAll();
        }
    }

    static void Main(string[] args) {
        Console.WriteLine("Remote host: "); string host = Console.ReadLine();
        CConnectionContext cc = new CConnectionContext(host, 20901, "usqlite_client", "pwd_for_usqlite");
        using (CSocketPool<CSqlite> spSqlite = new CSocketPool<CSqlite>()) {
            if (!spSqlite.StartSocketPool(cc, 1, 2)) {
                Console.WriteLine("No connection to sqlite server and press any key to close the demo ......");
                Console.Read(); return;
            }
            CSqlite sqlite = spSqlite.AsyncHandlers[0];
            //Use the above bad implementation to replace original SocketProAdapter.ClientSide.CAsyncDBHandler.Open method
            //at file socketpro/src/SproAdapter/asyncdbhandler.cs
            Console.WriteLine("Doing Demo_Cross_Locking_Dead_Lock ......");
            Demo_Cross_Locking_Dead_Lock(sqlite);
            TestCreateTables(sqlite);
            bool ok = sqlite.WaitAll(); sqlite = null;
            Console.WriteLine("mysample.db created, opened and shared by two sessions"); Console.WriteLine();
            CSqlite []vSqlite = spSqlite.AsyncHandlers;
            for (int n = 1; n < vSqlite.Length; ++n) {
                //make sure second handler/socket to open the same database mysample.db
                vSqlite[n].Open(sample_database, (handler, res, errMsg) => {
                    if (res != 0) Console.WriteLine("Open: res = {0}, errMsg: {1}", res, errMsg);
                }); ok = vSqlite[n].WaitAll();
            }
            //execute manual transactions concurrently with transaction overlapping on the same session at client side
            var tasks = new[] {
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Wrong, spSqlite),
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Wrong, spSqlite),
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Wrong, spSqlite)
            }; Demo_Multiple_SendRequest_MultiThreaded_Wrong(spSqlite); Task.WaitAll(tasks);
            Console.WriteLine("Demo_Multiple_SendRequest_MultiThreaded_Wrong completed"); Console.WriteLine();
            //execute manual transactions concurrently without transaction overlapping on the same session at client side by lock/unlock
            tasks = new[] {
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock, spSqlite),
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock, spSqlite),
                Task.Factory.StartNew(Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock, spSqlite)
            }; Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock(spSqlite); Task.WaitAll();
            Console.WriteLine("Demo_Multiple_SendRequest_MultiThreaded_Correct_Lock_Unlock completed"); Console.WriteLine();
            Console.WriteLine("Press any key to close the application ......"); Console.Read();
        }
    }
}
