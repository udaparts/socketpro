"use strict";

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidSqlite); //or sidOdbc, sidSqlite, sidMysql
global.p = p;

//create a connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var db = p.Seek(); //seek an async DB handler

if (!db.Open('sakila', (res, err) => {
        if (res) console.log({
            ec: res,
            em: err
        });
    }, canceled => {
        console.log(canceled ? 'request canceled' : 'session closed');
    })) {
    console.log(db.Socket.Error);
    return;
}

//make long strings for testing long text and blob objects
var wstr = '';
while (wstr.length < 256 * 1024) {
	wstr += '广告做得不那么夸张的就不说了，看看这三家，都是正儿八经的公立三甲，附属医院，不是武警，也不是部队，更不是莆田，都在卫生部门直接监管下，照样明目张胆地骗人。';
}
var str = '';
while (str.length < 512 * 1024) {
	str += 'The epic takedown of his opponent on an all-important voting day was extraordinary even by the standards of the 2016 campaign -- and quickly drew a scathing response from Trump.';
}

function TestCreateTables(db) {
    //track final result event, but ignore row, metat and cancel events
    if (!db.Execute('CREATE TABLE COMPANY(ID INT8 PRIMARY KEY NOT NULL,name CHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)', (res, err) => {
            if (res) console.log({
                ec: res,
                em: err
            });
        })) {
        return false;
    }
    if (!db.Execute("CREATE TABLE EMPLOYEE(EMPLOYEEID INT8 PRIMARY KEY NOT NULL unique,CompanyId INT8 not null,name NCHAR(64)NOT NULL,JoinDate DATETIME not null default(datetime('now')),IMAGE BLOB,DESCRIPTION NTEXT,Salary real,FOREIGN KEY(CompanyId)REFERENCES COMPANY(id))", (res, err) => {
            if (res) console.log({
                ec: res,
                em: err
            });
        })) {
        return false;
    }
    return true;
}
if (!TestCreateTables(db)) {
    console.log(db.Socket.Error);
    return;
}
if (!db.Execute('delete from employee;delete from company', (res, err, affected) => {
        console.log({
            ec: res,
            em: err,
            aff: affected
        });
    })) {
    console.log(db.Socket.Error);
    return;
}

function TestPreparedStatements(db) {
    if (!db.Prepare("Select datetime('now');INSERT OR REPLACE INTO COMPANY(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)", (res, err) => {
            if (res) console.log({
                ec: res,
                em: err
            });
        })) {
        return false;
    }

    //set an array of parameter data
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', '66000000000.15', //1st set
     2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600000000.12, //2nd set
     3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000000000.14]; //3rd set

    //send three sets in one shot
    if (!db.Execute(vParam, (res, err, affected, fails, oks, id) => {
            console.log({
                ec: res,
                em: err,
                aff: affected,
                oks: oks,
                fails: fails,
                lastId: id
            });
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
    if (!db.Prepare('insert or replace into employee(EMPLOYEEID,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)', (res, err) => {
            if (res) console.log({
                ec: res,
                em: err
            });
        })) {
        return false;
    }
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();

    blob.SaveString(wstr);
    //1st set
    //blob.PopBytes() -- convert all data inside blob memory into an array of bytes
    buff.SaveObject(1).SaveObject(1).SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject('254000.15');

    blob.SaveAString(str);
    //2nd set
    buff.SaveObject(2).SaveObject(1).SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(str).SaveObject(20254000.35);

    blob.SaveAString(str).SaveString(wstr);
    //3rd set
    buff.SaveObject(3).SaveObject(2).SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(6254000.42);

    //send three sets in one shot
    if (!db.Execute(buff, (res, err, affected, fails, oks, id) => {
            console.log({
                ec: res,
                em: err,
                aff: affected,
                oks: oks,
                fails: fails,
                lastId: id
            });
        })) {
        return false;
    }
    return true;
}
if (!InsertBLOBByPreparedStatement(db)) {
    console.log(db.Socket.Error);
    return;
}

if (!db.Execute("SELECT * from company;Select datetime('now')", (res, err, affected, fails, oks, id) => {
        console.log({
            ec: res,
            em: err,
            aff: affected,
            oks: oks,
            fails: fails,
            lastId: id
        });
    }, data => {
        console.log(data);
    }, meta => {
        //console.log(meta);
    })) {
    console.log(db.Socket.Error);
    return;
}

if (!db.Execute('select name, joindate, salary from employee', (res, err, affected, fails, oks, id) => {
        console.log({
            ec: res,
            em: err,
            aff: affected,
            oks: oks,
            fails: fails,
            lastId: id
        });
    }, data => {
        console.log(data);
    }, meta => {
        //console.log(meta);
    })) {
    console.log(db.Socket.Error);
    return;
}

function TestBatch(db) {
    var sql = "delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee values(?,?,?,?,?,?,?)|SELECT * from company;select employeeid,companyid,name,joindate,salary from employee;Select datetime('now')";
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();

    //1st set
    //INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    buff.SaveObject(1).SaveObject('Google Inc.').SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15);
    //insert into employee(employeeid,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)
    buff.SaveObject(1).SaveObject(1); //Google company id
    blob.SaveString(wstr); //UNICODE string
    buff.SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(254000.15);

    //2nd set
    buff.SaveObject(2).SaveObject('Microsoft Inc.').SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject('93600000000.12');
    buff.SaveObject(2).SaveObject(1); //Google company id
    blob.SaveAString(str); //ASCII string
    buff.SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(str, 'a').SaveObject(20254000);

    //3rd set
    buff.SaveObject(3).SaveObject('Apple Inc.').SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14);
    buff.SaveObject(3).SaveObject(2); //Microsoft company id
    blob.SaveAString(str).SaveString(wstr);
    buff.SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject('6254000.15');

    //first, execute delete from employee;delete from company
    //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //third, three sets of insert into employee values(?,?,?,?,?,?,?)
    //last, SELECT * from company;select * from employee;Select datetime('now')
    if (!db.ExecuteBatch(SPA.DB.TransIsolation.ReadCommited, sql, buff, (res, err, affected, fails, oks, id) => {
            console.log({
                ec: res,
                em: err,
                aff: affected,
                oks: oks,
                fails: fails,
                lastId: id
            });
        }, (data, proc) => {
			console.log(data); //output output parameters
        }, meta => {
            //console.log(meta);
        }, dbH => {
            console.log('Batch header comes');
        }, canceled => {
            console.log(canceled ? 'Request canceled' : 'Session closed');
        }, SPA.DB.RollbackPlan.rpDefault, "|")) {
        console.log(db.Socket.Error);
        return false;
    }
    return true;
}
if (!TestBatch(db)) {
    console.log(db.Socket.Error);
    return;
}

async function executeSql(db, sql, rows, meta) {
    try {
        //use execute instead of Execute for Promise
        var result = await db.execute(sql, rows, meta)
        console.log(result);
    } catch (err) {
        console.log(err);
    }
}
executeSql(db, "SELECT * from company;Select datetime('now')", data => {
    console.log(data);
}, meta => {
    //console.log(meta);
});
