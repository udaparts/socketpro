#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
from spa.clientside import CSocketPool, CConnectionContext, CAsyncQueue, CUQueue
from spa import tagBaseRequestID

TEST_QUEUE_KEY = "queue_name_0"
idMessage0 = tagBaseRequestID.idReservedTwo + 100
idMessage1 = tagBaseRequestID.idReservedTwo + 101
idMessage2 = tagBaseRequestID.idReservedTwo + 102

def TestEnqueue(aq):
    print('Going to enqueue 1024 messages ......')
    idMessage = 0
    n = 0
    while n < 1024:
        s = str(n) + ' Object test'
        m = n % 3
        if m == 0:
            idMessage = idMessage0
        elif m == 1:
            idMessage = idMessage1
        else:
            idMessage = idMessage2
        # enqueue two unicode strings and one int
        ok = aq.Enqueue(TEST_QUEUE_KEY, idMessage, CUQueue().SaveString('SampleName').SaveString(s).SaveInt(n))
        n += 1
        if not ok:
            return False
    return True

def TestDequeue(aq):
    def cbResultReturned(idReq, q):
        if idReq == idMessage0 or idReq == idMessage1 or idReq == idMessage2:
            # parse a dequeued message which should be the same as
            # the above enqueued message (two unicode strings and one int)
            s = 'message id=' + str(idReq) + ', name=' + q.LoadString() + ', str=' + q.LoadString() + ', index=' + str(q.LoadInt())
            print(s)
            return True
        return False # not processed
    aq.ResultReturned = cbResultReturned

    def cbDequeue(sender, messageCount, fileSize, messages, bytes):
        s = 'Total message count=' + str(messageCount) + ', queue file size=' + str(fileSize) + ', messages dequeued='\
            + str(messages) + ', message bytes dequeued=' + str(bytes)
        print(s)
        if messageCount > 0:
            # there are more messages left at server queue, keep on dequeuing
            sender.Dequeue(TEST_QUEUE_KEY, sender.LastDequeueCallback)
    print('Going to dequeue messages ......')
    aq.Dequeue(TEST_QUEUE_KEY, cbDequeue)

    # optionally, add one extra to improve processing concurrency at both client and server sides
    # for better performance and through-output
    aq.Dequeue(TEST_QUEUE_KEY, cbDequeue)

with CSocketPool(CAsyncQueue) as spAq:
    print('Remote async queue server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'SomeUserId', 'APassword')
    ok = spAq.StartSocketPool(cc, 1)
    aq = spAq.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(aq.Socket.ErrorCode))
        exit(0)

    # Optionally, you can enqueue messages with transaction style by calling the methods
    # StartQueueTrans and EndQueueTrans in pair
    f0 = aq.startQueueTrans(TEST_QUEUE_KEY)
    TestEnqueue(aq)
    f1 = aq.endQueueTrans()
    fut0 = aq.flushQueue(TEST_QUEUE_KEY)
    print('StartQueueTrans return error code: ' + str(f0.result()))
    print('EndQueueTrans return error code: ' + str(f1.result()))
    print('Server queue info: ' + str(fut0.result()))

    TestDequeue(aq)
    aq.WaitAll()

    # get a server queue info, remaining message count and queue file size in bytes with default option oMemoryCached
    fut0 = aq.flushQueue(TEST_QUEUE_KEY)
    fut1 = aq.getKeys()

    print('Server queue info: ' + str(fut0.result()))
    print('Keys: ' + str(fut1.result()))

    print('Press any key to close the application ......')
    sys.stdin.readline()
