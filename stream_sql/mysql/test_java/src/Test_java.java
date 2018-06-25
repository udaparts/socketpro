
import SPA.ClientSide.*;
import SPA.UDB.*;
import SPA.*;

public class Test_java {

    static void TestCreateTables(CMysql mysql, CMysql.DExecuteResult er) {
        String create_database = "Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb";
        boolean ok = mysql.Execute(create_database, er);
        String create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        ok = mysql.Execute(create_table, er);
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        ok = mysql.Execute(create_table, er);
        String create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(20,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now() into p_last_dt;END";
        ok = mysql.Execute(create_proc, er);
    }

    static void InsertBLOBByPreparedStatement(CMysql mysql, CMysql.DResult dr, CMysql.DExecuteResult er) {
        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "广告�?�得�?那么夸张的就�?说了，看看这三家，都是正儿八�?的公立三甲，附属医院，�?是武警，也�?是部队，更�?是莆田，都在�?�生部门直接监管下，照样明目张胆地骗人。";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
        String sqlInsert = "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)";
        boolean ok = mysql.Prepare(sqlInsert, dr);
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
        vData.add(20254000.09);

        //third set of data
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(6254000.12);

        //send three sets of parameterized data in one shot for processing
        ok = mysql.Execute(vData, er);
    }

    static void TestPreparedStatements(CMysql mysql, CMysql.DResult dr, CMysql.DExecuteResult er) {
        String sql_insert_parameter = "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)";
        boolean ok = mysql.Prepare(sql_insert_parameter, dr);

        CDBVariantArray vData = new CDBVariantArray();

        //first set
        vData.add(1);
        vData.add("Google Inc.");
        vData.add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.add(66000000000.12);

        //second set
        vData.add(2);
        vData.add("Microsoft Inc.");
        vData.add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.add(93600000000.21);

        //third set
        vData.add(3);
        vData.add("Apple Inc.");
        vData.add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.add(234000000000.14);

        ok = mysql.Execute(vData, er);
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20902;
        cc.UserId = "root";
        cc.Password = "Smash123";

        CSocketPool<CMysql> spMysql = new CSocketPool<>(CMysql.class);
        boolean ok = spMysql.StartSocketPool(cc, 1, 1);
        CMysql mysql = spMysql.getAsyncHandlers()[0];
        if (!ok) {
            System.out.println("No connection error code = " + mysql.getAttachedClientSocket().getErrorCode());
            in.nextLine();
            return;
        }
        CMysql.DResult dr = new CMysql.DResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg) {
                System.out.format("res = %d, errMsg: %s", res, errMsg);
                System.out.println();
            }
        };
        CMysql.DExecuteResult er = new CMysql.DExecuteResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                System.out.println();
            }
        };

        ok = mysql.Open(null, dr);
        final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra = new java.util.ArrayList<>();
        TestCreateTables(mysql, er);
        ok = mysql.Execute("delete from employee;delete from company", er);
        TestPreparedStatements(mysql, dr, er);
        InsertBLOBByPreparedStatement(mysql, dr, er);
        ok = mysql.Execute("SELECT * from company;select * from employee;select curtime()", er, new CMysql.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        }, new CMysql.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        });
        CDBVariantArray vPData = new CDBVariantArray();
        //first set
        vPData.add(1);
        vPData.add(1.52);
        vPData.add(0);

        //second set
        vPData.add(2);
        vPData.add(2.11);
        vPData.add(0);

        TestStoredProcedure(mysql, dr, er, ra, vPData);
        CDBVariantArray vData = TestBatch(mysql, er, ra);
        ok = mysql.WaitAll();
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

    static CDBVariantArray TestBatch(CMysql mysql, CMysql.DExecuteResult er, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        //sql with delimiter '|'
        String sql = "delete from employee;delete from company|"
                + "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|"
                + "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|"
                + "SELECT * from company;select * from employee;select curtime()|"
                + "call sp_TestProc(?,?,?)";
        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }

        CDBVariantArray vData = new CDBVariantArray();
        CScopeUQueue sbBlob = new CScopeUQueue();

        //first set
        vData.add(1);
        vData.add("Google Inc.");
        vData.add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.add(66000000000.12);

        vData.add(1); //google company id
        vData.add("Ted Cruz");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(254000.15);

        vData.add(1);
        vData.add(1.52);
        vData.add(0);

        //second set
        vData.add(2);
        vData.add("Microsoft Inc.");
        vData.add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.add(93600000000.21);

        vData.add(1); //google company id
        vData.add("Donald Trump");
        vData.add(new java.util.Date());
        sbBlob.getUQueue().SetSize(0);
        sbBlob.Save(str);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(str);
        vData.add(20254000.09);

        vData.add(2);
        vData.add(2.11);
        vData.add(0);

        //third set
        vData.add(3);
        vData.add("Apple Inc.");
        vData.add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.add(234000000000.14);

        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(6254000.12);

        vData.add(0);
        vData.add(6.16);
        vData.add(0);

        CMysql.DRows r = new CMysql.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        };

        CMysql.DRowsetHeader rh = new CMysql.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        };

        CMysql.DRowsetHeader batchHeader = new CMysql.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //called one time only before calling rh, r and er
            }
        };

        CMysql.DDiscarded discarded = new CMysql.DDiscarded() {
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
        boolean ok = mysql.ExecuteBatch(tagTransactionIsolation.tiUnspecified, sql, vData, er, r, rh, batchHeader, null, tagRollbackPlan.rpDefault, discarded, "|");
        return vData;
    }

    static void TestStoredProcedure(CMysql mysql, CMysql.DResult dr, CMysql.DExecuteResult er, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vPData) {
        boolean ok = mysql.Prepare("call sp_TestProc(?,?,?)", dr);
        CMysql.DRows r = new CMysql.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        };

        CMysql.DRowsetHeader rh = new CMysql.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        };
        ok = mysql.Execute(vPData, er, r, rh);
    }
}
