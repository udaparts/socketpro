//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
var p=cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite

//create a connection context
var cc = cs.newCC('localhost',20902,'root','Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc,1)) {
	console.log(p.Error);
	return;
}
var db = p.Seek(); //seek an async DB handler
if (!db.Open('sakila', (res, err)=>{
	if(res) console.log({ec:res, em:err});
}, canceled=>{
	console.log(canceled ? 'request canceled' : 'session closed');
})) {
	console.log(db.Socket.Error);
	return;
}

function TestCreateTables(db) {
	//track final result event, but ignore row, metat and cancel events
	if (!db.Execute('Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb',
		(res, err)=>{if(res) console.log({ec:res, em:err});})) {
		return false;
	}
	if (!db.Execute('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)',
		(res, err)=>{if(res) console.log({ec:res, em:err});})) {
		return false;
	}
	if (!db.Execute('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))',
		(res, err)=>{if(res) console.log({ec:res, em:err});})) {
		return false;
	}
	if (!db.Execute('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime)BEGIN select * from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END',
		(res, err)=>{if(res) console.log({ec:res, em:err});})) {
		return false;
	}
	return true;
}
if (!TestCreateTables(db)) {
	console.log(db.Socket.Error);
	return;
}
if (!db.Execute('delete from employee;delete from company', (res, err, affected)=>{
		console.log({ec:res, em:err, aff:affected});
	})) {
	console.log(db.Socket.Error);
	return;
}

function TestPreparedStatements(db) {
	if (!db.Prepare('INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)', (res, err)=>{
		if(res) console.log({ec:res, em:err});
	})) {
		return false;
	}
	var buff = SPA.newBuffer();
	
	//prepare three sets of parameter data by use of method SaveObject
	
	//pay attention to below data type hints used by method SaveObject
	//'c' = char, 'b' = byte
	//'s' = short, 'us' = unsigned short
	//'i' = int, 'ui' = unsigned int
	//'l' = long (8 bytes), 'ul' = unsigned long (8 bytes)
	//'f' = float, 'd' == double
	//'a' = ASCII or UTF8 string. Otherwsie, unicode string
	//'dec' = decimal
	//true or false for boolean doesn't require any hint
	//Buffer for byte array doesn't require any hint
	
	//1st set 'i' = int, 'a' = ASCII chars, 'd' = double
	buff.SaveObject(1, 'i').SaveObject('Google Inc.', 'a').SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15, 'd');
	//2nd set
	buff.SaveObject(2, 'i').SaveObject('Microsoft Inc.', 'a').SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject(93600000000.12, 'd');
    //third set
	buff.SaveObject(3, 'i').SaveObject('Apple Inc.', 'a').SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14, 'd');
	
	//send three sets in one shot
	if (!db.Execute(buff, (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	})) {
		return false;
	}
	return true;
}

if (!TestPreparedStatements(db)) {
	console.log(db.Socket.Error);
	return;
}

function InsertBLOBByPreparedStatement(db) {
	var wstr='';
    while (wstr.length < 128 * 1024) {
        wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
    }
	var str = '';
    while (str.length < 256 * 1024) {
        str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
    }
	if (!db.Prepare('insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)', (res, err)=>{
		if(res) console.log({ec:res, em:err});
	})) {
		return false;
	}
	var buff = SPA.newBuffer();
	var blob = SPA.newBuffer();
	
	blob.SaveString(wstr);
	//1st set
	//blob.PopBytes() -- convert all data inside blob memory into an array of bytes
	buff.SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject('254000.15');
	
	blob.SaveAString(str);
	//2nd set
	buff.SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(str).SaveObject(20254000.35);
	
	blob.SaveAString(str).SaveString(wstr);
	//3rd set
	buff.SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(6254000.42);
	
	//send three sets in one shot
	if (!db.Execute(buff, (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	})) {
		return false;
	}
	return true;
}
if (!InsertBLOBByPreparedStatement(db)) {
	console.log(db.Socket.Error);
	return;
}

if (!db.Execute('SELECT * from company;select curtime()', (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	}, data=>{
		console.log(data);
	}, meta=>{
		//console.log(meta);
	})) {
	console.log(db.Socket.Error);
	return;
}

if (!db.Execute('select name, joindate, salary from employee', (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	}, data=>{
		console.log(data);
	}, meta=>{
		//console.log(meta);
	})) {
	console.log(db.Socket.Error);
	return;
}

function TestStoredProcedure(db) {
	if (!db.Prepare('call mysqldb.sp_TestProc(?,?,?)', (res, err)=>{
		if(res) console.log({ec:res, em:err});
	})) {
		return false;
	}
	var buff = SPA.newBuffer();
	//1st set
	buff.SaveObject(1).SaveObject(1.25).SaveObject();
	//2nd set
	buff.SaveObject(2).SaveObject(1.14).SaveObject();
	//3rd set
	buff.SaveObject(0).SaveObject(2.18).SaveObject();
	if (!db.Execute(buff, (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	}, (data, proc)=>{
		if (proc) {
			console.log(data); //output output parameters
		}
	}, meta=>{
		//console.log(meta);
	})) {
		console.log(db.Socket.Error);
		return false;
	}
	return true;
}
if (!TestStoredProcedure(db)) {
	console.log(db.Socket.Error);
	return;
}

function TestBatch(db) {
	var sql = 'delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from company;select * from employee;select curtime()|call sp_TestProc(?,?,?)';
	var wstr='';
    while (wstr.length < 128 * 1024) {
        wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
    }
	var str = '';
    while (str.length < 256 * 1024) {
        str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
    }
	var buff = SPA.newBuffer();
	var blob = SPA.newBuffer();
	
	//1st set
	//INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
	buff.SaveObject(1).SaveObject('Google Inc.').SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15);
	//insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
	buff.SaveObject(1); //Google company id
	blob.SaveString(wstr); //UNICODE string
	buff.SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(254000.15);
	//call sp_TestProc(?,?,?)
	buff.SaveObject(1).SaveObject(1.25).SaveObject();
	
	//2nd set
	buff.SaveObject(2).SaveObject('Microsoft Inc.').SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject('93600000000.12');
	buff.SaveObject(1); //Google company id
	blob.SaveAString(str); //ASCII string
	buff.SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(str, 'a').SaveObject(20254000);
	buff.SaveObject(2).SaveObject(1.14).SaveObject();
	
	//3rd set
	buff.SaveObject(3).SaveObject('Apple Inc.').SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14);
	buff.SaveObject(2); //Microsoft company id
	blob.SaveAString(str).SaveString(wstr);
	buff.SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject('6254000.15');
	buff.SaveObject(0).SaveObject(8.16).SaveObject();
	
	//first, execute delete from employee;delete from company
    //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //third, three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
    //fourth, SELECT * from company;select * from employee;select curtime()
    //last, three sets of call sp_TestProc(?,?,?)
	if (!db.ExecuteBatch(SPA.DB.TransIsolation.Unspecified, sql, buff, (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, aff:affected, oks:oks, fails:fails, lastId:id});
	}, (data, proc)=>{
		if (proc) {
			console.log(data); //output output parameters
		}
	}, meta=>{
		//console.log(meta);
	}, dbH=>{
		console.log('Batch header comes');
	}, canceled=>{
		console.log(canceled ? 'Request canceled' : 'Session closed');
	}, SPA.DB.RollbackPlan.rpDefault, "|")) {
		console.log(db.Socket.Error);
		return false;
	}
	return true;
}
/*
if (!TestBatch(db)) {
	console.log(db.Socket.Error);
	return;
}
*/