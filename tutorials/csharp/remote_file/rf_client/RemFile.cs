
using System;
using SocketProAdapter;
using SocketProAdapter.ClientSide;
using System.IO;

public class RemotingFile : CAsyncServiceHandler
{
    public RemotingFile()
        : base(RemFileConst.sidRemotingFile)
    {
        m_sh = new CStreamHelper(this);
    }

    public CStreamHelper StreamHelper
    {
        get
        {
            return m_sh;
        }
    }
    private CStreamHelper m_sh;
}
