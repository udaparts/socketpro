using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
using System.Diagnostics;

public class CMyPeer : CClientPeer
{
    uint m_dbIndex;
    uint m_nCalls;

    public static CServerQueue m_mq = new CServerQueue();

    protected override void OnSwitchFrom(uint oldServiceId)
    {
        Push.Subscribe(1, 2, 7);
        object obj = "This is a test message";
        Push.Publish(obj, 1, 2, 7);

        Zip = true;
        ZipLevel = tagZipLevel.zlBestSpeed;

        m_dbIndex = 0;
        m_nCalls = 0;
    }

    protected override void OnBaseRequestCame(tagBaseRequestID reqId)
    {
        Console.WriteLine("Base request id = " + reqId.ToString());
    }

    [RequestAttr(TestMeConst.idDequeue, true)]
    void Dodequeue(uint messageCount)
    {
		DateTime dtPrev = DateTime.Now;
        ulong res = Dequeue(m_mq.Handle, messageCount, true);
		//Console.WriteLine("Dequeue time in ms = " + (DateTime.Now - dtPrev).TotalMilliseconds.ToString());
        uint bytes = (uint)(res >> 32);
        uint dequedMessages = (uint)(res & 0xFFFFFFFF);
    }

    [RequestAttr(TestMeConst.idDoRequest0, true)]
    void DoRequest0(sbyte aChar, char aWChar, sbyte[] str, string wstr, ushort us, double d, bool b, double dt, out CScopeUQueue q)
    {
        Debug.Assert(aWChar == '?');
        Debug.Assert(wstr.IndexOf("???????90?????") != -1);
        q = new CScopeUQueue();
        q.Save(aChar).Save(aWChar).Save(str).Save(wstr).Save(us).Save(d).Save(b).Save(dt);
        ulong index = m_mq.Enqueue(12346, aChar, aWChar, str, wstr, us, d, b, dt);
    }

    [RequestAttr(TestMeConst.idBadRequest, true)]
    string BadRequest(uint n, string input)
    {
        string res = input;
		DateTime dtPrev = DateTime.Now;
        ulong index = m_mq.Enqueue(TestMeConst.idDoRequest1, input, res);
		//Console.WriteLine("Enqueue time in ms = " + (DateTime.Now - dtPrev).TotalMilliseconds.ToString());
        return res;
    }

    [RequestAttr(TestMeConst.idSleep, true)]
    void Sleep(uint time)
    {
        System.Threading.Thread.Sleep((int)time);
    }

    [RequestAttr(TestMeConst.idOpenDb, true)]
    void OpenDb(sbyte[] connString, out bool ok)
    {
        ok = true;
        ++m_dbIndex;
    }

    [RequestAttr(TestMeConst.idEcho)]
    void Echo(sbyte[] input, out sbyte[] output)
    {
        output = input;
        ++m_nCalls;
    }
}
