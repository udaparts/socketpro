package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"gspa/gdb"
	"testing"
	"time"
)

var cc2Postgres = gcs.CConnectionContext{Host: "windesk", UserId: "postgres", Password: "Smash123", Port: 20901}
var dbName = "postgres"

func Test_postgres_db_simple_calls_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Open(dbName, 0, func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f1 := db.BeginTrans(func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
	})
	f2 := db.Prepare("select * from actor where actor_id=?", func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
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

func Test_postgres_db_execute_sql(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("select * from actor where actor_id between 11 and 20", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
		fmt.Println("Data: ", vData)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_postgres_db_execute_SQLs_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open(dbName, gdb.USE_QUERY_BATCHING),
		db.Execute("CREATE TABLE IF NOT EXISTS Company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income double precision not null)", nil, nil),
		db.Execute("CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigserial not null,CompanyId bigint not null,name char(64)NOT NULL,JoinDate TIMESTAMP default null,IMAGE bytea,DESCRIPTION text,Salary DECIMAL(14,2),FOREIGN KEY(CompanyId)REFERENCES company(id))", nil, nil),
		db.Execute("create or replace function sp_TestProc(p_company_id int,inout p_sum_salary decimal(14,2),out p_last_dt timestamp)as $func$ select sum(salary)+p_sum_salary,localtimestamp from employee where companyid>=p_company_id $func$ language sql", nil, nil),
		db.Execute("CREATE OR REPLACE PROCEDURE test_sp(id integer,INOUT mymoney numeric,INOUT dt timestamp) LANGUAGE 'plpgsql' AS $BODY$ BEGIN select mymoney+sum(salary),localtimestamp into mymoney,dt from employee where companyid>id;END $BODY$", nil, nil),
	}
	for _, f := range vF {
		print_db(f)
	}
}

func Test_postgres_db_receiving_blobs(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("SELECT * FROM employee", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_postgres_db_with_simple_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{11, 15}
	db := gdb.FindDB(sp.Seek())
	f0 := db.ExecuteEx("SELECT CURRENT_TIMESTAMP(6),LOCALTIMESTAMP(6),CURRENT_TIME(6);"+
		"SELECT * FROM actor where actor_id between ? and ?;SELECT * from not_exist",
		&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
			fmt.Println(proc, vData)
		}, func(db *gdb.CDBHandler) {
			fmt.Println("meta columns", len(*db.GetColumnInfo()))
		}, nil)
	print_db(f0)
}

func Test_postgres_db_with_simple_parepared_statement(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{int32(11), int32(15)}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Prepare("SELECT * FROM actor where actor_id between ? and ?", nil)
	f1 := db.ExecuteParameter(&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println(proc, vData)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
	print_db(f1)
}

func Test_postgres_db_by_prepared_statements(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open(dbName, 0), db.Execute("delete from employee;delete from company", nil, nil)}
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
		db.Prepare("insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)", nil),
		db.ExecuteParameter(&params, nil, nil))
	for _, f := range vF {
		print_db(f)
	}
}

func Test_postgres_db_with_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open(dbName, 0), db.Execute("delete from employee;delete from company", nil, nil)}
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
	sql = "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);"
	sql += "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,20254000.23);"
	sql += "insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(2,?,?,?,?,6254000.18)"
	vF = append(vF, db.ExecuteEx(sql, &params, nil, nil, nil))
	for _, f := range vF {
		print_db(f)
	}
}

func Test_postgres_db_store_procedure_by_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	sql := "SELECT CURRENT_TIMESTAMP(6),LOCALTIMESTAMP(6),CURRENT_TIME(6);call test_sp(1,?,null);select 1,2.4,3;call test_sp(?,?,null)"
	params := []interface{}{1276.54, 0, 234.32}
	f0 := db.ExecuteEx(sql, &params, func(sender *gdb.CDBHandler, proc bool, vData []interface{}) {
		ma := *sender.GetColumnInfo()
		for _, a := range ma {
			fmt.Println(a)
		}
		cols := len(ma)
		fmt.Println(cols, proc, vData)
	}, func(sender *gdb.CDBHandler) {
	}, nil)
	print_db(f0)
}

func Test_postgres_db_function_by_prepared_parameters(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Prepare("SELECT sp_testproc(?,?)", nil)
	params := []interface{}{
		1, 12.46,
		2, 531.14,
		0, 131.14}
	f1 := db.ExecuteParameter(&params, func(sender *gdb.CDBHandler, proc bool, vData []interface{}) {
		ma := *sender.GetColumnInfo()
		cols := len(ma)
		fmt.Println(cols, proc, vData)
	}, func(sender *gdb.CDBHandler) {
	}, nil)
	print_db(f0)
	print_db(f1)
}

func Test_postgres_db_batch(t *testing.T) {
	sp := gcs.NewPool(gdb.SidPostgres)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc2Postgres, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Open(dbName, 0)
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

	//sql with delimiter '|'
	sql := "delete from employee;delete from company|" +
		"INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|" +
		"insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|" +
		"SELECT * from company;select * from employee;select CURRENT_TIME(6)|" +
		"SELECT sp_testproc(?,?)"

	params := []interface{}{
		//1st set
		1, "Google Inc.", "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA", 66000020070.15,
		1, "Ted Cruz", time.Now(), blob.Save(wstr).PopBytes(), wstr, 254000.37,
		1, 2345.23,
		//2nd set
		2, "Microsoft Inc.", "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA", 93600080010.37,
		1, "Donald Trump", time.Now(), blob.Save(astr).PopBytes(), astr, 20254000.85,
		2, 881.24,
		//3rd set
		3, "Apple Inc.", "1 Infinite Loop, Cupertino, CA 95014, USA", 234000070003.09,
		2, "Hillary Clinton", time.Now(), blob.Save(astr, wstr).PopBytes(), wstr, 6254000.55,
		0, 74562.34}

	f1 := db.ExecuteBatch(gdb.ReadCommited, sql, &params, func(sender *gdb.CDBHandler, proc bool, vData []interface{}) {
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
	}, func(sender *gdb.CDBHandler) {
	}, "|", gdb.Default, nil)
	print_db(f0)
	print_db(f1)
}
