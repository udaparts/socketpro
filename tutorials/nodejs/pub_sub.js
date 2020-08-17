'use strict';
//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');
const sid = SPA.SID.sidReserved + 1; //hello world service id

//hello world service supports the following three requests
const idSayHello = SPA.BaseID.idReservedTwo + 1;
const idSleep = idSayHello + 1;
const idEcho = idSleep + 1;

var cs = SPA.CS;
cs.TLS.CA = 'ca.cert.pem'; //'root' on windows for server certificate authentication

//create a socket pool object
var p = cs.newPool(sid);
global.p = p;

//track various message events if neccessary
p.Push = function () { //2
    process.stdout.write('Online Message Push: ');
    console.log(arguments);
};

//create a secure connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123', 0); //0 = NoEncryption or 1 = TLSv1.x

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var hw = p.Seek(); //seek an async hello world handler
var messenger = hw.Socket.Push; //get an associated messenger

//prepare a real complex structure for a remote request
var data = {
    nullStr: null, objNull: null, aDate: new Date(), aDouble: 1234.567, aBool: true, unicodeStr: 'Unicode',
    asciiStr: 'ASCII', objBool: true, objString: 'test', objArrString: ['Hello', 'world'], objArrInt: [1, 76890]
};
console.log(data); //Source data

(async () => {
    try {
        // all f0, f1, f2, f3, and f4 are promises for CUQueue
        // all the following five requests are streamed with in-line batching for best network effiency
        var f0 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Mary').SaveString('Smith'));
        var f1 = hw.sendRequest(idSleep, SPA.newBuffer().SaveInt(5000));

        //serialize and de-serialize a complex structure with a specific order,
        //pay attention to both serialization and de-serialization,
        //which must be in agreement with server implementation

        //echo a complex object
        var f2 = hw.sendRequest(idEcho, SPA.newBuffer().Save(q => {
            q.SaveString(data.nullStr).SaveObject(data.objNull).SaveDate(data.aDate);
            q.SaveDouble(data.aDouble).SaveBool(data.aBool).SaveString(data.unicodeStr);
            q.SaveAString(data.asciiStr).SaveObject(data.objBool).SaveObject(data.objString);
            q.SaveObject(data.objArrString).SaveObject(data.objArrInt);

            console.log('complex object echo buffer size: ' + q.getSize());
        }));
        var f3 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Hillary').SaveString('Clinton'));
        var f4 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Download').SaveString('Trump'));
        var f5 = hw.sendRequest(idSayHello, SPA.newBuffer().SaveString('Jack').SaveString('Smith'));

        //send a message to a user
        var ok = messenger.SendUserMessage('some_user_id', 'A test message from node.js');

        //send a message to three groups of connected clients
        ok = messenger.Publish('A test publish message from node.js', [1, 3, 7]);

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
