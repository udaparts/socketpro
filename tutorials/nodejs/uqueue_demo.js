"use strict";

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js'); //1

//create a memory buffer or queue for data serialization and de-serialization
var buf = SPA.newBuffer(); //2

var data = {
	nullStr: null,
	objNull: null,
	aDate: new Date(),
	aDouble: 1234.567,
	aBool: true,
	unicodeStr: 'Unicode',
	asciiStr: 'ASCII',
	objBool: true,
	objString: 'test',
	objArrString: ['Hello', 'world'],
	objArrInt: [1, 76890]
};
console.log(data); //source data

//serialize member values into buffer q with a specific order, which must be in agreement with server implementation
buf.SaveString(data.nullStr); //4 bytes for length  //3
buf.SaveObject(data.objNull); //2 bytes for data type
buf.SaveDate(data.aDate); //8 bytes for ulong with accuracy to 1 micro-second
buf.SaveDouble(data.aDouble); //8 bytes
buf.SaveBool(data.aBool); //1 byte
buf.SaveString(data.unicodeStr); //4 bytes for string length + (length * 2) bytes for string data -- UTF16 low-endian
buf.SaveAString(data.asciiStr); //4 bytes for ASCII string length + length bytes for string data
buf.SaveObject(data.objBool); //2 bytes for data type + 2 bytes for variant bool
buf.SaveObject(data.objString); //2 bytes for data type + 4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
buf.SaveObject(data.objArrString); //2 bytes for data type + 4 bytes for array size + (4 bytes for string length + (length * 2) bytes for string data) * arraysize -- UTF16-lowendian
buf.SaveObject(data.objArrInt); //2 bytes for data type + 4 bytes for array size + arraysize * 4 bytes for int data

console.log('Bytes in buffer before loading: ' + buf.getSize());
//de-serialize once result comes from server
var res = { //4
	nullStr: buf.LoadString(),
	objNull: buf.LoadObject(),
	aDate: buf.LoadDate(),
	aDouble: buf.LoadDouble(),
	aBool: buf.LoadBool(),
	unicodeStr: buf.LoadString(),
	asciiStr: buf.LoadAString(),
	objBool: buf.LoadObject(),
	objString: buf.LoadObject(),
	objArrString: buf.LoadObject(),
	objArrInt: buf.LoadObject()
};
console.log(res); //Returned data
console.log('Bytes in buffer after loading: ' + buf.getSize());

console.log('++++++ use callbacks for saving and loading +++++');
res = buf.Save(q => { //5
	q.SaveString(data.nullStr).SaveObject(data.objNull).SaveDate(data.aDate).SaveDouble(data.aDouble).
	SaveBool(data.aBool).SaveString(data.unicodeStr).SaveAString(data.asciiStr).SaveObject(data.objBool).
	SaveObject(data.objString).SaveObject(data.objArrString).SaveObject(data.objArrInt);
}).Load(q => { //6
	console.log('Bytes in buffer before loading: ' + q.getSize());
	var d = {
		nullStr: q.LoadString(),
		objNull: q.LoadObject(),
		aDate: q.LoadDate(),
		aDouble: q.LoadDouble(),
		aBool: q.LoadBool(),
		unicodeStr: q.LoadString(),
		asciiStr: q.LoadAString(),
		objBool: q.LoadObject(),
		objString: q.LoadObject(),
		objArrString: q.LoadObject(),
		objArrInt: q.LoadObject()
	};
	console.log('Bytes in buffer after loading: ' + q.getSize());
	return d;
}
);
console.log(res);
