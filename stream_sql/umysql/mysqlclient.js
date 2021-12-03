'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidMysql); //sidPostgres, sidMsSql or other
global.p = p;
//p.QueueName = 'qmysql';
//create a connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var db = p.Seek(); //seek an async DB handler

//make long strings for testing long text and blob objects
var g_wstr = '';
while (g_wstr.length < 128 * 1024) {
    g_wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
}
var g_str = '';
while (g_str.length < 256 * 1024) {
    g_str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
}

function TestCreateTables(db) {
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
    var vParam = [1, 'Google Inc.', "1600 Amphith'eatre Parkway, Mountain View, CA 94043, USA", 66000700020.15,
        2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA',
        'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA'];
    var sql = 'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?);select now(6);' +
        'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,93600090060.12);select 1,2,3;' +
        'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(3,?,?,234000300070.14)';
    return [db.executeEx(sql, vParam, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        console.log(meta);
    })];
}

function TestPreparedStatement(db) {
    var pp = db.prepare('INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)');
    //set an array of parameter data
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66000700060.15,
        2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600090040.12,
        3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000300070.14];
    return [pp, db.execute(vParam)];
}

function TestBlobExecuteEx(db) {
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();

    blob.SaveString(g_wstr);
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject('Ted Cruz').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject(254090.15, "dec");

    blob.SaveAString(g_str);
    //2nd set
    buff.SaveObject('Donald Trump').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_str).SaveObject(20254070.35, "dec");

    blob.SaveAString(g_str).SaveString(g_wstr);
    //3rd set
    buff.SaveObject('Hillary Clinton').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr);
	
	//for call sp_TestProc with output salary in decimal
    buff.SaveObject(1276.54, "dec").SaveObject(new Date()); //line 91

    var sql = 'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);' +
        'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);select 245;' +
        'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(2,?,?,?,?,6254030.42);' +
        'call sp_TestProc(1,? out,? out)';

    var pe = db.executeEx(sql, buff, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        console.log(meta);
    });
    return [pe];
}

function TestStoreProcedureByPreparedStatement(db) {
	var pp = db.prepare('call sp_TestProc(?,?,?)');
    var vParam = [1, 9811.25, null, //1st set
        0, 45321.14, null, //2nd set
        2, 2342.18, null]; //3rd set
	//process multiple sets of parameters in one shot
    var pe = db.execute(vParam, (data, proc, cols) => {
        if (proc) console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        //console.log(meta);
    });
    return [pp, pe];
}

function TestBLOBByPreparedStatement(db) {
    var pp = db.prepare('insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)');
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    blob.SaveString(g_wstr);
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject(254020.15);

    blob.SaveAString(g_str);
    //2nd set
    buff.SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_str).SaveObject(20254020.35);

    blob.SaveAString(g_str).SaveString(g_wstr);
    //3rd set
    buff.SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).
        SaveObject(blob.PopBytes()).SaveObject(g_wstr).SaveObject(6254070.42);
    var pe = db.execute(buff); //line 125
    return [pp, pe];
}

(async () => {
    try {
        //open a default database with query batching enabled
        var res = await db.open('', 2); //line 146
        console.log(res);
        if (res.ec) {
            return;
        }
        console.log('Streaming all requests with the best network efficiency');
        var vTables = TestCreateTables(db); //line 152
        var pd0 = [db.execute('delete from employee;delete from company')]; //line 153
        var pp0 = TestPreparedStatement(db); //line 154
        var pp1 = TestBLOBByPreparedStatement(db); //line 155
		var pp2 = TestStoreProcedureByPreparedStatement(db); //line 156
        var pd1 = [db.execute('delete from employee;delete from company')]; //line 157
        var pex0 = TestExecuteEx(db); //line 158
        var pex1 = TestBlobExecuteEx(db); //line 159

        console.log('Waiting all responses .....');
        var all_p = [].concat(vTables, pd0, pp0, pp1, pp2, pd1, pex0, pex1);
        console.log(await Promise.all(all_p)); //line 163
    } catch (ex) {
        console.log(ex); //line 165
    }
})();
