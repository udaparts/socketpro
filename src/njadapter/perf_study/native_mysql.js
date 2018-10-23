var mysql = require('mysql');

var con = mysql.createConnection({
  host: "10.16.50.19",
  user: "root",
  password: "Smash123"
});

//ALTER USER 'root'@'%' IDENTIFIED WITH mysql_native_password BY 'Smash123'
con.connect((err) => {
	if (err) {
		console.log(err);
		return;
	}
	var start = new Date();
	var count = 10000;
	for (var n = 0; n < count; ++n) {
		con.query('SELECT * FROM sakila.actor WHERE actor_id between 11 and 20', (err, result, fields)=>{});
	}
	con.query('SELECT * FROM sakila.actor WHERE actor_id between 11 and 20', (err, result, fields)=>{
		console.log('Time required: ' + (new Date() - start));
	});
});
