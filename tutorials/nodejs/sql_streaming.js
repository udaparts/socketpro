'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js'); //line 4
var cs = SPA.CS; //CS == Client side line 5

//create a socket pool object
var p = cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite, sidMsSql; line 8
global.p = p;

//create an instance of connection context
var cc = cs.newCC('localhost', 20902, 'root', 'Smash123'); //line 12

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) { //line 15
    console.log(p.Error); //line 16
    return;
}

//make long strings for testing long text and blob objects between lines 21 and 28
var wstr = '';
while (wstr.length < 256 * 1024) {
    wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
}
var str = '';
while (str.length < 512 * 1024) {
    str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
}

var db = p.Seek(); //seek an opened async DB handler, line 30
function setDBAndTables(db) {
    var pC0 = db.execute('Create database if not exists mysqldb character set utf8 collate ' + //line 32
        'utf8_general_ci;USE mysqldb');
    var pC1 = db.execute('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)' +
        'NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)');
    var pC2 = db.execute('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY,' +
        'CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,' +
        'DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))');
    var pC3 = db.execute('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(p_company_id int,' +
        'inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime(6))BEGIN select name,joindate,salary from ' +
        'employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from ' +
        'employee where companyid>=p_company_id;select now(6)into p_last_dt;END'); //line 42
    return [pC0, pC1, pC2, pC3]; //line 43
}

function setCompanyByExecuteEx(db) {
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 6600900.15,
        'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', //2nd set
        'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA']; //3rd set
    var sqls = 'INSERT INTO company VALUES(?,?,?,?);select now(6);' + //line 50
        'INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(2,?,?,9360700.12);' +
        'INSERT INTO company VALUES(3,?,?,2340400.14);Select * FROM company'; //line 52
    var pi0 = db.executeEx(sqls, vParam, (data, proc, cols) => { //line 53
        //console.log({ data: data, proc: proc, cols: cols });
    }, (meta) => {
        //console.log(meta);
    });
    return [pi0];
}

function setEmployeeByExecuteEx(db) {
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    blob.SaveString(wstr); //line 64
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(wstr).SaveObject('254000.15');

    blob.SaveAString(str); //line 70
    //2nd set
    buff.SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(str).SaveObject(2025400.35);

    blob.SaveAString(str).SaveString(wstr); //line 75
    //3rd set
    buff.SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(wstr).SaveObject(625400.42);

    buff.SaveObject('Bad Name'); //for WHERE Name<>?
    buff.SaveObject(62345.67).SaveObject(new Date()); //for call sp_TestProc(1,? out,? out)

    var sqls = 'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?);' +
        'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(1,?,?,?,?,?);' +
        'insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(2,?,?,?,?,?);' +
        'Select EmployeeId,Name,JoinDate,Salary FROM employee WHERE Name<>?;call sp_TestProc(1,? out,? out)'
    var pi1 = db.executeEx(sqls, buff, (data, proc, cols) => { //line 87
        //console.log({ data: data, proc: proc, cols: cols });
    }, (meta) => {
        //console.log(meta);
    });
    return [pi1];
}

function setCompanyByPrepareStatement(db) {
    var fp = db.prepare('INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)'); //line 96
    //set an array of parameter data
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66030000.15,
        2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93607000.12,
        3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234009000.14]; //3rd set
    return [fp, db.execute(vParam)]; //line 101
}

function setEmployeeByPrepareStatement(db) {
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    
    var fp = db.prepare('insert into employee(CompanyId,name,JoinDate,image,' +
        'DESCRIPTION,Salary)values(?,?,?,?,?,?)'); //line 109
    blob.SaveString(wstr); //line 110
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(wstr).SaveObject('254000.25'); //line 114

    blob.SaveAString(str); //line 116
    //2nd set
    buff.SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(str).SaveObject(2025400.38); //119

    blob.SaveAString(str).SaveString(wstr); //line 121
    //3rd set
    buff.SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).
        SaveObject(wstr).SaveObject(625400.41); //124
    return [fp, db.execute(buff)]; //line 125
}

function callStoredProcedureByPrepareStatement(db) {
    var vParam = [1, 25561.25, null, //1st set
        2, 91567.14, null, //2nd set
        0, 71561.18, null]; //3rd set
    var pp = db.prepare('call mysqldb.sp_TestProc(?,?,?)'); //line 132
    var psp = db.execute(vParam, (data, proc, cols) => { //line 133
        if (proc) console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        //console.log(meta);
    });
    return [pp, psp];
}

(async () => {
    try {
        var res = await db.open(''); //line 143
        if (res.ec) {
            console.log(res.em);
            return;
        }
        var plist = setDBAndTables(db); //line 148

        var pbt0 = [db.beginTrans(SPA.DB.TransIsolation.ReadCommited)]; //line 150
        var pd0 = [db.execute('delete from employee;delete from company')];
        var pi0 = setCompanyByExecuteEx(db);
        var pi1 = setEmployeeByExecuteEx(db);
        var pet0 = [db.endTrans(SPA.DB.RollbackPlan.rpDefault)]; //line 154

        var pbt1 = [db.beginTrans(SPA.DB.TransIsolation.ReadCommited)]; //line 156
        var pd1 = [db.execute('delete from employee;delete from company')];
        var pip0 = setCompanyByPrepareStatement(db);
        var pip1 = setEmployeeByPrepareStatement(db);
        var psp = callStoredProcedureByPrepareStatement(db);
        var pet1 = [db.endTrans(SPA.DB.RollbackPlan.rpDefault)]; //line 161

        var pq = db.execute("select * from company;select * from employee", //line 163
            (data, proc, cols) => {
                //console.log({ data: data, proc: proc, cols: cols });
            }, (meta) => {
                //console.log(meta);
        });
        var all_p = [].concat(plist, pbt0, pd0, pi0, pi1, pet0, pbt1, pd1, pip0, pip1, psp, pet1, pq);
        console.log(await Promise.all(all_p)); //line 170
    } catch (ex) {
        console.log(ex); //line 172
    }
})();
