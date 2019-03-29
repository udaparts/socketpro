
using System;

namespace SocketProAdapter
{
    public class CMasterSlaveBase<THandler> : ClientSide.CSocketPool<THandler>
        where THandler : ClientSide.CAsyncServiceHandler, new()
    {
        private string m_dbDefalut;

        public string DefaultDBName
        {
            get
            {
                return m_dbDefalut;
            }
        }

        protected CMasterSlaveBase(string defaultDB, uint recvTimeout, bool autoConn)
            : base(autoConn, recvTimeout)
        {
            m_dbDefalut = defaultDB;
        }

        protected CMasterSlaveBase(string defaultDB, uint recvTimeout, bool autoConn, uint connTimeout)
            : base(autoConn, recvTimeout, connTimeout)
        {
            m_dbDefalut = defaultDB;
        }

        protected CMasterSlaveBase(string defaultDB, uint recvTimeout, bool autoConn, uint connTimeout, uint svsId)
            : base(autoConn, recvTimeout, connTimeout, svsId)
        {
            m_dbDefalut = defaultDB;
        }

        protected CMasterSlaveBase(string defaultDB, uint recvTimeout)
            : base(true, recvTimeout)
        {
            m_dbDefalut = defaultDB;
        }
        protected CMasterSlaveBase(string defaultDB)
            : base(true, ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT)
        {
            m_dbDefalut = defaultDB;
        }
    }
}
