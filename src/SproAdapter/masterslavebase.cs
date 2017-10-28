
using System;

namespace SocketProAdapter
{
    public class CMasterSlaveBase<THandler> : ClientSide.CSocketPool<THandler>
        where THandler : ClientSide.CAsyncServiceHandler, new()
    {
        private uint m_nRecvTimeout = ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT;
        private string m_dbDefalut;

        public string DefaultDBName
        {
            get
            {
                return m_dbDefalut;
            }
        }

        public uint RecvTimeout
        {
            get
            {
                return m_nRecvTimeout;
            }
        }

        protected CMasterSlaveBase(string defaultDB, uint recvTimeout)
            : base(true, recvTimeout)
        {
            m_dbDefalut = defaultDB;
            m_nRecvTimeout = recvTimeout;
        }
        protected CMasterSlaveBase(string defaultDB)
            : base(true, ClientSide.CClientSocket.DEFAULT_RECV_TIMEOUT)
        {
            m_dbDefalut = defaultDB;
        }
    }
}
