var SPA=require('njadapter.js');
var p=SPA.CS.newPool(259);
var cs = SPA.CS;
p.setPoolEvent((spe)=>{if(spe!=15)console.log(spe);});
p.setReturned((reqId, q)=>{
	console.log(reqId);
	console.log(q.getMaxBufferSize());
});
p.setAllProcessed((reqId)=>{
	console.log(reqId);
});
p.StartPool([cs.newCC('localhost',20902,'root','Smash123'),cs.newCC('localhost',20902,'root','Smash123')],3);

var file = p.Seek();
file.getSvsId();
file.getFilesQueued();
file.getFileSize();

var func = function(spe) {
	console.log(spe);
};
p.setPoolEvent(func);