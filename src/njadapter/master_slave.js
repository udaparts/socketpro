//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
var cs = SPA.CS; //CS == Client side

//create a global socket pool object
master = cs.newPool(SPA.SID.sidMysql, 'mysqldb'); //or sidOdbc, sidSqlite

//create a connection context
var cc_master = cs.newCC('windesk', 20902, 'root', 'Smash123');

//start a socket pool having two sessions to a remote master server
if (!master.Start(cc_master, 2)) {
    console.log(master.Error);
    return;
}
var db = master.Seek(); //seek an async DB handler from master pool
var cache = master.Cache;

console.log('');
console.log('Cached DB Tables');
console.log(cache.DbTable);

console.log('');
console.log('Keys');
console.log(cache.FindKeys('sakila', 'actor'));

console.log('');
console.log('Table Meta');
console.log(cache.GetMeta('sakila', 'actor'));

console.log('');
console.log('Rows: ' + cache.GetRowCount('sakila', 'actor'));

console.log('');
console.log('Colums: ' + cache.GetFields('sakila', 'actor'));

console.log('');
console.log('sakila.actor last_name Ordinal: ' + cache.FindOrdinal('sakila', 'actor', 'last_name'));

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35');
var tbl = cache.Between('sakila', 'actor', 0, 5, 35);
console.log(tbl.Data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id BETWEEN 5 and 35 ORDER BY actor_id DESC');
var ok = tbl.Sort(0, true);
console.log(tbl.Data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id IN(9,10) ORDER BY actor_id DESC');
var sub = tbl.In(0, [9, 10]);
console.log(sub.Data);

console.log('');
console.log('SELECT * FROM sakila.actor WHERE actor_id NOT IN(24,9,10) ORDER BY actor_id DESC');
sub = tbl.NotIn(0, [24, 9, 10]);
console.log(sub.Data);

//create slave pool from master
var slave = master.NewSlave();
var cc_slave = cs.newCC('localhost', 20902, 'root', 'Smash123');
//start a socket pool having four sessions to remote slave servers
if (!slave.Start([cc_slave, cc_master], 4)) {
    console.log(slave.Error);
    return;
}
db = slave.Seek();
console.log('');
console.log('SELECT curtime();SELECT * FROM company');
if (!db.Execute('SELECT curtime();SELECT * FROM company', (res, err, affected, fails, oks, id) => {
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
        console.log(meta);
    })) {
    console.log(db.Socket.Error);
    return;
}
