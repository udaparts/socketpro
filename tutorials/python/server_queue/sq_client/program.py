import sys
from spa.clientside import CSocketPool, CConnectionContext, CAsyncQueue, CUQueue, CSocketError, CServerError as Se
from spa import tagBaseRequestID
from concurrent.futures import Future as future

TEST_QUEUE_KEY = "queue_name_0"
idMessage0 = tagBaseRequestID.idReservedTwo + 100
idMessage1 = tagBaseRequestID.idReservedTwo + 101
idMessage2 = tagBaseRequestID.idReservedTwo + 102


def test_enqueue(aq):
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
        if not aq.Enqueue(TEST_QUEUE_KEY, idMessage, CUQueue().SaveString('SampleName').SaveString(s).SaveInt(n)):
            aq.throw('Enqueue', CAsyncQueue.idEnqueue)
        n += 1

def test_dequeue(aq):
    def cbResultReturned(idReq, q):
        if idReq == idMessage0 or idReq == idMessage1 or idReq == idMessage2:
            # parse a dequeued message which should be the same as
            # the above enqueued message (two unicode strings and one int)
            s = 'message id=' + str(idReq) + ', name=' + q.LoadString() + \
                ', str=' + q.LoadString() + ', index=' + str(q.LoadInt())
            print(s)
            return True
        return False  # not processed
    aq.ResultReturned = cbResultReturned

    f = future()
    aborted = CAsyncQueue.get_aborted(f, 'Dequeue', CAsyncQueue.idDequeue)
    se = CAsyncQueue.get_se(f)
    def cbDequeue(aq, messageCount, fileSize, messages, bytes):
        if bytes:
            s = 'Total message count=' + str(messageCount) + ', queue file size=' + \
                str(fileSize) + ', messages dequeued=' + str(messages) + ', bytes dequeued=' + str(bytes)
            print(s)
        if messageCount > 0:
            # there are more messages left at server queue, we re-send a request to dequeue
            aq.Dequeue(TEST_QUEUE_KEY, aq.LastDequeueCallback, 0, aborted, se)
        else:
            try:
                if not f.done():
                    f.set_result({'messages': messageCount, 'fsize': fileSize, 'msgsDequeued': messages, 'bytes': bytes})
            except Exception as ex:
                pass

    print('Going to dequeue messages ......')
    # optionally, add one extra to improve processing concurrency
    # at both client and server sides for better performance and through-output
    if not (aq.Dequeue(TEST_QUEUE_KEY, cbDequeue, 0, aborted, se) and aq.Dequeue(TEST_QUEUE_KEY, cbDequeue, 0, aborted, se)):
        aq.throw('Dequeue', CAsyncQueue.idDequeue)
    return f

with CSocketPool(CAsyncQueue) as spAq:
    print('Remote async queue server host: ')
    cc = CConnectionContext(sys.stdin.readline(), 20901, 'PythonUser', 'TooMuchSecret')
    ok = spAq.StartSocketPool(cc, 1)
    aq = spAq.AsyncHandlers[0]
    if not ok:
        print('No connection error code = ' + str(aq.Socket.ErrorCode))
        exit(0)
    try:
        # Optionally, you can enqueue messages with transaction style
        # by calling the methods StartQueueTrans and EndQueueTrans in pair
        # aq.startQueueTrans(TEST_QUEUE_KEY)
        test_enqueue(aq)
        # aq.endQueueTrans()
        print(test_dequeue(aq).result())
    except Se as ex:  # an exception from remote server
        print(ex)
    except CSocketError as ex:  # a communication error
        print(ex)
    except Exception as ex:
        print('Unexpected error: ' + str(ex))  # invalid parameter, bad de-serialization, and so on
    print('Press any key to close the application ......')
    sys.stdin.readline()
