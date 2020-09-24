'use strict';

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');

const sid = SPA.SID.sidReserved + 1; //hello world service id

//hello world service supports the following three requests
const idSayHello = SPA.BaseID.idReservedTwo + 1;
const idSleep = idSayHello + 1;
const idEcho = idSleep + 1;

var cs = SPA.CS; //CS == Client side

//create a global socket pool object
var p = cs.newPool(sid);
global.p = p;

//create a connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var hw = p.Seek(); //seek an async hello world handler

//prepare a real complex structure for a remote request
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
console.log(data); //Source data

(async () => {
    try {
        // all f0, f1, f2, f3, f4 and f5 are promises for CUQueue
        // all the following five requests are streamed with in-line batching for best network effiency
        var f0 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Jone').SaveString('Dole'));
        var f1 = hw.sendRequest(idSleep, SPA.newBuffer().SaveUInt(5000));

        //serialize and de-serialize a complex structure with a specific order,
        //pay attention to both serialization and de-serialization,
        //which must be in agreement with server implementation

        //echo a complex object
        var f2 = hw.sendRequest(idEcho, SPA.newBuffer().Save(q => {
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

            console.log('complex object echo buffer size: ' + q.getSize());
        }));
        var f3 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Hillary').SaveString('Clinton'));
        var f4 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Donald').SaveString('Trump'));
        var f5 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Jack').SaveString('Smith'));

        console.log((await f0).LoadString());
        console.log('Sleep returning result size: ' + (await f1).getSize()); //should be zero because server side return nothing
        var q = await f2;
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
        console.log(d);
        console.log((await f3).LoadString());
        console.log((await f4).LoadString());
        console.log((await f5).LoadString());
    } catch (err) {
        console.log(err);
    }
})();
