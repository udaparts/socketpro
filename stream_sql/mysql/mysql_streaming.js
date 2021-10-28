'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite
global.p = p;
//p.QueueName = 'qmysql';
//create a connection context
var cc = cs.newCC('localhost', 20902, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var db = p.Seek(); //seek an async DB handler

//make long strings for testing long text and blob objects
var g_wstr = '';
while (g_wstr.length < 256 * 1024) {
    g_wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
}
var g_str = '';
while (g_str.length < 512 * 1024) {
    g_str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
}

function TestCreateTables(db) {
    console.log('---- test creating a database, two tables, and one store procedure ----');
    var pe0 = db.execute('Create database if not exists mysqldb character ' +
        'set utf8 collate utf8_general_ci;USE mysqldb');
    var pe1 = db.execute('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)' +
        'NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)');
    var pe2 = db.execute('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT ' +
        'NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE ' +
        'MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))');
    var pe3 = db.execute('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,' +
        'inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime(6))BEGIN select name,joindate,salary from employee ' +
        'where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee ' +
        'where companyid>=p_company_id;select now(6)into p_last_dt;END');
    return [pe0, pe1, pe2, pe3];
}

function TestExecuteEx(db) {
    //set an array of parameter data
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66000700000.15,
        'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA',
        'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA'];
	var sql = 'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);' +
		'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,93600090000.12);' +
		'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234000300000.14)';
    return [db.executeEx(sql, vParam)];
}

function TestBLOBByPreparedStatement(db) {
    console.log('**** test prepare statements with long texts and blobs ****');
    var pp = db.prepare('insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)');
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    blob.SaveString(g_wstr);
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject('254000.15');
    blob.SaveAString(g_str);
    //2nd set
    buff.SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_str).SaveObject(20254000.35);
    blob.SaveAString(g_str).SaveString(g_wstr);
    //3rd set
    buff.SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject(6254000.42);
    var pe = db.execute(buff);
    return [pp, pe];
}

function TestStoreProcedureByExecuteEx(db) {
    console.log('%%%% Test four sql statements plus a store procedure with record sets %%%%');
    var ps1 = db.execute('select * from employee', (data, proc, cols) => {
        //console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        console.log(meta);
    });
    var vParam = [9811.25, null, //1st set
        45321.14, null, //2nd set
        2342.18, null]; //3rd set
		
	//process multiple sets of parameters in one shot & pay attention to out for returning results
	var sql = 'SELECT * from company;call mysqldb.sp_TestProc(1,? out,? out);' +
		'call mysqldb.sp_TestProc(2,? out,? out);select curtime();' +
		'call mysqldb.sp_TestProc(3,? out,? out);select 1,2,3';
    var pe = db.executeEx(sql, vParam, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        console.log(meta);
    });
    return [ps1, pe];
}

function TestBatch(db) {
    console.log('&&&& Test a big batch of statements with manual transaction &&&&');
    var sql = 'delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|' +
        'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from ' +
        'company;select name,joindate,salary from employee;select curtime(6)|call sp_TestProc(?,?,?)';
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    //1st set
    //INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    buff.SaveObject(1).SaveObject('Google Inc.').
        SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15);
    //insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
    buff.SaveObject(1); //Google company id
    blob.SaveString(g_wstr); //UNICODE string
    buff.SaveObject('Ted Cruz').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject(254000.15);
    //call sp_TestProc(?,?,?)
    buff.SaveObject(1).SaveObject(1.25).SaveObject();
    //2nd set
    buff.SaveObject(2).SaveObject('Microsoft Inc.').
        SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject('93600000000.12');
    buff.SaveObject(1); //Google company id
    blob.SaveAString(g_str); //ASCII string
    buff.SaveObject('Donald Trump').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_str, 'a').SaveObject(20254000);
    buff.SaveObject(2).SaveObject(1.14).SaveObject();
    //3rd set
    buff.SaveObject(3).SaveObject('Apple Inc.').
        SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14);
    buff.SaveObject(2); //Microsoft company id
    blob.SaveAString(g_str).SaveString(g_wstr);
    buff.SaveObject('Hillary Clinton').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject('6254000.15');
    buff.SaveObject(0).SaveObject(8.16).SaveObject();
    //1st, start a manual transaction
    //2nd, execute delete from employee;delete from company
    //3rd, prepare a sql statement for insert into company
    //4th, execute three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //5th, prepare a sql statement for insert into employee
    //6th, execute three sets of insert into employee
    // (CompanyId, name, JoinDate, image, DESCRIPTION, Salary)values(?,?,?,?,?,?)
    //7th, SELECT * from company;select * from employee;select curtime()
    //8th, prepare sp_TestProc(?,?,?)
    //9th, three sets of call sp_TestProc(?,?,?)
    //last, end manual transaction
    var pB = db.executeBatch(SPA.DB.TransIsolation.ReadCommited, sql, buff, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, (meta) => {
        console.log(meta);
    }, '|', () => {
        console.log('---- Batch header comes ----');
    });
    return [pB];
}

(async () => {
    try {
        var res = await db.open(''); //open a default database
        console.log(res);
        if (res.ec) {
            return;
        }
        var vTables = TestCreateTables(db);
        var pd = [db.execute('delete from employee;delete from company')];
        var vPs0 = TestExecuteEx(db);
        var vPs1 = TestBLOBByPreparedStatement(db);
        var vPst = TestStoreProcedureByExecuteEx(db);
        var pB = TestBatch(db);
        var all_p = [].concat(vTables, pd, vPs0, vPs1, vPst, pB);
        console.log(await Promise.all(all_p));
    } catch (ex) {
        console.log(ex);
    }
})();
