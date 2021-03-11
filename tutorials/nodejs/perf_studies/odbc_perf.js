const SPA = require('nja.js');
const cs = SPA.CS; //CS == Client side
cs.TLS.CA = 'ca.cert.pem';

p = cs.newPool(SPA.SID.sidOdbc); //or sidOdbc, sidSqlite, sidMysql

if (!p.Start(cs.newCC('windesk', 20903, 'sa', 'Smash123'), 8)) {
    console.log(p.Error);
    return;
}

function TestPerf() {
    var db = p.Seek();
    if (!db.Opened) {
        db.Open('sakila');
        //db.Prepare('SELECT * FROM actor where actor_id=?');
    }
    var count = 3000;
    var start = new Date();
    //var vParam = [start.getTime() % 200 + 1];
    var stmt = 'SELECT * FROM actor where actor_id=' + (start.getTime() % 200 + 1);
    for (var n = 0; n < count; ++n) {
        db.Execute(stmt, (res, err, affected, fails, oks, id) => {
        }, (data, proc, cols) => {
        });
    }
    db.Execute(stmt, (res, err, affected, fails, oks, id) => {
        if (res) console.log(err);
        console.log('Time required: ' + (new Date() - start));
    }, (data, proc, cols) => {
        //console.log({ data: data, proc: proc, cols: cols });
    }, (meta) => {
        //console.log(meta);
    });
}
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
