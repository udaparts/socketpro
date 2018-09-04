var SPA=require('njadapter.js');
var p=SPA.CS.newPool(268435457);
var cs = SPA.CS;
p.Start(cs.newCC('localhost',20901,'root','Smash123'),1);
var hw = p.Seek();

var buffer = new ArrayBuffer(8);
var int32View = new Int32Array(buffer);
int32View[0] = 1;
int32View[1] = 76890;
//echo a complex data structure
var data = {
	nullStr:null,
	objNull:null,
	aDate:new Date(),
	aDouble:1234.567,
	aBool:true,
	unicodeStr:'Unicode',
	asciiStr:'ASCII',
	objBool:true,
	objString:'test',
	objArrString:['Hello', 'world'],
	objArrInt:int32View
};
console.log(data);

//create a buffer
var buf = SPA.newBuffer();

//serialize and de-serialize a complex structure with a specific order,
//pay attention to both serialization and de-serialization,
//which must be in agreement with server implementation

//serialize
buf.SaveString(data.nullStr).SaveObject(data.objNull).SaveDate(data.aDate).SaveDouble(data.aDouble).SaveBool(data.aBool).SaveString(data.unicodeStr).SaveAString(data.asciiStr).SaveObject(data.objBool).SaveObject(data.objString).SaveObject(data.objArrString).SaveObject(data.objArrInt, 'i');

//echo a complex object
hw.SendRequest(8196, buf, q=>{
	//de-serialize once result comes from server
	var d = {	nullStr:q.LoadString(),
				objNull:q.LoadObject(),
				aDate:q.LoadDate(),
				aDouble:q.LoadDouble(),
				aBool:q.LoadBool(),
				unicodeStr:q.LoadString(),
				asciiStr:q.LoadAString(),
				objBool:LoadObject(),
				objString:q.LoadObject(),
				objArrString:q.LoadObject(),
				objArrInt:q.LoadObject()
			};
	console.log(d);
});

hw.SendRequest(8194, SPA.newBuffer().SaveString('Mary').SaveString('Smith'), q=>{
	console.log(q.LoadString());
});

//sleep 5000 ms
hw.SendRequest(8195, SPA.newBuffer().SaveInt(5000), q=>{
	console.log('sleep returned');
});

hw.SendRequest(8194, SPA.newBuffer().SaveString('Jone').SaveString('Dole'), q=>{
	console.log(q.LoadString());
});