/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Diagnostics;

//server implementation for service CServerQueue
public class CServerQueuePeer : CClientPeer
{
    static CServerQueue m_sq = new CServerQueue();
    static CServerQueuePeer()
    {
        uint ttl = 720 * 3600;
        m_sq = CSocketProServer.QueueManager.StartQueue("ServerQueueTest", ttl);
    }
    long m_lSwitchTime = 0;
    long m_lEnqueueTime = 0;
    Stopwatch m_sw = new Stopwatch();

    protected override void OnResultsSent()
    {
        uint bytes = BytesInSendingBuffer;
        bytes = 0;
    }

    protected override void OnRequestArrive(ushort requestId, uint len)
    {
#if MEASURE_TIME
        m_sw.Reset();
        m_sw.Start();
#endif
    }

    [RequestAttr(SQueueConst.idEnqueueCServerQueue, false)]
    private ulong Enqueue(string name, string message, int aInt)
    {
#if MEASURE_TIME
        m_sw.Stop();
        m_lSwitchTime += (long)(m_sw.Elapsed.TotalMilliseconds * 1000);
        m_sw.Reset();
        m_sw.Start();
#endif
        ulong res = m_sq.Enqueue(SQueueConst.idMyMessage, name, message, aInt);
        if (res == ulong.MaxValue || res == 0)
            Console.WriteLine("Failed in enqueuing " + aInt);
#if MEASURE_TIME
        m_sw.Stop();
        m_lEnqueueTime += (long)(m_sw.Elapsed.TotalMilliseconds * 1000);
#endif
        return res;
    }

    [RequestAttr(SQueueConst.idDequeueCServerQueue, true)]
    private ulong MyDequeue(uint count, bool autoEventWhenAvailable)
    {
        ulong dequeuedCount = 0;
        ulong res = Dequeue(m_sq.Handle, count, autoEventWhenAvailable, 0);
        while (res != 0 && res != ulong.MaxValue)
        {
            ulong bytes = (res >> 32);
            ulong dequeued = (res & uint.MaxValue);
            dequeuedCount += dequeued;
            if (dequeuedCount >= count)
                break;
            res = Dequeue(m_sq.Handle, (uint)1024, autoEventWhenAvailable, 0);
        }
        return dequeuedCount;
    }

    [RequestAttr(SQueueConst.idQueryTimes)]
    private long QueryTimes(out long time)
    {
        time = m_lSwitchTime / 1000;
        return m_lEnqueueTime / 1000;
    }

    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        Console.WriteLine("Base request id = " + reqId.ToString());
    }

    [RequestAttr(SQueueConst.idDoEnqueueNumbers)]
    private void DoEnqueueNumbers(uint number, uint sum)
    {
#if MEASURE_TIME
        m_sw.Stop();
        m_lSwitchTime += (long)(m_sw.Elapsed.TotalMilliseconds * 1000);
        m_sw.Reset();
        m_sw.Start();
#endif
        ulong res = m_sq.Enqueue(SQueueConst.idMyNumbers, number, sum);
#if MEASURE_TIME
        m_sw.Stop();
        m_lEnqueueTime += (long)(m_sw.Elapsed.TotalMilliseconds * 1000);
#endif
    }

    [RequestAttr(SQueueConst.idDoDequeueNumbers, true)]
    private ulong DoDequeueNumbers()
    {
        ulong dequeuedCount = 0;
        ulong res = Dequeue(m_sq.Handle, 200, true);
        if (res != 0 && res != ulong.MaxValue)
            dequeuedCount = (res & uint.MaxValue);
        return dequeuedCount;
    }
}
