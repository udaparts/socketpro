//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
p = cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite

//create a connection context
var cc = cs.newCC('10.16.50.19', 20902, 'root', 'Smash123');

//ALTER USER 'root'@'%' IDENTIFIED WITH caching_sha2_password BY 'Smash123'
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

function TestPerf(db) {
	var start = new Date();
	var count = 10000;
	for (var n = 0; n < count; ++n) {
		db.Execute('SELECT * FROM sakila.actor WHERE actor_id between 11 and 20', (res, err, affected, fails, oks, id) => {}, data=>{}, meta=>{});
	}
	db.Execute('SELECT * FROM sakila.actor WHERE actor_id between 11 and 20', (res, err, affected, fails, oks, id) => {
		console.log('Time required: ' + (new Date() - start));
	}, data=>{
		//console.log(data);
	}, meta=>{});
}

TestPerf(db);
