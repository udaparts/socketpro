"use strict";

//loading SocketPro adapter (nja.js + njadapter.node) for nodejs
var SPA = require('nja.js');

//define message request ids
const idMessage0 = SPA.BaseID.idReservedTwo + 100;
const idMessage1 = SPA.BaseID.idReservedTwo + 101;
const idMessage2 = SPA.BaseID.idReservedTwo + 102;
const TEST_QUEUE_KEY = 'queue_name_0';

var cs = SPA.CS; //CS == Client side

//create a socket pool object
var p = cs.newPool(SPA.SID.sidQueue);
global.p = p;

//create a connection context
var cc = cs.newCC('localhost', 20901, 'root', 'Smash123');

//start a socket pool having one session to a remote server
if (!p.Start(cc, 1)) {
    console.log(p.Error);
    return;
}
var sq = p.Seek(); //seek an async persistent message queue handler

function testEnqueue(sq) {
    var idMsg, n, ok = true, buff = SPA.newBuffer(); const count = 1024;
    console.log('Going to enqueue 1024 messages ......');
    for (n = 0; n < count; ++n) {
        var str = n + ' Object test';
        switch (n % 3) {
            case 0:
                idMsg = idMessage0; break;
            case 1:
                idMsg = idMessage1; break;
            default:
                idMsg = idMessage2; break;
        }
        ok = sq.Enqueue(TEST_QUEUE_KEY, idMsg, buff.SaveString('SampleName').SaveString(str).SaveInt(n)); //1
        if (!ok) break;
    }
    console.log(n + ' messages enqueued');
    return ok;
}
sq.ResultReturned = (id, q) => { //2
    switch (id) {
        case idMessage0:
        case idMessage1:
        case idMessage2:
            //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
            var name = q.LoadString(), str = q.LoadString(), index = q.LoadInt(); //3
            console.log('message id=' + id + ', name=' + name + ', str=' + str + ', index=' + index);
            return true; //true -- result has been processed
        default:
            break;
    }
    return false;
};

var cb = function (mc, fsize, msgs, bytes) { //4
    console.log('Dequeue result: Remaining messages=' + mc + ', queue file size=' + fsize + ', {messages=' + msgs + ', bytes=' + bytes + '} dequeued');
    if (mc) {
        console.log('Keeping on Dequeuing ......');
        sq.Dequeue(TEST_QUEUE_KEY, cb); //5
    }
};
function testDequeue(sq) {
    console.log('Going to Dequeue messages ......');
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    return sq.Dequeue(TEST_QUEUE_KEY, cb); //6
}

var ok = testEnqueue(sq); //7
ok = testDequeue(sq); //8

async function doMyCalls(sq) {
    try {
        console.log('Going to call Flush ......');
        var ok = sq.Flush(TEST_QUEUE_KEY, (mc, fsize) => { //9
            console.log({
                msgs: mc,
                fsize: fsize
            });
        });

        console.log('++++ use getKeys instead of GetKeys for Promise ++++');
        console.log(await sq.getKeys()); //10

        console.log('++++ use flush instead of Flush for Promise ++++');
        console.log(await sq.flush(TEST_QUEUE_KEY)); //11
    } catch (err) {
        console.log(err);
    }
}
doMyCalls(sq); //12
