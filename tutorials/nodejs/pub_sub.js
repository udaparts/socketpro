"use strict";

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');

const sid = SPA.SID.sidReserved + 1; //hello world service id

//hello world service supports the following three requests
const idSayHello = SPA.BaseID.idReservedTwo + 1;
const idSleep = idSayHello + 1;
const idEcho = idSleep + 1;

var cs = SPA.CS; //CS == Client side
cs.TLS.CA = 'ca.cert.pem'; //or 'root' on windows for server certificate authentication

function onLineMessage() {
    process.stdout.write('Online Message Push: ');
    console.log(arguments);
}

//create a socket pool object
var p = cs.newPool(sid);
global.p = p;

//track various events if neccessary
p.Push = onLineMessage;

//create a secure connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123', 0); //1 0 == NoEncryption, and 1 == TLSv1.x

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var hw = p.Seek(); //seek an async hello world handler
var msg = hw.Socket.Push; //2

//streaming all the following five requests and two messenger message requests
var ok = hw.SendRequest(idSayHello, SPA.newBuffer().SaveString('Mary').SaveString('Smith'), q => {
    console.log(q.LoadString());
});

//sleep 5000 ms at server side
ok = hw.SendRequest(idSleep, SPA.newBuffer().SaveInt(5000), q => {
    console.log('Sleep returned');
});

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
console.log(data);

//echo a complex object
ok = hw.SendRequest(idEcho, SPA.newBuffer().Save(q => {
    q.SaveString(data.nullStr).SaveObject(data.objNull).SaveDate(data.aDate);
    q.SaveDouble(data.aDouble).SaveBool(data.aBool).SaveString(data.unicodeStr);
    q.SaveAString(data.asciiStr).SaveObject(data.objBool).SaveObject(data.objString);
    q.SaveObject(data.objArrString).SaveObject(data.objArrInt);
}), q => {
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
});

async function asyncWait(hw, fName, lName) {
    try {
        //use sendRequest instead of SendRequest for Promise
        console.log(await hw.sendRequest(idSayHello, SPA.newBuffer().SaveString(fName).SaveString(lName), q => {
            return q.LoadString();
        }));
    } catch (err) {
        console.log(err);
    }
}

//send a request by use of Promise, async and await
asyncWait(hw, 'Hillary', 'Clinton');

//send a message to a user
ok = msg.SendUserMessage('some_user_id', 'A test message from node.js'); //3

//send a message to three groups of connected clients
ok = msg.Publish('A test publish message from node.js', [1, 3, 7]); //4
