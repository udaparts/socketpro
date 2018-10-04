//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
master=cs.newPool(SPA.SID.sidMysql, 'sakila'); //or sidOdbc, sidSqlite

//create a connection context
var cc_master = cs.newCC('localhost',20902,'root','Smash123');

//start a socket pool having two sessions to a remote master server
if (!master.Start(cc_master,2)) {
	console.log(master.getError());
	return;
}
var db = master.Seek(); //seek an async DB handler from master pool
var cache = master.getCache();

var dbTable = cache.getDbTable();
console.log(dbTable);

var keys = cache.FindKeys('sakila', 'actor');
console.log(keys);

var meta = cache.GetMeta('sakila', 'actor');
console.log(meta);
var rows = cache.GetRowCount('sakila', 'actor');
console.log(rows);
var columns = cache.GetFields('sakila', 'actor');
console.log(columns);
var ordinal = cache.FindOrdinal('sakila', 'actor', 'last_name');
console.log(ordinal);

var tbl = cache.Between('sakila', 'actor', 0, 5, 35);
var data = tbl.getData();
console.log(data);

var ok = tbl.Sort(0, true);
data = tbl.getData();
console.log(data);

var sub = tbl.In(0, [9,10]);
data = sub.getData();
console.log(data);

tbl.Append(sub);
data = tbl.getData();
console.log(data);

sub = tbl.NotIn(0, [24, 9, 10]);
data = sub.getData();
console.log(data);

slave = master.NewSlave();
var cc_slave = cs.newCC('ws-yye-1',20902,'root','Smash123');
//start a socket pool having four sessions to remote slave servers
if (!slave.Start([cc_slave, cc_master],4)) {
	console.log(slave.getError());
	return;
}
db = slave.Seek();



