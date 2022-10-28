package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"gspa/gdb"
	"testing"
	"time"
)

var cc2Mssql = gcs.CConnectionContext{Host: "windesk", UserId: "sa", Password: "Smash123", Port: 20901}

func Test_mssql_db_simple_calls_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Open("sakila", 0, func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f1 := db.BeginTrans(func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f2 := db.Prepare("select * from sakila..actor where actor_id=?", func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f3 := db.EndTrans(func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f4 := db.Close(func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	print_db(f0)
	print_db(f1)
	print_db(f2)
	print_db(f3)
	fmt.Println(db.GetDBManagementSystem(), db.GetParameters(), db.GetOutputs(), db.GetConnection(), db.IsOpened(), db.GetCallReturn())
	print_db(f4)
}

func Test_mssql_db_execute_sql(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("select * from sakila..actor where actor_id between 11 and 20", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
		fmt.Println("Data: ", vData)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_mssql_db_execute_SQLs_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("sakila", 0),
		db.Execute("use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE name='mydevdb')"+
			"BEGIN CREATE DATABASE mydevdb; END;USE mydevdb", nil, nil),
		db.Execute("IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company')create table "+
			"company(ID bigint PRIMARY KEY NOT NULL,name varCHAR(64)NOT NULL,ADDRESS varCHAR(256)"+
			"not null,Income float not null)", nil, nil),
		db.Execute("IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee')create table"+
			" employee(EMPLOYEEID bigint identity PRIMARY KEY,CompanyId bigint not null,name varCHAR(64)"+
			"NOT NULL,JoinDate DATETIME2(3)default null,MyIMAGE varbinary(max),DESCRIPTION nvarchar(max)"+
			",Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))", nil, nil),
		db.Execute("IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc')"+
			"drop proc sp_TestProc", nil, nil),
		db.Execute("CREATE PROCEDURE sp_TestProc(@p_company_id int,@p_sum_salary float OUT,"+
			"@p_last_dt datetime2(3) out)as select * from employee where companyid>=@p_company_id;"+
			"select @p_sum_salary=sum(salary)+@p_sum_salary from employee where companyid>="+
			"@p_company_id;select @p_last_dt=SYSDATETIME()", nil, nil),
	}
	for _, f := range vF {
		print_db(f)
	}
}

func Test_mssql_db_receiving_blobs(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("SELECT * FROM mydevdb..employee", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_mssql_db_with_simple_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{11, 15}
	db := gdb.FindDB(sp.Seek())
	f0 := db.ExecuteEx("SELECT SYSDATETIME(),SYSDATETIMEOFFSET(),SYSUTCDATETIME();"+
		"SELECT * FROM sakila..actor where actor_id between ? and ?;SELECT * from sakila.not_exist",
		&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
			fmt.Println(proc, vData)
		}, func(db *gdb.CDBHandler) {
			fmt.Println("meta columns", len(*db.GetColumnInfo()))
		}, nil)
	print_db(f0)
}

func Test_mssql_db_with_simple_parepared_statement(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{int32(11), int32(15)}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Prepare("SELECT * FROM sakila..actor where actor_id between ? and ?", nil)
	f1 := db.ExecuteParameter(&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println(proc, vData)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
	print_db(f1)
}

func Test_mssql_db_by_prepared_statements(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("mydevdb", 0), db.Execute("delete from employee;delete from company", nil, nil)}
	params := []interface{}{
		1, "Google Inc.", "1600 Amphi'theatre Par\\kway, Mountain View, CA 94043, USA", 66007000800.15,
		2, "Microsoft Inc.", "700 Bell'evue Way NE- 22nd Floor, Bell\\evue, WA 98804, USA", 93600300600.12,
		3, "Apple Inc.", "1 Infinite Loop, Cupertino, CA 95014, USA", 234056780097.147,
	}
	vF = append(vF,
		db.Prepare("INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)", nil),
		db.ExecuteParameter(&params, nil, nil))

	wstr := "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。"
	for len(wstr) < 3*256*1024 {
		wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。"
	}
	astr := gspa.AStr("The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.")
	for len(astr) < 256*1024 {
		astr += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump."
	}
	blob := gspa.MakeBuffer()
	defer blob.Recycle()
	params = []interface{}{
		1, "Ted Cruz", time.Now(), blob.Save(wstr).PopBytes(), wstr, 254000.75,
		1, "Donald Trump", time.Now(), blob.Save(astr).PopBytes(), astr, 20254000.23,
		2, "Hillary Clinton", time.Now(), blob.Save(astr).Save(wstr).PopBytes(), wstr, 6254000.18,
	}
	vF = append(vF,
		db.Prepare("insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(?,?,?,?,?,?)", nil),
		db.ExecuteParameter(&params, nil, nil))
	for _, f := range vF {
		print_db(f)
	}
}

func Test_mssql_db_with_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("mydevdb", 0), db.Execute("delete from employee;delete from company", nil, nil)}
	params := []interface{}{
		1, "Google Inc.", "1600 Amphi'theatre Par\\kway, Mountain View, CA 94043, USA", 66007000800.15,
		"Microsoft Inc.", "700 Bell'evue Way NE- 22nd Floor, Bell\\evue, WA 98804, USA", 93600300600.12,
		"Apple Inc.", "1 Infinite Loop, Cupertino, CA 95014, USA", 234056780097.147,
	}
	sql := "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);"
	sql += "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,?);"
	sql += "INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,?);"
	vF = append(vF, db.ExecuteEx(sql, &params, nil, nil, nil))

	wstr := "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。"
	for len(wstr) < 3*256*1024 {
		wstr += "广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。"
	}
	astr := gspa.AStr("The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.")
	for len(astr) < 256*1024 {
		astr += "The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump."
	}
	blob := gspa.MakeBuffer()
	defer blob.Recycle()
	params = []interface{}{
		"Ted Cruz", time.Now(), blob.Save(wstr).PopBytes(), wstr, 254000.75,
		"Donald Trump", time.Now(), blob.Save(astr).PopBytes(), astr,
		"Hillary Clinton", time.Now(), blob.Save(astr).Save(wstr).PopBytes(), wstr,
	}
	sql = "insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(1,?,?,?,?,?);"
	sql += "insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(1,?,?,?,?,20254000.23);"
	sql += "insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(2,?,?,?,?,6254000.18)"
	vF = append(vF, db.ExecuteEx(sql, &params, nil, nil, nil))
	for _, f := range vF {
		print_db(f)
	}
}

func Test_mssql_db_store_procedure_by_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	sql := "exec mydevdb..sp_TestProc 1,? out,? out;select CURRENT_TIMESTAMP;exec mydevdb..sp_TestProc 2,? out,? out;exec mydevdb..sp_TestProc 4,? out,? out;select 1,2.4,3"
	params := []interface{}{gspa.DECIMAL{Scale: 2, Lo64: 1246}, time.Now(),
		gspa.DECIMAL{Scale: 2, Lo64: 53114}, time.Now(),
		gspa.DECIMAL{Scale: 2, Lo64: 13114}, time.Now()}
	f0 := db.ExecuteEx(sql, &params, func(sender *gdb.CDBHandler, proc bool, vData []interface{}) {
		ma := *sender.GetColumnInfo()
		/*
			for _, a := range ma {
				fmt.Println(a)
			}
		*/
		cols := len(ma)
		if cols == 7 {
			fmt.Println("Rows", len(vData)/len(*sender.GetColumnInfo()), "cols", cols)
		} else {
			fmt.Println(proc, vData)
		}
	}, func(sender *gdb.CDBHandler) {
	}, nil)
	print_db(f0)
}

func Test_mssql_db_store_procedure_by_prepared_parameters(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMssql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Mssql, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vP := []gdb.CParameterInfo{
		{
			Direction: gdb.Input,
			DataType:  gspa.Vint32,
		}, {
			Direction: gdb.InputOutput,
			DataType:  gspa.Vdecimal,
			Scale:     2,
		}, {
			Direction: gdb.Output,
			DataType:  gspa.Vdate,
		},
	}
	f0 := db.Prepare("exec mydevdb..sp_TestProc ?,?,?", nil, vP)
	params := []interface{}{
		1, gspa.DECIMAL{Scale: 2, Lo64: 2315}, time.Now(),
		2, gspa.DECIMAL{Scale: 2, Lo64: 53114}, time.Now(),
		0, gspa.DECIMAL{Scale: 2, Lo64: 13117}, time.Now()}
	f1 := db.ExecuteParameter(&params, func(sender *gdb.CDBHandler, proc bool, vData []interface{}) {
		ma := *sender.GetColumnInfo()
		/*
			for _, a := range ma {
				fmt.Println(a)
			}
		*/
		cols := len(ma)
		if cols == 7 {
			fmt.Println("Rows", len(vData)/len(*sender.GetColumnInfo()), "cols", cols)
		} else {
			fmt.Println(proc, vData)
		}
	}, func(sender *gdb.CDBHandler) {
	}, nil)
	print_db(f0)
	print_db(f1)
}
