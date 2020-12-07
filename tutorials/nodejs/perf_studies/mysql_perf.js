const mysql = require('mysql');
const pool = mysql.createPool({
    connectionLimit: 4,
    host: 'windesk',
    user: 'nodejs',
    password: 'Smash123',
    database: 'sakila'
});

function PerfTest() {
    var count = 1180;
    var stmt = 'SELECT * FROM actor WHERE actor_id between 11 and 12';
    var start = new Date();
    for (var n = 0; n < count; ++n) {
        pool.query(stmt, (error, results, fields) => {
            if (error) throw error;
        });
    }
    pool.query(stmt, (error, results, fields) => {
        if (error) throw error;
        console.log('Time required: ' + (new Date() - start));
    });
}

setInterval(PerfTest, 500);
setInterval(PerfTest, 500);
setInterval(PerfTest, 500);
