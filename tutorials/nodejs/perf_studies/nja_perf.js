const SPA = require('nja.js');
const cs = SPA.CS; //CS == Client side

p = cs.newPool(SPA.SID.sidMysql); //or sidOdbc, sidSqlite, sidMysql

if (!p.Start(cs.newCC('windesk', 20902, 'root', 'Smash123'), 6)) {
    console.log(p.Error);
    return;
}

function TestPerf() {
    var db = p.Seek();
    if (!db.Opened) {
        db.Open('sakila');
    }
    var count = 3600;
    var stmt = 'SELECT * FROM actor where actor_id between 11 and 12';
    var start = new Date();
    for (var n = 0; n < count; ++n) {
        db.Execute(stmt, (res, err, affected, fails, oks, id) => {
        }, (data, proc, cols) => {
        });
    }
    db.Execute(stmt, (res, err, affected, fails, oks, id) => {
        console.log('Time required: ' + (new Date() - start));
    }, (data, proc, cols) => {
        //console.log({ data: data, proc: proc, cols: cols });
    });
}
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
