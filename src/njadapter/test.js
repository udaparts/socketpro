var SPA=require('njadapter.js');
var p=SPA.CS.newPool(259);
var cs = SPA.CS;
p.Start(cs.newCC('localhost',20901,'root','Smash123'),1);
var rf = p.Seek();
var ok = rf.Download('spfile1.test', 'jvm.lib');
ok = rf.Download('spfile1.test', 'jvm.lib', (errMsg, res) => {
	console.log('errMsg=' + errMsg + ', res=' + res);
}, (pos, fileSize)=>{
	console.log(pos/fileSize);
});

/*
var hw = p.Seek();
hw.SendRequest(8194, SPA.newBuffer().SaveString('Jack').SaveString('Smith'), (buffer)=>{
	console.log(buffer.LoadString());
});
hw.SendRequest(8195, SPA.newBuffer().SaveUInt(30000));
hw.SendRequest(8194, SPA.newBuffer().SaveString('Hilary').SaveString('Clinton'), (buffer)=>{
	console.log(buffer.LoadString());
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