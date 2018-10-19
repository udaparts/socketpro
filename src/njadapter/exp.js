var express = require('express');
var app = express();
var ms = require('webpool.js');
global.master_slave = ms;

app.get('/cached_tables', function(req, res) {
    res.send('Cached tables: ' + JSON.stringify(ms.Cache.DbTable));
});

app.get('/cache', function(req, res) {
    var tbl_name = req.query.tbl;
    var ordinal = parseInt(req.query.ordinal);
    var v0 = req.query.v0;
    var v1 = req.query.v1;
    var tbl = ms.Cache.Between('sakila', tbl_name, ordinal, v0, v1);
    res.send('Cached ' + tbl_name + ' records: ' + JSON.stringify(tbl.Data));
});

app.listen(3000);
