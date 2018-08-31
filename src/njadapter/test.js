var SPA=require('njadapter.js');

var p=SPA.CS.newPool(257);
var cs = SPA.CS;
p.Start(cs.newCC('localhost',20902,'root','Smash123'),1);
var aq = p.Seek();
aq.GetKeys((v)=>{console.log(v);});
aq.Enqueue("queue_name_0", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.Enqueue("queue_name_1", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.Enqueue("queue_name_x", 8293, SPA.newBuffer().SaveString('SampleName').SaveString('Smith').SaveInt(2));
aq.Close('queue_name_0', (err)=>{console.log(err);});
aq.Close('queue_name_x', (err, aq)=>{console.log(err);console.log(aq);});

/*
var p=SPA.CS.newPool(268435457);
var cs = SPA.CS;
p.Start(cs.newCC('localhost',20901,'root','Smash123'),1);
var hw = p.Seek();
hw.SendRequest(8194, SPA.newBuffer().SaveString('Jack').SaveString('Smith'), (buffer)=>{
	console.log(buffer.LoadString());
});
hw.SendRequest(8195, SPA.newBuffer().SaveUInt(30000));
hw.SendRequest(8194, SPA.newBuffer().SaveString('Hilary').SaveString('Clinton'), (buffer)=>{
	console.log(buffer.LoadString());
});
*/

/*
var p=SPA.CS.newPool(259);
var cs = SPA.CS;
p.Start(cs.newCC('localhost',20901,'root','Smash123'),1);
var rf = p.Seek();
var ok = rf.Download('spfile1.test', 'jvm.lib');
ok = rf.Download('spfile1.test', 'jvm.lib', (errMsg, res) => {
	console.log('errMsg=' + errMsg + ', res=' + res);
}, (pos, fileSize)=>{
	console.log(100 * pos/fileSize);
});
ok = rf.Upload('spfile1.test', 'jvm_copy.lib', (errMsg, res) => {
	console.log('errMsg=' + errMsg + ', res=' + res);
}, (pos, fileSize)=>{
	console.log(100 * pos/fileSize);
}, (canceled)=>{
	console.log(canceled);
});
*/

p.setPoolEvent((spe)=>{if(spe!=15)console.log(spe);});
p.setReturned((reqId, q)=>{
	console.log(reqId);
	console.log(q.getMaxBufferSize());
});
p.setAllProcessed((reqId)=>{
	console.log(reqId);
});
var func = function(spe) {
	console.log(spe);
};
p.setPoolEvent(func);