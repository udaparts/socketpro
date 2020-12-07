const mysqlx = require('@mysql/xdevapi');

const client = mysqlx.getClient(
    { host: 'windesk', port: 33060, user: 'root', password: 'Smash123', schema: 'sakila' },
    { pooling: { enabled: true, maxIdleTime: 30000, maxSize: 3, queueTimeout: 10000 } }
);

function TestPerf() {
    client.getSession().then((session) => {
        var row = null, count = 2500;
        var stmt = 'SELECT * FROM actor WHERE actor_id between 11 and 12';
        var start = new Date();
        for (var n = 0; n < count; ++n) {
            session.sql(stmt).execute()
                .then((res) => {
                    while (row = res.fetchOne()) {
                    }
                });
        }
        session.sql(stmt).execute()
            .then((res) => {
                while (row = res.fetchOne()) {
                    //console.log(row);
                }
                console.log('Time required: ' + (new Date() - start));
                //put session back into pool for reuse in the future
                session.close();
            });
    });
}
setInterval(TestPerf, 500);
setInterval(TestPerf, 500);
