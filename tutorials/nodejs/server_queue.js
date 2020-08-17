'use strict';
var SPA = require('nja.js');

//define message request ids
const idMessage0 = SPA.BaseID.idReservedTwo + 100;
const idMessage1 = SPA.BaseID.idReservedTwo + 101;
const idMessage2 = SPA.BaseID.idReservedTwo + 102;
const TEST_QUEUE_KEY = 'queue_name_0';

var cs = SPA.CS;
var p = cs.newPool(SPA.SID.sidQueue);
global.p = p;

//start a socket pool having one session to a remote server
if (!p.Start(cs.newCC('localhost', 20901, 'root', 'Smash123'), 1)) {
    console.log(p.Error);
    return;
}
var sq = p.Seek(); //seek an async server queue handler

function testEnqueue(sq) {
    var idMsg, n, ok = true, buff = SPA.newBuffer(); const count = 1024;
    console.log('Going to enqueue 1024 messages ......');
    for (n = 0; n < count; ++n) {
        var str = n + ' Object test';
        switch (n % 3) {
            case 0: idMsg = idMessage0; break;
            case 1: idMsg = idMessage1; break;
            default: idMsg = idMessage2; break;
        }
        ok = sq.Enqueue(TEST_QUEUE_KEY, idMsg, buff.SaveString('SampleName').SaveString(str).SaveInt(n));
        if (!ok) break;
    }
    console.log(n + ' messages enqueued');
    return ok;
}
sq.ResultReturned = (id, q) => {
    switch (id) {
        case idMessage0: case idMessage1: case idMessage2:
            //parse a dequeued message which should be the same as the above enqueued message (two unicode strings and one int)
            var name = q.LoadString(), str = q.LoadString(), index = q.LoadInt(); //3
            //console.log('message id=' + id + ', name=' + name + ', str=' + str + ', index=' + index);
            return true; //true -- result has been processed
        default: break;
    }
    return false;
};

var cb = function (mc, fsize, msgs, bytes) {
    console.log('Dequeue result: Remaining messages=' + mc + ', queue file size=' + fsize + ', {messages=' + msgs + ', bytes=' + bytes + '} dequeued');
    if (mc) {
        console.log('Keeping on Dequeuing ......');
        sq.Dequeue(TEST_QUEUE_KEY, cb);
    }
};
function testDequeue(sq) {
    console.log('Going to Dequeue messages ......');
    //optionally, add one extra to improve processing concurrency at both client and server sides for better performance and through-output
    return sq.Dequeue(TEST_QUEUE_KEY, cb);
}

var ok = testEnqueue(sq);
ok = testDequeue(sq);
(async () => {
    try {
        console.log('Going to call GetKeys and Flush without promises ......');
        sq.GetKeys((keys) => { console.log(keys); });
        sq.Flush(TEST_QUEUE_KEY, (mc, fsize) => {
            console.log({ msgs: mc, fsize: fsize });
        });
        console.log('++++ use getKeys and flush instead of GetKeys and Flush, respectively with Promises ++++');
        console.log(await sq.getKeys());
        console.log(await sq.flush(TEST_QUEUE_KEY));
        //var my_arr = await Promise.all([sq.getKeys(), sq.flush(TEST_QUEUE_KEY)]);
        //console.log(my_arr);
    } catch (err) {
        console.log(err);
    }
})();
