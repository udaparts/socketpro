var express = require('express');
var app = express();
var bodyParser = require('body-parser');
var ms = require('./webpool.js');
global.master_slave = ms;

// Create application/x-www-form-urlencoded parser
var urlencodedParser = bodyParser.urlencoded({
    extended: false
})
app.use(express.static('public'));

app.get('/cached_tables', function(req, res) {
    res.send('Cached tables: ' + JSON.stringify(ms.Cache.DbTable));
});

app.get('/cache_tbl.htm', function(req, res) {
    res.sendFile(__dirname + '/cache_tbl.htm');
});
app.get('/between', function(req, res) {
    var tbl_name = req.query.tbl;
    var ordinal = parseInt(req.query.ordinal);
    var v0 = req.query.v0;
    var v1 = req.query.v1;
    //get table from real-time updateable cache
    var tbl = ms.Cache.Between('sakila', tbl_name, ordinal, v0, v1);
    res.send('Cached ' + tbl_name + ' records: ' + JSON.stringify(tbl.Data));
});

app.get('/mma.htm', function(req, res) {
    res.sendFile(__dirname + '/mma.htm');
});
app.post('/mma', urlencodedParser, function(req, res) {
    var filter = req.body.idFilter;
    var sql = 'SELECT MAX(amount),MIN(amount),AVG(amount) FROM payment';
    if (filter) sql += (' WHERE ' + filter);
    var db = ms.slave.SeekByQueue(); //get a DB hander from slave pool for reading
    db.Execute(sql, (ec, err) => {
        if (ec) res.send(JSON.stringify({ec: ec, em: err}));
    }, data => {
        res.send(JSON.stringify(data));
    });
});

app.get('/inserts', function(req, res) {
    const na = 'Master database not accessible';
    var db = ms.master.Seek(); //get a DB hander from master pool for writing and updating
    if (db) {
        var vParam = [1, 'Ted Cruz', new Date(),
		1, 'Donald Trump', new Date(),
		2, 'Hillary Clinton', new Date()];
        //prepare + begin-trans + three inserts + end-trans
        if (db.ExecuteBatch(ms.spa.DB.TransIsolation.ReadCommited,
                'INSERT INTO mysample.EMPLOYEE(CompanyId,Name,JoinDate)VALUES(?,?,?)',
                vParam, (ec, err, affected, fails, oks, id) => {
                    if (ec)
                        res.send(JSON.stringify({ec: err, em: err}));
                    else
                        res.send('Last employeeid = ' + id + ', affected = ' + affected);
                }, null, null, null, (canceled) => {
                    res.send(canceled ? 'Request canceled' : 'Database session closed');
                })) {
            return;
        }
    }
    res.send(na);
});
app.listen(20901);
