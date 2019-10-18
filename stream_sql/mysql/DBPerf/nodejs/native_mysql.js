var mysqlx = require('@mysql/xdevapi');
mysqlx.getSession({
    host: "10.16.50.19",
    user: "root",
    password: "Smash123"
}).then((session)=> {
	var row;
	var stmt = 'SELECT * FROM sakila.actor WHERE actor_id between 11 and 20';
	var start = new Date();
	var count = 100000;
	for (var n = 0; n < count; ++n) {
		res = session.sql(stmt)
		.execute()
		.then((res)=>{
			while (row = res.fetchOne()) {
			}
		});
	}
	res = session.sql(stmt)
	.execute()
	.then((res)=>{
		while (row = res.fetchOne()) {
		}
		console.log('Time required: ' + (new Date() - start));
	});
});
