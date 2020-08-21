import SPA.*;
import SPA.UDB.*;
import SPA.ClientSide.*;
import java.util.ArrayList;

public class Test_java {

    static ArrayList<UFuture<CMysql.SQLExeInfo>> TestCreateTables(CMysql mysql) throws CSocketError {
        ArrayList<UFuture<CMysql.SQLExeInfo>> v = new ArrayList<>();
        v.add(mysql.execute("Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb"));
        String create_table = "CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income decimal(15,2)not null)";
        v.add(mysql.execute(create_table));
        create_table = "CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))";
        v.add(mysql.execute(create_table));
        String create_proc = "DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary decimal(20,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now() into p_last_dt;END";
        v.add(mysql.execute(create_proc));
        return v;
    }

    static UFuture<CMysql.SQLExeInfo> InsertBLOBByPreparedStatement(CMysql mysql) throws CSocketError {
        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }

        mysql.Prepare("insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)");
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
        return mysql.execute(vData);
    }

    static UFuture<CMysql.SQLExeInfo> TestPreparedStatements(CMysql mysql) throws CSocketError {
        mysql.Prepare("INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)");
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

        return mysql.execute(vData);
    }

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20902;
        cc.UserId = "root";
        cc.Password = "Smash123";

        try (CSocketPool<CMysql> spMysql = new CSocketPool<>(CMysql.class)) {
            if (!spMysql.StartSocketPool(cc, 1)) {
                System.out.println("No connection error code = " + spMysql.getAsyncHandlers()[0].getSocket().getErrorCode());
                in.nextLine();
                return;
            }
            final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra = new java.util.ArrayList<>();
            CMysql mysql = spMysql.getAsyncHandlers()[0];
            try {
                UFuture<ErrInfo> fOpen = mysql.open("");
                UFuture<ErrInfo> fbt = mysql.beginTrans();
                ArrayList<UFuture<CMysql.SQLExeInfo>> vC = TestCreateTables(mysql);
                UFuture<CMysql.SQLExeInfo> fD = mysql.execute("delete from employee;delete from company");
                UFuture<ErrInfo> fet = mysql.endTrans();
                UFuture<CMysql.SQLExeInfo> fP0 = TestPreparedStatements(mysql);
                UFuture<CMysql.SQLExeInfo> fP1 = InsertBLOBByPreparedStatement(mysql);
                UFuture<CMysql.SQLExeInfo> fS = mysql.execute("SELECT * from company;select * from employee;select curtime()", (db, lstData) -> {
                    //rowset data come here
                    int last = ra.size() - 1;
                    Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                    item.second.addAll(lstData);
                }, (db) -> {
                    //rowset header comes here
                    CDBColumnInfoArray vColInfo = db.getColumnInfo();
                    CDBVariantArray vData = new CDBVariantArray();
                    Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                    ra.add(item);
                });
                CDBVariantArray vPData = new CDBVariantArray();
                UFuture<CMysql.SQLExeInfo> fStore = TestStoredProcedure(mysql, ra, vPData);

                CDBVariantArray vData = new CDBVariantArray();
                UFuture<CMysql.SQLExeInfo> fB = TestBatch(mysql, ra, vData);

                System.out.println("All SQL requests are streamed, and waiting for results ......");
                System.out.println(fOpen.get());
                System.out.println(fbt.get());
                for (UFuture<CMysql.SQLExeInfo> f : vC) {
                    System.out.println(f.get());
                }
                System.out.println(fD.get());
                System.out.println(fet.get());
                System.out.println(fP0.get());
                System.out.println(fP1.get());
                System.out.println(fS.get());
                System.out.println(fStore.get());
                System.out.println(fB.get());
            } catch (CSocketError | CServerError ex) {
                System.out.println(ex);
            }
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
    }

    static UFuture<CMysql.SQLExeInfo> TestBatch(CMysql mysql, java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vData) throws CSocketError {
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

        try (CScopeUQueue sbBlob = new CScopeUQueue()) {

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

            //first, start manual transaction
            //second, execute delete from employee;delete from company
            //third, prepare and execute three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
            //fourth, prepare and execute three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
            //fifth, SELECT * from company;select * from employee;select curtime()
            //sixth, prepare and execute three sets of call sp_TestProc(?,?,?)
            //last, commit all transaction if no error happens, and rollback if there is an error
            return mysql.executeBatch(tagTransactionIsolation.tiReadCommited, sql, vData, (db, lstData) -> {
                //rowset data come here
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }, (db) -> {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = db.getColumnInfo();
                CDBVariantArray v = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, v);
                ra.add(item);
            }, "|");
        }
    }

    static UFuture<CMysql.SQLExeInfo> TestStoredProcedure(CMysql mysql, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra, CDBVariantArray vPData) throws CSocketError {
        mysql.Prepare("call sp_TestProc(?,?,?)");
        //first set
        vPData.add(1);
        vPData.add(1.52);
        vPData.add(0);

        //second set
        vPData.add(2);
        vPData.add(2.11);
        vPData.add(0);
        return mysql.execute(vPData, (db, lstData) -> {
            //rowset data come here
            int last = ra.size() - 1;
            Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
            item.second.addAll(lstData);
        }, (db) -> {
            //rowset header comes here
            CDBColumnInfoArray vColInfo = db.getColumnInfo();
            CDBVariantArray vData = new CDBVariantArray();
            Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
            ra.add(item);
        });
    }
}

