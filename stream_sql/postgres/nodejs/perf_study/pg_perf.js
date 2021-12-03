const { Pool } = require('pg');

const pool = new Pool({
    host: 'acer',
    user: 'postgres',
    password: 'Smash123',
    database: 'sakila',
    max: 8
});

pool.connect((err, client, release) => {
    if (err) {
        return console.error('Error acquiring client', err.stack);
    }
});

function TestPerf() {
    var count = 3200;
    var start = new Date();
    var time = start.getTime();
    var stmt = 'SELECT * FROM actor where actor_id=' + (time % 200 + 1);
    for (var n = 0; n < count; ++n) {
        pool.query(stmt, (err, res) => {
            if (err) {
                return console.error('Error executing query', err.stack);
            }
            //console.log(res.rows)
        });
    }
    pool.query('select 1', (err, res) => {
        if (err) {
            return console.error('Error executing query', err.stack);
        }
        console.log('Time required: ' + (new Date() - start));
        //console.log(res.rows)
    });
}

setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
