var SPA=require('njadapter.js');
var p=SPA.CS.newPool(257);
var cs = SPA.CS;
var b = p.Start(cs.newCC('localhost',20902,'root','Smash123'),1);
console.log(b);
var aq = p.Seek();
b = aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2), (index)=>{
	console.log(index);
});
console.log(b);
		
function getKeys() {
	return new Promise(function(resolve, reject){
		if(!aq.GetKeys((v) => {
				console.log(v);
				resolve(v);
			}, (cancled)=>{
				reject(cancled ? 'request canceled' : 'connection closed');
			}
		))
			reject('connection closed');
	});
}

async function asyncKeys() {
	let result;
	console.log('Befor await');
	try {
		result = await getKeys();
	} catch (err) {
		console.error(err);
		throw err;
	}
	console.log('After await 1');
	console.log(result);
	console.log('After await 2');
}
asyncKeys();
b = aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2), (index)=>{
	console.log(index);
});
console.log(b);

/*
aq.GetKeys((v)=>{console.log(v);});
aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.Enqueue("queue_name_1", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.Enqueue("queue_name_x", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_0', (err)=>{console.log(err);});
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_x', (err, aq)=>{console.log(err);console.log(aq);});
aq.GetKeys((v)=>{console.log(v);});
aq.Close('queue_name_1', (err, aq)=>{console.log(err);console.log(aq);});
aq.GetKeys((v)=>{console.log(v);});
*/
