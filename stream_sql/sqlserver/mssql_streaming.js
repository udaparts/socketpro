'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidMsSql);
global.p = p;

//create a connection context to SQL Server DB plugin running at port 20903
var cc = cs.newCC('localhost', 20901, 'sa', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var db = p.Seek() //seek an async DB handler
if (!db.Open('', (res, err) => {
    if (res) console.log({ ec: res, em: err });
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
    if (!db.Execute("use master;IF NOT EXISTS(SELECT * FROM sys.databases WHERE name='mydevdb')BEGIN CREATE DATABASE mydevdb; END;USE mydevdb", (res, err) => {
        if (res) console.log({ ec: res, em: err });
    })) {
        return false;
    }
    if (!db.Execute("IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='company') create table company(ID bigint PRIMARY KEY NOT NULL,name varCHAR(64)NOT NULL,ADDRESS varCHAR(256)not null,Income float not null)", (res, err) => {
        if (res) console.log({ ec: res, em: err });
    })) {
        return false;
    }
    if (!db.Execute("IF NOT EXISTS(SELECT * FROM sys.tables WHERE name='employee')create table employee(EMPLOYEEID bigint IDENTITY PRIMARY KEY,CompanyId bigint not null,name varCHAR(64)NOT NULL,JoinDate DATETIME2(3)default null,MyIMAGE varbinary(max),DESCRIPTION nvarchar(max),Salary decimal(15,2),FOREIGN KEY(CompanyId)REFERENCES company(id))", (res, err) => {
        if (res) console.log({ ec: res, em: err });
    })) {
        return false;
    }
    var drop_proc = "IF EXISTS(SELECT * FROM sys.procedures WHERE name='sp_TestProc')drop proc sp_TestProc";
    if (!db.Execute(drop_proc, (res, err) => {
        if (res) console.log({ ec: res, em: err });
    })) {
        return false;
    }
    if (!db.Execute("CREATE PROCEDURE sp_TestProc(@p_company_id int,@p_sum_salary float OUT,@p_last_dt datetime2(3) out)as select * from employee where companyid>=@p_company_id;select @p_sum_salary=sum(salary)+@p_sum_salary from employee where companyid>=@p_company_id;select @p_last_dt=SYSDATETIME()", (res, err) => {
        if (res) console.log({ ec: res, em: err });
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
    console.log({ ec: res, em: err, aff: affected });
})) {
    console.log(db.Socket.Error);
    return;
}

function TestPreparedStatements(db) {
    //No need to set parameter infos if all of them are inputs
    if (!db.Prepare('INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)', (res, err) => {
        if (res) console.log({ ec: res, em: err });
    })) {
        return false;
    }

    //set an array of parameter data
    var vParam = [1, 'Google Inc.', '1600 Amphitheatre Parkway, Mountain View, CA 94043, USA', '66000000000.15', //1st set
        2, 'Microsoft Inc.', '700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA', 93600000000.12, //2nd set
        3, 'Apple Inc.', '1 Infinite Loop, Cupertino, CA 95014, USA', 234000000000.14]; //3rd set

    //send three sets in one shot
    if (!db.Execute(vParam, (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    })) {
        return false;
    }
    return true;
}
if (!TestPreparedStatements(db)) {
    console.log(db.Socket.Error);
    return;
}

function TestBLOBByPreparedStatement(db) {
    //No need to set parameter infos if all of them are inputs
    if (!db.Prepare('insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(?,?,?,?,?,?)', (res, err) => {
        if (res) console.log({ ec: res, em: err });
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
    if (!db.Execute(buff, (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    })) {
        return false;
    }
    return true;
}
if (!TestBLOBByPreparedStatement(db)) {
    console.log(db.Socket.Error);
    return;
}

if (!db.Execute('SELECT * from company;Select getdate()', (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    }, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        //console.log(meta);
    })) {
    console.log(db.Socket.Error);
    return;
}

if (!db.Execute('select name,joindate,salary from employee', (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    }, (data, proc, cols) => {
        console.log({ data: data, proc: proc, cols: cols });
    }, meta => {
        //console.log(meta);
    })) {
    console.log(db.Socket.Error);
    return;
}

function TestStoredProcedure(db) {
    var types = SPA.DB.DataType;
    var pds = SPA.DB.ParamDirection;
    //All parameter infos must be set if one or more InputOutput or Output parameters are involved
    var vParamInfo = [{ DataType: types.Int },
    { Direction: pds.InputOutput, DataType: types.Double },
    { Direction: pds.Output, DataType: types.DateTime }];
    if (!db.Prepare('exec mydevdb.dbo.sp_TestProc ?,?,?', (res, err) => {
        if (res) console.log({ ec: res, em: err });
    }, null, vParamInfo)) {
        return false;
    }
    var dt = new Date(1970, 1, 1);
    var vParam = [1, 1.25, dt, //1st set
        2, 1.14, dt, //2nd set
        0, 2.18, dt]; //3rd set
    if (!db.Execute(vParam, (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    }, (data, proc, cols) => {
        if (proc) {
            console.log({ data: data, proc: proc, cols: cols });
        }
    }, meta => {
        console.log(meta);
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
    var sql = 'delete from employee;delete from company|INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)|insert into employee(CompanyId,name,JoinDate,myimage,DESCRIPTION,Salary)values(?,?,?,?,?,?)|SELECT * from company;select name,joindate,salary from employee;select getdate()|exec mydevdb.dbo.sp_TestProc ?,?,?';
    var buff = SPA.newBuffer();
    var blob = SPA.newBuffer();
    var types = SPA.DB.DataType;
    var pds = SPA.DB.ParamDirection;
    //All parameter infos must be set if one or more InputOutput or Output parameters are involved
    var vParamInfo = [{ DataType: types.Int }, { DataType: types.AStr, ColumnSize: 64 }, { DataType: types.AStr, ColumnSize: 256 }, { DataType: types.Double },
    { DataType: types.Int }, { DataType: types.AStr, ColumnSize: 64 }, { DataType: types.DateTime }, { DataType: types.Binary, ColumnSize: 0x7fffffff }, { DataType: types.WStr, ColumnSize: 0x7fffffff }, { DataType: types.Double },
    { DataType: types.Int }, { Direction: pds.InputOutput, DataType: types.Double }, { Direction: pds.Output, DataType: types.DateTime }];

    var dt = new Date(1970, 1, 1);

    //1st set
    //INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    buff.SaveObject(1).SaveObject('Google Inc.').SaveObject('1600 Amphitheatre Parkway, Mountain View, CA 94043, USA').SaveObject(66000000000.15);
    //insert into employee(CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?)
    buff.SaveObject(1); //Google company id
    blob.SaveString(wstr); //UNICODE string
    buff.SaveObject('Ted Cruz').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject(254000.15);
    //call sp_TestProc(?,?,?)
    buff.SaveObject(1).SaveObject(1.25).SaveObject(dt);

    //2nd set
    buff.SaveObject(2).SaveObject('Microsoft Inc.').SaveObject('700 Bellevue Way NE- 22nd Floor, Bellevue, WA 98804, USA').SaveObject('93600000000.12');
    buff.SaveObject(1); //Google company id
    blob.SaveAString(str); //ASCII string
    buff.SaveObject('Donald Trump').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(str).SaveObject(20254000);
    buff.SaveObject(2).SaveObject(1.14).SaveObject(dt);

    //3rd set
    buff.SaveObject(3).SaveObject('Apple Inc.').SaveObject('1 Infinite Loop, Cupertino, CA 95014, USA').SaveObject(234000000000.14);
    buff.SaveObject(2); //Microsoft company id
    blob.SaveAString(str).SaveString(wstr);
    buff.SaveObject('Hillary Clinton').SaveObject(new Date()).SaveObject(blob.PopBytes()).SaveObject(wstr).SaveObject('6254000.15');
    buff.SaveObject(0).SaveObject(8.16).SaveObject(dt);

    //first, execute delete from employee;delete from company
    //second, three sets of INSERT INTO company(ID,NAME,ADDRESS,Income)VALUES(?,?,?,?)
    //third, three sets of insert into employee(EMPLOYEEID,CompanyId,name,JoinDate,image,DESCRIPTION,Salary)values(?,?,?,?,?,?,?)
    //fourth, SELECT * from company;select name,joindate,salary from employee;Select getdate()
    //last, three sets of exec mydevdb.dbo.sp_TestProc ?,?,?
    if (!db.ExecuteBatch(SPA.DB.TransIsolation.ReadCommited, sql, buff, (res, err, affected, fails, oks, id) => {
        console.log({ ec: res, em: err, aff: affected, oks: oks, fails: fails, lastId: id });
    }, (data, proc, cols) => {
        if (proc) {
            console.log({ data: data, proc: proc, cols: cols });
        }
    }, meta => {
        //console.log(meta);
    }, '|', () => {
        console.log('Batch header comes');
    }, null, true, SPA.DB.RollbackPlan.rpDefault, vParamInfo)) {
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
executeSql(db, 'SELECT * from company;Select getdate()', (data, proc, cols) => {
    console.log({ data: data, proc: proc, cols: cols });
}, meta => {
    console.log(meta);
});
