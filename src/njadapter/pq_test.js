var SPA=require('nja.js');
const idMessage0 = SPA.BaseID.idReservedTwo + 100;
const idMessage1 = SPA.BaseID.idReservedTwo + 101;
const idMessage2 = SPA.BaseID.idReservedTwo + 102;
const TEST_QUEUE_KEY = 'queue_name_0';
var p=SPA.CS.newPool(SPA.SID.sidQueue);
var cs = SPA.CS;
var b = p.Start(cs.newCC('localhost',20902,'root','Smash123'),1);
console.log(b);
var sq = p.Seek();
function testEnqueue(sq) {
	var idMsg;
	var ok = true;
	var buff = SPA.newBuffer();
	console.log('Going to enqueue 1024 messages ......');
	for (var n=0; n < 1024; ++n) {
		var str = '' + n + ' Object test';
		switch(n%3) {
		case 0:
			idMsg = idMessage0;
			break;
		case 1:
			idMsg = idMessage1;
			break;
		default:
			idMsg = idMessage2;
			break;
		}
		ok = sq.Enqueue(TEST_QUEUE_KEY, idMsg, buff.SaveString('SampleName').SaveString(str).SaveInt(n));
		if (!ok) break;
	}
	return ok;
}

function testDequeue(sq) {
	
	
}




b = aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2), (index)=>{
	console.log(index);
});
console.log(b);
b = aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
console.log(b);
b = aq.Enqueue("queue_name_1", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
console.log(b);

function getKeys(sq) {
	return new Promise(function(res, rej){
		if(!sq.GetKeys((v) => {
				res(v);
			}, (cancled)=>{
				rej(cancled ? 'request canceled' : 'connection closed');
			}
		))
			rej('connection closed');
	});
}
async function asyncKeys(sq) {
	let result;
	try {
		result = await getKeys(sq);
	} catch (err) {
		console.error(err);
		throw err;
	}
	return result;
}
console.log(asyncKeys(sq));

b = aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2), (index)=>{
	console.log(index);
});
console.log(b);

aq.Enqueue("queue_name_x", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_0', (err)=>{console.log(err);});
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_x', (err, aq)=>{console.log(err);console.log(aq);});
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_1', (err, aq)=>{console.log(err);console.log(aq);});
aq.GetKeys((v)=>{console.log(v);});

