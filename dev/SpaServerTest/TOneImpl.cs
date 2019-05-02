/* **** including all of defines, service id(s) and request id(s) ***** */
using System;
using SocketProAdapter;
using SocketProAdapter.ServerSide;
//server implementation for service CTOne
public class CTOnePeer : CClientPeer
{
    protected override void OnSwitchFrom(uint nServiceID)
    {
        //initialize the object here
        ++m_nCount;
    }

    protected override void OnReleaseResource(bool closing, uint nInfo)
    {
        if (closing)
        {
            //closing the socket with error code = nInfo
        }
        else
        {
            //switch to a new service with the service id = nInfo
        }

        //release all of your resources here as early as possible
    }

    private int m_nCount = 0;
    [RequestAttr(TOneConst.idQueryCountCTOne)]
    private int QueryCount()
    {
        ++m_nCount;
        ++m_nGlobalFastCount;
        lock (m_cs)
        {
            ++m_nGlobalCount;
        }
        return m_nCount;
    }

    private static object m_cs = new object();
    private static int m_nGlobalCount = 0;
    [RequestAttr(TOneConst.idQueryGlobalCountCTOne)]
    private int QueryGlobalCount()
    {
        ++m_nCount;
        ++m_nGlobalFastCount;
        lock (m_cs)
        {
            ++m_nGlobalCount;
            return m_nGlobalCount;
        }
    }

    private static int m_nGlobalFastCount = 0;
    [RequestAttr(TOneConst.idQueryGlobalFastCountCTOne)]
    private int QueryGlobalFastCount()
    {
        ++m_nCount;
        ++m_nGlobalFastCount;
        lock (m_cs)
        {
            ++m_nGlobalCount;
        }
        return m_nGlobalFastCount;
    }

    [RequestAttr(TOneConst.idSleepCTOne, true)]
    private void Sleep(int nTime)
    {
        ++m_nCount;
        System.Threading.Thread.Sleep(nTime);
        lock (m_cs)
        {
            ++m_nGlobalCount;
        }
    }

    [RequestAttr(TOneConst.idEchoCTOne)]
    private object Echo(object objInput)
    {
        ++m_nCount;
        ++m_nGlobalFastCount;
        lock (m_cs)
        {
            ++m_nGlobalCount;
        }
        return objInput;
    }

    [RequestAttr(TOneConst.idEchoExCTOne)]
    private bool EchoEx(sbyte[] str, string wstr, MyStruct ms, out sbyte[] strOut, out string wstrOut)
    {
        strOut = str;
        wstrOut = wstr + ms.ToString();
        ++m_nCount;
        ++m_nGlobalFastCount;
        lock (m_cs)
        {
            ++m_nGlobalCount;
        }
        return true;
    }

}

