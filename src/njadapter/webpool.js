//configuration
var config = {
	work_dir:'C:\\ProgramData\\myweb_work',
	defaultDb:'sakila',
	master:{
		sessions:1,
		queueName:'qmaster',
		hosts:[	{host:'ws-yye-1',
			port:20902,
			uid:'root',
			pwd:'Smash123'}
		]
	},
	slave:{
		sessions:2,
		queueName:'qslave',
		hosts:[	{	host:'10.16.20.33',
				port:20902,
				uid:'root',
				pwd:'Smash123'
			}, {	host:'ws-yye-1',
				port:20902,
				uid:'root',
				pwd:'Smash123'
			}, {	host:'104.154.160.127',
				port:20902,
				uid:'root',
				pwd:'Smash123',
				zip:true
			}
		]
	}
};
exports.config = config;

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
exports.spa = SPA;
var cs = SPA.CS; //CS == Client side

//set working directory for client message queues
cs.Queue.setWorkingDir(config.work_dir);

function getCCs(hosts) {
	var ccs = [];
	for (var i = 0; i < hosts.length; ++i) {
		cc = hosts[i];
		ccs.push(cs.newCC(cc.host, cc.port, cc.uid, cc.pwd, cc.em, cc.zip, cc.v6, cc.anyData));
	}
	return ccs;
}

//create a global socket pool object for master
var master=cs.newPool(SPA.SID.sidMysql, config.defaultDb); //or sidOdbc for MS SQL Server
master.setQueueName(config.master.queueName);
exports.master = master;

//start a socket pool to one or more remote master servers
if (!master.Start(getCCs(config.master.hosts), config.master.sessions)) {
	console.log('Master pool starting error');
	console.log(master.getError());
}
exports.getCache = function() {return master.getCache();}
master.setAutoMerge(false); //no auto merge for persistent message queue

//create a global socket pool object for slave
var slave = master.NewSlave();
slave.setQueueName(config.slave.queueName);
slave.setAutoMerge(false);
exports.slave = slave;

//start a socket pool having four sessions to remote slave servers
if (!slave.Start(getCCs(config.slave.hosts), config.slave.sessions)) {
	console.log('Slave pool starting error');
	console.log(slave.getError());
}
slave.setAutoMerge(config.slave.hosts.length > 1);