const sql = require('mssql');
var config = {
    user: 'sa',
    password: 'Smash123',
    server: 'windesk',
    database: 'sakila',
};

sql.connect(config, (err) => {
    if (err) console.log(err);
});

function TestPerf() {
    var count = 800;
    var req = new sql.Request();
    var start = new Date();
    var stmt = 'SELECT * FROM actor where actor_id=' + (start.getTime() % 200 + 1);
    for (var n = 0; n < count; ++n) {
        req.query(stmt, (err, result) => {
	    if (err) console.log(err);
	});
    }
    req.query(stmt, (err, result) => {
	if (err) console.log(err);
	else {
            //console.log(result.recordset);
	}
	console.log('Time required: ' + (new Date() - start));
    });
}

setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);

