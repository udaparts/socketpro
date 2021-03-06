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
//p.QueueName = 'qname';

//start a socket pool having one session to a remote server
if (!p.Start(cs.newCC('localhost', 20901, 'root', 'Smash123'), 1)) {
    console.log(p.Error);
    return;
}

var sq = p.Seek(); //seek an async server queue handler
function testEnqueue(sq) {
    var idMsg, n, buff = SPA.newBuffer(); const count = 1024;
    console.log('Going to enqueue 1024 messages ......');
    for (n = 0; n < count; ++n) {
        var str = n + ' Object test';
        switch (n % 3) {
            case 0: idMsg = idMessage0; break;
            case 1: idMsg = idMessage1; break;
            default: idMsg = idMessage2; break;
        }
        if (!sq.Enqueue(TEST_QUEUE_KEY, idMsg, buff.SaveString('SampleName').SaveString(str).SaveInt(n))) {
            sq.throw(cs.Queue.ReqIds.idEnqueue);
        }
        //buff will been reset to empty automatically after calling Enqueue with node.js
        //sleep(20);
    }
    console.log(n + ' messages enqueued');
}
function sleep(milliseconds) {
    var start = new Date().getTime();
    for (var i = 0; i < 1e7; i++) {
        if ((new Date().getTime() - start) > milliseconds) {
            break;
        }
    }
}

sq.ResultReturned = (id, q) => {
    switch (id) {
        case idMessage0: case idMessage1: case idMessage2:
            //parse a dequeued message which should be the same as
            //the above enqueued message (two unicode strings and one int)
            var name = q.LoadString(), str = q.LoadString(), index = q.LoadInt();
            console.log('message id=' + id + ', name=' + name + ', str=' + str + ', index=' + index);
            return true; //true -- result has been processed
        default: break;
    }
    return false;
};
function testDequeue(sq) {
    console.log('Going to Dequeue messages ......');
    return new Promise((res, rej) => {
        var aborted = (canceled) => {
            sq.set_aborted(rej, cs.Queue.ReqIds.idDequeue, canceled);
        };
        var se = (errMsg, errCode, errWhere, id) => {
            sq.set_exception(rej, errMsg, errCode, errWhere, id);
        };
        var cb = function (mc, fsize, msgs, bytes) {
            if (bytes) {
                console.log('Dequeue result: Remaining messages=' + mc + ', queue file size=' +
                    fsize + ', {messages=' + msgs + ', bytes=' + bytes + '} dequeued');
            }
            if (mc) {
                console.log('Keeping on Dequeuing ......');
                sq.Dequeue(TEST_QUEUE_KEY, cb, aborted, 0, se);
            }
            else {
                res({ msgs: mc, fsize: fsize, msgsDequeued: msgs, bytes: bytes });
            }
        };
        //add an extra Dequeue call for better dequeue performance
        if (!(sq.Dequeue(TEST_QUEUE_KEY, cb, aborted, 0, se) &&
            sq.Dequeue(TEST_QUEUE_KEY, cb, aborted, 0, se))) {
            sq.raise(rej, cs.Queue.ReqIds.idDequeue);
        }
    });
}

(async () => {
    try {
        testEnqueue(sq);
        console.log(await testDequeue(sq));
        console.log('Going to call GetKeys and Flush without promises ......');
        if (!sq.GetKeys((keys) => { console.log(keys); })) {
            sq.throw(cs.Queue.ReqIds.idGetKeys);
        }
        if (!sq.Flush(TEST_QUEUE_KEY, (mc, fsize) => {
            console.log({ msgs: mc, fsize: fsize });
        })) {
            sq.throw(cs.Queue.ReqIds.idFlush);
        }
        console.log('++++ use getKeys and flush instead of GetKeys and Flush, respectively with Promises ++++');
        console.log(await Promise.all([sq.getKeys(), sq.flush(TEST_QUEUE_KEY)]));
    } catch (err) {
        console.log(err);
    }
})();
