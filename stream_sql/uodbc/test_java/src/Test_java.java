
import SPA.ClientSide.*;
import SPA.UDB.*;
import SPA.*;
import java.math.BigDecimal;

public class Test_java {

    static void TestCreateTables(COdbc odbc, COdbc.DExecuteResult er) {
        String create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci";
        boolean ok = odbc.Execute(create_database, er);
        String use_database = "USE mysqldb";
        ok = odbc.Execute(use_database, er);
        String create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        ok = odbc.Execute(create_table, er);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64) NOT NULL,JoinDate DATETIME default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        ok = odbc.Execute(create_table, er);
        String drop_proc = "DROP PROCEDURE IF EXISTS sp_TestProc";
        ok = odbc.Execute(drop_proc, er);
        String create_proc = "CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary double,out p_last_dt datetime) BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END";
        ok = odbc.Execute(create_proc, er);
    }

    static void InsertBLOBByPreparedStatement(COdbc odbc, COdbc.DResult dr, COdbc.DExecuteResult er) {
        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
        String sqlInsert = "insert into employee(CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?)";
        boolean ok = odbc.Prepare(sqlInsert, dr);
        CDBVariantArray vData = new CDBVariantArray();

        CScopeUQueue sbBlob = new CScopeUQueue();

        //first set of data
        vData.add(1); //google company id
        vData.add("Ted Cruz");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(254000.15);

        //second set of data
        vData.add(1); //google company id
        vData.add("Donald Trump");
        vData.add(new java.util.Date());
        sbBlob.getUQueue().SetSize(0);
        sbBlob.Save(str);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(str);
        vData.add(20254000.23);

        //third set of data
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(6254000.02);

        //send three sets of parameterized data in one shot for processing
        ok = odbc.Execute(vData, er);
    }

    static CDBVariantArray TestBatch(COdbc odbc, COdbc.DExecuteResult er, COdbc.DRows r, COdbc.DRowsetHeader rh, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        String sql = "delete from employee;delete from company;"
                + "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);"
                + "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?);"
                + "SELECT * from company;select * from employee;select curtime();"
                + "call sp_TestProc(?,?,?)";

        CDBVariantArray vData = new CDBVariantArray();
        CUQueue sb = new CUQueue();

        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }

        //first set of data
        vData.add(1);
        vData.add("Google Inc.");
        vData.add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.add(66000000000.15);
        vData.add(1); //google company id
        vData.add("Ted Cruz");
        vData.add(new java.util.Date());
        sb.Save(wstr);
        vData.add(sb.GetBuffer());
        vData.add(wstr);
        vData.add(254000.15);
        vData.add(1);
        vData.add(new BigDecimal("2.35"));
        vData.add(0);

        //second set of data
        vData.add(2);
        vData.add("Microsoft Inc.");
        vData.add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.add(93600000000.21);
        vData.add(1); //google company id
        vData.add("Donald Trump");
        vData.add(new java.util.Date());
        sb.SetSize(0);
        sb.Save(str);
        vData.add(sb.GetBuffer());
        vData.add(str);
        vData.add(20254000.23);
        vData.add(2);
        vData.add(new BigDecimal("0.12"));
        vData.add(0);

        //third set of data
        vData.add(3);
        vData.add("Apple Inc.");
        vData.add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.add(234000000000.04);
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        sb.Save(wstr);
        vData.add(sb.GetBuffer());
        vData.add(wstr);
        vData.add(6254000.02);
        vData.add(0);
        vData.add(new BigDecimal("3.12"));
        vData.add(0);

        COdbc.DRowsetHeader batchHeader = new COdbc.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //called before rh, r and er
            }
        };

        COdbc.DDiscarded discarded = new COdbc.DDiscarded() {
            @Override
            public void invoke(CAsyncServiceHandler dbHandler, boolean canceled) {
                //called when canceling or socket closed if client queue is NOT used
            }
        };

        //first, execute delete from employee;delete from company
        //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        //third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        //fourth, SELECT * from company;select * from employee;select curtime()
        //last, three sets of call sp_TestProc(?,?,?)
        boolean ok = odbc.ExecuteBatch(tagTransactionIsolation.tiUnspecified, sql, vData, er, r, rh, batchHeader, null, tagRollbackPlan.rpDefault, discarded);
        return vData;
    }

    static void TestPreparedStatements(COdbc odbc, COdbc.DResult dr, COdbc.DExecuteResult er) {
        String sql_insert_parameter = "INSERT INTO company(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
        boolean ok = odbc.Prepare(sql_insert_parameter, dr);

        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.add(1);
        vData.add("Google Inc.");
        vData.add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.add(66000000000.15);

        //second set
        vData.add(2);
        vData.add("Microsoft Inc.");
        vData.add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.add(93600000000.21);

        //third set
        vData.add(3);
        vData.add("Apple Inc.");
        vData.add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.add(234000000000.04);

        ok = odbc.Execute(vData, er);
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20901;
        cc.UserId = "uodbc_client_java";
        cc.Password = "pwd_for_uodbc";

        CSocketPool<COdbc> spOdbc = new CSocketPool<>(COdbc.class, true, 600000);
        boolean ok = spOdbc.StartSocketPool(cc, 1, 1);
        COdbc odbc = spOdbc.getAsyncHandlers()[0];
        if (!ok) {
            System.out.println("No connection error code = " + odbc.getAttachedClientSocket().getErrorCode());
            in.nextLine();
            return;
        }
        COdbc.DResult dr = new COdbc.DResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg) {
                System.out.format("res = %d, errMsg: %s", res, errMsg);
                System.out.println();
            }
        };
        COdbc.DExecuteResult er = new COdbc.DExecuteResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg);
                System.out.println();
            }
        };

        ok = odbc.Open("dsn=ToMySQL;uid=root;pwd=Smash123", dr);
        final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra = new java.util.ArrayList<>();
        COdbc.DRows r = new COdbc.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        };

        COdbc.DRowsetHeader rh = new COdbc.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        };

        TestCreateTables(odbc, er);
        ok = odbc.Execute("delete from employee", er);
        ok = odbc.Execute("delete from company", er);
        TestPreparedStatements(odbc, dr, er);
        InsertBLOBByPreparedStatement(odbc, dr, er);
        ok = odbc.Execute("SELECT * from company", er, r, rh);
        ok = odbc.Execute("select * from employee", er, r, rh);
        ok = odbc.Execute("select curtime()", er, r, rh);

        final CDBVariantArray vPData = TestStoredProcedure(odbc, dr, er, ra);
        ok = odbc.WaitAll();

        final CDBVariantArray vData = TestBatch(odbc, er, r, rh, ra);
        ok = odbc.WaitAll();

        ok = odbc.Tables("sakila", "", "%", "TABLE", er, r, rh);
        ok = odbc.WaitAll();

        ok = odbc.Execute("use sakila", er);
        Pair<CDBColumnInfoArray, CDBVariantArray> tables = ra.get(ra.size() - 1);
        int columns = tables.first.size();
        int num_tables = tables.second.size() / columns;
        for (int n = 0; n < num_tables; ++n) {
            String sql = "select * from " + tables.second.get(n * columns + 2).toString();
            ok = odbc.Execute(sql, er, r, rh);
        }
        ok = odbc.WaitAll();

        int index = 0;
        System.out.println();
        System.out.println("+++++ Start rowsets +++");
        for (Pair<CDBColumnInfoArray, CDBVariantArray> a : ra) {
            System.out.format("Statement index = %d", index);
            if (a.first.size() > 0) {
                System.out.format(", rowset with columns = %d, records = %d.", a.first.size(), a.second.size() / a.first.size());
                System.out.println();
            } else {
                System.out.println(", no rowset received.");
            }
            ++index;
        }
        System.out.println("+++++ End rowsets +++");
        System.out.println();
        System.out.println("Press any key to close the application ......");
        in.nextLine();
    }

    static CDBVariantArray TestStoredProcedure(COdbc odbc, COdbc.DResult dr, COdbc.DExecuteResult er, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        boolean ok = odbc.Prepare("call sp_TestProc(?,?,?)", dr);
        COdbc.DRows r = new COdbc.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        };
        COdbc.DRowsetHeader rh = new COdbc.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        };
        CDBVariantArray vPData = new CDBVariantArray();
        //first set
        vPData.add(1);
        vPData.add(new BigDecimal("2.35"));
        vPData.add(0);

        //second set
        vPData.add(2);
        vPData.add(new BigDecimal("0.12"));
        vPData.add(0);
        ok = odbc.Execute(vPData, er, r, rh);
        return vPData;
    }
}
