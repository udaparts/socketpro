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
    var idMsg;
    var ok = true;
    var buff = SPA.newBuffer();
    console.log('Going to enqueue 1024 messages ......');
    for (var n = 0; n < 1024; ++n) {
        var str = '' + n + ' Object test';
        switch (n % 3) {
            case 0:
                idMsg = idMessage0;
                break;
            case 1:
                idMsg = idMessage1;
                break;
            default:
                idMsg = idMessage2;
                break;
        }
        ok = sq.Enqueue(TEST_QUEUE_KEY, idMsg, buff.SaveString('SampleName').SaveString(str).SaveInt(n));
        if (!ok) break;
    }
    return ok;
}

sq.ResultReturned = (id, q) => {
    switch (id) {
        case idMessage0:
        case idMessage1:
        case idMessage2:
            //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
            var name = q.LoadString();
            var str = q.LoadString();
            var index = q.LoadInt();
            console.log('message id=' + id + ', name=' + name + ', str=' + str + ', index=' + index);
            return true;
        default:
            break;
    }
	return false;
};

var cb = function(mc, fsize, msgs, bytes) {
    console.log('Total message count=' + mc + ', queue file size=' + fsize + ', messages dequeued=' + msgs + ', message bytes dequeued=' + bytes);
    if (mc) {
        sq.Dequeue(TEST_QUEUE_KEY, cb);
    }
};

function testDequeue(sq) {
    console.log('Going to dequeue messages ......');
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    return sq.Dequeue(TEST_QUEUE_KEY, cb);
}

var ok = testEnqueue(sq);
testDequeue(sq);

async function asyncKeys(sq) {
    try {
        var ok = sq.Flush(TEST_QUEUE_KEY, (mc, fsize) => {
            console.log({
                msgs: mc,
                fsize: fsize
            });
        });

        //use getKeys instead of GetKeys for Promise
        var result = await sq.getKeys();
        console.log(result);

        //use flush instead of Flush for Promise
        result = await sq.flush(TEST_QUEUE_KEY);
        console.log(result);
    } catch (err) {
        console.log(err);
    }
}
console.log(asyncKeys(sq));
