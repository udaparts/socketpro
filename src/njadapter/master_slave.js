//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA=require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
p=cs.newPool(SPA.SID.sidMysql, 'sakila'); //or sidOdbc, sidSqlite

//create a connection context
var cc = cs.newCC('localhost',20902,'root','Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc,4)) {
	console.log(p.getError());
	return;
}
var db = p.Seek(); //seek an async DB handler
var cache = p.getCache();

var dbIp = cache.getDbIp();
var dbName= cache.getDbName();
var updater= cache.getUpdater();
var ms = cache.getMS();
var dbTable = cache.getDbTable();

console.log(dbIp);
console.log(dbName);
console.log(updater);
console.log(ms);
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

var tbl = cache.Find('sakila', 'actor', 0, SPA.Cache.Op.gt, 20);
meta = tbl.getMeta();
console.log(meta);
tbl.Dispose();




