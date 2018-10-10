//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
master=cs.newPool(SPA.SID.sidMysql, 'mysqldb'); //or sidOdbc, sidSqlite

//create a connection context
var cc_master = cs.newCC('localhost',20902,'root','Smash123');

//start a socket pool having two sessions to a remote master server
if (!master.Start(cc_master,2)) {
	console.log(master.getError());
	return;
}
var db = master.Seek(); //seek an async DB handler from master pool
//get real-time updateable cache
var cache = master.getCache();

console.log('');
console.log('Cached DB Tables');
var dbTable = cache.getDbTable();
console.log(dbTable);

console.log('');
console.log('Keys');
var keys = cache.FindKeys('sakila', 'actor');
console.log(keys);

console.log('');
console.log('Table Meta');
var meta = cache.GetMeta('sakila', 'actor');
console.log(meta);

console.log('');
var rows = cache.GetRowCount('sakila', 'actor');
console.log('Rows: ' + rows);

console.log('');
var columns = cache.GetFields('sakila', 'actor');
console.log('Colums: ' + columns);

console.log('');
var ordinal = cache.FindOrdinal('sakila', 'actor', 'last_name');
console.log('Ordinal: ' + ordinal);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35');
var tbl = cache.Between('sakila', 'actor', 0, 5, 35);
var data = tbl.getData();
console.log(data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35 ORDER BY actor_id DESC');
var ok = tbl.Sort(0, true);
data = tbl.getData();
console.log(data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id IN(9,10) ORDER BY actor_id DESC');
var sub = tbl.In(0, [9,10]);
data = sub.getData();
console.log(data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id NOT IN(24,9,10) ORDER BY actor_id DESC');
sub = tbl.NotIn(0, [24, 9, 10]);
data = sub.getData();
console.log(data);

//create slave pool from master
slave = master.NewSlave();
var cc_slave = cs.newCC('windesk',20902,'root','Smash123');
//start a socket pool having four sessions to remote slave servers
if (!slave.Start([cc_slave, cc_master],4)) {
	console.log(slave.getError());
	return;
}
db = slave.Seek(); //seek an async DB handler from slave pool
console.log('');
console.log('SELECT curtime();SELECT * FROM company');
if (!db.Execute('SELECT curtime();SELECT * FROM company', (res, err, affected, fails, oks, id)=>{
		console.log({ec:res, em:err, affected:affected, oks:oks, fails:fails, lastId:id});
	}, data=>{
		console.log(data);
	}, meta=>{
		console.log(meta);
	})) {
	console.log(db.getSocket().getError());
	return;
}
