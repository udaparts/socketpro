'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidSqlite); //or sidOdbc, sidMysql
global.p = p;

if (!p.Start(cs.newCC('localhost', 20901, 'root', 'Smash123'), 1)) {
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
        var res = await db.open('usqlite.db');
        if (res.ec) {
            console.log(res.em);
            return;
        }

        console.log('++++ test creating two tables ++++');
        var pCt0 = db.execute('CREATE TABLE IF NOT EXISTS COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)'+
            'NOT NULL, ADDRESS varCHAR(256)not null, Income float not null)');
        var pCt1 = db.execute("CREATE TABLE IF NOT EXISTS EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique," +
            "CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime" +
            "('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))");

        var pd = db.execute('delete from employee;delete from company');

        console.log('++++ test prepare statements ++++');
        var pp = db.prepare("Select datetime('now');" +
            "INSERT OR REPLACE INTO COMPANY(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)");
        //set an array of parameter data
        var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', 66000000000.15,
            2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600000000.12,
            3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000000000.14]; //3rd set
        var ppi = db.execute(vParam, (data, proc, cols) => {
            console.log({ data: data, proc: proc, cols: cols });
        }, (meta) => {
            //console.log(meta);
        });

        console.log('**** test prepare statements with long texts and blobs ****');
        var pp1 = db.prepare("insert or replace into employee(EMPLOYEEID,CompanyId,name,JoinDate,image," +
            "DESCRIPTION,Salary)values(?,?,?,?,?,?,?)");
        var buff = SPA.newBuffer();
        var blob = SPA.newBuffer();

        blob.SaveString(wstr);
        //1st set
        //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
        buff.SaveObject(1).SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(254000.15);

        blob.SaveAString(str);
        //2nd set
        buff.SaveObject(2).SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(str).SaveObject(20254000.35);

        blob.SaveAString(str).SaveString(wstr);
        //3rd set
        buff.SaveObject(3).SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(6254000.42);
        var ppi1 = db.execute(buff);

        console.log('&&&& Test a big batch of statements with manual transaction &&&&');
        var sql = "delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES" +
            "(?,?,?,?)|insert into employee values(?,?,?,?,?,?,?)|SELECT * from company;select employeeid," +
            "companyid,name,joindate,salary from employee;Select datetime('now')";
        buff = SPA.newBuffer();
        blob = SPA.newBuffer();

        //1st set
        //INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        buff.SaveObject(1).SaveObject('Google Inc.').
            SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15);
        //insert into employee(employeeid,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)
        buff.SaveObject(1).SaveObject(1); //Google company id
        blob.SaveString(wstr); //UNICODE string
        buff.SaveObject('Ted Cruz').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(254000.15);

        //2nd set
        buff.SaveObject(2).SaveObject('Microsoft Inc.').
            SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject(93600000000.12);
        buff.SaveObject(2).SaveObject(1); //Google company id
        blob.SaveAString(str); //ASCII string
        buff.SaveObject('Donald Trump').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(str, 'a').SaveObject(20254000.32);

        //3rd set
        buff.SaveObject(3).SaveObject('Apple Inc.').
            SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14);
        buff.SaveObject(3).SaveObject(2); //Microsoft company id
        blob.SaveAString(str).SaveString(wstr);
        buff.SaveObject('Hillary Clinton').SaveObject(new Date()).
            SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(6254000.15);
        //first, start a manual transaction
        //second, execute delete from employee;delete from company
        //third, prepare a sql statement for insert into company
        //fourth, execute three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
        //fifth, prepare a sql statement for insert into employee
        //sixth, execute three sets of insert into employee values(?,?,?,?,?,?,?)
        //seventh, SELECT * from company;select employeeid,companyid,name,joindate,salary from employee;
        //      Select datetime('now')
        //last, end manual transaction
        var pBatch = db.executeBatch(SPA.DB.TransIsolation.ReadCommited, sql, buff, (data, proc, cols) => {
            console.log({ data: data, proc: proc, cols: cols });
        }, (meta) => {
            //console.log(meta);
        }, '|', () => {
            console.log('----- Batch header comes -----');
        });
        console.log(await Promise.all([pCt0, pCt1, pd, pp, ppi, pp1, ppi1, pBatch]));
    } catch (ex) {
        console.log(ex);
    }
})();
