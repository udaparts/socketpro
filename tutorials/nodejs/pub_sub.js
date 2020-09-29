'use strict';
//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
const sid = SPA.SID.sidReserved + 1; //hello world service id

//hello world service supports the following three requests
const idSayHello = SPA.BaseID.idReservedTwo + 1;
const idSleep = idSayHello + 1;
const idEcho = idSleep + 1;

var cs = SPA.CS;
//'root' on windows for server certificate authentication
cs.TLS.CA = 'ca.cert.pem'; //linux, ignored on windows

//create a socket pool object
var p = cs.newPool(sid);
global.p = p;

//track various message events if neccessary
p.Push = function () { //2
    process.stdout.write('Online Message Push: ');
    console.log(arguments);
};

//create a secure connection context
//0 = NoEncryption or 1 = TLSv1.x
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123', 1);

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var hw = p.Seek(); //seek an async hello world handler
var messenger = hw.Socket.Push; //get an associated messenger

//prepare a real complex structure for a remote request
var data = {
    nullStr: null, objNull: null, aDate: new Date(), aDouble: 1234.567, aBool: true,
    unicodeStr: 'Unicode', asciiStr: 'ASCII', objBool: true, objString: 'test',
    objArrString: ['Hello', 'world'], objArrInt: [1, 76890]
};
console.log(data); //Source data

(async () => {
    try {
        // all f0, f1, f2, f3, f4 and f5 are promises for CUQueue
        // streaming requests with in-line batching for best network effiency
        var f0 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Mary').SaveString('Smith'));
        var f1 = hw.sendRequest(idSleep, SPA.newBuffer().SaveInt(5000));

        //serialize and de-serialize a complex structure with a specific order,
        //pay attention to both serialization and de-serialization,
        //which must be in agreement with server implementation

        //echo a complex object
        var f2 = hw.sendRequest(idEcho, SPA.newBuffer().Save(buff => {
            buff.SaveString(data.nullStr).SaveObject(data.objNull).SaveDate(data.aDate);
            buff.SaveDouble(data.aDouble).SaveBool(data.aBool).SaveString(data.unicodeStr);
            buff.SaveAString(data.asciiStr).SaveObject(data.objBool).SaveObject(data.objString);
            buff.SaveObject(data.objArrString).SaveObject(data.objArrInt);

            console.log('complex object echo buffer size: ' + buff.getSize());
        }));
        var f3 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Hillary').SaveString('Clinton'));
        var f4 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Donald').SaveString('Trump'));
        var f5 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Jack').SaveString('Smith'));

        //messenger.Subscribe([1, 2, 3]);

        //send a message to a user
        var ok = messenger.SendUserMessage('some_user_id', 'A test message from node.js');

        //send a message to three groups of connected clients
        ok = messenger.Publish('A test publish message from node.js', [1, 3, 7]);

        console.log((await f0).LoadString());
        //should be zero because server side return nothing
        console.log('Sleep returning result size: ' + (await f1).getSize());
        var buff = await f2;
        //de-serialize once result comes from server
        var d = {
            nullStr: buff.LoadString(),
            objNull: buff.LoadObject(),
            aDate: buff.LoadDate(),
            aDouble: buff.LoadDouble(),
            aBool: buff.LoadBool(),
            unicodeStr: buff.LoadString(),
            asciiStr: buff.LoadAString(),
            objBool: buff.LoadObject(),
            objString: buff.LoadObject(),
            objArrString: buff.LoadObject(),
            objArrInt: buff.LoadObject()
        };
        console.log(d);
        console.log((await f3).LoadString());
        console.log((await f4).LoadString());
        console.log((await f5).LoadString());

        //messenger.Unsubscribe();
    } catch (err) {
        console.log(err);
    }
})();
