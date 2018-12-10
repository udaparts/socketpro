"use strict";

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');

//create a memory buffer or queue for data serialization and de-serialization
var buf = SPA.newBuffer();

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
console.log(data);

var res = buf.Save(q => {
		//serialize member values into buffer q with a specific order, which must be in agreement with server implementation
		q.SaveString(data.nullStr); //4 bytes for length
		q.SaveObject(data.objNull); //2 bytes for data type
		q.SaveDate(data.aDate); //8 bytes for ulong with accuracy to 1 micro-second
		q.SaveDouble(data.aDouble); //8 bytes
		q.SaveBool(data.aBool); //1 byte
		q.SaveString(data.unicodeStr); //4 bytes for string length + (length * 2) bytes for string data -- UTF16 low-endian
		q.SaveAString(data.asciiStr); //4 bytes for ASCII string length + length bytes for string data
		q.SaveObject(data.objBool); //2 bytes for data type + 2 bytes for variant bool
		q.SaveObject(data.objString); //2 bytes for data type + 4 bytes for string length + (length * 2) bytes for string data -- UTF16-lowendian
		q.SaveObject(data.objArrString); //2 bytes for data type + 4 bytes for array size + (4 bytes for string length + (length * 2) bytes for string data) * arraysize -- UTF16-lowendian
		q.SaveObject(data.objArrInt); //2 bytes for data type + 4 bytes for array size + arraysize * 4 bytes for int data
	}).Load(q => {
		//de-serialize once result comes from server
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
		return d;
	}
);
console.log(res);
