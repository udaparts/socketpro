
import SPA.ClientSide.*;
import SPA.UDB.*;
import SPA.*;

public class test_java {

    public static void main(String[] args) {
        CConnectionContext cc = new CConnectionContext();
        System.out.println("Remote host: ");
        java.util.Scanner in = new java.util.Scanner(System.in);
        cc.Host = in.nextLine();
        cc.Port = 20901;
        cc.UserId = "usqlite_client_java";
        cc.Password = "pwd_for_usqlite";

        CSocketPool<CSqlite> spSqlite = new CSocketPool<>(CSqlite.class);
        boolean ok = spSqlite.StartSocketPool(cc, 1, 1);
        CSqlite sqlite = spSqlite.getAsyncHandlers()[0];
        if (!ok) {
            System.out.println("No connection error code = " + sqlite.getAttachedClientSocket().getErrorCode());
            in.nextLine();
            return;
        }
        ok = sqlite.Open("", new CSqlite.DResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg) {
                System.out.format("res = %d, errMsg: %s", res, errMsg);
                System.out.println();
            }
        });
        TestCreateTables(sqlite);
        java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> lstRowset = new java.util.ArrayList<>();

        ok = sqlite.BeginTrans();
        TestPreparedStatements(sqlite, lstRowset);
        InsertBLOBByPreparedStatement(sqlite, lstRowset);
        ok = sqlite.EndTrans();
        TestBatch(sqlite, lstRowset);
        sqlite.WaitAll();
        int index = 0;
        System.out.println();
        System.out.println("+++++ Start rowsets +++");
        for (Pair<CDBColumnInfoArray, CDBVariantArray> a : lstRowset) {
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

    static void TestBatch(CSqlite sqlite, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        CDBVariantArray vParam = new CDBVariantArray();
        vParam.add(1); //ID
        vParam.add(2); //EMPLOYEEID
        //there is no manual transaction if isolation is tiUnspecified
        boolean ok = sqlite.ExecuteBatch(tagTransactionIsolation.tiUnspecified,
                "Select datetime('now');select * from COMPANY where ID=?;select * from EMPLOYEE where EMPLOYEEID=?", vParam,
                new CSqlite.DExecuteResult() {
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                        System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                        System.out.println();
                    }
                }, new CSqlite.DRows() {
                    //rowset data come here
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                        int last = ra.size() - 1;
                        Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                        item.second.addAll(lstData);
                    }
                }, new CSqlite.DRowsetHeader() {
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler) {
                        //rowset header comes here
                        CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                        CDBVariantArray vData = new CDBVariantArray();
                        Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                        ra.add(item);
                    }
                });
        vParam.clear();
        vParam.add(1); //ID
        vParam.add(2); //EMPLOYEEID
        vParam.add(2); //ID
        vParam.add(3); //EMPLOYEEID
        //Same as sqlite.BeginTrans();
        //Select datetime('now');select * from COMPANY where ID=1;select * from COMPANY where ID=2;Select datetime('now');
        //select * from EMPLOYEE where EMPLOYEEID=2;select * from EMPLOYEE where EMPLOYEEID=3
        //ok = sqlite.EndTrans();
        ok = sqlite.ExecuteBatch(tagTransactionIsolation.tiReadCommited,
                "Select datetime('now');select * from COMPANY where ID=?;Select datetime('now');select * from EMPLOYEE where EMPLOYEEID=?", vParam,
                new CSqlite.DExecuteResult() {
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                        System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                        System.out.println();
                    }
                }, new CSqlite.DRows() {
                    //rowset data come here
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                        int last = ra.size() - 1;
                        Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                        item.second.addAll(lstData);
                    }
                }, new CSqlite.DRowsetHeader() {
                    @Override
                    public void invoke(CAsyncDBHandler dbHandler) {
                        //rowset header comes here
                        CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                        CDBVariantArray vData = new CDBVariantArray();
                        Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                        ra.add(item);
                    }
                });
    }

    static void TestPreparedStatements(CSqlite sqlite, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        String sql_insert_parameter = "Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID, NAME, ADDRESS, Income) VALUES (?, ?, ?, ?)";
        boolean ok = sqlite.Prepare(sql_insert_parameter, new CSqlite.DResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg) {
                System.out.format("res = %d, errMsg: %s", res, errMsg);
                System.out.println();
            }
        });

        CDBVariantArray vData = new CDBVariantArray();
        vData.add(1);
        vData.add("Google Inc.");
        vData.add("1600 Amphitheatre Parkway, Mountain View, CA 94043, USA");
        vData.add(66000000000.0);

        vData.add(2);
        vData.add("Microsoft Inc.");
        vData.add("700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA");
        vData.add(93600000000.0);

        vData.add(3);
        vData.add("Apple Inc.");
        vData.add("1 Infinite Loop, Cupertino, CA 95014, USA");
        vData.add(234000000000.0);

        //send three sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, new CSqlite.DExecuteResult() {

            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                System.out.println();
            }
        }, new CSqlite.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        }, new CSqlite.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        });
    }

    static void InsertBLOBByPreparedStatement(CSqlite sqlite, final java.util.ArrayList<Pair<CDBColumnInfoArray, CDBVariantArray>> ra) {
        String wstr = "";
        while (wstr.length() < 128 * 1024) {
            wstr += "近日，一则极具震撼性的消息，在中航工业的干部职工中悄然流传：中航工业科技委副主任、总装备部先进制造技术专家组组长、原中航工业制造所所长郭恩明突然失联。老郭突然失联，在中航工业和国防科技工业投下了震撼弹，也给人们留下了难以解开的谜团，以正面形象示人的郭恩明，为什么会涉足谍海，走上不归路，是被人下药被动失足？还是没能逃过漂亮“女间谍“的致命诱惑？还是仇视社会主义，仇视航空工业，自甘堕落与国家与人民为敌？";
        }
        String str = "";
        while (str.length() < 256 * 1024) {
            str += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.";
        }
        String sqlInsert = "insert or replace into employee(EMPLOYEEID, CompanyId, name, JoinDate, image, DESCRIPTION, Salary) values(?, ?, ?, ?, ?, ?, ?);select * from employee where employeeid = ?";
        boolean ok = sqlite.Prepare(sqlInsert, new CSqlite.DResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg) {
                System.out.format("res = %d, errMsg: %s", res, errMsg);
                System.out.println();
            }
        });

        CDBVariantArray vData = new CDBVariantArray();
        CScopeUQueue sbBlob = new CScopeUQueue();

        //first set of data
        vData.add(1);
        vData.add(1); //google company id
        vData.add("Ted Cruz");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(254000.0);
        vData.add(1);

        //second set of data
        vData.add(2);
        vData.add(1); //google company id
        vData.add("Donald Trump");
        vData.add(new java.util.Date());
        sbBlob.getUQueue().SetSize(0);
        sbBlob.Save(str);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(str);
        vData.add(20254000.0);
        vData.add(2);

        //third set of data
        vData.add(3);
        vData.add(2); //Microsoft company id
        vData.add("Hillary Clinton");
        vData.add(new java.util.Date());
        sbBlob.Save(wstr);
        vData.add(sbBlob.getUQueue().GetBuffer());
        vData.add(wstr);
        vData.add(6254000.0);
        vData.add(3);

        //send three sets of parameterized data in one shot for processing
        ok = sqlite.Execute(vData, new CSqlite.DExecuteResult() {

            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                System.out.println();
            }
        }, new CSqlite.DRows() {
            //rowset data come here
            @Override
            public void invoke(CAsyncDBHandler dbHandler, CDBVariantArray lstData) {
                int last = ra.size() - 1;
                Pair<CDBColumnInfoArray, CDBVariantArray> item = ra.get(last);
                item.second.addAll(lstData);
            }
        }, new CSqlite.DRowsetHeader() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler) {
                //rowset header comes here
                CDBColumnInfoArray vColInfo = dbHandler.getColumnInfo();
                CDBVariantArray vData = new CDBVariantArray();
                Pair<CDBColumnInfoArray, CDBVariantArray> item = new Pair<>(vColInfo, vData);
                ra.add(item);
            }
        });
    }

    static void TestCreateTables(CSqlite sqlite) {
        String create_table = "CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL, name CHAR(64) NOT NULL, ADDRESS varCHAR(256) not null, Income float not null)";
        boolean ok = sqlite.Execute(create_table, new CSqlite.DExecuteResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                System.out.println();
            }
        });
        create_table = "CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique, CompanyId INT8 not null, name NCHAR(64) NOT NULL, JoinDate DATETIME not null default(datetime('now')), IMAGE BLOB, DESCRIPTION NTEXT, Salary real, FOREIGN KEY(CompanyId) REFERENCES COMPANY(id))";
        ok = sqlite.Execute(create_table, new CSqlite.DExecuteResult() {
            @Override
            public void invoke(CAsyncDBHandler dbHandler, int res, String errMsg, long affected, long fail_ok, Object lastRowId) {
                System.out.format("affected = %d, fails = %d, oks = %d, res = %d, errMsg: %s, last insert id = %s", affected, (int) (fail_ok >> 32), (int) fail_ok, res, errMsg, lastRowId.toString());
                System.out.println();
            }
        });
    }
}
