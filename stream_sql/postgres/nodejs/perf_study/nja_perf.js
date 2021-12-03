const SPA = require('nja.js');
const cs = SPA.CS; //CS == Client side

//sidMysql, sidMsSql or other
p = cs.newPool(SPA.SID.sidPostgres);

if (!p.Start(cs.newCC('acer', 20901, 'postgres', 'Smash123'), 5)) {
    console.log(p.Error);
    return;
}

function TestPerf() {
    var db = p.Seek();
    if (!db.Opened) {
        db.open('sakila', 2);
    }
    var count = 3900;
    var start = new Date();
    var time = start.getTime();
    var stmt = 'SELECT * FROM actor where actor_id=' + (time % 200 + 1);
    for (var n = 0; n < count; ++n) {
        db.Execute(stmt, (res, err, affected, fails, oks, id) => {
        }, (data, proc, cols) => {
        });
    }
    db.Execute('select 1', (res, err, affected, fails, oks, id) => {
        console.log('Time required: ' + (new Date() - start));
    }, (data, proc, cols) => {
        //console.log({ data: data, proc: proc, cols: cols });
    });
}
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
