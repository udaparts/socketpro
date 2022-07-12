package tests

import (
	"fmt"
	"gspa"
	"gspa/gcs"
	"gspa/gdb"
	"reflect"
	"testing"
	"time"
)

func Test_conluminfo_0(t *testing.T) {
	buffer := gspa.MakeBuffer()
	info := make(gdb.CDBColumnInfoArray, 0)
	var info0 gdb.CDBColumnInfoArray
	info0.LoadFrom(info.SaveTo(buffer))
	if !reflect.DeepEqual(info, info0) {
		t.Errorf("Test_db_conluminfo_0 info0: %+v, want: %+v", info0, info)
	}
}

func Test_conluminfo_1(t *testing.T) {
	buffer := gspa.MakeBuffer()
	info := make(gdb.CDBColumnInfoArray, 1)
	f := new(gdb.CDBColumnInfo)
	f.DataType = gspa.Vutf16
	f.DisplayName = "Test"
	f.DBPath = "Sakila"
	f.TablePath = "actor"
	f.ColumnSize = 1024
	info[0] = f
	var info0 gdb.CDBColumnInfoArray
	info0.LoadFrom(info.SaveTo(buffer))
	if !reflect.DeepEqual(info, info0) {
		t.Errorf("Test_db_conluminfo_0 info0: %+v, want: %+v", info0, info)
	} else {
		fmt.Println(info[0])
	}
}

func print_db(f <-chan interface{}) {
	res := <-f //wait until result comes from remote server
	switch t := res.(type) {
	case *gspa.ErrInfo:
		fmt.Println("Error info", res.(*gspa.ErrInfo))
	case *gdb.SQLExeInfo:
		fmt.Println("SQLExeInfo", res.(*gdb.SQLExeInfo))
	case gcs.SocketError: //session error
		fmt.Println(res.(gcs.SocketError))
	case gcs.ServerError: //error from remote server
		fmt.Println(res.(gcs.ServerError))
	default:
		fmt.Println("Unknown data type: ", t)
	}
}

func Test_db_simple_calls_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
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
	f2 := db.Prepare("select * from sakila.actor where actor_id=?", func(db *gdb.CDBHandler, err *gspa.ErrInfo) {
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

func Test_db_execute_sql(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("select * from sakila.actor where actor_id between 11 and 20", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_db_execute_SQLs_in_streaming(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("sakila", gdb.USE_QUERY_BATCHING),
		db.Execute("Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb", nil, nil),
		db.Execute("CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)", nil, nil),
		db.Execute("CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))", nil, nil),
		db.Execute("DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int, inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime(6))BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now(6)into p_last_dt;END", nil, nil),
	}
	for _, f := range vF {
		print_db(f)
	}
}

func Test_db_receiving_blobs(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Execute("SELECT * FROM mysqldb.employee", func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println("rows", len(vData)/len(*db.GetColumnInfo()), "proc", proc)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
}

func Test_db_with_simple_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{11, 15}
	db := gdb.FindDB(sp.Seek())
	f0 := db.ExecuteEx("select curtime(6);SELECT * FROM sakila.actor where actor_id between ? and ?;select * from sakila.not_exist",
		&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
			fmt.Println(proc, vData)
		}, func(db *gdb.CDBHandler) {
			fmt.Println("meta columns", len(*db.GetColumnInfo()))
		}, nil)
	print_db(f0)
}

func Test_db_with_simple_parepared_statement(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	params := []interface{}{int32(11), int32(15)}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Prepare("SELECT * FROM sakila.actor where actor_id between ? and ?", nil)
	f1 := db.ExecuteParameter(&params, func(db *gdb.CDBHandler, proc bool, vData []interface{}) {
		fmt.Println(proc, vData)
	}, func(db *gdb.CDBHandler) {
		fmt.Println("meta columns", len(*db.GetColumnInfo()))
	})
	print_db(f0)
	print_db(f1)
}

func Test_db_by_prepared_statements(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("mysqldb", 0), db.Execute("delete from employee;delete from company", nil, nil)}
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

func Test_db_with_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	vF := []<-chan interface{}{db.Open("mysqldb", 0), db.Execute("delete from employee;delete from company", nil, nil)}
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

func Test_db_store_procedure_by_session_variables(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	sql := "call mysqldb.sp_TestProc(1,? out,? out);select now(6);call mysqldb.sp_TestProc(2,? out,? out);call mysqldb.sp_TestProc(4,? out,? out);select 1,2.4,3"
	params := []interface{}{12.46, 0, 531.14, 0, 131.14, 0}
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

func Test_db_store_procedure_by_prepared_parameters(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Prepare("call mysqldb.sp_TestProc(?,?,?)", nil)
	params := []interface{}{
		1, 12.46, 0,
		2, 531.14, 0,
		0, 131.14, 0}
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

func Test_db_batch(t *testing.T) {
	sp := gcs.NewPool(gdb.SidMysql)
	sp.SetPoolEvent(func(spe gcs.SocketPoolEvent, h *gcs.CAsyncHandler) {
		if spe == gcs.SpeSocketCreated {
			db := new(gdb.CDBHandler)
			db.Tie(h)
		}
	}).SetAutoConn(false)
	ok := sp.StartSocketPool(cc_hw, 1)
	defer sp.ShutdownPool()
	if !ok {
		ah := sp.GetHandlers()[0]
		t.Errorf("error code: %d, error message: %s", ah.GetErrorCode(), ah.GetErrorMsg())
		return
	}
	db := gdb.FindDB(sp.Seek())
	f0 := db.Open("mysqldb", 0)
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
		"SELECT * from company;select * from employee;select curtime(6)|" +
		"call sp_TestProc(?,?,?)"

	params := []interface{}{
		//1st set
		1, "Google Inc.", "1600 Amphitheatre Parkway, Mountain View, CA 94043, USA", 66000020070.15,
		1, "Ted Cruz", time.Now(), blob.Save(wstr).PopBytes(), wstr, 254000.37,
		1, 2345.23, 0,
		//2nd set
		2, "Microsoft Inc.", "700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA", 93600080010.37,
		1, "Donald Trump", time.Now(), blob.Save(astr).PopBytes(), astr, 20254000.85,
		2, 881.24, 0,
		//3rd set
		3, "Apple Inc.", "1 Infinite Loop, Cupertino, CA 95014, USA", 234000070003.09,
		2, "Hillary Clinton", time.Now(), blob.Save(astr, wstr).PopBytes(), wstr, 6254000.55,
		0, 74562.34, 0}

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
