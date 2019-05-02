
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.Diagnostics;

public class CMySvsHandler : CAsyncServiceHandler
{
    public CMySvsHandler()
        : base(TestMeConst.sidTestService)
    {
    }

    public void Sleep(uint time)
    {
        m_ok = ProcessR0(TestMeConst.idSleep, time);
    }

    public bool OpenDb(byte[] asciiString)
    {
        bool res;
        m_ok = ProcessR1(TestMeConst.idOpenDb, asciiString, out res);
        return res;
    }

    public sbyte[] Echo(byte[] input)
    {
        sbyte[] res;
        m_ok = ProcessR1(TestMeConst.idEcho, input, out res);
        return res;
    }

    public string BadRequest(uint n, string input)
    {
        string str;
        m_ok = ProcessR1(TestMeConst.idBadRequest, n, input, out str);
        return str;
    }

    CUQueue DoRequest0(sbyte aChar, char aWChar, byte[] asciiStr, string wstr, ushort us, double d, bool b, DateTime dt)
    {
        CUQueue q;
        m_ok = ProcessR1(TestMeConst.idDoRequest0, aChar, aWChar, asciiStr, wstr, us, d, b, dt, out q);
        return q;
    }

    void DodequeueAsync(uint messageCount)
    {
        m_ok = SendRequest(TestMeConst.idDequeue, messageCount, delegate(CAsyncResult ar) { });
    }

    protected override void OnResultReturned(ushort sRequestId, CUQueue UQueue)
    {
        switch (sRequestId)
        {
            case TestMeConst.idEcho:
                UQueue.Load(out m_out);
                break;
            case TestMeConst.idOpenDb:
                UQueue.Load(out m_ok);
                break;
            case TestMeConst.idDoRequest0:
                m_q.SetSize(0);
                m_q.Push(UQueue.GetBuffer(), UQueue.GetSize());
                UQueue.SetSize(0);
                break;
            case TestMeConst.idDoRequest1:
                {
                    string input;
                    string res;
                    UQueue.Load(out input).Load(out res);
                    //Console.WriteLine(" +++ " + res + " ++++");
                    Debug.Assert(input == res);
                }
                break;
            case TestMeConst.idDoIdle:
                {
                    int n;
                    ulong ms;
                    sbyte[] s;
                    UQueue.Load(out ms).Load(out n).Load(out s);
                    Console.WriteLine("ms = " + ms.ToString() + ", n = " + n.ToString() + ", name = " + CUQueue.ToString(s));
                }
                break;
            default:
                break;
        }
    }

    private bool m_ok;
    private CUQueue m_q = new CUQueue();
    private sbyte[] m_out;
}