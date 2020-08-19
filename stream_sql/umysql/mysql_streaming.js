'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite
global.p = p;

//create a connection context
var cc = cs.newCC('localhost', 20901, 'SomeUserId', 'SomePasswordForSocketProServer');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var db = p.Seek(); //seek an async DB handler

//make long strings for testing long text and blob objects
var wstr = '';
while (wstr.length < 256 * 1024) {
    wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
}
var str = '';
while (str.length < 512 * 1024) {
    str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
}

(async () => {
    try {
        var res = await db.open('host=localhost;port=3306;user=root;pwd=Smash123;db=mysqldb');
        if (res.ec) {
            console.log(res.em);
            return;
        }
        console.log('++++ test creating a database, two tables, and one store procedure ++++');
        var fCt0 = db.execute('Create database if not exists mysqldb character set utf8 collate utf8_general_ci;USE mysqldb');
        var fCt1 = db.execute('CREATE TABLE IF NOT EXISTS company(ID bigint PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income Decimal(21,2)not null)');
        var fCt2 = db.execute('CREATE TABLE IF NOT EXISTS employee(EMPLOYEEID bigint AUTO_INCREMENT PRIMARY KEY NOT NULL unique,CompanyId bigint not null,name CHAR(64)NOT NULL,JoinDate DATETIME(6)default null,IMAGE MEDIUMBLOB,DESCRIPTION MEDIUMTEXT,Salary DECIMAL(25,2),FOREIGN KEY(CompanyId)REFERENCES company(id))');
        var fCt3 = db.execute('DROP PROCEDURE IF EXISTS sp_TestProc;CREATE PROCEDURE sp_TestProc(in p_company_id int,inout p_sum_salary DECIMAL(25,2),out p_last_dt datetime)BEGIN select name,joindate,salary from employee where companyid>=p_company_id;select sum(salary)+p_sum_salary into p_sum_salary from employee where companyid>=p_company_id;select now()into p_last_dt;END');
        var fd = db.execute('delete from employee;delete from company');
        console.log(await Promise.all([fCt0, fCt1, fCt2, fCt3, fd]));

        console.log('++++ test prepare statements ++++');
        var fp = db.prepare('INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)');
        //set an array of parameter data
        var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66000000000.15, //1st set
            2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600000000.12, //2nd set
            3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000000000.14]; //3rd set
        var fpi = db.execute(vParam);
        console.log(await Promise.all([fp, fpi]));

        console.log('**** test prepare statements with long texts and blobs ****');
        fp = db.prepare('insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)');
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
        fpi = db.execute(buff);
        console.log(await Promise.all([fp, fpi]));

        console.log('%%%% Test three sql statements plus a store procedure with record sets %%%%');
        var vParam = [1, 1.25, null, //1st set
            2, 1.14, null, //2nd set
            0, 2.18, null]; //3rd set
        var fs0 = db.execute('SELECT * from company;select curtime()', (data, proc, cols) => {
            console.log({ data: data, proc: proc, cols: cols });
        }, meta => {
            console.log(meta);
        });
        var fs1 = db.execute('select * from employee', (data, proc, cols) => {
            //console.log({ data: data, proc: proc, cols: cols });
        }, meta => {
            console.log(meta);
        });
        fp = db.prepare('call mysqldb.sp_TestProc(?,?,?)');
        var fsp = db.execute(vParam, (data, proc, cols) => {
            console.log({ data: data, proc: proc, cols: cols });
        }, meta => {
            console.log(meta);
        });
        console.log(await Promise.all([fs0, fs1, fp, fsp]));

        console.log('&&&& Test a big batch of statements with manual transaction &&&&');

        var sql = 'delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from company;select name,joindate,salary from employee;select curtime()|call sp_TestProc(?,?,?)';
        buff = SPA.newBuffer();
        blob = SPA.newBuffer();

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
        //second, prepare a sql statement for insert into company
        //third, execute three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        //fourth, prepare a sql statement for insert into employee
        //fifth, execute three sets of insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
        //sixth, SELECT * from company;select * from employee;select curtime()
        //last, three sets of call sp_TestProc(?,?,?)
        var fBatch = db.executeBatch(SPA.DB.TransIsolation.ReadCommited, sql, buff, (data, proc, cols) => {
            console.log({ data: data, proc: proc, cols: cols });
        }, (meta) => {
            console.log(meta);
        }, '|', () => {
            console.log('Batch header comes');
        });
        console.log(await fBatch);
    } catch (ex) {
        console.log(ex);
    }
})();
